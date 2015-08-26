/*
 * Copyright (C) 2014-2015 Veronika Sokova <xsokov00@stud.fit.vutbr.cz>
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

#include <cl/code_listener.h>

extern "C" {
#include "../config_cl.h"
#include "../version_cl.h"

#include <unistd.h> // PID
}

#include <cl/cldebug.hh>
#include <cl/cl_msg.hh>
#include "clplug.hh"

#include "llvm/Transforms/Scalar.h" // createLowerSwitchPass
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm/Support/CommandLine.h" // namespace cl
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/DebugInfo.h"

#include <fstream>
#include <cstring> // memcpy
#include <queue>

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

// unsupport location
struct cl_loc cl_loc_known = {
    0,                             // .file
    0,                             // .line
    0,                             // .column
    0,                             // .sysp
};

static struct cl_type builtinIntType = {
    -1,                            // uid
    CL_TYPE_INT,                   // code
    cl_loc_known,                  // loc
    CL_SCOPE_GLOBAL,               // scope
    "<builtin_int>",               // *name
    sizeof(int),                   // FIXME size
    0,                             // item_cnt
    nullptr,                       // *items
    0,                             // array_size
    false,                         // is_unsigned
    false,                         // is_const
    CL_PTR_TYPE_NOT_PTR            // ptr_type
};

struct cl_type_item allocaArg[3] = {
	{nullptr, nullptr, 0},         // return
	{&builtinIntType, nullptr, 0}, // size
	{&builtinIntType, nullptr, 0}  // align
};

static struct cl_type allocaFncType = {
    -2,                            // uid
    CL_TYPE_FNC,                   // code
    cl_loc_known,                  // loc
    CL_SCOPE_GLOBAL,               // scope
    "<builtin_alloca>",            // *name
    0,                             // size
    3,                             // item_cnt - return + size + align
    allocaArg,                     // *items
    0,                             // array_size
    false                          // is_unsigned
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

void CLPass::cleanAll() {

	cl->destroy(cl);
	cl_global_cleanup();
	freeTypeTable();
	freeVarTable();
}

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

void CLPass::freeOperand(const struct cl_operand *clo) {

	freeAccessor(clo->accessor);
	if (clo->code == CL_OPERAND_CST && clo->data.cst.code == CL_TYPE_STRING
	    && clo->data.cst.data.cst_string.value != nullptr)
		delete [] clo->data.cst.data.cst_string.value;
	delete clo;
}

void CLPass::freeInitial(struct cl_initializer *init) {

    while (init) {
        struct cl_initializer *next = init->next;

        if (init->insn.code == CL_INSN_UNOP) {
            freeOperand(init->insn.data.insn_unop.dst);
            freeOperand(init->insn.data.insn_unop.src);
        } else if (init->insn.code == CL_INSN_BINOP) {
            freeOperand(init->insn.data.insn_binop.dst);
            freeOperand(init->insn.data.insn_binop.src1);
            freeOperand(init->insn.data.insn_binop.src2);
        } else {
			CL_ERROR("free invalid operand.");
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

/// debug information for instruction
void CLPass::findLocation(Instruction *i, struct cl_loc *loc) {

	*loc = cl_loc_known;
	if (DebugLoc dbg = i->getDebugLoc()) {
		loc->line = dbg.getLine();
		loc->column = dbg.getCol();
	}
}

/// get operand of intiger type
void CLPass::getIntOperand(int num, struct cl_operand *clo) {
	clo->code = CL_OPERAND_CST;
	clo->scope = CL_SCOPE_GLOBAL;
	clo->type = &builtinIntType;
	clo->accessor = nullptr;
	clo->data.cst.code = clo->type->code;
	clo->data.cst.data.cst_int.value = num;
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

    std::string cfg = ((CLDryRun)? "unfold_switch,unify_labels_gl" : "unify_labels_fnc");
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

    if (!CLDryRun) {
        configCL = "listener=\"easy\"";
        if (!CLArgs.empty()) {
            configCL += " listener_args=\""+ CLArgs + "\"";
        }
        configCL += " clf=\""+ cfg +"\"";
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
        CLPrint::warn,  // warn
        CLPrint::error, // error
        CLPrint::note,  // note
        CLPrint::die,   // die
        CLVerbose       // debug_level
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

	// name of source file
	if (NamedMDNode *NMD = M.getNamedMetadata("llvm.dbg.cu")) {
        std::string tmp = cast<DICompileUnit>(NMD->getOperand(0))->getFilename().str();
        unsigned len = tmp.length() + 1;
        char *tmpPtr = new char [len]; // delete into code listener
        std::memcpy(tmpPtr, tmp.c_str(), len);
        cl_loc_known.file = tmpPtr;
	} else {
		cl_loc_known.file = M.getModuleIdentifier().c_str();
	}

    // one module for one source file
    cl->file_open(cl, cl_loc_known.file); // open source file
    CL_DEBUG("start "<< __func__<<" for "<<cl_loc_known.file);

    for (Module::iterator F = M.begin(), FE = M.end(); F != FE; ++F)
    { // functions

        CL_DEBUG("pass analyzes Function " << F->getName().str() << "()");

        if (F->isDeclaration()) continue;
        handleFunction(F); // open function

        for (Function::iterator BB = F->begin(), BE = F->end(); BB != BE; ++BB)
        { // basic blocks

            CL_DEBUG("pass analyzes BasicBlock " << BB->getName().str() 
                     << " - " << BB->size() << " instructions");
            // LABEL NAME
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
    CL_DEBUG("end of convert");
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

    clt->loc = cl_loc_known; //FIXME MetaData -> DIType

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
            handleFunctionType(cast<FunctionType>(t), clt);
        }
            break;

        case Type::VectorTyID:
            CL_WARN("unsupport type VECTOR");
            clt->code = CL_TYPE_UNKNOWN;
            break;

        default: // MetadataTyID, X86_MMXTyID
            CL_WARN("unknown type");
            clt->code = CL_TYPE_UNKNOWN;
            break;
    }
    //    errs() << "ID<TY<<<<<" << clt->uid << ">>>>>";
	if (CLVerbose > 2) {
		cltToStream(std::cerr, clt, /*depth*/ 0);
		std::cerr<<"\n";
	}
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
    clt->is_unsigned = !t->getSignBit(); //FIXME DIType
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
    // ret type
    clt->items[0].name = nullptr; 
    clt->items[0].type = handleType(ft->getReturnType());
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

        item->name = nullptr; // FIXME DIDerivedType

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
    //------------------------
    fnc->type = handleType(F->getFunctionType());
    fnc->accessor = nullptr;

    fnc->code = CL_OPERAND_CST;
    fnc->scope = ((F->hasInternalLinkage())? CL_SCOPE_STATIC : CL_SCOPE_GLOBAL);
    fnc->data.cst.code = CL_TYPE_FNC;
    fnc->data.cst.data.cst_fnc.uid = F->getContext().getMDKindID(F->getName());
    // I hope that is unique

    fnc->data.cst.data.cst_fnc.is_extern = F->isDeclaration();

    fnc->data.cst.data.cst_fnc.loc = cl_loc_known;
    if (DISubprogram *DI = getDISubprogram(F)) {
		// DISubprogram without column
		fnc->data.cst.data.cst_fnc.loc.line = DI->getLine();
	}
}

