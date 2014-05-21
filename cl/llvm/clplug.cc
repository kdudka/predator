/*
 * Copyright (C) 2014 Veronika Sokova <xsokov00@stud.fit.vutbr.cz>
 *
 * This file is part of llvm/predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

extern "C" {
#include <cl/code_listener.h>

#include "../config_cl.h"
#include "../version_cl.h"

#include <unistd.h> // PID
}

#include <cl/cldebug.hh>
#include <cl/cl_msg.hh>
#include "clplug.hh"

//#include "llvm/PassRegistry.h"
//#include "llvm/PassManager.h"
//#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Support/CommandLine.h" // namespace cl
#include "llvm/DebugInfo.h" // loc

#include <fstream>
#include <cstring> // memcpy

/*
FIXME location info

    I->getDebugLoc() // const DebugLoc
        DL->getLine() // riadok unsigned
        DL->getCol() // stlpec unsigned
        DL->getScope

DebugLoc 
loc.line = I->getDebugLoc().getLine()
loc.column = I->getDebugLoc().getCol()
loc.sysp =

resp.

    if (MDNode *N = I->getMetadata("dbg")) {
        DILocation Loc(N);
        loc.file = strdup(Loc.getFilename().str().c_str());
        loc.line = Loc.getLineNumber();
        loc.column = Loc.getColumnNumber();
        loc.sysp = 0;
    } else {
            loc = cl_loc_known;
    }
*/

/**
 * @file clplug.cc
 */

using namespace llvm;


/// Error detection - errors
int CLPrint::cntErrors = 0;
/// Error detection - warnings
int CLPrint::cntWarnings = 0;

static std::string sufix = ".so";

/// for pass registration
static std::string fullName = "Code Listener Analyzer";

// FIXME unsupport loc
struct cl_loc cl_loc_known = {
    0,  // .file
    0,  // .line
    0,  // .column
    0,  // .sysp
};

static struct cl_type builtinIntType = {
    -1, // uid
    CL_TYPE_INT, // code
    cl_loc_known, // loc
    CL_SCOPE_GLOBAL, // scope
    "<builtin_int>",// *name
    sizeof(int), // size
    0, //item_cnt - return + size + align
    nullptr, // *items
    0, // array_size
    false // is_unsigned
};

struct cl_type_item allocaArg[3] = {{nullptr, nullptr, 0}, {&builtinIntType, nullptr, 0}, {&builtinIntType, nullptr, 0}};

static struct cl_type allocaFncType = {
    -2, // uid
    CL_TYPE_FNC, // code
    cl_loc_known, // loc
    CL_SCOPE_GLOBAL, // scope
    "<builtin_alloca>",// *name
    0, // size
    3, //item_cnt - return + size + align
    allocaArg, // *items
    0, // array_size
    false // is_unsigned
};

static std::string description = "\n   "+ fullName + " [code listener SHA1 " + CL_GIT_SHA1 + "]\n"
+"\n   USAGE: opt -lowerswitch -load lib"+ plugName + sufix +" -"
+ plugName +" [options] <input bitecode file>";


/// Command line options for opt help
cl::OptionCategory CLOptionCategory("Code Listener options",
        description.c_str());

cl::opt<std::string> CLArgs("args",
        cl::desc("Args given to analyzer"),
        cl::ValueRequired,
        cl::value_desc("peer-args"),
        cl::cat(CLOptionCategory));
static cl::opt<bool> CLDryRun("dry-run",
        cl::desc("Do not run the analyzer"),
        cl::init(false), cl::cat(CLOptionCategory));
static cl::opt<std::string> CLPPFilename("dump-pp",
        cl::desc("Dump linearized code"),
        cl::ValueOptional,
        cl::value_desc("filename"),
        cl::init("-"), cl::cat(CLOptionCategory));
static cl::opt<bool> CLDumpType("dump-types",
        cl::desc("Dump also type info"),
        cl::init(false), cl::cat(CLOptionCategory));
static cl::opt<std::string> CLGenFilename("gen-dot",
        cl::desc("Generate CFGs"),
        cl::ValueOptional,
        cl::value_desc("filename"),
        cl::init("-"), cl::cat(CLOptionCategory));
static cl::opt<std::string> CLPIDFilename("pid-file",
        cl::desc("Write PID of self to <filename>"),
        cl::ValueRequired,
        cl::value_desc("filename"),
        cl::cat(CLOptionCategory));
static cl::opt<bool> CLPreserve("preserve-ec",
        cl::desc("Do not affect exit code"),
        cl::init(false), cl::cat(CLOptionCategory));
static cl::opt<std::string> CLTypeFilename("type-dot",
        cl::desc("Generate type graphs"),
        cl::ValueRequired,
        cl::value_desc("filename"),
        cl::cat(CLOptionCategory));
static cl::opt<int> CLVerbose("verbose",
        cl::desc("Turn on verbose mode"),
        cl::ValueOptional,
        cl::value_desc("uint"),
        cl::init(0), cl::cat(CLOptionCategory));


/// just simple text, if verbose=3
#define CL_DEBUG3(toStream) do {     \
    if (CLVerbose < 3)               \
    break;                           \
    errs() << toStream;              \
} while (0)

//=========================================================================

void CLPass::freeTypeTable(void) {

    for (TypeMap::iterator t = TypeTable.begin(), te = TypeTable.end(); t != te; ++t) {
        if (t->second->item_cnt != 0)
            delete [] t->second->items;
        if (t->second->name != nullptr)
            delete [] t->second->name;

        delete t->second;
    }
    TypeTable.clear();
}

void CLPass::freeVarTable(void) {

    for (VarMap::iterator v = VarTable.begin(), ve = VarTable.end(); v != ve; ++v) {
        // freeInitial(v->second->initial); FIXME
        if (v->second->name != nullptr)
            delete [] v->second->name;
        freeInitial(v->second->initial);
        delete v->second;
    }
    VarTable.clear();
}

void CLPass::freeInitial(struct cl_initializer *init) {
    while (init) {
        struct cl_initializer *next = init->next;

        if (init->insn.code == CL_INSN_UNOP) {
            struct cl_operand *clo = const_cast<struct cl_operand *>(init->insn.data.insn_unop.dst);
            freeAccessor(clo->accessor);
            delete clo;
            clo = const_cast<struct cl_operand *>(init->insn.data.insn_unop.src);
            freeAccessor(clo->accessor);
            delete clo;
        }

        delete init;
        init = next;
    }
}

