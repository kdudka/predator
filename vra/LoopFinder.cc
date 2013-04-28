/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   LoopFinder.cc
* @brief  Implementation of the class that computes for every block the upper
*         limit that represents how many times loops will be entered.
* @date   2013
*/

#undef NDEBUG   // It is necessary for using assertions.

#include <iostream>
#include <stack>
#include <set>
#include <cassert>

#include <boost/foreach.hpp>
#include "LoopFinder.h"
#include "Utility.h"

using CodeStorage::Block;
using CodeStorage::Fnc;
using CodeStorage::TTargetList;
using CodeStorage::Insn;
using CodeStorage::TOperandList;

using std::ostream;
using std::endl;
using std::stack;
using std::set;
using std::vector;

const unsigned long LoopFinder::MaxTripCountOfTheLoopBeforeStop = 1000;

namespace {
	bool lt(const Number &n1, const Number &n2)
	{
		return n1 < n2;
	}

	bool gt(const Number &n1, const Number &n2)
	{
		return n1 > n2;
	}

	bool lte(const Number &n1, const Number &n2)
	{
		return n1 <= n2;
	}

	bool gte(const Number &n1, const Number &n2)
	{
		return n1 >= n2;
	}
}

LoopFinder::BlockToUpperLimit LoopFinder::blockToUpperLimit;

/**
* @brief Checks if instruction @a insn modifies variable @a variable in the way
*        we are able to used to compute loop trip count.
*/
bool LoopFinder::addUnopInsn(const Insn *insn, const cl_operand *variable,
	vector<const Insn*> &insns)
{
	// There are two operands for unary operations.
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0]; // [0] - destination
	const struct cl_operand &src = opList[1]; // [1] - source

	if (variable->data.var->uid != dst.data.var->uid) {
		// This instruction cannot change our variable.
		return true;
	}

	if (!((CL_OPERAND_CST == src.code) ||
		(variable->data.var->uid == src.data.var->uid)  )) {
		// Source operand can be only loop variable or constant. We have no
		// information about other variables.
		return false;
	}

	insns.push_back(insn);
	return true;
}

/**
* @brief Checks if instruction @a insn modifies variable @a variable in the way
*        we are able to used to compute loop trip count.
*/
bool LoopFinder::addBinopInsn(const Insn *insn, const cl_operand *variable,
	vector<const Insn*> &insns)
{
	// There are three operands for unary operations.
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0]; // [0] - destination
	const struct cl_operand &src1 = opList[1]; // [1] - source
	const struct cl_operand &src2 = opList[2]; // [2] - source

	if (variable->data.var->uid != dst.data.var->uid) {
		// This instruction cannot change our variable.
		return true;
	}

	if (!((variable->data.var->uid == src1.data.var->uid) ||
		 (CL_OPERAND_CST == src1.code))) {
		// First source operand can be a variable or a constant.
		return false;
	}

	if (CL_OPERAND_CST != src2.code) {
		// Second source operand can be only a constant.
		return false;
	}
	insns.push_back(insn);

	return true;
}

/**
* @brief We are not able to compute changes of loop variable @a variable if it
*        gets the value from calling function. So, we only check if this calling
*        instruction modifies our variable.
*/
bool LoopFinder::addCallInsn(const Insn *insn, const cl_operand *variable)
{
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0]; // [0] - destination
	return (variable != &dst);
}

/**
* @brief Finds all relevant instructions and checks if they are helpful, which means
*        that we can compute trip count of the loop that is representable by @a
*        block. Relevant instructions has as a destination @a variable.
*/
bool LoopFinder::findRelevantInsns(const Block *block, const cl_operand *variable,
	vector<const Insn*> &insns)
{
	BOOST_REVERSE_FOREACH(const Insn *insn, *block) {
		switch (insn->code) {
			case CL_INSN_NOP:
			case CL_INSN_JMP:
			case CL_INSN_COND:
			case CL_INSN_RET:
			case CL_INSN_ABORT:
			case CL_INSN_SWITCH:
			case CL_INSN_LABEL:
				// We are not interested.
				break;

			case CL_INSN_UNOP:
				if (!LoopFinder::addUnopInsn(insn, variable, insns)) {
					// If this instruction modifies our variable but in the way
					// we are not able to use.
					return false;
				}
				break;

			case CL_INSN_BINOP:
				if (!LoopFinder::addBinopInsn(insn, variable, insns)) {
					// If this instruction modifies our variable but in the way
					// we are not able to use.
					return false;
				}
				break;

			case CL_INSN_CALL:
				if (!LoopFinder::addCallInsn(insn, variable)) {
					// If this instruction modifies our variable but in the way
					// we are not able to use.
					return false;
				}
				break;

			default:
				assert(false && "Invalid insn->code in findRelevantInsns()!");
				return false;
		}
	}
	return true;
}