/// handle for open function, when is call from Module, iterate argument list
void CLPass::handleFunction(Function *F) {

    struct cl_operand fnc;
    handleFncOperand(F, &fnc);
    if (fnc.data.cst.data.cst_fnc.name == nullptr)
        return;
    cl->fnc_open(cl, &fnc);
    delete [] fnc.data.cst.data.cst_fnc.name;

    for (Function::arg_iterator FA = F->arg_begin(), FAE = F->arg_end(); FA != FAE; ++FA)
    { // args
        struct cl_operand arg;
        handleOperand(FA, &arg);
        cl->fnc_arg_decl(cl, FA->getArgNo()+1, &arg); // from 1->
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
    i.loc = cl_loc_known; // artificial instruction
    i.data.insn_jmp.label = name.c_str(); // LABEL NAME
    cl->insn(cl, &i);
}

/// handle for constant, setup from cl_type in operand
/// if is bool constant (TRUE, FALSE), type must be CL_TYPE_INT for access in union data
/// return true, if set up accesor
bool CLPass::handleBasicConstant(Value *v, struct cl_operand *clo) {

    clo->code = CL_OPERAND_CST;
    clo->scope = CL_SCOPE_GLOBAL; // maybe?
    clo->type = handleType(v->getType());
    clo->data.cst.code = clo->type->code;

    CL_DEBUG3("  CONSTANT [type=" << v->getType()->getTypeID() << "/" 
              << clo->type->code << "/"<<clo->data.cst.code <<"]\n");

	if (isa<ConstantArray>(v) || isa<ConstantStruct>(v) || 
        isa<ConstantDataSequential>(v)) {
		CL_ERROR("composite constant literals are not basic constant");
		return false;
	}
	if (isa<UndefValue>(v)) {
		clo->data.cst.data.cst_int.value = 0; // anything - is error
		clo->data.cst.code = CL_TYPE_INT;
		CL_ERROR("undefined constant");
		return false;
	}

    switch (clo->data.cst.code) {
		// all ConstantInt
        case CL_TYPE_BOOL:
        case CL_TYPE_CHAR:
            clo->data.cst.code = CL_TYPE_INT; // union type
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
            if (isa<ConstantPointerNull>(v)) { // nullptr
                clo->data.cst.data.cst_int.value = 0;
                clo->data.cst.code = CL_TYPE_INT; // union type
                break;
            }

        default:
            CL_WARN("unknown constant");
            break;
    }
	return false;
}

bool CLPass::isStringLiteral(Instruction *vi) {

	if (GetElementPtrInst *gep = dyn_cast<GetElementPtrInst>(vi)) 
		if (GlobalVariable *gv = dyn_cast<GlobalVariable>(gep->getPointerOperand())) 
			if (ConstantDataSequential *c = dyn_cast<ConstantDataSequential>(gv->getInitializer())) 
				if (gep->hasAllZeroIndices() && 
					gv->isConstant() && 
					gv->hasUnnamedAddr() && 
					gv->hasPrivateLinkage() &&
					c->isCString() ) // is all necessary?
					return true;
	return false;
}

/// handle for @b CL_TYPE_STRING
/// don't set operand's type (must called)
void CLPass::handleStringLiteral(ConstantDataSequential *c, struct cl_operand *clo) {

    if (!c->isCString()) {
		CL_ERROR("not c-string literal");
		return;
	}
    CL_DEBUG3("  STRING LITERAL\n");
    clo->code = CL_OPERAND_CST;
    clo->scope = CL_SCOPE_GLOBAL;
    clo->type = nullptr;
    clo->data.cst.code = CL_TYPE_STRING;

    std::string tmp = c->getAsString().str();
    unsigned len = tmp.length() + 1;
    char * tmpPtr = new char [len]; // alloc string
    std::memcpy(tmpPtr, tmp.c_str(), len);
    clo->data.cst.data.cst_string.value = tmpPtr;

}

/// handle for local and global/static variables, return cl_var, which 
/// find in varTable, or create new
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

	clv->loc = cl_loc_known;
    
    if (v->hasName()) { // named variable
        std::string tmp = v->getName().str();
        unsigned len = tmp.length() + 1;
        char * tmpPtr = new char [len];
        std::memcpy(tmpPtr, tmp.c_str(), len);
        clv->name = tmpPtr;
        clv->artificial = false;

		if (v->isUsedByMetadata()) { //FIXME DIVariable
			//if (NamedMDNode *NMD = M.getNamedMetadata(tmp)) {
				//if (dyn_cast<DIVariable>(NMD->getOperand(0)))
					CL_DEBUG3(" \\--> have metadata\n");
			//} 
		}
//        if( DbgDeclareInst *ddi = FindAllocaDbgDeclare(v)) {
//			getDebugLoc()
//			getVariable()
//		}
        
    } else { // register
        clv->name = nullptr;
        clv->artificial = true;
    }

    if (isa<GlobalVariable>(v)) {
		handleGlobalVariable(cast<GlobalVariable>(v), clv);
    } else {
        clv->initial =  nullptr;
        clv->initialized = false;
        clv->is_extern = false;
    }

    return clv;
}