void CLPass::freeAccessor(struct cl_accessor *acc) {
    while (acc) {
        struct cl_accessor *next = acc->next;

        if (CL_ACCESSOR_DEREF_ARRAY == acc->code) {
            struct cl_operand *clo = acc->data.array.index;
            freeAccessor(clo->accessor);
            delete clo;
        }

        delete acc;
        acc = next;
    }
}

/// create CL object for grouping another CL objects by command line options
void CLPass::setup(void) {

    /* void */ cl::ParseEnvironmentOptions("opt",plugName.c_str());

    writePID();

    // alloc CL object --> use Dudka's code
    cl = cl_chain_create();
    if (!cl) exit(EXIT_FAILURE); //return false;

#if CL_DEBUG_LOCATION
    appendListener("listener=\"locator\"");
#endif

    const char *cfg = ((CLDryRun)? "unfold_switch,unify_labels_gl" : "unify_labels_fnc");
    std::string configCL;

    if (CLPPFilename != "-") {
        configCL = "listener=\"";
        configCL += ((CLDumpType)? "pp_with_types" : "pp");
        configCL += "\" listener_args=\""+ CLPPFilename +"\" clf=\""+ cfg +"\"";
        appendListener(configCL.c_str());
        configCL.clear();
    }

    if (CLGenFilename != "-") {
        configCL = "listener=\"dotgen\" listener_args=\""+ CLGenFilename
            +"\" clf=\""+ cfg +"\"";
        appendListener(configCL.c_str());
        configCL.clear();
    }

    if (!CLTypeFilename.empty()) {
        configCL = "listener=\"typedot\" listener_args=\""+ CLTypeFilename
            +"\" clf=\""+ cfg +"\"";
        appendListener(configCL.c_str());
        configCL.clear();
    }


    if (!CLArgs.empty()) {
        configCL = "listener=\"easy\" listener_args=\""+ CLArgs
            +"\" clf=\""+ cfg +"\"";
        appendListener(configCL.c_str());
        configCL.clear();
    }


}

/// append CL object 
void CLPass::appendListener(const char *config) {
    struct cl_code_listener *cl_new = cl_code_listener_create(config);

    if (!cl_new) {
        cl->destroy(cl);
        exit(EXIT_FAILURE); //return false;
    } else {
        cl_chain_append(cl, cl_new);
    }
}

/// write pid to the file, if is set -pid-file from cmd
void CLPass::writePID(void) {

    if (CLPIDFilename.empty())
        return;

    std::ofstream file;
    if (file.fail()) {
        CL_ERROR("PID File no open.");
        exit(EXIT_FAILURE);
    }
    file.open (CLPIDFilename.c_str(), std::ofstream::out);
    file << getpid() << '\n';
    file.close();
}


/// initialization CL before pass is run, setup callbacks for message
bool CLPass::doInitialization(Module &) {

    // callbacks for messages
    struct cl_init_data data = {
        CLPrint::empty, // debug
        CLPrint::warn, // warn
        CLPrint::error, // error
        CLPrint::note, // note
        CLPrint::die, // die
        CLVerbose // debug_level
    };
    if (data.debug_level) data.debug = CLPrint::debug;

    cl_global_init(&data);

    setup();

    CL_DEBUG("pass successfully initialized");
    return true;
}

/// Analysis of source file as Module
/// return true, if the module was modified (never)
bool CLPass::runOnModule(Module &M) {

    if (CLVerbose > 2)
        M.dump();

    DL = new DataLayout(&M);

    voidType = Type::getVoidTy(M.getContext());

    cl_loc_known.file = M.getModuleIdentifier().c_str();

    // one module for one source file
    cl->file_open(cl, cl_loc_known.file); // open source file
    CL_DEBUG("start "<< __func__);

    /*    errs() << "\nGlobal variables:\n";
        for (Module::global_iterator GV = M.global_begin(), GVE = M.global_end(); GV != GVE; ++GV)
        { // global variable
        errs() << "ID<GV<<<<<" << M.getMDKindID(GV->getName()) << "\n";
        handleGlobalVariable(GV, &var);
        }*/

    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F)
    { // functions
        //        int fncUID = M.getMDKindID(F->getName());
        CL_DEBUG("pass analyzes Function " << F->getName().str() << "()");

        if (F->isDeclaration()) continue;
        handleFunction(F); // open function

        for (Function::iterator BB = F->begin(), BE = F->end(); BB != BE; ++BB)
        { // basic blocks

            //            errs() << "ID<BB<<<<<" << M.getMDKindID(BB->getName()) << "\n";
            CL_DEBUG("pass analyzes BasicBlock " << BB->getName().str() << " - "
                    << BB->size() << " instructions");

            cl->bb_open(cl, BB->getName().str().c_str()); // open basic block

            for (BasicBlock::iterator I = BB->begin(), IE = BB->end(); I != IE; ++I)
            { // instructions
                handleInstruction(I);
            }
        }
        cl->fnc_close(cl); // close function
    }

    cl->file_close(cl); // close source file

    delete DL;
    CL_DEBUG("end of analysis");
    return false;
}


/// handle for type, return cl_type, which find in typeTable, or create new
/// LLVM don't understand void* -> char*
struct cl_type *CLPass::handleType(Type *t) {

    struct cl_type *clt = nullptr;
    //    errs() << "addr(type) = " << t ;
    TypeMap::const_iterator item = TypeTable.find(t);
    if (item == TypeTable.end()) { // not found
        clt = new struct cl_type;
        std::memset(clt, 0, sizeof(*clt));
        clt->uid = cntUID++;
        TypeTable.insert({t,clt});
    }
    else { // exist
        return item->second;
    }

    clt->loc = cl_loc_known; //FIXME