/**
* @brief Computes how unary instruction @a insn affects the loop variable that
*        starts on @a initialNum.
*/
void LoopFinder::processUnop(const Insn *insn, Number &initialNum)
{
	const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn->subCode);

	// Gets the necessary information about instruction.
	const TOperandList &opList = insn->operands;
	// const struct cl_operand &dst = opList[0];   // [0] - destination
	const struct cl_operand &src1 = opList[1];  // [1] - source 1

	Number srcNum(0, sizeof(int), true);

	if (CL_OPERAND_VAR == src1.code) {
		srcNum = initialNum;
	} else if (CL_OPERAND_CST == src1.code) {
		srcNum = Utility::convertOperandToNumber(&src1);
	}

	switch (code) {
		case CL_UNOP_ASSIGN:
			// Assignment must be solved extra because of the assignment of
			// the structure to another structure.
			initialNum = initialNum.assign(srcNum);
			return;

		case CL_UNOP_TRUTH_NOT:
			// Truth negation.
			initialNum = initialNum.assign(
				Number(logicalNot(srcNum), sizeof(int), true));
			break;

		case CL_UNOP_BIT_NOT:
			// Bit negation.
			initialNum = initialNum.assign(bitNot(srcNum));
			break;

		case CL_UNOP_MINUS:
			// Unary minus.
			initialNum = initialNum.assign(-srcNum);
			break;

		case CL_UNOP_ABS:
			// Absolute value.
			initialNum = initialNum.assign(abs(srcNum));
			break;

		case CL_UNOP_FLOAT:
			// Casting to float.
			initialNum = initialNum.assign(intToFloat(srcNum));
			break;

		default:
			assert(false && "Invalid insn->subCode!");
			break;
	}
}

/**
* @brief Computes how binary instruction @a insn affects the loop variable that
*        starts on @a initialNum.
*/
bool LoopFinder::processBinop(const Insn *insn, Number &initialNum)
{
	const enum cl_binop_e code = static_cast<enum cl_binop_e>(insn->subCode);

	// Gets the necessary information about instruction.
	const TOperandList &opList = insn->operands;
	// const struct cl_operand &dst = opList[0];   // [0] - destination
	const struct cl_operand &src1 = opList[1];  // [1] - source 1
	const struct cl_operand &src2 = opList[2];  // [2] - source 2

	Number src1Num(0, sizeof(int), true);
	Number src2Num(0, sizeof(int), true);

	if (CL_OPERAND_VAR == src1.code) {
		src1Num = initialNum;
	} else if (CL_OPERAND_CST == src1.code) {
		src1Num = Utility::convertOperandToNumber(&src1);
	}

	if (CL_OPERAND_VAR == src2.code) {
		src2Num = initialNum;
	} else if (CL_OPERAND_CST == src2.code) {
		src2Num = Utility::convertOperandToNumber(&src2);
	}

	switch (code) {
		case CL_BINOP_EQ:
			// Equal.
			initialNum = initialNum.assign(
				Number(src1Num == src2Num, sizeof(int), true));
			break;

		case CL_BINOP_NE:
			// Unequal.
			initialNum = initialNum.assign(
				Number(src1Num != src2Num, sizeof(int), true));
			break;

		case CL_BINOP_LT:
			// Less than.
			initialNum = initialNum.assign(
				Number(src1Num < src2Num, sizeof(int), true));
			break;

		case CL_BINOP_GT:
			// Greater than.
			initialNum = initialNum.assign(
				Number(src1Num > src2Num, sizeof(int), true));
			break;

		case CL_BINOP_LE:
			// Less or equal than.
			initialNum = initialNum.assign(
				Number(src1Num <= src2Num, sizeof(int), true));
			break;

		case CL_BINOP_GE:
			// Greater or equal than
			initialNum = initialNum.assign(
				Number(src1Num >= src2Num, sizeof(int), true));
			break;

		case CL_BINOP_TRUTH_AND:
			// Logical AND.
			initialNum = initialNum.assign(
				Number(logicalAnd(src1Num, src2Num), sizeof(int), true));
			break;

		case CL_BINOP_TRUTH_OR:
			// Logical OR.
			initialNum = initialNum.assign(
				Number(logicalOr(src1Num, src2Num), sizeof(int), true));
			break;

		case CL_BINOP_TRUTH_XOR:
			// Logical XOR.
			initialNum = initialNum.assign(
				Number(logicalXor(src1Num, src2Num), sizeof(int), true));
			break;

		case CL_BINOP_PLUS:
			// Addition.
			initialNum = initialNum.assign(src1Num + src2Num);
			break;

		case CL_BINOP_MINUS:
			// Subtraction.
			initialNum = initialNum.assign(src1Num - src2Num);
			break;

		case CL_BINOP_MULT:
			// Multiplication.
			initialNum = initialNum.assign(src1Num * src2Num);
			break;

		case CL_BINOP_EXACT_DIV:
			// Exact division of integral numbers.
			initialNum = initialNum.assign(exact_div(src1Num, src2Num));
			break;

		case CL_BINOP_TRUNC_DIV:
			// Division of integral numbers with the remainder.
			initialNum = initialNum.assign(trunc_div(src1Num, src2Num));
			break;

		case CL_BINOP_TRUNC_MOD:
			// Modulo of integral numbers.
			initialNum = initialNum.assign(trunc_mod(src1Num, src2Num));
			break;

		case CL_BINOP_RDIV:
			// Division of floating-point numbers.
			initialNum = initialNum.assign(rdiv(src1Num, src2Num));
			break;

		case CL_BINOP_MIN:
			// Min value.
			initialNum = initialNum.assign(min(src1Num, src2Num));
			break;

		case CL_BINOP_MAX:
			// Max value.
			initialNum = initialNum.assign(max(src1Num, src2Num));
			break;

		case CL_BINOP_POINTER_PLUS:
			// Pointer addition.
			break;

		case CL_BINOP_BIT_AND:
			// Bit AND.
			initialNum = initialNum.assign(bitAnd(src1Num, src2Num));
			break;

		case CL_BINOP_BIT_IOR:
			// Bit OR.
			initialNum = initialNum.assign(bitOr(src1Num, src2Num));
			break;

		case CL_BINOP_BIT_XOR:
			// Bit XOR.
			initialNum = initialNum.assign(bitXor(src1Num, src2Num));
			break;

		case CL_BINOP_LSHIFT:
			// Left shift.
			initialNum = initialNum.assign(bitLeftShift(src1Num, src2Num));
			break;

		case CL_BINOP_RSHIFT:
			// Right shift.
			initialNum = initialNum.assign(bitRightShift(src1Num, src2Num));
			break;

		case CL_BINOP_LROTATE:
			// Left rotate.
			return false;

		case CL_BINOP_RROTATE:
			// Right rotate.
			return false;

		default:
			assert(false && "Invalid insn->subCode in computeAnalysisForBinop()!");
			break;
	}
	return true;
}