/// all global/static variables in LLVM are pointer type -> not 
/// correspond with code listener, use as
/// <ptrTy> "<globName>" := & <valueTy> constant
void CLPass::handleGlobalVariable(GlobalVariable *gv, struct cl_var *clv) {

	CL_DEBUG3("  GLOBAL VAR PTR\n");

	clv->is_extern = gv->isDeclaration(); //hasExternalLinkage(); // ???
	clv->initial =  nullptr; 
	if ((clv->initialized = gv->hasUniqueInitializer()) == false)
		return;

	Constant *c = gv->getInitializer();

	switch (c->getValueID()) {
		case Value::GlobalAliasVal:
		case Value::GlobalVariableVal:
		case Value::FunctionVal:
		case Value::ConstantIntVal:
		case Value::ConstantFPVal:
		case Value::ConstantPointerNullVal:
			handleSimplyInitializer(c, clv, &CLPass::handleOperand);
			break;

		case Value::ConstantExprVal: {
			Instruction *ci = cast<ConstantExpr>(c)->getAsInstruction();
			if (isStringLiteral(ci)) {
				// string literal
				handleSimplyInitializer(c, clv, &CLPass::handleOperand);
			} else if (isa<GetElementPtrInst>(ci)) { 
				// GEP
				handleSimplyInitializer(ci, clv, &CLPass::handleGEPOperand);
			} else if (ci->isCast()) { 
				// cast
				handleSimplyInitializer(ci, clv, &CLPass::handleCastOperand);
			} else {
				CL_WARN("unsupported global initializer with expression: "
				        << ci->getOpcodeName());
				clv->initialized = false;
			}
			break;
		}

		case Value::ConstantDataArrayVal:
			if (cast<ConstantDataArray>(c)->isCString()) {
				handleSimplyInitializer(c, clv, &CLPass::handleOperand);
				break;
			} // else is array literal
		case Value::ConstantArrayVal:
		case Value::ConstantAggregateZeroVal:
		case Value::ConstantStructVal:
			handleAggregateLiteralInitializer(c, clv);
			break;

		case Value::UndefValueVal: // is possible ?
			CL_DEBUG3("undef value global initializer");
			clv->initialized = false;
			break;

		case Value::BlockAddressVal:
		case Value::ConstantVectorVal:
		case Value::ConstantDataVectorVal:
		default:
			CL_WARN("unsupported global initializer ["<<c->getValueID()<<"]");
			clv->initialized = false;
			break;
	}

}

/// create assign initializer for global/static variables
/// getSrcOperand -> from where is src taken FIXME > delete
void CLPass::handleSimplyInitializer(Value *c, struct cl_var *clv,
                                     funcSrc getSrcOperand) {

	CL_DEBUG3("  GLOBAL CONSTANT ["<<c->getValueID()<<"]\n");

	struct cl_operand *src = new struct cl_operand;
	struct cl_operand *dst = new struct cl_operand;
	dst->code = CL_OPERAND_VAR;
	dst->scope = CL_SCOPE_STATIC;
	dst->type = handleType(c->getType());
	dst->accessor = nullptr;
	dst->data.var = clv;

	clv->initial = new struct cl_initializer;
	clv->initial->next = nullptr;

	(this->*getSrcOperand)(c, src);

	if (CLVerbose > 2) {
		std::cerr << "    initializer: ";
		operandToStream(std::cerr, *src);
		CL_DEBUG3("\n");
	}
	// assign instruction
	clv->initial->insn.loc = clv->loc; // from variable
	clv->initial->insn.code = CL_INSN_UNOP;
	clv->initial->insn.data.insn_unop.code = CL_UNOP_ASSIGN;
dst->type = src->type;
	clv->initial->insn.data.insn_unop.dst = dst;
	clv->initial->insn.data.insn_unop.src = src;
}