    switch (t->getTypeID()) {
        case Type::VoidTyID:
            clt->code = CL_TYPE_VOID;
            clt->size = 0;
            clt->item_cnt = 0;
            break;

        case Type::HalfTyID:
        case Type::FloatTyID:
        case Type::DoubleTyID:
        case Type::X86_FP80TyID:
        case Type::FP128TyID:
        case Type::PPC_FP128TyID:
            clt->code = CL_TYPE_REAL;
            clt->size = t->getPrimitiveSizeInBits() / CHAR_BIT;
            clt->item_cnt = 0;
            break;

        case Type::LabelTyID: // BasicBlock only
            CL_WARN("BasicBlock is not correct operand type");
            break;

        case Type::IntegerTyID: // CL_TYPE_CHAR or CL_TYPE_BOOL
            clt->code = CL_TYPE_INT;
            handleIntegerType(cast<IntegerType>(t), clt);
            break;

        case Type::StructTyID: // CL_TYPE_UNION
            clt->code = CL_TYPE_STRUCT;
            handleStructType(cast<StructType>(t), clt);
            break;

        case Type::ArrayTyID: {
            clt->code = CL_TYPE_ARRAY;
            clt->item_cnt = 1;
            clt->array_size = t->getArrayNumElements(); //cast<ArrayType>(t)->getNumElements();

            clt->items = new struct cl_type_item [1];
            clt->items[0].type = handleType(t->getSequentialElementType());
            //(cast<SequentialType>(t)->getElementType(), tmp);

            clt->size = clt->array_size * clt->items[0].type->size;
        }
            break;

        case Type::PointerTyID: {
            clt->code = CL_TYPE_PTR;
            clt->size = DL->getPointerSize();
            clt->item_cnt = 1;

            clt->items = new struct cl_type_item [1];
            clt->items[0].type = handleType(t->getSequentialElementType());
        }
            break;

        case Type::FunctionTyID: {
            clt->code = CL_TYPE_FNC;
            //            struct cl_type *tmp = clt;
            handleFunctionType(cast<FunctionType>(t), clt);
            //            clt = tmp;
        }
            break;

        case Type::VectorTyID:
            CL_WARN("unsupport type VECTOR");
        default: // MetadataTyID, X86_MMXTyID
            clt->code = CL_TYPE_UNKNOWN;
            break;
    }
    //    errs() << "ID<TY<<<<<" << clt->uid << ">>>>>";
    return clt;
}

/// handle for integer types (bool, char and int)
/// LLVM IR does not differentiate between signed and unsigned integers
void CLPass::handleIntegerType(IntegerType *t, struct cl_type *clt) {

    clt->item_cnt = 0;
    clt->size = t->getBitWidth(); // bits
    if (clt->size == 1) {
        clt->code = CL_TYPE_BOOL;
    }    else if (clt->size == CHAR_BIT) {
        clt->code = CL_TYPE_CHAR;
        clt->size /= CHAR_BIT;
    }    else {
        clt->size /= CHAR_BIT;
    }
    clt->is_unsigned = !t->getSignBit(); //FIXME
    //    errs() << (t->getBitMask() & t->getSignBit()) <<  ((t->getSignBit())? " sign" : " unsign") << " - ";
}

/// handle for function type
/// begin at item_cnt = 1 (after return type)
void CLPass::handleFunctionType(FunctionType *ft, struct cl_type *clt) {

    clt->scope = CL_SCOPE_GLOBAL;
    clt->name = nullptr;
    clt->size = 0;
    clt->item_cnt = 1; // free for ret type

    unsigned params = ft->getNumParams();
    clt->items = new struct cl_type_item [params + 1];

    for (FunctionType::param_iterator p = ft->param_begin(), pe = ft->param_end(); p != pe; ++p)
    {
        //        errs() << clt->item_cnt << ":";
        struct cl_type_item *item = &clt->items[clt->item_cnt ++];
        item->name = nullptr;
        item->type = handleType(*p);
    }
}
// deklaruje pointer, ktoreho data nemozu byt menene pomocou ukazovatela
// const int *p = &someInt;
// deklaruje pointer, ktory nemoze byt zmeneny na nieco ine
// int * const p = &someInt;

/// handle for struct and union
/// in LLVM IR is union = struct about one biggest item
void CLPass::handleStructType(StructType *st, struct cl_type *clt) {

    clt->item_cnt = 0;

    if (st->hasName()) {
        std::string tmp = st->getName().str();

        if (tmp.find("union") == 0)
            clt->code = CL_TYPE_UNION; // offset is 0 - always 1 biggest element
        unsigned len = tmp.length() + 1;
        char * tmpPtr = new char [len];
        std::memcpy(tmpPtr, tmp.c_str(), len);
        clt->name = tmpPtr;
    } else {
        clt->name = nullptr;
    }
    if (st->isOpaque()) {// not body
        return;
    }
    //    if (!st->isLiteral()) ;// not unique, is definition -> continue read items

    const StructLayout *SL = DL->getStructLayout(st);
    clt->size = SL->getSizeInBytes();
    clt->items = new struct cl_type_item [st->getNumElements()];

    for(StructType::element_iterator e = st->element_begin(), ee = st->element_end(); e != ee; ++e)
    {
        struct cl_type_item *item = &clt->items[clt->item_cnt];
        item->type = handleType(*e);

        item->name = nullptr; //FIXME

        item->offset = SL->getElementOffset(clt->item_cnt ++);

    }

}


/// handle for function operand  @b CL_TYPE_FNC
void CLPass::handleFncOperand(Function *F, struct cl_operand *fnc) {

    if (F->hasName()) {
        std::string tmp = F->getName().str();

        unsigned len = tmp.length() + 1;
        char * tmpPtr = new char [len]; // alloc name
        std::memcpy(tmpPtr, tmp.c_str(), len);
        fnc->data.cst.data.cst_fnc.name = tmpPtr;
    } else {
        fnc->data.cst.data.cst_fnc.name = nullptr;
    }

    // find or create
    fnc->type = handleType(F->getFunctionType());
    // return type
    fnc->type->items[0].name = nullptr;
    fnc->type->items[0].type = handleType(F->getReturnType());

    fnc->code = CL_OPERAND_CST;
    fnc->scope = ((F->hasInternalLinkage())? CL_SCOPE_STATIC : CL_SCOPE_GLOBAL);
    fnc->accessor = nullptr;
    fnc->data.cst.code = CL_TYPE_FNC;
    fnc->data.cst.data.cst_fnc.uid = F->getContext().getMDKindID(F->getName());
    // I hope that is unique

    fnc->data.cst.data.cst_fnc.is_extern = F->isDeclaration();
    fnc->data.cst.data.cst_fnc.loc = cl_loc_known; //FIXME

    //"Allocsize = " << G->getDataLayout()->getTypeAllocSize(F->getType());
}

/// handle for open function, when is call from Module, iterate argument list
void CLPass::handleFunction(Function *F) {

    struct cl_operand fnc;
    handleFncOperand(F, &fnc);
    if (fnc.data.cst.data.cst_fnc.name == nullptr)
        return;
    cl->fnc_open(cl, &fnc);
    delete [] fnc.data.cst.data.cst_fnc.name;

    //        if (F->getFunctionType()->getNumParams() == 0)
    //            errs() << "FUNCTION Argument VOID\n";
    //        else
    for (Function::arg_iterator FA = F->arg_begin(), FAE = F->arg_end(); FA != FAE; ++FA)
    { // args
        //                errs() << "  Argument no. " << FA->getArgNo();
        struct cl_operand arg;
        handleOperand(FA, &arg);
        cl->fnc_arg_decl(cl, FA->getArgNo(), &arg);

    }
    // set entry label
    std::string name;
    BasicBlock *BB = &F->getEntryBlock();
    if (!BB->hasName()) {
        name = "<label"+ std::to_string(bbUID++) +">";
        BB->setName(name);
    } else {
        name = BB->getName().str();
    }

    struct cl_insn i;
    i.code = CL_INSN_JMP;
    i.loc = cl_loc_known; //FIXME
    i.data.insn_jmp.label = name.c_str();
    cl->insn(cl, &i);
}