/**
* @brief Computes loop trip count. The loop variable starts with the @a initial.
*        The comparison of type is stored in @a type and @a initial is compared
*        with @a constant. In the vector @a insns, instructions from the loop body
*        are stored.
*/
unsigned long LoopFinder::computeTripCountOfLoop(const cl_operand *variable,
	const cl_operand *constant, const cl_operand *initial, const enum cl_binop_e type,
	const vector<const Insn*> &insns)
{
	if (insns.size() == 0) {
		// There are no instructions changing loop variable. Probably infinite loop.
		return 0;
	}

	bool (*compare)(const Number &, const Number &);
	// Set the type according the type of loop variable.
	Number initialNum = Utility::convertOperandToNumber(variable);
	initialNum = initialNum.assign(Utility::convertOperandToNumber(initial));
	Number constantNum = Utility::convertOperandToNumber(constant);

	switch (type) {
		case CL_BINOP_LT:
			compare = lt;
			break;

		case CL_BINOP_GT:
			compare = gt;
			break;
		case CL_BINOP_LE:

			compare = lte;
			break;

		case CL_BINOP_GE:
			compare = gte;
			break;

		default:
			compare = NULL;
			assert(false);
			break;
	}

	// Maximal number of loop's execution.
	unsigned long tripCount = 0;

	while (compare(initialNum, constantNum)) {
		// We can afford compute this because it is really ambitious.
		if (tripCount > LoopFinder::MaxTripCountOfTheLoopBeforeStop) {
			return 0;
		}

		BOOST_FOREACH(const Insn* insn, insns) {
			const enum cl_insn_e code = insn->code;

			// If there is not change between previous computation an new computation
			// we have to stop.
			Number prevNum = initialNum;

			switch (code) {
				case CL_INSN_UNOP:
					LoopFinder::processUnop(insn, initialNum);
					break;

				case CL_INSN_BINOP:
					if (!LoopFinder::processBinop(insn, initialNum)) {
						// The operation was bit rotate. We are not able to handle it.
						return 0;
					}
					break;

				default:
					break;
			}

			if (prevNum == initialNum) {
				// We are not able to compute trip count. Infinite loop.
				return 0;
			}
		}
		++tripCount;
	}

	return tripCount;
}

