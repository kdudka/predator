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


#ifndef H_CLPLUG_H
#define H_CLPLUG_H

#include <cl/code_listener.h>

#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <unordered_map>


extern std::string plugName;// = "cl";

using namespace llvm;

//===----------------------------------------------------------------------===//
/// CLPrint struct - callbacks for plug-in messages
///
struct CLPrint {

    static int cntErrors;
    static int cntWarnings;

    static void last(void) {
        errs() << " [-" << plugName <<"]\n";
    }

    static void empty(const char *) {}

    static void debug(const char *msg) {
//        if (errs().is_displayed()) errs().changeColor(raw_ostream::CYAN, true, false);
//        errs() << "debug:";
        errs().resetColor() << msg;
        last();
    }

    static void note(const char *msg) {
//        if (errs().is_displayed()) errs().changeColor(raw_ostream::BLACK, true, false);
//        errs() << "note:";
        errs().resetColor() << msg;
        last();
    }

    static void warn(const char *msg) {
//        if (errs().is_displayed()) errs().changeColor(raw_ostream::MAGENTA, true, false);
//        errs() << "warning:";
        errs().resetColor() << msg;
        ++cntWarnings;
        last();
    }

    static void error(const char *msg) {
//        if (errs().is_displayed()) errs().changeColor(raw_ostream::RED, true, false);
//        errs() << "error:";
        errs().resetColor() << msg;
        last();
        ++cntErrors;
    }

    static void die(const char *msg) {
        errs() << msg;
    }

};


//===----------------------------------------------------------------------===//
/// CLPass struct - This is pass, which assign LLVM object to CL object
///
struct CLPass : public ModulePass {

    public:
        static char ID;

        /// The type for the table of operand types.
        typedef std::unordered_map<Type *,struct cl_type *> TypeMap;
        /// The type for the table of variables.
        typedef std::unordered_map<Value *,struct cl_var *> VarMap;
        /// wrapper for global initialization
        typedef bool (CLPass::*funcSrc)(Value*, struct cl_operand *);
        /// The type for FIFO item
        typedef struct queueTrio {
            Constant *elm;
            struct cl_accessor *firstAcc;
            struct cl_accessor **addrNextAcc;
        } Ttrio;

    private:
        unsigned cntUID = 0; ///< generator unique id for variables and types
        int bbUID = 1;       ///< generator unique id for basic blocks
        TypeMap TypeTable;   ///< Table for types
        Type *voidType;      ///< actual unused (for create function type)

        VarMap VarTable;     ///< Table for values
        DataLayout *DL;
        struct cl_code_listener *cl;///< Code Listener object

    public:
        CLPass() : ModulePass(ID) {}
        virtual bool doInitialization(Module &);
        virtual bool runOnModule(Module &);
        virtual bool doFinalization(Module &);

    private:
        void setup(void);
        void writePID(void);
        void appendListener(const char *);

        void getIntOperand(int, struct cl_operand *);
        void findLocation(Instruction *, struct cl_loc *);

        void cleanAll(void);
        void freeTypeTable(void);
        void freeVarTable(void);
        void freeOperand(const struct cl_operand *);
        void freeInitial(struct cl_initializer *);
        void freeAccessor(struct cl_accessor *);

        struct cl_accessor *copyAccessor(struct cl_accessor *);
        void depthCopyAccessor(struct cl_accessor **, 
                               struct cl_accessor ***);

        void handleFunction(Function *);

        /* Instructions */
        void handleInstruction(Instruction *);
        void handleAllocaInstruction(AllocaInst *);
        void handleBinInstruction(Instruction *);
        enum cl_binop_e getCLCode(Instruction *);
        void handleCmpInstruction(Instruction *);
        enum cl_binop_e getCLCodePredic(enum CmpInst::Predicate);
        void handleBranchInstruction(BranchInst *);
        void testCompareInst(Value *, struct cl_operand *);
        inline bool testPhi(BasicBlock *);
        void insertPhiAssign(BasicBlock *, BasicBlock *, const char *);
        void handleSelectInstruction(SelectInst *);
        void handleUnaryInstruction(Instruction *);
        void handleCallInstruction(CallInst *);

        /* Operands */
        bool handleOperand(Value *, struct cl_operand *);
        bool handleLoadOperand(Value *, struct cl_operand *);
        bool handleCastOperand(Value *, struct cl_operand *);
        bool handleGEPOperand(Value *, struct cl_operand *);
        struct cl_operand *insertOffsetAcc(Value *, struct cl_operand *);
        void handleFncOperand(Function *, struct cl_operand *);

        /* Constants and Variables */
        bool handleBasicConstant(Value *, struct cl_operand *);
        bool isStringLiteral(Instruction *);
        void handleStringLiteral(ConstantDataSequential *, struct cl_operand *);
        struct cl_var *handleVariable(Value *);
        void handleGlobalVariable(GlobalVariable *, struct cl_var *);
        void handleSimplyInitializer(Value *, struct cl_var *,
                                     funcSrc);
        void handleAggregateLiteralInitializer(Constant *, 
                                               struct cl_var *);
        void insertAssign(struct cl_initializer ***, 
                          struct cl_operand *, struct cl_operand *);

        /* Types */
        struct cl_type *handleType(Type *);
        void handleIntegerType(IntegerType *, struct cl_type *);
        void handleStructType(StructType *, struct cl_type *);
        void handleFunctionType(FunctionType *, struct cl_type *);

};


#endif /* H_CLPLUG_H */