/// for global/static variables 
/// splits initializer for structure / array literal into assignments
void CLPass::handleAggregateLiteralInitializer(Constant *c, 
                                               struct cl_var *clv) {

	struct cl_operand *where = new struct cl_operand;
	where->code = CL_OPERAND_VAR;
	where->scope = CL_SCOPE_GLOBAL; // ?
	where->type = handleType(c->getType());
	where->accessor = nullptr;
	where->data.var = clv;

	struct cl_initializer **addrInit = &(clv->initial);
	struct cl_operand *dst, *src;

	std::queue< Ttrio > elmFIFO;
	Ttrio act = {c, nullptr, nullptr}; // actual element
	elmFIFO.push(act);

	while (!elmFIFO.empty()) { // removing recursion

		act = elmFIFO.front(); elmFIFO.pop();
		Type *tt = (act.elm)->getType();

		if (!tt->isAggregateType()) {
			// elementary type -> insert assign

			// source operand
			src = new struct cl_operand;
			if (isa<ConstantExpr>(act.elm)) {
				Instruction *ci = dyn_cast<ConstantExpr>(act.elm)->getAsInstruction();
				if (isStringLiteral(ci)) {
					handleOperand(act.elm, src);
				} else if (isa<GetElementPtrInst>(ci)) {
					handleGEPOperand(ci, src);
				} else if (ci->isCast()) {
					handleCastOperand(ci, src);
				} else {
					CL_WARN("unsupported global initializer with expression: "
							<< ci->getOpcodeName());
					delete src; freeAccessor(act.firstAcc);
					continue;
				}
			} else {
				handleOperand(act.elm, src);
			}

			// destination operand
			dst = new struct cl_operand;
			memcpy(dst,where,sizeof(*dst));
			dst->accessor = act.firstAcc;
			dst->type = src->type;

			if (CLVerbose > 2) {
				std::cerr<<"/>>~~~~~~~~~~~~\\\n";
				operandToStream(std::cerr, *src);
				std::cerr << "\n";
				acToStream(std::cerr, dst->accessor, false);
				std::cerr<<"\\~~~~~~~~~~~~~~/\n";
			}

			insertAssign(&addrInit, dst, src);
			continue;
		}

		// aggregate type -> push all elements
		unsigned numElms;
		if (isa<StructType>(tt))
			numElms =  tt->getStructNumElements();
		else if (isa<ArrayType>(tt))
			numElms =  tt->getArrayNumElements();
		else {
			CL_ERROR("unsupported type for aggregate initializer");
			continue;
		}

		for (unsigned i = 0; i < numElms; ++i) {

			Ttrio nxt; // new element
			nxt.elm = (act.elm)->getAggregateElement(i);

			struct cl_accessor *tmp = new struct cl_accessor;
			if (act.firstAcc == nullptr) {
				nxt.firstAcc = tmp;
				nxt.addrNextAcc = &tmp;
			} else {
				nxt.firstAcc = act.firstAcc;
				nxt.addrNextAcc = act.addrNextAcc;
				depthCopyAccessor(&nxt.firstAcc, &nxt.addrNextAcc);
			}
			
			*(nxt.addrNextAcc) = tmp;
			tmp->type = handleType(tt); 
			tmp->next = nullptr;

			switch (tmp->type->code) {
				case CL_TYPE_STRUCT :
					tmp->code = CL_ACCESSOR_ITEM;
					tmp->data.item.id = i;
					break;

				case CL_TYPE_ARRAY :
					tmp->code = CL_ACCESSOR_DEREF_ARRAY;
					tmp->data.array.index = new struct cl_operand;
					getIntOperand(i, tmp->data.array.index);
					break;

				default: break;
			}

			nxt.addrNextAcc = &(tmp->next);

			elmFIFO.push(nxt);
		}
		freeAccessor(act.firstAcc); // prefix before copy
	}

}

/// copy accessor structure
struct cl_accessor *CLPass::copyAccessor(struct cl_accessor *acc) {

	struct cl_accessor *tmp = new struct cl_accessor;
	memcpy(tmp, acc, sizeof(*tmp));

	if (CL_ACCESSOR_DEREF_ARRAY == acc->code) {
		struct cl_operand *clo = new struct cl_operand;
		memcpy(clo, acc->data.array.index, sizeof(*clo));
		tmp->data.array.index = clo;
	}
	return tmp;
}