/**
* @brief Find initial value in the given block @a block for the variable @a variable
*        and result stores in @a initial.
*
* @param[in] block Block in which we search for the initial value.
* @param[in] variable Variable for which we try to find initial value.
* @param[in] initial Found initial value.
*
* @return If initial value was found, @c true is returned. Otherwise, @c false is
*         returned.
*/
bool LoopFinder::findInitialValueForCondVar(const Block *block,
	const cl_operand* variable, const cl_operand **initial)
{
	BOOST_REVERSE_FOREACH(const Insn *insn, *block) {
		if (CL_INSN_UNOP == insn->code) {
			// We need an unary operation.
			const enum cl_unop_e subCode = static_cast<enum cl_unop_e>(insn->subCode);
			if (CL_UNOP_ASSIGN == subCode) {
				// We need an assignment.
				const TOperandList &opList = insn->operands;
				const struct cl_operand &dst = opList[0]; // [0] - destination
				const struct cl_operand &src = opList[1]; // [1] - source
				if (dst.data.var->uid == variable->data.var->uid) {
					if (CL_OPERAND_CST == src.code) {
						// Constant assigned to the loop variable.
						*initial = &src;
						return true;
					}
					else {
						// Variable assigned to the loop variable.
						return false;
					}
				}
			}
		}
	}
	return false;
}

/**
* @brief Processes the instruction that precede condition. It should be the
*        computation for condition.
*
* @param[in] insn Condition instruction.
* @param[in] prevInsn Previous instruction to the condition.
* @param[out] variable Left operand from the comparison will be stored here.
* @param[out] constant Right operand from the comparison will be stored here.
* @param[out] type Type of the comparison will be stored here.
*
* @return If we were able to process the instruction @a prevInsn, then @c true
*         is returned. Otherwise, @c false is returned.
*/
bool LoopFinder::processPreviousInsn(const Insn *insn, const Insn *prevInsn,
	const cl_operand **variable, const cl_operand **constant, enum cl_binop_e &type)
{
	if (prevInsn == NULL) {
		// There is not previous instruction.
		return false;
	}

	// Gets the code of previous instruction.
	const enum cl_insn_e prevCode = prevInsn->code;
	if (prevCode != CL_INSN_BINOP) {
		// The previous instruction must be binary operation.
		return false;
	}

	// Gets the sub-code of the previous instruction.
	type = static_cast<enum cl_binop_e>(prevInsn->subCode);
	if ((type != CL_BINOP_LT) && (type != CL_BINOP_GT) &&
		(type != CL_BINOP_LE) && (type != CL_BINOP_GE)) {
		// The previous instruction must comparison: <, >, <=, >=.
		return false;
	}

	// Gets the necessary information about previous instruction.
	const TOperandList &opListPrev = prevInsn->operands;
	const struct cl_operand &dst = opListPrev[0];   // [0] - destination
	const struct cl_operand &src1 = opListPrev[1];  // [1] - source 1
	const struct cl_operand &src2 = opListPrev[2];  // [2] - source 2

	if (src1.code != CL_OPERAND_VAR) {
		// We need to have variable on the left side.
		return false;
	}

	if (src2.code != CL_OPERAND_CST) {
		// We need to have constant on the right side.
		return false;
	}

	const TOperandList &opListCond = insn->operands;
	const struct cl_operand &cond = opListCond[0];  // [0] - condition

	if (cond.data.var->uid != dst.data.var->uid) {
		// Since insn represents condition, we need to find if the condition variable
		// is identical with destination variable from the previous instruction.
		return false;
	}

	*variable = &src1;
	*constant = &src2;

	return true;
}

/**
* @brief Finds the condition in the given block @a block. We are able to process
*        condition that have the following shape: (variable [<,>,<=,>=] constant).
*        Variable on the left side must be user variable, not artificial one.
*
* @param[in] block Block in which we will search for the condition.
* @param[out] variable Left operand from the comparison will be stored here.
* @param[out] constant Right operand from the comparison will be stored here.
* @param[out] type Type of the comparison will be stored here.
*
* @return If condition is found, it returns @c true and appropriate variables are set.
*         Otherwise, @c false is returned.
*/
bool LoopFinder::checkCondition(const Block *block, const cl_operand **variable,
	const cl_operand **constant, enum cl_binop_e &type)
{
	const Insn *prevInsn = NULL;
	BOOST_FOREACH(const Insn *insn, *block) {
		if (CL_INSN_COND == insn->code) {
			// We are not able to process previous instruction.
			if (!LoopFinder::processPreviousInsn(insn, prevInsn, variable,
				constant, type)) {
				return false;
			}

			// We need to have non-artificial variable on the left side.
			if ((*variable)->data.var->artificial) {
				return false;
			} else {
				return true;
			}
		}
		prevInsn = insn;
	}

	return false;
}