/// handle for constant, setup from cl_type in operand
/// if is bool constant (TRUE, FALSE), type must be CL_TYPE_INT for access in union data
void CLPass::handleConstant(Value *v, struct cl_operand *clo) {

    clo->code = CL_OPERAND_CST;
    clo->scope = CL_SCOPE_GLOBAL; // maybe?
    clo->type = handleType(v->getType());
    clo->data.cst.code = clo->type->code;

    CL_DEBUG3(" CONSTANT [type=" << v->getType()->getTypeID() << "]\n");

    if (isa<ConstantDataSequential>(v) && cast<ConstantDataSequential>(v)->isString()) {
        clo->data.cst.code = CL_TYPE_STRING;
        CL_DEBUG3("\\-> STRING CST\n");
        std::string tmp = cast<ConstantDataSequential>(v)->getAsString().str();
        unsigned len = tmp.length() + 1;
        char * tmpPtr = new char [len]; // alloc string
        std::memcpy(tmpPtr, tmp.c_str(), len);
        clo->data.cst.data.cst_string.value = tmpPtr;
    }

    switch (clo->data.cst.code) {
        case CL_TYPE_BOOL:
            clo->data.cst.code = CL_TYPE_INT; // because ConstantInt
        case CL_TYPE_CHAR: // best choice
        case CL_TYPE_INT:
            if (clo->type->is_unsigned)
                clo->data.cst.data.cst_uint.value = dyn_cast<ConstantInt>(v)->getValue().getZExtValue();
            else
                clo->data.cst.data.cst_int.value = dyn_cast<ConstantInt>(v)->getValue().getSExtValue();
            break;

        case CL_TYPE_REAL: {
            llvm::APFloat tmp = dyn_cast<ConstantFP>(v)->getValueAPF();
            bool losesinfo;
            //enum llvm::APFloat::opStatus state
            if (APFloat::opOK != tmp.convert( APFloat::IEEEdouble, APFloat::rmTowardZero , &losesinfo))
                CL_WARN("FP constant don't convert to double");
            clo->data.cst.data.cst_real.value = tmp.convertToDouble();
        }
            break;

        case CL_TYPE_PTR:
            if (isa<ConstantPointerNull>(v)) {// nullptr or isa<ConstantAggregateZero>
                clo->data.cst.data.cst_int.value = 0;
                clo->data.cst.code = CL_TYPE_INT;
            }

        case CL_TYPE_STRING: // not support
            break;

        default:
            CL_WARN("unkown constant");
            break;
    }

}

/// handle for local and global variables, return cl_var, which find in varTable, or create new
/// variable don't have to name (register)
struct cl_var *CLPass::handleVariable(Value *v) {

    struct cl_var *clv = nullptr;

    VarMap::const_iterator item = VarTable.find(v);
    if (item == VarTable.end()) { // not found
        clv = new struct cl_var;
        std::memset(clv, 0, sizeof(*clv));
        clv->uid = cntUID++;
        VarTable.insert({v, clv});
    }
    else { // exist
        return item->second;
    }

    clv->loc = cl_loc_known; //FIXME

    if (v->hasName()) {
        std::string tmp = v->getName().str();
        unsigned len = tmp.length() + 1;
        char * tmpPtr = new char [len];
        std::memcpy(tmpPtr, tmp.c_str(), len);
        clv->name = tmpPtr;
        clv->artificial = false;
    }    else {
        clv->name = nullptr;
        clv->artificial = true;
    }

    if (isa<GlobalVariable>(v)) {
        clv->artificial = false;
        clv->is_extern = cast<GlobalVariable>(v)->hasExternalLinkage();
        clv->initial =  nullptr; // or struct cl_initializer initial
        clv->initialized = !clv->is_extern;
        if (cast<GlobalVariable>(v)->isConstant()) {
            struct cl_operand *src = new struct cl_operand;
            struct cl_operand *dst = new struct cl_operand;
            dst->code = CL_OPERAND_VAR;
            dst->type = handleType(v->getType());
            dst->data.var = clv;

            clv->initial = new struct cl_initializer;
            clv->initial->next = nullptr;

            std::memset(src, 0, sizeof(*src));
            handleConstant(v, src);
            clv->initial->insn.code = CL_INSN_UNOP;
            clv->initial->insn.data.insn_unop.code = CL_UNOP_ASSIGN;

            dst->accessor = new struct cl_accessor;
            dst->accessor->code = CL_ACCESSOR_DEREF; // *
            dst->accessor->type = dst->type;
            dst->type = src->type;
            dst->accessor->next = nullptr;

            clv->initial->insn.data.insn_unop.dst = dst;
            clv->initial->insn.data.insn_unop.src = src;

        } // else FIXME

    } else {
        clv->initial =  nullptr; // or struct cl_initializer initial
        clv->initialized = false;
        clv->is_extern = false;
    }

    // for clang/llvm 3.5
    //const StructLayout *getStructLayout (StructType *Ty) const

    return clv;
}