/// copy chained list of accessors
/// @param ptrFirstAcc return first accessor
/// @param ptrAddrNextAcc return address for next accessor
void CLPass::depthCopyAccessor(struct cl_accessor **ptrFirstAcc, 
                               struct cl_accessor ***ptrAddrNextAcc) {

	*ptrAddrNextAcc = ptrFirstAcc;
	struct cl_accessor *acc = *ptrFirstAcc;
	if (!acc)
		return;

	struct cl_accessor *tmp = copyAccessor(acc);
	*ptrFirstAcc = tmp;
	*ptrAddrNextAcc = &(tmp->next);

	for (acc = acc->next ; acc; acc = acc->next) {
		tmp = copyAccessor(acc);
		**ptrAddrNextAcc = tmp;
		*ptrAddrNextAcc = &(tmp->next);
	}

}

/// assignment instruction for global/static variable initializer
/// @param ptrAddrInit return address for next initializer
void CLPass::insertAssign(struct cl_initializer ***ptrAddrInit, 
                          struct cl_operand *dst,
                          struct cl_operand *src) {

	struct cl_initializer *init = new struct cl_initializer;
	**ptrAddrInit = init;

	init->insn.loc = dst->data.var->loc; // from dst (always variable)
	init->insn.code = CL_INSN_UNOP;
	init->insn.data.insn_unop.code = CL_UNOP_ASSIGN;
	init->insn.data.insn_unop.dst = dst;
	init->insn.data.insn_unop.src = src;
	init->next = nullptr;

	*ptrAddrInit = &(init->next);
}

/// specifies concrete operand of instruction - most important function
/// if is global alias or constant expression, this function is called recursively
/// no operand if code == CL_OPERAND_VOID
/// return true, if insert one accessor
bool CLPass::handleOperand(Value *v, struct cl_operand *clo) {

	std::memset(clo, 0, sizeof(*clo));
    clo->code = CL_OPERAND_VOID;

    if (v == nullptr) return false;

    clo->accessor = nullptr;

    if (isa<Argument>(v)) {               // func. argument
                                          // FIXME same as variable ?
        CL_DEBUG3(" ARGUMENT\n");
        clo->code = CL_OPERAND_VAR;
        clo->scope = CL_SCOPE_FUNCTION;
        clo->type = handleType(v->getType());
        clo->data.var = handleVariable(v);

    } else if (isa<Constant>(v)) {

        if (isa<GlobalVariable>(v)) {     // global/static variable

            CL_DEBUG3(" GLOBAL VAR\n");
            clo->code = CL_OPERAND_VAR;
            clo->scope = (cast<GlobalVariable>(v)->hasLocalLinkage())? 
                          CL_SCOPE_STATIC : CL_SCOPE_GLOBAL;
            clo->type = handleType(v->getType());
            clo->data.var = handleVariable(v);

			clo->accessor = new struct cl_accessor; // FIXME possible memory leak
			clo->accessor->code = CL_ACCESSOR_REF; // &
			clo->accessor->type = handleType(cast<GlobalVariable>(v)->getValueType());
			clo->accessor->next = nullptr;
			
			return true;

        } else if (isa<Function>(v)) {    // function
            CL_DEBUG3(" FUNCTION\n");
            handleFncOperand(cast<Function>(v), clo);
            // function is pointer, but function can be a pointer
            // possible different type in ... store
            // clo->type = handleType(v->getType());

        } else if (isa<GlobalAlias>(v)) {
            CL_DEBUG3(" GLOBAL ALIAS\n");
            return handleOperand(cast<GlobalAlias>(v)->getAliasee(), clo);
        } else if (isa<ConstantExpr>(v)) {

			Instruction *vi = cast<ConstantExpr>(v)->getAsInstruction();
			if (isStringLiteral(vi)) {    // constant literal
				handleStringLiteral(cast<ConstantDataSequential>((
				                     cast<GlobalVariable>(
				                      vi->getOperand(0)))->getInitializer()),
				                    clo);
				clo->type = handleType(vi->getType());
			} else {                      // constant expression
				CL_DEBUG3("CONSTANT EXPR\n");
				handleInstruction(vi); // recursion
				return handleOperand(vi, clo);
			}

        } else {                          // just constant
            handleBasicConstant(v, clo);
        }

    } else {                              // named variable or register

        CL_DEBUG3(" VARIABLE\n");
        clo->code = CL_OPERAND_VAR;
        clo->scope = CL_SCOPE_FUNCTION;
        clo->type = handleType(v->getType());
        clo->data.var = handleVariable(v);

    }
	return false;
}

