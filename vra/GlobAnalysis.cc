/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   GlobAnalysis.cc
* @brief  Because for now only intra-procedural analysis is used, we need to
*         recognize if a global variable can be changed in the program or not.
* @date   2013
*/

#undef NDEBUG   // It is necessary for using assertions.

#include <iterator>
#include <iostream>
#include <stack>
#include <cassert>

#include "GlobAnalysis.h"
#include "Range.h"
#include "Number.h"
#include "Utility.h"
#include "OperandToMemoryPlace.h"

using std::cout;
using std::endl;
using std::iterator;
using std::ostream;
using std::stack;
using std::set;

using CodeStorage::Storage;
using CodeStorage::Fnc;
using CodeStorage::Block;
using CodeStorage::VarDb;
using CodeStorage::Var;
using CodeStorage::VAR_GL;
using CodeStorage::TTargetList;
using CodeStorage::Insn;
using CodeStorage::TOperandList;

GlobAnalysis::GlobVarInit GlobAnalysis::globVarInit;
std::set<int> GlobAnalysis::idOfGlobVarSet;
ValueAnalysis::MemoryPlaceToRangeMap GlobAnalysis::globVarMap;

/**
* @brief Stores the unique id of all global variables used in the program
*        represented by the model stored in @a stor.
*/
void GlobAnalysis::storeGlobVar(const CodeStorage::Storage &stor)
{
	const VarDb &vars = stor.vars;

	for (const Var& var : vars) {
		if (VAR_GL == var.code) {
			GlobAnalysis::idOfGlobVarSet.insert(var.uid);
		}
	}
}

/**
* @brief Initializes that none of the global variables can be modified. This will
*        be changed after the analysis.
*/
void GlobAnalysis::initGlobVar()
{
	for (int uid : GlobAnalysis::idOfGlobVarSet) {
		GlobAnalysis::globVarInit[uid] = false;
	}
}

/**
* @brief Returns @c true if the variable identified by @a uid is global, @c false
*        otherwise.
*/
bool GlobAnalysis::isGlobal(int uid)
{
	return (GlobAnalysis::idOfGlobVarSet.find(uid) !=
			GlobAnalysis::idOfGlobVarSet.end());
}

/**
* @brief If the destination variable in @a insn is a global variable, this function
*        sets that it is modified.
*/
void GlobAnalysis::setIfModified(const Insn *insn)
{
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dest = opList[0];   // [0] - destination

	// On the left side must be variable.
	if (dest.data.var != NULL) {
		int uid = dest.data.var->uid;
		if (isGlobal(uid)) {
			GlobAnalysis::globVarInit[uid] = true;
		}
	}
}

/**
* @brief Computes the glob analysis for the given instruction @a insn.
*/
void GlobAnalysis::computeGlobAnalysisForInsn(const Insn *insn)
{
	const enum cl_insn_e code = insn->code;

	switch (code) {
		case CL_INSN_UNOP:
		case CL_INSN_BINOP:
		case CL_INSN_CALL:
			GlobAnalysis::setIfModified(insn);
			break;

		default:
			// CL_INSN_NOP, CL_INSN_JMP, CL_INSN_COND, CL_INSN_ABORT, CL_INSN_SWITCH
			// CL_INSN_LABEL, CL_INSN_RET
			break;
	}
}

/**
* @brief Computes the glob analysis for the given block @a block.
*/
void GlobAnalysis::computeGlobAnalysisForBlock(const Block *block)
{
	for (const Insn *insn : *block) {
		GlobAnalysis::computeGlobAnalysisForInsn(insn);
	}
}

/**
* @brief Computes the glob analysis for the given function @a fnc.
*/
void GlobAnalysis::computeGlobAnalysisForFnc(const Fnc &fnc)
{
	// Schedulers.
	stack<const Block *> todoStack;
	set<const Block *> doneSet;

	const Block *entryBlock = fnc.cfg.entry();
	todoStack.push(entryBlock);

	while (!todoStack.empty()) {
		const Block *block = todoStack.top();
		todoStack.pop();
		GlobAnalysis::computeGlobAnalysisForBlock(block);
		doneSet.insert(block);

		// Gets the successors of the processed block.
		const TTargetList &succs = block->targets();
		for (const TTargetList::value_type &succ : succs) {
			if (doneSet.find(succ) == doneSet.end()) {
				// We schedule successors of this block that were not processed
				// before.
				todoStack.push(succ);
			}
		}
	}
}

/**
* @brief Computes if the global variables can be modified by some function in the
*        program stored in @a stor.
*/
void GlobAnalysis::computeGlobAnalysis(const CodeStorage::Storage &stor)
{
	// Gets unique ids of all global variables.
	GlobAnalysis::storeGlobVar(stor);

	// Sets that none of the global variables can be modified. This will be
	// changed during analysis.
	GlobAnalysis::initGlobVar();

	for (const Fnc* pFnc : stor.fncs) {
		const Fnc &fnc = *pFnc;

		if (!isDefined(fnc))
			continue;

		GlobAnalysis::computeGlobAnalysisForFnc(fnc);
	}

	// Initializes the map storing ranges for global variables.
	GlobAnalysis::initGlobVarMap(stor);
}