/**
* @brief Sets the given value @a value for the given block @a block.
*/
void LoopFinder::setValueForBlock(const Block *block, unsigned long value)
{
	LoopFinder::blockToUpperLimit[block] = value;
}

/**
* @brief Computes the maximal number of loop's traverse, where condition for loop
*        is stored in @a block. If this condition is evaluated as @c true, then
*        the next block will be @a thenBlock.
*/
void LoopFinder::computeLoopAnalysisForPatternWhile(const Block *block,
	const Block *thenBlock)
{
	LoopFinder::setValueForBlock(block, 0);

	// Finds the previous block. We need to find that predecessor that is not
	// block's successor.
	const TTargetList &preds = block->inbound();
	const Block *prevBlock;
	if (preds[0] != thenBlock) {
		prevBlock = preds[0];
	} else {
		prevBlock = preds[1];
	}

	// Checks if we can handle the condition in the given block. If can handle it,
	// fill the appropriate variables.
	const cl_operand* variable = NULL;
	const cl_operand* constant = NULL;
	enum cl_binop_e type;
	if (!LoopFinder::checkCondition(block, &variable, &constant, type)) {
		LoopFinder::setValueForBlock(thenBlock, 0);
		return;
	}

	// Finds the initial value for the variable.
	const cl_operand* initial = NULL;
	if (!LoopFinder::findInitialValueForCondVar(prevBlock, variable, &initial)) {
		LoopFinder::setValueForBlock(thenBlock, 0);
		return;
	}

	// Finds if the instructions from thenBlock working with loop variable are
	// computable for us. If they are, it creates an vector of relevant instructions.
	vector<const Insn*> insns;
	if (!LoopFinder::findRelevantInsns(thenBlock, variable, insns)) {
		LoopFinder::setValueForBlock(thenBlock, 0);
		return;
	}

	// Computes how many times loop will be executed.
	unsigned long value = LoopFinder::computeTripCountOfLoop(
		variable, constant, initial, type, insns);
	LoopFinder::setValueForBlock(thenBlock, value);
}

/**
* @brief Computes the maximal number of loop's traverse, where condition for loop
*        is stored in @a block. If this condition is evaluated as @c true, then
*        the next block will be @a ifBlock.
*/
void LoopFinder::computeLoopAnalysisForPatternWhileIf(const Block *block,
	const Block *ifBlock)
{
	LoopFinder::setValueForBlock(block, 0);

	const TTargetList &succsIf = ifBlock->targets();
	const Block *thenBlock = succsIf[0];
	const Block *elseBlock = succsIf[1];

	// Finds the previous block. We need to find that predecessor that is not
	// block's successor. It is the predecessor of the given block.
	const TTargetList &preds = block->inbound();
	const Block *prevBlock;
	if (preds[0] != elseBlock) {
		prevBlock = preds[0];
	} else {
		prevBlock = preds[1];
	}

	const cl_operand* variable = NULL;
	const cl_operand* constant = NULL;
	enum cl_binop_e type;
	const cl_operand* initial = NULL;
	vector<const Insn*> insns;
	if (!LoopFinder::checkCondition(block, &variable, &constant, type) ||
		// Checks if we can handle the condition in the given block.
		!LoopFinder::findInitialValueForCondVar(prevBlock, variable, &initial) ||
		// Finds the initial value for the variable.
		LoopFinder::blockModifiesVar(thenBlock, variable) ||
		// Checks if there is instruction modifying condition variable.
		!LoopFinder::findRelevantInsns(ifBlock, variable, insns) ||
		!LoopFinder::findRelevantInsns(elseBlock, variable, insns)
		// Finds if instructions working with loop variable are computable for us
		) {
		LoopFinder::setValueForBlock(ifBlock, 0);
		LoopFinder::setValueForBlock(thenBlock, 0);
		LoopFinder::setValueForBlock(elseBlock, 0);
		return;
	}

	// Computes how many times loop will be executed.
	unsigned long value = LoopFinder::computeTripCountOfLoop(
		variable, constant, initial, type, insns);
	LoopFinder::setValueForBlock(ifBlock, value);
	LoopFinder::setValueForBlock(thenBlock, value);
	LoopFinder::setValueForBlock(elseBlock, value);
}