/// specifies concrete operand of instruction - most important function
/// if is global alias or constant expression, this function is called recursively
/// no operand if code == CL_OPERAND_VOID
void CLPass::handleOperand(Value *v, struct cl_operand *clo) {

    clo->code = CL_OPERAND_VOID;

    if (v == nullptr) return;

    /*
       switch (v->getValueID()) {
       case Value::ArgumentVal:
    //handleArgument(cast<Argument>(v));
    break;

    case Value::BasicBlockVal:
    CLPrint::error("invalid operand BasickBlock");
    break;

    case Value::FunctionVal:                         // function
    errs() << "\n FUNCTION \n";
    handleFncOperand(cast<Function>(v), clo);
    break;

    case Value::GlobalAliasVal:
    break;

    case Value::GlobalVariableVal:                  // global variable
    errs() << "\n GLOBAL VAR \n";
    clo->code = CL_OPERAND_VAR;
    clo->scope = CL_SCOPE_GLOBAL;
    //handleGlobalVariable(cast<GlobalVariable>(v), clv);
    break;

    case Value::UndefValueVal:
    break;

    case Value::BlockAddressVal:                    // constant
    case Value::ConstantExprVal:
    case Value::ConstantAggregateZeroVal:
    case Value::ConstantDataArrayVal:
    case Value::ConstantDataVectorVal:
    case Value::ConstantIntVal:
    case Value::ConstantFPVal:
    case Value::ConstantArrayVal:
    case Value::ConstantStructVal:
    case Value::ConstantVectorVal:

    case Value::ConstantPointerNullVal:
    handleConstant(v, clo);
    break;

    case Value::InstructionVal:                      // named variable or register
    clo->code = CL_OPERAND_VAR;
    clo->scope = CL_SCOPE_FUNCTION;
    clo->data.var = handleVariable(v);

    case Value::MDNodeVal:
    case Value::MDStringVal:
    case Value::InlineAsmVal:
    default:
    CLPrint::warn("unsupport operand");
    break;
    }
    */
    clo->accessor = nullptr;

    if (isa<Argument>(v)) {             // func. argument

        CL_DEBUG3(" ARGUMENT\n");
        clo->code = CL_OPERAND_VAR;
        clo->scope = CL_SCOPE_FUNCTION;
        clo->type = handleType(v->getType());
        clo->data.var = handleVariable(v);

    } else if (isa<Constant>(v)) {

        if (isa<GlobalVariable>(v)) {     // global variable
            CL_DEBUG3(" GLOBAL VAR\n");
            clo->code = CL_OPERAND_VAR;
            clo->scope = CL_SCOPE_GLOBAL;
            clo->type = handleType(v->getType());
            clo->data.var = handleVariable(v);
        } else if (isa<Function>(v)) {    // function
            CL_DEBUG3(" FUNCTION\n");
            handleFncOperand(cast<Function>(v), clo);
        } else if (isa<GlobalAlias>(v)) {
            CL_DEBUG3(" GLOBAL ALIAS\n");
            handleOperand(cast<GlobalAlias>(v)->getAliasee(), clo);
        } else if (isa<ConstantExpr>(v)) {

            CL_DEBUG3(" CONSTANT EXPR\n");
            Instruction *vi = cast<ConstantExpr>(v)->getAsInstruction();
            handleInstruction(vi); // recursion
            handleOperand(vi, clo);

        } else {                          // just constant
            handleConstant(v, clo);
        }

    } else {                            // named variable or register

        CL_DEBUG3(" VARIABLE\n");
        clo->code = CL_OPERAND_VAR;
        clo->scope = CL_SCOPE_FUNCTION;
        clo->type = handleType(v->getType());
        clo->data.var = handleVariable(v);

    }

}


/// handle of instruction, support just C-liked instruction
/// not intrinsic
/// in this function is: Ret, Unreachable, Load and Store
void CLPass::handleInstruction(Instruction *I) {

    CL_DEBUG("Inst code = " << I->getOpcodeName() << " [" << I->getOpcode() << "]");
    CL_DEBUG3(*I<<"\n");

    if (I->isBinaryOp()) { // BINOP
        handleBinInstruction(I);
        return;
    } else if (I->isCast()) { // UNOP
        handleCastInstruction(cast<CastInst>(I));
        return;
    }

    struct cl_insn i;
    std::memset(&i, 0, sizeof(i));

    i.loc = cl_loc_known; //FIXME

    struct cl_operand dst, src;

    switch (I->getOpcode()) {
        // Terminator instruction

        case Instruction::Ret: // ReturnInst
            i.code = CL_INSN_RET;
            handleOperand(cast<ReturnInst>(I)->getReturnValue(), &src);
            i.data.insn_ret.src = &src;
            cl->insn(cl, &i);
            return; //docasne
            break;

        case Instruction::Br: // BranchInst
            handleBranchInstruction(cast<BranchInst>(I));
            return;

        case Instruction::Unreachable : // UnreachableInst
            i.code = CL_INSN_ABORT;
            cl->insn(cl, &i);
            return;
            break;

            //Memory operators
        case Instruction::Alloca: {// AllocaInst - Stack management
            handleAllocaInstruction(cast<AllocaInst>(I));
            //            struct cl_var *clv = handleVariable(I);
            //            clv->initialized = true;
            return;                 // nothing interesting for program
        }

        case Instruction::Load: // LoadInst
            i.code = CL_INSN_UNOP;
            i.data.insn_unop.code = CL_UNOP_ASSIGN;
            handleOperand(I->getOperand(0), &src);

            handleOperand(I, &dst);
            if (src.code == CL_OPERAND_VAR)
            {
                src.accessor = new struct cl_accessor;
                src.accessor->code = CL_ACCESSOR_DEREF; // *
                src.accessor->type = src.type;
                src.type = dst.type;
                src.accessor->next = nullptr;
            }
            i.data.insn_unop.dst = &dst;
            i.data.insn_unop.src = &src;
            cl->insn(cl, &i); 
            freeAccessor(src.accessor);
            return; //docasne
            break;

        case Instruction::Store: // StoreInst
            i.code = CL_INSN_UNOP;
            i.data.insn_unop.code = CL_UNOP_ASSIGN;
            handleOperand(I->getOperand(0), &src);
            handleOperand(I->getOperand(1), &dst);
            if (dst.code == CL_OPERAND_VAR)
            {
                dst.accessor = new struct cl_accessor;
                dst.accessor->code = CL_ACCESSOR_DEREF; // *
                dst.accessor->type = dst.type;
                dst.type = src.type;
                dst.accessor->next = nullptr;
            }
            i.data.insn_unop.dst = &dst;
            i.data.insn_unop.src = &src;
            cl->insn(cl, &i);
            freeAccessor(dst.accessor);
            return; //docasne
            break;

        case Instruction::GetElementPtr: // GetElementPtrInst - is accessor for struct
            // and array, not "instruction"
            handleGEPInstruction(cast<GetElementPtrInst>(I));
            return;

        case Instruction::Fence: // FenceInst
            i.code = CL_INSN_NOP;
            break;

            // Comparison instructions
        case Instruction::ICmp: // ICmpInst
        case Instruction::FCmp: // FCmpInst
            handleCmpInstruction(I);
            return;

        case Instruction::Call: // CallInst
            if (isa<IntrinsicInst>(I)) {
                CL_WARN("intrinsic instructions are not support");
            } else {
                handleCallInstruction(cast<CallInst>(I));
            }
            return;

        case Instruction::Select: // SelectInst
            handleSelectInstruction(cast<SelectInst>(I));
            return;

        case Instruction::PHI: // PHINode - is eliminated in terminal instructions
            return;

        case Instruction::ExtractValue: // ExtractValueInst : extract from aggregate
        case Instruction::InsertValue: // InsertValueInst : insert into aggregate

            // unsupport terminator instruction
        case Instruction::Switch: // SwitchInst : option opt -lowerswitch
        case Instruction::IndirectBr: // IndirectBrInst
        case Instruction::Invoke: // InvokeInst
        case Instruction::Resume : // ResumeInst
            // unsupport memory operators
        case Instruction::AtomicCmpXchg: // AtomicCmpXchgInst
        case Instruction::AtomicRMW: // AtomicRMWInst
        default:
            CL_WARN("unsupport instruction "<< I->getOpcodeName());
            return;

    }

    cl->insn(cl, &i);

}