/// handle of instruction, support just C-liked instruction
/// not intrinsic
/// in this function is: Ret, Unreachable, Load and Store
void CLPass::handleInstruction(Instruction *I) {

    CL_DEBUG("Inst code = " << I->getOpcodeName() << " [" << I->getOpcode() << "]");
    CL_DEBUG3(*I<<"\n");

	unsigned opcode = I->getOpcode();

    if (I->isBinaryOp()) { // BINOP
        handleBinInstruction(I);
        return;
    } else if (I->isCast() || opcode == Instruction::Load || 
               opcode == Instruction::GetElementPtr) { // UNOP
        handleUnaryInstruction(I);
        return;
    }

    struct cl_insn i;
    std::memset(&i, 0, sizeof(i));
    findLocation(I, &i.loc); 
    struct cl_operand dst, src;

    switch (opcode) {
        // Terminator instruction

        case Instruction::Ret: // ReturnInst
            i.code = CL_INSN_RET;
            handleOperand(cast<ReturnInst>(I)->getReturnValue(), &src);
            i.data.insn_ret.src = &src;
            cl->insn(cl, &i);
            break;

        case Instruction::Br: // BranchInst
            handleBranchInstruction(cast<BranchInst>(I));
            break;

        case Instruction::Unreachable : // UnreachableInst
            i.code = CL_INSN_ABORT;
            cl->insn(cl, &i);
            break;

        //Memory operators

        case Instruction::Alloca: {// AllocaInst - Stack management
            handleAllocaInstruction(cast<AllocaInst>(I));
            break;
        }

        case Instruction::Store: {// StoreInst
            i.code = CL_INSN_UNOP;
            i.data.insn_unop.code = CL_UNOP_ASSIGN;
            handleOperand(I->getOperand(0), &src);
            bool notEmptyAcc = handleOperand(I->getOperand(1), &dst);
            if (dst.code == CL_OPERAND_VAR)
            {
				if (notEmptyAcc && dst.accessor->code == CL_ACCESSOR_REF) {
					dst.type = dst.accessor->type;
					freeAccessor(dst.accessor);
					dst.accessor = nullptr;
				} else {
					struct cl_accessor **acc = (notEmptyAcc)? &(dst.accessor->next) : &(dst.accessor);
					*acc = new struct cl_accessor;
					(*acc)->code = CL_ACCESSOR_DEREF; // *
					(*acc)->type = dst.type;
					dst.type = const_cast<struct cl_type *>(dst.type->items[0].type); 
					(*acc)->next = nullptr;
				} // because *& -> empty accessor
            }
            if (dst.type != src.type) { 
				// must same (problem with type for CL_TYPE_FNC)
				CL_DEBUG("different types in store instruction!");
				src.type = dst.type;
			}
            i.data.insn_unop.dst = &dst;
            i.data.insn_unop.src = &src;
            cl->insn(cl, &i);
            freeAccessor(dst.accessor);
            break; }//docasne

        // Comparison instructions

        case Instruction::ICmp: // ICmpInst
        case Instruction::FCmp: // FCmpInst
            handleCmpInstruction(I);
            break;

        // Other instructions

        case Instruction::Call: // CallInst
            if (isa<DbgInfoIntrinsic>(I)) { // not instruction
                return;
            }
//            if (isa<IntrinsicInst>(I) && !isa<MemIntrinsic>(I)) {
//                CL_WARN("intrinsic instructions are not supported");
//            } else {
                handleCallInstruction(cast<CallInst>(I));
//            }
            break;

        case Instruction::Select: // SelectInst
            handleSelectInstruction(cast<SelectInst>(I));
            break;

        case Instruction::PHI: // PHINode - is eliminated in terminal instructions
            break;

        case Instruction::Switch: // SwitchInst : option opt -lowerswitch
			CL_ERROR("instruction switch");
			break;

        case Instruction::ExtractValue: // ExtractValueInst : extract from aggregate
        case Instruction::InsertValue: // InsertValueInst : insert into aggregate

        // unsupport terminator instruction
        case Instruction::IndirectBr: // IndirectBrInst
        case Instruction::Invoke: // InvokeInst
        case Instruction::Resume : // ResumeInst
        // unsupport memory operators
        case Instruction::Fence: // FenceInst
        case Instruction::AtomicCmpXchg: // AtomicCmpXchgInst
        case Instruction::AtomicRMW: // AtomicRMWInst
        default:
            CL_WARN("unsupport instruction "<< I->getOpcodeName());
            break;
    }
}

/// alocation on stack
/// create own function __builtin_alloca_with_align and call it with argumets:
/// allocated size (N*typeSize) and align
void CLPass::handleAllocaInstruction(AllocaInst *I) {

    struct cl_operand dst, fnc, size, align;

    struct cl_type *allocated = handleType(I->getAllocatedType());

    if (I->isArrayAllocation()) {
        handleOperand(I->getArraySize(), &size);
        size.data.cst.data.cst_int.value *= allocated->size;
    } else {
        getIntOperand(allocated->size, &size);
    }
    getIntOperand(I->getAlignment(), &align);

    handleOperand(I, &dst);

    fnc.type = &allocaFncType;
    // return type
    fnc.type->items[0].name = nullptr;
    fnc.type->items[0].type = handleType(I->getType()); // pointer

    fnc.code = CL_OPERAND_CST;
    fnc.scope = CL_SCOPE_GLOBAL;
    fnc.accessor = nullptr;
    fnc.data.cst.code = CL_TYPE_FNC;
    fnc.data.cst.data.cst_fnc.uid = -1;
    fnc.data.cst.data.cst_fnc.is_extern = true;
    fnc.data.cst.data.cst_fnc.loc = cl_loc_known; // artificial function
    fnc.data.cst.data.cst_fnc.name = "__builtin_alloca_with_align";

    // location from dst (always variable)
    cl->insn_call_open(cl, &(dst.data.var->loc), &dst, &fnc);

    cl->insn_call_arg(cl, 1, &size); // from 1->
    cl->insn_call_arg(cl, 2, &align);

    cl->insn_call_close(cl);
}