/**
* @brief Computes the maximal number of loop's traverse, where condition for loop
*        is stored in @a block. If this condition is evaluated as @c true, then
*        the next block will be @a ifBlock.
*/
void LoopFinder::computeLoopAnalysisForPatternWhileIfElse(const Block *block,
	const Block *ifBlock)
{
	LoopFinder::setValueForBlock(block, 0);

	const TTargetList &succsIf = ifBlock->targets();
	const Block *thenBlock = succsIf[0];
	const Block *elseBlock = succsIf[1];
	const Block *gotoBlock = (thenBlock->targets())[0];

	// Finds the previous block. We need to find that predecessor that is not
	// block's successor. It is the predecessor of the given block.
	const TTargetList &preds = block->inbound();
	const Block *prevBlock;
	if (preds[0] != gotoBlock) {
		prevBlock = preds[0];
	} else {
		prevBlock = preds[1];
	}

	const cl_operand* variable = NULL;
	const cl_operand* constant = NULL;
	enum cl_binop_e type;
	const cl_operand* initial = NULL;
	vector<const Insn*> insns;
	if (!LoopFinder::checkCondition(block, &variable, &constant, type) ||
		// Checks if we can handle the condition in the given block.
		!LoopFinder::findInitialValueForCondVar(prevBlock, variable, &initial) ||
		// Finds the initial value for the variable.
		LoopFinder::blockModifiesVar(thenBlock, variable) ||
		LoopFinder::blockModifiesVar(elseBlock, variable) ||
		// Checks if there is instruction modifying condition variable.
		!LoopFinder::findRelevantInsns(ifBlock, variable, insns) ||
		!LoopFinder::findRelevantInsns(gotoBlock, variable, insns)
		// Finds if instructions working with loop variable are computable for us
		) {
		LoopFinder::setValueForBlock(ifBlock, 0);
		LoopFinder::setValueForBlock(thenBlock, 0);
		LoopFinder::setValueForBlock(elseBlock, 0);
		LoopFinder::setValueForBlock(gotoBlock, 0);
		return;
	}

	// Computes how many times loop will be executed.
	unsigned long value = LoopFinder::computeTripCountOfLoop(
		variable, constant, initial, type, insns);
	LoopFinder::setValueForBlock(ifBlock, value);
	LoopFinder::setValueForBlock(thenBlock, value);
	LoopFinder::setValueForBlock(elseBlock, value);
	LoopFinder::setValueForBlock(gotoBlock, value);
}

/**
* @brief Returns @c true if block @a block modifies variable @a variable. Otherwise,
*        it return @c false.
*/
bool LoopFinder::blockModifiesVar(const Block *block, const cl_operand *variable)
{
	BOOST_FOREACH(const Insn *insn, *block) {
		// Checks if this instruction can modify variable.
		enum cl_insn_e code = insn->code;
		if (CL_INSN_UNOP == code || CL_INSN_BINOP == code || CL_INSN_CALL == code) {
			// Only these instructions use destination operand.
			const TOperandList &opList = insn->operands;
			const struct cl_operand &dst = opList[0];   // [0] - destination
			assert(dst.data.var != NULL);
			if (variable->data.var->uid == dst.data.var->uid) {
				// This instruction changes our variable.
				return true;
			}
		}
	}
	return false; // Variable cannot be modified in this block.
}

/**
* @brief Returns @c true if the given @a block contains the condition that
*        represents the entry point to the simple "while" loop. Otherwise,
*        it return @c false.
*/
bool LoopFinder::isPatternWhile(const Block *block, const Block *thenBlock)
{
	// Gets the predecessors and successors of the processed block.
	const TTargetList &preds = block->inbound();
	const TTargetList &succs = block->targets();

	if ((preds.size() != 2) || (succs.size() != 2)) {
		// The processed block must have two predecessors and two successors for
		// this loop pattern.
		return false;
	}

	if ((thenBlock != preds[0]) && (thenBlock != preds[1])) {
		// Its successor from the if-then branch must be also its predecessor.
		return false;
	}

	// Gets the predecessors and successors of the if-then block.
	const TTargetList &thenPreds = thenBlock->inbound();
	const TTargetList &thenSuccs = thenBlock->targets();

	if ((thenPreds.size() != 1) || (thenSuccs.size() != 1)) {
		// The if-then successor of the currently processed block must have only one
		// predecessor and one successor.
		return false;
	}

	return true;
}