/// alocation on stack
/// create own function __alloca and call it with argumets:
/// allocated size (N*typeSize) and align
void CLPass::handleAllocaInstruction(AllocaInst *I) {

    struct cl_operand dst, fnc, size, align;

    struct cl_type *allocated = handleType(I->getAllocatedType());

    if (I->isArrayAllocation()) {
        handleOperand(I->getArraySize(), &size);
        size.data.cst.data.cst_int.value *= allocated->size;
    } else {
        size.code = CL_OPERAND_CST;
        size.scope = CL_SCOPE_GLOBAL;
        size.type = &builtinIntType;
        size.accessor = nullptr;
        size.data.cst.code = size.type->code;
        size.data.cst.data.cst_int.value = allocated->size;
    }
    align.code = CL_OPERAND_CST;
    align.scope = CL_SCOPE_GLOBAL;
    align.type = &builtinIntType;
    align.accessor = nullptr;
    align.data.cst.code = size.type->code;
    align.data.cst.data.cst_int.value = I->getAlignment();

    //    I->getType(); // pointer

    handleOperand(I, &dst);

    fnc.type = &allocaFncType;
    // return type
    fnc.type->items[0].name = nullptr;
    fnc.type->items[0].type = handleType(I->getType());

    fnc.code = CL_OPERAND_CST;
    fnc.scope = CL_SCOPE_GLOBAL;
    fnc.accessor = nullptr;
    fnc.data.cst.code = CL_TYPE_FNC;
    fnc.data.cst.data.cst_fnc.uid = -1;
    fnc.data.cst.data.cst_fnc.is_extern = true;
    fnc.data.cst.data.cst_fnc.loc = cl_loc_known; //FIXME
    fnc.data.cst.data.cst_fnc.name = "__alloca";

    cl->insn_call_open(cl, &cl_loc_known, &dst, &fnc);

    cl->insn_call_arg(cl, 0, &size);
    cl->insn_call_arg(cl, 1, &align);

    cl->insn_call_close(cl);

}

/// in CL accessors for array and struct
void CLPass::handleGEPInstruction(GetElementPtrInst *I) {

    struct cl_insn i;
    i.loc = cl_loc_known; //FIXME
    i.code = CL_INSN_UNOP;
    i.data.insn_unop.code = CL_UNOP_ASSIGN;

    struct cl_operand src, dst;
    handleOperand(I->getPointerOperand(), &src);
    handleOperand(I, &dst);
    i.data.insn_unop.dst = &dst;
    i.data.insn_unop.src = &src;

    struct cl_accessor **accPrev = &src.accessor;
    struct cl_accessor *acc;
    for (GetElementPtrInst::op_iterator op = I->idx_begin(), ope = I->idx_end(); op != ope; ++op) {
        Value *v = *op;

        acc = new struct cl_accessor;
        *accPrev = acc;

        acc->type = src.type;

        acc->next = nullptr;

        switch (src.type->code) {
            case CL_TYPE_STRUCT: {
                //                errs() << "struct "; // must be constant
                int num = cast<ConstantInt>(v)->getValue().getSExtValue();
                src.type = const_cast<struct cl_type *>(src.type->items[num].type); // must be const in cl_type_item ?
                acc->code = CL_ACCESSOR_ITEM;
                acc->data.item.id = num;
            }
            break;

            case CL_TYPE_PTR:
            //                errs() << "ptr ";
            src.type = const_cast<struct cl_type *>(src.type->items[0].type);
            acc->code = CL_ACCESSOR_DEREF;

            // if not 0 -> CL_ACCESSOR_OFFSET

            break;

            case CL_TYPE_ARRAY:
            //                errs() << "array ";
            src.type = const_cast<struct cl_type *>(src.type->items[0].type);

            acc->code = CL_ACCESSOR_DEREF_ARRAY;
            acc->data.array.index = new struct cl_operand;
            handleOperand(v, acc->data.array.index);
            break;

            case CL_TYPE_UNION: // never happend
            default:
            CL_ERROR("invalid type for accessor");
            break;
        }
        accPrev = &(acc->next);
    }


    // is ...PtrInst, always return REF
    {
        *accPrev = new struct cl_accessor;
        (*accPrev)->code = CL_ACCESSOR_REF; // &
        (*accPrev)->type = src.type;
        src.type = dst.type;
        (*accPrev)->next = nullptr;
    }


    /*// it's the same thing
      {
      dst.accessor = new struct cl_accessor;
      dst.accessor->code = CL_ACCESSOR_DEREF; // *
      dst.accessor->type = dst.type;
      dst.type = src.type;
      dst.accessor->next = nullptr;
      }
      */
    if (CLVerbose > 2)
        acToStream(std::cerr, src.accessor, false);

    cl->insn(cl, &i);
    freeAccessor(src.accessor);
    freeAccessor(dst.accessor);
}

/// branch for conditional and unconditional jump
void CLPass::handleBranchInstruction(BranchInst *I) {

    struct cl_insn i;
    i.loc = cl_loc_known; //FIXME
    std::string bbName1, bbName2;

    if (I->isConditional()) {

        i.code = CL_INSN_COND;
        struct cl_operand src;
        handleOperand(I->getCondition(), &src);
        i.data.insn_cond.src = &src;

        //then
        BasicBlock *bb = I->getSuccessor(0);
        if (!bb->hasName()) {
            bbName1 = "<label"+ std::to_string(bbUID++) +">";
            bb->setName(bbName1);
        } else {
            bbName1 = bb->getName().str();
        }
        i.data.insn_cond.then_label = bbName1.c_str();
        //else
        bb = I->getSuccessor(1);
        if (!bb->hasName()) {
            bbName2 = "<label"+ std::to_string(bbUID++) +">";
            bb->setName(bbName2);
        } else {
            bbName2 = bb->getName().str();
        }
        i.data.insn_cond.else_label = bbName2.c_str();

    } else {

        i.code = CL_INSN_JMP;
        // unconditional
        BasicBlock *bb = I->getSuccessor(0);

        testPhi(I->getParent(), bb);

        if (!bb->hasName()) {
            bbName1 = "<label"+ std::to_string(bbUID++) +">";
            bb->setName(bbName1);
        } else {
            bbName1 = bb->getName().str();
        }
        i.data.insn_jmp.label = bbName1.c_str();
    }

    cl->insn(cl, &i);
}