/// branch for conditional and unconditional jump
void CLPass::handleBranchInstruction(BranchInst *I) {

    struct cl_insn i;
    findLocation(I, &i.loc);
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
        i.data.insn_cond.then_label = bbName1.c_str(); // LABEL NAME
        //else
        bb = I->getSuccessor(1);
        if (!bb->hasName()) {
            bbName2 = "<label"+ std::to_string(bbUID++) +">";
            bb->setName(bbName2);
        } else {
            bbName2 = bb->getName().str();
        }
        i.data.insn_cond.else_label = bbName2.c_str(); // LABEL NAME

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
        i.data.insn_jmp.label = bbName1.c_str(); // LABEL NAME
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
    findLocation(I, &i.loc); 
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
    findLocation(I, &i.loc); 
    findLocation(I, &j.loc); 

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
            return CL_BINOP_UNKNOWN;
    }
}

/// create binary instruction
void CLPass::handleBinInstruction(Instruction *I) {

    struct cl_insn i;
    i.code = CL_INSN_BINOP;
    findLocation(I, &i.loc); 
    i.data.insn_binop.code = getCLCode(I);
    if (i.data.insn_binop.code == CL_BINOP_UNKNOWN) {
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
            return CL_BINOP_UNKNOWN;
    }
}

/// create compare (binary) instruction
void CLPass::handleCmpInstruction(Instruction *I) {

    struct cl_insn i;
    i.code = CL_INSN_BINOP;
    findLocation(I, &i.loc);
    i.data.insn_binop.code = getCLCodePredic(cast<CmpInst>(I)->getPredicate());
    if (i.data.insn_binop.code == CL_BINOP_UNKNOWN) {
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

/// create assign (floating cast) instruction
void CLPass::handleUnaryInstruction(Instruction *I) {

	struct cl_insn i;
	std::memset(&i, 0, sizeof(i));

	i.code = CL_INSN_UNOP;
	i.data.insn_unop.code = ((I->getOpcode() == Instruction::UIToFP) ||
	                         (I->getOpcode() == Instruction::SIToFP))?
	                         CL_UNOP_FLOAT :
	                         CL_UNOP_ASSIGN;
	findLocation(I, &i.loc);

	struct cl_operand dst, src;
	handleOperand(I, &dst);

	if (isa<LoadInst>(I))
		handleLoadOperand(I, &src);
	else if (isa<GetElementPtrInst>(I))
		handleGEPOperand(I, &src);
	else
		handleCastOperand(I, &src);

	i.data.insn_unop.dst = &dst;
	i.data.insn_unop.src = &src;

	if (CLVerbose > 2)
		acToStream(std::cerr, src.accessor, false);

	cl->insn(cl, &i); 
	freeAccessor(src.accessor);
}

/// source operand for load instruction (always as pointer -> *)
/// return true, if insert one accessor
bool CLPass::handleLoadOperand(Value *v, struct cl_operand *src) {

	LoadInst *I = cast<LoadInst>(v);
	bool notEmptyAcc = handleOperand(I->getOperand(0), src);

	if (src->code == CL_OPERAND_VAR) {
		if (notEmptyAcc && src->accessor->code == CL_ACCESSOR_REF) {
			src->type = src->accessor->type;
			freeAccessor(src->accessor);
			src->accessor = nullptr;
			return false;
		} else {
			struct cl_accessor **acc = (notEmptyAcc)? &(src->accessor->next) : &(src->accessor);
			*acc = new struct cl_accessor;
			(*acc)->code = CL_ACCESSOR_DEREF; // *
			(*acc)->type = src->type;
			src->type = const_cast<struct cl_type *>(src->type->items[0].type);// handleType(I->getType()); // result
			(*acc)->next = nullptr;
			return true;
		} // because *& -> empty accessor
	}
	return notEmptyAcc;
}

/// source operand for cast instructions, but if instruction is bitcast,
/// it's possible that is accessor for union type
/// TODO testing more situations
/// return true, if insert accessor(s)
bool CLPass::handleCastOperand(Value *v, struct cl_operand *src) {

	CastInst *I = cast<CastInst>(v);
	bool notEmptyAcc = handleOperand(I->getOperand(0), src);

	if (src->type->code == CL_TYPE_FNC) {
		CL_ERROR("unsupport cast from function type"); 
		return false;
	}
	
	struct cl_type *resultType = handleType(I->getType());

	if(resultType->code == CL_TYPE_BOOL && src->code == CL_OPERAND_CST) {
		src->type = resultType;
	}

	if (isa<BitCastInst>(I) && src->type->code == CL_TYPE_PTR &&
			src->type->items[0].type->code == CL_TYPE_UNION) {
		// BitCastInst - is accessor for union, not "instruction"
		struct cl_accessor **accPrev = (notEmptyAcc)? &(src->accessor->next) : &(src->accessor);
		struct cl_accessor *acc;

		acc = new struct cl_accessor;
		*accPrev = acc;

		acc->code = CL_ACCESSOR_DEREF; // *
		acc->type = src->type;
		src->type = const_cast<struct cl_type *>(src->type->items[0].type);
		acc->next = new struct cl_accessor;

		acc->next->code = CL_ACCESSOR_ITEM; // .
		acc->next->type = src->type;
//		src->type = const_cast<struct cl_type *>(resultType->items[0].type);
		src->type = const_cast<struct cl_type *>(src->type->items[0].type);
		acc->next->data.item.id = 0;
		acc->next->next = new struct cl_accessor;

		acc->next->next->code = CL_ACCESSOR_REF; // &
		acc->next->next->type = src->type;
		src->type = resultType;
		acc->next->next->next = nullptr;
	}
	return notEmptyAcc;
}


/// source operand for GEP instruction
/// in CL accessors for array and structure
/// return true, if insert accessor(s)
bool CLPass::handleGEPOperand(Value *v, struct cl_operand *src) {

	GetElementPtrInst *I = cast<GetElementPtrInst>(v);
	bool notEmptyAcc = handleOperand(I->getPointerOperand(), src); // Operand(0)

	struct cl_accessor **accPrev = (notEmptyAcc)? &(src->accessor->next) : &(src->accessor);
	struct cl_accessor *acc;
	   //GetElementPtrInst::op_iterator
	for (auto op = I->idx_begin(), ope = I->idx_end(); op != ope; ++op) {
		Value *v = *op;

		acc = new struct cl_accessor;
		*accPrev = acc;

		acc->type = src->type;

		acc->next = nullptr;

		switch (src->type->code) {
			case CL_TYPE_STRUCT: {
				//                errs() << "struct "; // must be constant
				int num = cast<ConstantInt>(v)->getValue().getSExtValue();
				src->type = const_cast<struct cl_type *>(src->type->items[num].type); // must be const in cl_type_item ?
				acc->code = CL_ACCESSOR_ITEM;
				acc->data.item.id = num;
			}
			break;

			case CL_TYPE_PTR:
				//                errs() << "ptr ";
				src->type = const_cast<struct cl_type *>(src->type->items[0].type);
				acc->code = CL_ACCESSOR_DEREF;

				// TODO if not 0 -> CL_ACCESSOR_OFFSET

				break;

			case CL_TYPE_ARRAY:
				//                errs() << "array ";
				src->type = const_cast<struct cl_type *>(src->type->items[0].type);
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

	{ // is ...PtrInst, always return REF
		*accPrev = new struct cl_accessor;
		(*accPrev)->code = CL_ACCESSOR_REF; // &
		(*accPrev)->type = src->type;
		src->type =  handleType(I->getType()); // result
		(*accPrev)->next = nullptr;
	}
	// acToStream(std::cerr, src->accessor, false);
	return true;
}

/// hanlde for call instruction
/// with support for constant expression
void CLPass::handleCallInstruction(CallInst *I) {

    struct cl_operand dst, fnc;
    handleOperand(I, &dst);

    if (dst.code != CL_OPERAND_VOID && dst.type->code == CL_TYPE_VOID)
        dst.code = CL_OPERAND_VOID;

    handleOperand(I->getCalledValue(), &fnc);

	if (isa<MemIntrinsic>(I)) { // cut architecture info
		CL_DEBUG("memintrinsic");

		std::string tmp;
		if (isa<MemSetInst>(I))
			tmp = "llvm.memset";
		else if (isa<MemCpyInst>(I))
			tmp = "llvm.memcpy";
		else // MemMoveInst
			tmp = "llvm.memmove";

		unsigned len = tmp.length() + 1;
		char * tmpPtr = new char [len];
		std::memcpy(tmpPtr, tmp.c_str(), len);
		delete [] fnc.data.cst.data.cst_fnc.name; // old name
		fnc.data.cst.data.cst_fnc.name = tmpPtr;
	} 

	if (fnc.data.cst.data.cst_fnc.name == nullptr) {
		CL_DEBUG("indirect call of function");
	}

    // because arguments meybe constant expression -> create new 
    // instruction and argument is defacto new register ()
    // if call is open, can't insert another instructions
    const unsigned args = I->getNumArgOperands();
    struct cl_operand *arguments = new struct cl_operand[args];
    unsigned idx = 0;
    while (args != idx) {
        handleOperand(I->getArgOperand(idx), &arguments[idx]);
        ++idx;
    }

	struct cl_loc callLoc;
	findLocation(I, &callLoc);
    cl->insn_call_open(cl, &callLoc, &dst, &fnc);
    if (fnc.data.cst.data.cst_fnc.name != nullptr)
		delete [] fnc.data.cst.data.cst_fnc.name;

    for(unsigned i=0; i < args; ++i)
        cl->insn_call_arg(cl, i+1, &arguments[i]); // from 1->

    cl->insn_call_close(cl);
    delete [] arguments;
}


/// last function, clean up after pass, set CL on valid and setup exit code
bool CLPass::doFinalization (Module &) {

    CL_DEBUG("start analysis");

    // CL is valid
    cl->acknowledge(cl);

    cleanAll();

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

// registraction opt LLVM pass
// with -help > Optimizations available:
char CLPass::ID;
static RegisterPass<CLPass> X(plugName.c_str(), fullName.c_str());

// registraction clang plug-in without cmd options
static void registerCLPass(const PassManagerBuilder &pb,
                           legacy::PassManagerBase &pm) 
{
	if (pb.OptLevel == 0 && pb.SizeLevel == 0) {
		pm.add(createLowerSwitchPass()); // -lowerswitch
		pm.add(new CLPass);
	}
}

static RegisterStandardPasses
RegisterTestPass(PassManagerBuilder::EP_EnabledOnOptLevel0, registerCLPass);

/// for load extern symbols another analyzer
extern "C" { void plugin_init(void) { } }