/**
* @brief Returns @c true if the given block @a block represents a block with
*        the condition for simple @c if statement. The last block of the
*        @c if statement will be stored in @a gotoBlock.
*/
bool LoopFinder::isPatternIf(const Block *block, const Block **gotoBlock)
{
	const Block *thenBlock = NULL;
	const Block *elseBlock = NULL;

	BOOST_FOREACH(const Insn *insn, *block) {
		if (CL_INSN_COND == insn->code) {
			// This instruction represents condition. Gets the targets.
			const TTargetList &targets = insn->targets;
			thenBlock = targets[0];
			elseBlock = targets[1];

			const TTargetList &preds = block->inbound();
			const TTargetList &succs = block->targets();
			if ((preds.size() != 1) || (succs.size() != 2)) {
				// The processed block must have one predecessor and two successors
				// for this loop pattern.
				return false;
			}

			const TTargetList &thenPreds = thenBlock->inbound();
			const TTargetList &thenSuccs = thenBlock->targets();
			if ((thenPreds.size() != 1) || (thenSuccs.size() != 1)) {
				// The processed block must have one predecessor and one successor
				// for this loop pattern.
				return false;
			}

			if (thenSuccs[0] != elseBlock) {
				return false;
			}

			const TTargetList &elsePreds = elseBlock->inbound();
			const TTargetList &elseSuccs = elseBlock->targets();
			if ((elsePreds.size() != 2) || (elseSuccs.size() != 1)) {
				// The processed block must have one predecessor and one successor
				// for this loop pattern.
				return false;
			}

			*gotoBlock = elseBlock;
			return true;
		}
	}

	return false;
}

/**
* @brief Returns @c true if the given block @a block represents a block with
*        the condition for simple @c if-else statement. The last block of the
*        @c if-else statement will be stored in @a gotoBlock.
*/
bool LoopFinder::isPatternIfElse(const Block *block, const Block **gotoBlock)
{
	const Block *thenBlock = NULL;
	const Block *elseBlock = NULL;

	BOOST_FOREACH(const Insn *insn, *block) {
		if (CL_INSN_COND == insn->code) {
			// This instruction represents condition. Gets the targets.
			const TTargetList &targets = insn->targets;
			thenBlock = targets[0];
			elseBlock = targets[1];

			const TTargetList &preds = block->inbound();
			const TTargetList &succs = block->targets();
			if ((preds.size() != 1) || (succs.size() != 2)) {
				// The processed block must have one predecessor and two successors
				// for this loop pattern.
				return false;
			}

			const TTargetList &thenPreds = thenBlock->inbound();
			const TTargetList &thenSuccs = thenBlock->targets();
			if ((thenPreds.size() != 1) || (thenSuccs.size() != 1)) {
				// The processed block must have one predecessor and one successor
				// for this loop pattern.
				return false;
			}

			const TTargetList &elsePreds = elseBlock->inbound();
			const TTargetList &elseSuccs = elseBlock->targets();
			if ((elsePreds.size() != 1) || (elseSuccs.size() != 1)) {
				// The processed block must have one predecessor and one successor
				// for this loop pattern.
				return false;
			}

			if (thenSuccs[0] != elseSuccs[0]) {
				// There is one successor for both blocks and it must be the same.
				return false;
			}

			*gotoBlock = thenSuccs[0];

			const TTargetList &gotoPreds = (*gotoBlock)->inbound();
			const TTargetList &gotoSuccs = (*gotoBlock)->targets();
			if ((gotoPreds.size() != 2) || (gotoSuccs.size() != 1)) {
				return false;
			}

			if (gotoSuccs[0] != preds[0]) {
				return false;
			}

			return true;
		}
	}

	return false;
}

/**
* @brief Returns @c true if the given @a block contains the condition that
*        represents the entry point to the simple "while" loop with if statement.
*        Otherwise, it return @c false.
*/
bool LoopFinder::isPatternWhileIf(const Block *block, const Block *thenBlock)
{
	// Gets the predecessors and successors of the processed block.
	const TTargetList &preds = block->inbound();
	const TTargetList &succs = block->targets();

	if ((preds.size() != 2) || (succs.size() != 2)) {
		// The processed block must have two predecessors and two successors for
		// this loop pattern.
		return false;
	}

	const Block *gotoBlock;
	if (!LoopFinder::isPatternIf(thenBlock, &gotoBlock)) {
		return false;
	}

	if ((gotoBlock != preds[0]) && (gotoBlock != preds[1])) {
		// Its successor from the if-then branch must be also its predecessor.
		return false;
	}

	return true;
}