/**
* @brief Return @c true if the global variable is modified in some function, @c false
*        otherwise.
*/
bool GlobAnalysis::isModified(int uid)
{
	return GlobAnalysis::globVarInit[uid];
}

/**
* @brief Emits the computed information about global variables.
*/
ostream& GlobAnalysis::printGlobAnalysis(std::ostream &os)
{
	for (const ValueAnalysis::MemoryPlaceToRangeMap::value_type &g :
		GlobAnalysis::globVarMap) {
		os << g.first << ": " << g.second << endl;
	}
	return os;
}

/**
* @brief Computes initial that is represented by unary instruction @a insn.
*/
void GlobAnalysis::processInitialForUnop(const Insn *insn)
{
	// There are two operands for unary operations.
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0]; // [0] - destination
	const struct cl_operand &src = opList[1]; // [1] - source

	// Checks if left operand is valid.
	assert(dst.code == CL_OPERAND_VAR);

	// Checks if right operand is valid.
	assert(src.code == CL_OPERAND_CST);

	const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn->subCode);

	// Gets the destination memory place.
	const MemoryPlace *dstVar = OperandToMemoryPlace::convert(&dst);
	// Gets the source range.
	Range srcRange = Range(Utility::convertOperandToNumber(&src));
	// This is used only to set the type of the result range.
	// Necessary for correct behaviour of assign function from the Range class.
	Range resultRange = Utility::getMaxRange(dst);

	switch (code) {
		case CL_UNOP_ASSIGN:
			// Assignment must be solved extra because of the assignment of
			// the structure to another structure.
			if (dstVar->representsElementOfArray()) {
				// There is an array in this structure.
				Range result = unite(GlobAnalysis::globVarMap[dstVar], srcRange);
				resultRange = resultRange.assign(result);
			} else {
				// No array in this structure.
				resultRange = resultRange.assign(srcRange);
			}
			break;

		case CL_UNOP_TRUTH_NOT:
			// Truth negation.
			resultRange = resultRange.assign(logicalNot(srcRange));
			break;

		case CL_UNOP_BIT_NOT:
			// Bit negation.
			resultRange = resultRange.assign(bitNot(srcRange));
			break;

		case CL_UNOP_MINUS:
			// Unary minus.
			resultRange = resultRange.assign(-srcRange);
			break;

		case CL_UNOP_ABS:
			// Absolute value.
			resultRange = resultRange.assign(abs(srcRange));
			break;

		case CL_UNOP_FLOAT:
			// Casting to float.
			resultRange = resultRange.assign(intToFloat(srcRange));
			break;

		default:
			assert(false && "Invalid insn->subCode in computeAnalysisorUnop()!");
			break;
	}
	// Setting the new range for destination.
	GlobAnalysis::globVarMap[dstVar] = resultRange;
}