/// test, if instruction in next BasicBlock is phi
/// if yes, into BasicBlock (fromBB, witch called this function) added assign instruction
/// with value for fromBB
void CLPass::testPhi(BasicBlock *fromBB, BasicBlock *phiBB) {
    Instruction *I = phiBB->begin();
    //    errs() << "is phi? " << *I << "\n";
    if (!isa<PHINode>(I)) {
        return;
    }

    struct cl_insn i;
    i.code = CL_INSN_UNOP;
    i.loc = cl_loc_known; //FIXME
    i.data.insn_unop.code = CL_UNOP_ASSIGN;

    struct cl_operand dst, src;
    handleOperand(cast<PHINode>(I)->getIncomingValueForBlock(fromBB), &src);
    handleOperand(I, &dst);
    i.data.insn_unop.dst = &dst;
    i.data.insn_unop.src = &src;

    cl->insn(cl, &i);
}

/// decomposition of C ternar operand ()?:
/// create three new basic blocks for true, false and end
void CLPass::handleSelectInstruction(SelectInst *I) {

    struct cl_insn i, j;
    i.loc = cl_loc_known; //FIXME
    j.loc = cl_loc_known; //FIXME

    i.code = CL_INSN_COND;
    struct cl_operand src, opt, opf;
    handleOperand(I->getCondition(), &src);
    handleOperand(I->getTrueValue(), &opt); // true
    handleOperand(I->getFalseValue(), &opf); // false
    i.data.insn_cond.src = &src;

    std::string bbtrue = "<label"+ std::to_string(bbUID++) +">";
    std::string bbfalse = "<label"+ std::to_string(bbUID++) +">";
    std::string bbend = "<label"+ std::to_string(bbUID++) +">";

    i.data.insn_cond.then_label = bbtrue.c_str();
    i.data.insn_cond.else_label = bbfalse.c_str();
    cl->insn(cl, &i);

    i.code = CL_INSN_UNOP;
    i.data.insn_unop.code = CL_UNOP_ASSIGN;
    handleOperand(I, &src);
    i.data.insn_unop.dst = &src;

    j.code = CL_INSN_JMP;
    j.data.insn_jmp.label = bbend.c_str();

    cl->bb_open(cl, bbtrue.c_str());
    i.data.insn_unop.src = &opt;
    cl->insn(cl, &i);
    cl->insn(cl, &j);

    cl->bb_open(cl, bbfalse.c_str());
    i.data.insn_unop.src = &opf;
    cl->insn(cl, &i);
    cl->insn(cl, &j);

    cl->bb_open(cl, bbend.c_str());
}


/// what kind of binary instruction it's
// note for some instructions:
// NEG, FNeg = CL_UNOP_MINUS (SUB 0 a), NOT = CL_UNOP_BIT_NOT (XOR a -1)
enum cl_binop_e CLPass::getCLCode(Instruction *I) {

    switch(I->getOpcode()) {
        case Instruction::Add:
        case Instruction::FAdd:
            return CL_BINOP_PLUS;

        case Instruction::Sub:
        case Instruction::FSub:
            return CL_BINOP_MINUS;

        case Instruction::Mul:
        case Instruction::FMul:
            return CL_BINOP_MULT;

        case Instruction::UDiv:
        case Instruction::SDiv:
            if (cast<BinaryOperator>(I)->isExact())
                return CL_BINOP_EXACT_DIV;
            else
                return CL_BINOP_TRUNC_DIV;

        case Instruction::FDiv:
            return CL_BINOP_RDIV;

        case Instruction::URem:
        case Instruction::SRem: 
        case Instruction::FRem: // EXACT_MOD ?
            return CL_BINOP_TRUNC_MOD; 

        case Instruction::Shl:
            return CL_BINOP_LSHIFT;

        case Instruction::LShr:
        case Instruction::AShr: // EXACT_MOD ?
            return CL_BINOP_RSHIFT;

        case Instruction::And:
            return CL_BINOP_BIT_AND;

        case Instruction::Or:
            return CL_BINOP_BIT_IOR;

        case Instruction::Xor:
            return CL_BINOP_BIT_XOR;

        default:
            return CL_BINOP_BAD; // something
    }
}

/// create binary instruction
void CLPass::handleBinInstruction(Instruction *I) {

    struct cl_insn i;
    i.code = CL_INSN_BINOP;
    i.loc = cl_loc_known; //FIXME
    i.data.insn_binop.code = getCLCode(I);
    if (i.data.insn_binop.code == CL_BINOP_BAD) {
        CL_WARN("unknown binary operator");
        return;
    }

    struct cl_operand dst, src1, src2;
    handleOperand(cast<Value>(I), &dst);
    handleOperand(I->getOperand(0), &src1);
    handleOperand(I->getOperand(1), &src2);
    i.data.insn_binop.dst = &dst;
    i.data.insn_binop.src1 = &src1;
    i.data.insn_binop.src2 = &src2;
    cl->insn(cl, &i);

}

/// what kind of compare instruction it's
enum cl_binop_e CLPass::getCLCodePredic(enum CmpInst::Predicate p) {

    switch(p) {
        case CmpInst::ICMP_EQ:
        case CmpInst::FCMP_OEQ:
        case CmpInst::FCMP_UEQ:
            return CL_BINOP_EQ;

        case CmpInst::ICMP_NE:
        case CmpInst::FCMP_ONE:
        case CmpInst::FCMP_UNE:
            return CL_BINOP_NE;

        case CmpInst::ICMP_UGT:
        case CmpInst::ICMP_SGT:
        case CmpInst::FCMP_OGT:
        case CmpInst::FCMP_UGT:
            return CL_BINOP_GT;

        case CmpInst::ICMP_UGE:
        case CmpInst::ICMP_SGE:
        case CmpInst::FCMP_OGE:
        case CmpInst::FCMP_UGE:
            return CL_BINOP_GE;

        case CmpInst::ICMP_ULT:
        case CmpInst::ICMP_SLT:
        case CmpInst::FCMP_OLT:
        case CmpInst::FCMP_ULT:
            return CL_BINOP_LT;

        case CmpInst::ICMP_ULE:
        case CmpInst::ICMP_SLE:
        case CmpInst::FCMP_OLE:
        case CmpInst::FCMP_ULE:
            return CL_BINOP_LE;

        default:
            return CL_BINOP_BAD; // something
    }
}