/**
* @brief Returns @c true if the given @a block contains the condition that
*        represents the entry point to the simple "while" loop with if-else statement.
*        Otherwise, it return @c false.
*/
bool LoopFinder::isPatternWhileIfElse(const Block *block, const Block *thenBlock)
{
	// Gets the predecessors and successors of the processed block.
	const TTargetList &preds = block->inbound();
	const TTargetList &succs = block->targets();

	if ((preds.size() != 2) || (succs.size() != 2)) {
		// The processed block must have two predecessors and two successors for
		// this loop pattern.
		return false;
	}

	const Block *gotoBlock;
	if (!LoopFinder::isPatternIfElse(thenBlock, &gotoBlock)) {
		return false;
	}

	if ((gotoBlock != preds[0]) && (gotoBlock != preds[1])) {
		// Its successor from the if-then branch must be also its predecessor.
		return false;
	}
	return true;
}

/**
* @brief If the given @a block contains condition instruction, then this function
*        return the pointer to the block that will be executed after this condition
*        is evaluated as @c true. Otherwise, it returns @c NULL.
*/
const Block* LoopFinder::getThenBlock(const Block *block)
{
	BOOST_FOREACH(const Insn *insn, *block) {
		if (CL_INSN_COND == insn->code) {
			// This instruction represents condition. Gets the targets.
			const TTargetList &targets = insn->targets;
			const Block *thenBlock = targets[0];
			return thenBlock;
		}
	}
	return NULL;
}

/**
* @brief Computes the loops' analysis for the given block @a block. It checks mostly
*        used loops' patterns and compute the loop analysis for them. For other
*        loops set the upper limit to zero which means that we do not know and
*        this heuristic could not be used.
*/
void LoopFinder::computeLoopAnalysisForBlock(const Block *block)
{
	const Block *thenBlock = getThenBlock(block);
	if (thenBlock == NULL) {
		// If this block does not contain condition, we are not interested in
		// processing this block.
		if (LoopFinder::blockToUpperLimit.find(block) ==
			LoopFinder::blockToUpperLimit.end()) {
			LoopFinder::blockToUpperLimit[block] = 0;
		}
		return;
	}

	if (LoopFinder::isPatternWhile(block, thenBlock)) {
		LoopFinder::computeLoopAnalysisForPatternWhile(block, thenBlock);
	}

	if (LoopFinder::isPatternWhileIf(block, thenBlock)) {
		LoopFinder::computeLoopAnalysisForPatternWhileIf(block, thenBlock);
	}

	if (LoopFinder::isPatternWhileIfElse(block, thenBlock)) {
		LoopFinder::computeLoopAnalysisForPatternWhileIfElse(block, thenBlock);
	}
}

/**
* @brief Computes the loops' analysis for the given function @a fnc.
*/
void LoopFinder::computeLoopAnalysisForFnc(const Fnc &fnc)
{
	// Schedulers.
	stack<const Block *> todoStack;
	set<const Block *> doneSet;

	const Block *entryBlock = fnc.cfg.entry();
	todoStack.push(entryBlock);

	while (!todoStack.empty()) {
		const Block *block = todoStack.top();
		todoStack.pop();
		LoopFinder::computeLoopAnalysisForBlock(block);
		doneSet.insert(block);

		// Gets the successors of the processed block.
		const TTargetList &succs = block->targets();
		BOOST_FOREACH(const TTargetList::value_type &succ, succs) {
			if (doneSet.find(succ) == doneSet.end()) {
				// We schedule successors of this block that were not processed
				// before.
				todoStack.push(succ);
			}
		}
	}
}

/**
* @brief Computes the loops' upper limits for the given representation of analysed
*        program stored in @a stor.
*/
void LoopFinder::computeLoopAnalysis(const CodeStorage::Storage &stor)
{
	BOOST_FOREACH(const Fnc* pFnc, stor.fncs) {
		const Fnc &fnc = *pFnc;

		if (!isDefined(fnc))
			continue;

		LoopFinder::computeLoopAnalysisForFnc(fnc);
	}
}

/**
* @brief Returns upper limit for the given @a block.
*/
unsigned long LoopFinder::getUpperLimit(const Block *block)
{
	return LoopFinder::blockToUpperLimit[block];
}

/**
* @brief Emits the computed upper limits for each block into @a os.
*/
ostream& LoopFinder::printLoopAnalysis(std::ostream &os)
{
	BOOST_FOREACH(const BlockToUpperLimit::value_type &b,
		LoopFinder::blockToUpperLimit) {

		// Prints information for each block.
		os << b.first->name() << ": " << b.second
		   << std::endl;
	}
	return os;
}