/**
* @brief Computes initial that is represented by binary instruction @a insn.
*/
void GlobAnalysis::processInitialForBinop(const Insn *insn)
{
	// There are three operands for binary operation.
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0];   // [0] - destination
	const struct cl_operand &src1 = opList[1];  // [1] - source 1
	const struct cl_operand &src2 = opList[2];  // [2] - source 2

	// Checks if destination operand is valid.
	assert(dst.code == CL_OPERAND_VAR);
	MemoryPlace *dstVar = OperandToMemoryPlace::convert(&dst);

	// Checks if source operands are valid.
	assert(src1.code == CL_OPERAND_CST);
	assert(src2.code == CL_OPERAND_CST);

	// Gets the source ranges.
	Range srcRange1 = Range(Utility::convertOperandToNumber(&src1));
	Range srcRange2 = Range(Utility::convertOperandToNumber(&src2));

	// This is used only to set the type of the result range.
	// Necessary for correct behaviour of assign function from the Range class.
	Range resultRange = Utility::getMaxRange(dst);

	const enum cl_binop_e code = static_cast<enum cl_binop_e>(insn->subCode);

	switch (code) {
		case CL_BINOP_EQ:
			// Equal.
			resultRange = resultRange.assign(logicalEq(srcRange1, srcRange2));
			break;

		case CL_BINOP_NE:
			// Unequal.
			resultRange = resultRange.assign(logicalNeq(srcRange1, srcRange2));
			break;

		case CL_BINOP_LT:
			// Less than.
			resultRange = resultRange.assign(logicalLt(srcRange1, srcRange2));
			break;

		case CL_BINOP_GT:
			// Greater than.
			resultRange = resultRange.assign(logicalGt(srcRange1, srcRange2));
			break;

		case CL_BINOP_LE:
			// Less or equal than.
			resultRange = resultRange.assign(logicalLtEq(srcRange1, srcRange2));
			break;

		case CL_BINOP_GE:
			// Greater or equal than
			resultRange = resultRange.assign(logicalGtEq(srcRange1, srcRange2));
			break;

		case CL_BINOP_TRUTH_AND:
			// Logical AND.
			resultRange = resultRange.assign(logicalAnd(srcRange1, srcRange2));
			break;

		case CL_BINOP_TRUTH_OR:
			// Logical OR.
			resultRange = resultRange.assign(logicalOr(srcRange1, srcRange2));
			break;

		case CL_BINOP_TRUTH_XOR:
			// Logical XOR.
			resultRange = resultRange.assign(logicalXor(srcRange1, srcRange2));
			break;

		case CL_BINOP_PLUS:
			// Addition.
			resultRange = resultRange.assign(srcRange1 + srcRange2);
			break;

		case CL_BINOP_MINUS:
			// Subtraction.
			resultRange = resultRange.assign(srcRange1 - srcRange2);
			break;

		case CL_BINOP_MULT:
			// Multiplication.
			resultRange = resultRange.assign(srcRange1 * srcRange2);
			break;

		case CL_BINOP_EXACT_DIV:
			// Exact division of integral numbers.
			resultRange = resultRange.assign(exact_div(srcRange1, srcRange2));
			break;

		case CL_BINOP_TRUNC_DIV:
			// Division of integral numbers with the remainder.
			resultRange = resultRange.assign(trunc_div(srcRange1, srcRange2));
			break;

		case CL_BINOP_TRUNC_MOD:
			// Modulo of integral numbers.
			resultRange = resultRange.assign(trunc_mod(srcRange1, srcRange2));
			break;

		case CL_BINOP_RDIV:
			// Division of floating-point numbers.
			resultRange = resultRange.assign(rdiv(srcRange1, srcRange2));
			break;

		case CL_BINOP_MIN:
			// Min value.
			resultRange = resultRange.assign(min(srcRange1, srcRange2));
			break;

		case CL_BINOP_MAX:
			// Max value.
			resultRange = resultRange.assign(max(srcRange1, srcRange2));
			break;

		case CL_BINOP_POINTER_PLUS:
			// Pointer addition.
			break;

		case CL_BINOP_BIT_AND:
			// Bit AND.
			resultRange = resultRange.assign(bitAnd(srcRange1, srcRange2));
			break;

		case CL_BINOP_BIT_IOR:
			// Bit OR.
			resultRange = resultRange.assign(bitOr(srcRange1, srcRange2));
			break;

		case CL_BINOP_BIT_XOR:
			// Bit XOR.
			resultRange = resultRange.assign(bitXor(srcRange1, srcRange2));
			break;

		case CL_BINOP_LSHIFT:
			// Left shift.
			resultRange = resultRange.assign(bitLeftShift(srcRange1, srcRange2));
			break;

		case CL_BINOP_RSHIFT:
			// Right shift.
			resultRange = resultRange.assign(bitRightShift(srcRange1, srcRange2));
			break;

		case CL_BINOP_LROTATE:
			// Left rotate.
			resultRange = resultRange.assign(bitLeftRotate(srcRange1, srcRange2));
			break;

		case CL_BINOP_RROTATE:
			// Right rotate.
			resultRange = resultRange.assign(bitRightRotate(srcRange1, srcRange2));
			break;

		default:
			assert(false && "Invalid insn->subCode in computeAnalysisForBinop()!");
			break;
	}
	// Setting the new range for destination.
	GlobAnalysis::globVarMap[dstVar] = resultRange;
}

/**
* @brief Processes one initial for global variable.
*/
void GlobAnalysis::processInitial(const Insn *insn)
{
	const enum cl_insn_e code = insn->code;

	switch (code) {
		case CL_INSN_UNOP:
			GlobAnalysis::processInitialForUnop(insn);
			break;

		case CL_INSN_BINOP:
			GlobAnalysis::processInitialForBinop(insn);
			break;

		default:
			// CL_INSN_NOP, CL_INSN_JMP, CL_INSN_COND, CL_INSN_ABORT, CL_INSN_SWITCH,
			// CL_INSN_LABEL, CL_INSN_RET, CL_INSN_CALL:
			assert(false && "Invalid insn->code");
			return;
	}
}

/**
* @brief Initializes the global variable map.
*/
void GlobAnalysis::initGlobVarMap(const Storage &stor)
{
	for (int uid : GlobAnalysis::getGlobVar()) {
		if (GlobAnalysis::isModified(uid)) {
			// The global variable can be modified. There is no need to set
			// anything. In every function, first using of this variable causes
			// its setting to maximum.
		} else {
			// The global variable cannot be modified. The values are set according
			// the initializers.
			const VarDb &vars = stor.vars;
			const Var &var = vars[uid];

			for (const Insn* insn : var.initials) {
				processInitial(insn);
			}
		}
	}
}

