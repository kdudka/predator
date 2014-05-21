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


#ifndef H_CLPLUG_H
#define H_CLPLUG_H

extern "C" {
#include <cl/code_listener.h>
}

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

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

//public:
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

        struct cl_loc findLoc(Instruction *);

        void freeTypeTable(void);
        void freeVarTable(void);
        void freeInitial(struct cl_initializer *);
        void freeAccessor(struct cl_accessor *);

        void handleFunction(Function *);

        void handleInstruction(Instruction *);
        void handleAllocaInstruction(AllocaInst *);
        void handleBinInstruction(Instruction *);
        enum cl_binop_e getCLCode(Instruction *);
        void handleCmpInstruction(Instruction *);
        void handleGEPInstruction(GetElementPtrInst *);
        void handleCastInstruction(CastInst *);
        enum cl_binop_e getCLCodePredic(enum CmpInst::Predicate);
        void handleBranchInstruction(BranchInst *);
        void testPhi(BasicBlock *, BasicBlock *);
        void handleSelectInstruction(SelectInst *);
        void handleCallInstruction(CallInst *);

        void handleOperand(Value *, struct cl_operand *);
        void handleFncOperand(Function *, struct cl_operand *);
        void handleConstant(Value *, struct cl_operand *);
        struct cl_var *handleVariable(Value *);

        struct cl_type *handleType(Type *);
        void handleIntegerType(IntegerType *, struct cl_type *);
        void handleStructType(StructType *, struct cl_type *);
        void handleFunctionType(FunctionType *, struct cl_type *);

};


#endif /* H_CLPLUG_H */