/// create compare (binary) instruction
void CLPass::handleCmpInstruction(Instruction *I) {

    struct cl_insn i;
    i.code = CL_INSN_BINOP;
    i.loc = cl_loc_known; //FIXME
    i.data.insn_binop.code = getCLCodePredic(cast<CmpInst>(I)->getPredicate());
    if (i.data.insn_binop.code == CL_BINOP_BAD) {
        CL_WARN("unknown compare operator");
        return;
    }

    struct cl_operand dst, src1, src2;
    handleOperand(cast<Value>(I), &dst);
    handleOperand(I->getOperand(0), &src1);
    handleOperand(I->getOperand(1), &src2);
    i.data.insn_binop.dst = &dst;
    i.data.insn_binop.src1 = &src1;
    i.data.insn_binop.src2 = &src2;
    cl->insn(cl, &i);
}

/// create cast instruction, but if instruction is bitcast, it's possible
/// that accessor for union type
void CLPass::handleCastInstruction(CastInst *I) {

    struct cl_insn i;
    i.code = CL_INSN_UNOP;
    i.loc = cl_loc_known; //FIXME
    i.data.insn_unop.code = CL_UNOP_ASSIGN;

    if ((I->getOpcode() == Instruction::UIToFP) ||
            (I->getOpcode() == Instruction::SIToFP)) {
        i.data.insn_unop.code = CL_UNOP_FLOAT;
    }

    struct cl_operand dst, src;
    handleOperand(I->getOperand(0), &src);
    handleOperand(I, &dst);
    i.data.insn_unop.dst = &dst;
    i.data.insn_unop.src = &src;


    if(dst.type->code == CL_TYPE_BOOL && src.code == CL_OPERAND_CST) {
        src.type = dst.type;
    }



    if (isa<BitCastInst>(I) && src.type->code == CL_TYPE_PTR &&
            src.type->items[0].type->code == CL_TYPE_UNION) {
        // BitCastInst - is accessor for union not "instruction"
        struct cl_accessor **accPrev = &src.accessor;
        struct cl_accessor *acc;

        acc = new struct cl_accessor;
        *accPrev = acc;

        acc->code = CL_ACCESSOR_DEREF; // *
        acc->type = src.type;
        src.type = const_cast<struct cl_type *>(src.type->items[0].type);
        acc->next = new struct cl_accessor;

        acc->next->code = CL_ACCESSOR_ITEM; // .
        acc->next->type = src.type;
        src.type = const_cast<struct cl_type *>(dst.type->items[0].type);
        acc->next->data.item.id = 0;
        acc->next->next = new struct cl_accessor;

        acc->next->next->code = CL_ACCESSOR_REF; // &
        acc->next->next->type = src.type;
        src.type = dst.type;
        acc->next->next->next = nullptr;

        /* stupid! bitcast don't change memory
           } else if (src.type->items[0].type->code == CL_TYPE_PTR) {
        // bitcast i8** %ptr to i8*
        struct cl_accessor **accPrev = &src.accessor;
        struct cl_accessor *acc;

        acc = new struct cl_accessor;
         *accPrev = acc;

         acc->code = CL_ACCESSOR_DEREF; // *
         acc->type = src.type;
         src.type = const_cast<struct cl_type *>(src.type->items[0].type);
         acc->next = nullptr;

         } else if (dst.type->code == CL_TYPE_PTR &&
         dst.type->items[0].type->code == CL_TYPE_PTR) {
        // bitcast i8* %7 to i8**
        struct cl_accessor **accPrev = &src.accessor;
        struct cl_accessor *acc;

        acc = new struct cl_accessor;
         *accPrev = acc;

         acc->code = CL_ACCESSOR_REF; // &
         acc->type = src.type;
         src.type = const_cast<struct cl_type *>(dst.type);
         acc->next = nullptr;
         }*/

        if (CLVerbose > 2)
            acToStream(std::cerr, src.accessor, false);
}


cl->insn(cl, &i);
freeAccessor(src.accessor);
//    freeAccessor(dst.accessor);
}

/// hanlde for call instruction
/// now with support for constant expression
void CLPass::handleCallInstruction(CallInst *I) {

    struct cl_operand dst, fnc;
    handleOperand(I, &dst);

    if (dst.code != CL_OPERAND_VOID && dst.type->code == CL_TYPE_VOID)
        dst.code = CL_OPERAND_VOID;

    handleOperand(I->getCalledValue(), &fnc);

    if (fnc.data.cst.data.cst_fnc.name == nullptr)
        return;

    unsigned args = I->getNumArgOperands();
    struct cl_operand argumets[args];
    unsigned idx = 0;

    while (args != idx) {
        handleOperand(I->getArgOperand(idx), &argumets[idx]);
        ++idx;
    }

    cl->insn_call_open(cl, &cl_loc_known, &dst, &fnc);
    delete [] fnc.data.cst.data.cst_fnc.name;

    for(unsigned i=0; i < args; ++i)
        cl->insn_call_arg(cl, i, &argumets[i]);

    cl->insn_call_close(cl);
}

/// last function, clean up after pass, set CL on valid and setup exit code
bool CLPass::doFinalization (Module &) {

    // CL is valid
    cl->acknowledge(cl);

    cl->destroy(cl);
    cl_global_cleanup();
    freeTypeTable();
    freeVarTable();

    //used Dudka's code

    if (!CLPreserve) {
        if (CLPrint::cntErrors) {
            // this causes non-zero exit code
            CL_ERROR("Pass has detected some errors");
            exit(EXIT_FAILURE);
        }
        else if (CLPrint::cntWarnings) {
            // this causes non-zero exit code in case of -Werror
            CL_ERROR("Pass has reported some warnings");
            exit(EXIT_FAILURE);
        }
    }

    return true;
}



// with -help > Optimizations available:
char CLPass::ID;
RegisterPass<CLPass> X(plugName.c_str(), fullName.c_str());

/// for load extern symbols another analyzer
extern "C" { void plugin_init(void) { } }

/*
   static void registerMyPass(const PassManagerBuilder &pb, PassManagerBase &pm)
   {
   if (pb.OptLevel == 0 && pb.SizeLevel == 0) 
   pm.add(new CLPass);
   }

   static RegisterStandardPasses
   RegisterMyPass(PassManagerBuilder::EP_EnabledOnOptLevel0 , registerMyPass);
   */

/*

   clang -cc1 -emit-llvm -femit-all-decls  source.c -o - | opt -instnamer -mem2reg -print-module >/dev/null

   option (--time-passes) > get information about the execution time of your pass
   along with the other passes you queue up. 

*/
