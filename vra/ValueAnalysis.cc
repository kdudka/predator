/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   ValueAnalysis.cc
* @brief  Implementation of the class that performs value-range analysis
*         and stores the result.
* @date   2012
*/

#undef NDEBUG   // It is necessary for using assertions.

#include <boost/foreach.hpp>
#include <iostream>
#include <cassert>
#include <iterator>
#include <algorithm>

#include "Utility.h"
#include "ValueAnalysis.h"
#include "OperandToMemoryPlace.h"
#include "GlobAnalysis.h"

using CodeStorage::Fnc;
using CodeStorage::Storage;
using CodeStorage::ControlFlow;
using CodeStorage::Block;
using CodeStorage::Insn;
using CodeStorage::TTargetList;
using CodeStorage::TOperandList;
using std::string;
using std::ostream;
using std::endl;
using std::vector;
using std::cout;
using std::deque;
using std::sort;
using std::pair;

ValueAnalysis::BlockToTrimmedRangesMap ValueAnalysis::blockToTrimmedRangesMap;
ValueAnalysis::BlockToResultMap ValueAnalysis::blockToInputRangesMap;
ValueAnalysis::BlockToResultMap ValueAnalysis::blockToOutputRangesMap;
ValueAnalysis::SchedulerQueue ValueAnalysis::todoQueue;
ValueAnalysis::SchedulerSet ValueAnalysis::todoSet;
ValueAnalysis::BlockToCounterMap ValueAnalysis::blockToCounterMap;
LoopFinder::BlockToUpperLimit ValueAnalysis::tripCountOfBlockMap;

const unsigned ValueAnalysis::NumberOfPassesBeforeExpand = 1000;

namespace {

/**
* @brief Sorts memory places information alphabetically according to the names of
*        variables stored in memory places.
*/
bool sortBlockInfo(const ValueAnalysis::MemoryPlaceRangePair &f,
				   const ValueAnalysis::MemoryPlaceRangePair &s)
{
	return f.first->asString() < s.first->asString();
}

}

/**
* @brief Schedules the block @a block. If @a block was scheduled earlier, it does
*        nothing. Otherwise, it inserts @a block into schedulers.
*/
void ValueAnalysis::scheduleBlock(const Block *block) {
	if (todoSet.find(block) == todoSet.end()) {
		// If block was not scheduled, we schedule it.
		todoQueue.push(block);
		todoSet.insert(block);
	}
}

/**
* @brief Gets the range for the given operand @a src. If @a src represents constant,
*        new range is created from this constant. If @a src represents variable and
*        is stored in @a output, the corresponding range from @a output is returned.
*        Otherwise, maximal possible range is returned.
*/
Range ValueAnalysis::getRange(const struct cl_operand &src,
							  MemoryPlaceToRangeMap &output,
							  deque<int> indexes)
{
	Range srcRange;

	if (src.code == CL_OPERAND_CST) {
		// Right operand of the unary operation is a constant.
		Number num = Utility::convertOperandToNumber(&src);
		srcRange = Range(num);
	} else if (src.code == CL_OPERAND_VAR) {
		// Right operand of the unary operation is a variable.
		MemoryPlace *srcVar = OperandToMemoryPlace::convert(&src, indexes);
		if (output.find(srcVar) != output.end()) {
			srcRange = output[srcVar];
		} else {
			// If we do not know what is in the variable, we set the maximal
			// possible range. This is used also for the assignment of structure
			// to another structure.
			srcRange = Utility::getMaxRange(src, indexes);
			output[srcVar] = srcRange;
		}
	}

	return srcRange;
}

/**
* @brief Assigns simple element @a src into the simple element @a dst. Simple
*        element can be a variable, item in a structure or element of an array.
*
* @param[in] dst Destination operand.
* @param[in] src Source operand.
* @param[out] output Stores the result.
* @param[in] indSrc Indexes for source operand.
* @param[in] indDst Indexes for destination operand.
*/
void ValueAnalysis::assignSimpleElement(const struct cl_operand &dst,
										const struct cl_operand &src,
										MemoryPlaceToRangeMap &output,
										deque<int> indDst,
										deque<int> indSrc)
{
	// Preparing source operand.
	Range srcRange = ValueAnalysis::getRange(src, output, indSrc);

	// Preparing destination operand.
	MemoryPlace *dstVar = OperandToMemoryPlace::convert(&dst, indDst);
	Range dstRange = Utility::getMaxRange(dst, indDst);


	if (dstVar->representsElementOfArray()) {
		// There is an array in this structure.
		Range result = unite(output[dstVar], srcRange);
		dstRange = dstRange.assign(result);
		output[dstVar] = dstRange;
	} else {
		// No array in this structure.
		dstRange = dstRange.assign(srcRange);
		output[dstVar] = dstRange;
	}
}

/**
* @brief Generates indexes for the given type @a type. The indexes for one
*        access are stored in @a ind. Vector @a indVec stores indexes for all
*        accesses through the @a type.
*/
void ValueAnalysis::generateIndexes(const struct cl_type *type,
									deque<int> &ind,
									vector<deque<int> > &indVec)
{
	if (type->item_cnt == 0) {
		indVec.push_back(ind);
	} else {
		// We have to go further, there is at least one more nesting in the
		// structure.
		for (int i = 0; i != type->item_cnt; ++i) {
			ind.push_back(i);
			const struct cl_type *newType;
			newType = type->items[i].type;
			generateIndexes(newType, ind, indVec);
		}
	}

	ind.pop_back();
}

/**
* @brief Gets the type of the operand @a op.
*/
const struct cl_type *ValueAnalysis::getType(const struct cl_operand &op)
{
	if (op.accessor != NULL) {
		return op.accessor->type;
	} else {
		return op.type;
	}
}

/**
* @brief Assignment of @a src to @a dst. It is able to perform assignment of simple
*        operands and structures.
*
* @param[in] dst Destination operand.
* @param[in] src Source operand.
* @param[in,out] output Holds ranges for all memory places for currently processed
*                       block. It used for gaining the range of the source operand
*                       and also for storing new range of the destination operand.
*/
void ValueAnalysis::assign(const struct cl_operand &dst,
						   const struct cl_operand &src,
						   MemoryPlaceToRangeMap &output)
{
	// Checks if left operand is valid.
	assert(dst.code == CL_OPERAND_VAR);

	// Checks if right operand is valid.
	assert(src.code == CL_OPERAND_CST || src.code == CL_OPERAND_VAR);

	if ((CL_TYPE_STRUCT == src.type->code) && (CL_TYPE_STRUCT == dst.type->code)) {
		// Assignment of structure to another structure.
		deque<int> indSrc;
		deque<int> indDst;
		vector<deque<int> > indSrcVec;
		vector<deque<int> > indDstVec;
		const struct cl_type *typeSrc = getType(src);
		const struct cl_type *typeDst = getType(dst);

		// Generates all possible nesting for the source operand.
		generateIndexes(typeSrc, indSrc, indSrcVec);
		generateIndexes(typeDst, indDst, indDstVec);
		assert(indSrcVec.size() == indDstVec.size());

		for (size_t i = 0; i != indSrcVec.size(); ++i) {
			assignSimpleElement(dst, src, output, indDstVec[i], indSrcVec[i]);
		}
	} else {
		// Assignment of simple variable.
		assignSimpleElement(dst, src, output);
	}
}

/**
* @brief If the given @a block is found as a key in the given @a inputMap
*        then it returns the ranges associated with this key in @a inputMap.
*        Otherwise, it returns an empty map.
*/
ValueAnalysis::MemoryPlaceToRangeMap ValueAnalysis::getRanges(const Block* block,
													const BlockToResultMap &inputMap)
{
	BlockToResultMap::const_iterator it = inputMap.find(block);
	if (it != inputMap.end()) {
		return it->second;
	} else {
		MemoryPlaceToRangeMap emptyMap;
		return emptyMap;
	}
}

/**
* @brief If the given @a block is found as a key in @c blockToTrimmedRangesMap
*        then it returns the output ranges that get off the given @a block.
*        Otherwise, it returns an empty map.
*/
ValueAnalysis::TrimmedRangesMap ValueAnalysis::getTrimmedRanges(const Block* block)
{
	if (blockToTrimmedRangesMap.find(block) != blockToTrimmedRangesMap.end()) {
		return blockToTrimmedRangesMap[block];
	} else {
		TrimmedRangesMap emptyMap;
		return emptyMap;
	}
}

/**
* @brief Computes the partial input ranges for the given block @a current from
*        the given output ranges of its predecessor stored in @a outs and from
*        the given trimmed ranges of its predecessor stored in @a trimmed.
*/
ValueAnalysis::MemoryPlaceToRangeMap ValueAnalysis::computePartialInputRanges(
	const CodeStorage::Block *current, const MemoryPlaceToRangeMap &outs,
	const TrimmedRangesMap &trimmed)
{
	MemoryPlaceToRangeMap result;

	BOOST_FOREACH(const TrimmedRangesMap::value_type &trim, trimmed) {
		// Firstly, we choose all trimmed ranges that are valid for the given block
		// and store them as result.
		const struct TrimmedKey key = trim.first;
		const Range &range = trim.second;
		if (key.block == current) {
			// If the trimmed range was computed for the current block, we set
			// the variable for which the trimmed range was computed and store
			// this trimmed range into the result map.
			MemoryPlaceToRangeMap::const_iterator it = outs.find(key.varMp);
			if (it != outs.end() && !(intersect(range, it->second)).empty()) {
				result[key.varMp] = range;
			}
		}
	}

	BOOST_FOREACH(const MemoryPlaceToRangeMap::value_type &out, outs) {
		// Secondly, we choose the output ranges for variables that do not have
		// trimmed ranges set.
		const MemoryPlace *mp = out.first;
		const Range &range = out.second;
		if (result.find(mp) == result.end()) {
			// If there was no trimming for this memory place, we just add it.
			result[mp] = range;
		}
	}

	return result;
}

/**
* @brief Computes the input ranges of the @a current block from the output ranges and
*        trimmed ranges of predecessors' ranges. Trimmed ranges represents the ranges
*        that are trimmed according to some condition in the block.
*/
void ValueAnalysis::computeInputRanges(const CodeStorage::Block *current)
{
	// Gets the predecessor of the current block.
	const TTargetList &preds = current->inbound();

	// Stores the input ranges of the current block, output ranges or trimmed ranges
	// of its predecessors.
	MemoryPlaceToRangeMapVector outputOfPreds;
	outputOfPreds.push_back(ValueAnalysis::getRanges(current, blockToInputRangesMap));

	BOOST_FOREACH(const TTargetList::value_type &pred, preds) {
		// Get the output ranges of the predecessor.
		MemoryPlaceToRangeMap out = ValueAnalysis::getRanges(pred,
										blockToOutputRangesMap);

		if (blockToTrimmedRangesMap.find(pred) == blockToTrimmedRangesMap.end()) {
			// If there are no trimmed ranges, we store the output ranges of
			// predecessor.
			outputOfPreds.push_back(out);
		} else {
			// If there are trimmed ranges, we have to compute the result from
			// predecessor's output and trimmed ranges.
			TrimmedRangesMap trimmed = ValueAnalysis::getTrimmedRanges(pred);
			MemoryPlaceToRangeMap result
					= ValueAnalysis::computePartialInputRanges(current, out, trimmed);
			outputOfPreds.push_back(result);
		}
	}

	// Joins the results of predecessors. It represents the input ranges into
	// the currently processed block.
	MemoryPlaceToRangeMap inputToBlock;
	inputToBlock = ValueAnalysis::join(outputOfPreds);

	// Assigns the input ranges to the currently processed block.
	blockToInputRangesMap[current] = inputToBlock;
}

/**
* @brief Gets the ranges that are changing between @a oldResult and @a newResult
*        and expands these for faster convergence. Then, these new ranges are
*        stored for the output of the given @a block.
*/
void ValueAnalysis::expandChangingRanges(const Block* block,
										 const MemoryPlaceToRangeMap &oldResult,
										 const MemoryPlaceToRangeMap &newResult)
{

	MemoryPlaceToRangeMap result;
	for (MemoryPlaceToRangeMap::const_iterator it = oldResult.begin();
		 it != oldResult.end(); ++it) {

		const MemoryPlace *key = it->first;

		// Key must exists in both maps.
		assert(newResult.find(key) != newResult.end());

		Range oldRange = it->second;
		MemoryPlaceToRangeMap::const_iterator jt = newResult.find(key);
		Range newRange = jt->second;

		if (oldRange == newRange) {
			// Ranges do not change.
			result[key] = newRange;
		} else {
			// Ranges change after the last processing of the block.
			result[key] = newRange.expand();
		}
	}

	// Firstly, I assumed that the size of oldResult and newResult must be same.
	// However, this is not true. So, we have to add everything what is in
	// newResult to result.
	for (MemoryPlaceToRangeMap::const_iterator it = newResult.begin();
		 it != newResult.end(); ++it) {

		const MemoryPlace *key = it->first;
		if (result.find(key) == result.end()) {
			// If key is not in the result, we have to add it.
			result[key] = it->second;
		}
	}

	blockToOutputRangesMap[block] = result;
}

/**
* @brief Returns @c true if the given block contains only one instruction and
*        this instruction is @c goto. Otherwise, it returns @c false.
*/
bool ValueAnalysis::containOnlyGotoInsn(const Block *block)
{
	if ((block->size() == 1) && (((*block)[0])->code == CL_INSN_JMP)) {
		return true;
	}
	return false;
}

/**
* @brief Computes value-range analysis for the given @a fnc.
*/
void ValueAnalysis::computeAnalysisForFnc(const Fnc &fnc)
{
	const Block *entryBlock = fnc.cfg.entry();

	// Sets the ranges for global variables for the input of the entry block.
	blockToInputRangesMap[entryBlock] = GlobAnalysis::getGlobVarMap();

	todoQueue.push(entryBlock);
	todoSet.insert(entryBlock);

	while (!todoQueue.empty()) {
		const Block *block = todoQueue.front();
		todoQueue.pop();
		todoSet.erase(block);

		MemoryPlaceToRangeMap oldResult = ValueAnalysis::getRanges(block,
			blockToOutputRangesMap);

		unsigned long tripCount = LoopFinder::getUpperLimit(block);
		if ((tripCount != 0) &&
			(tripCount == ValueAnalysis::tripCountOfBlockMap[block]) ) {
			// This block was processed enough times.
			ValueAnalysis::tripCountOfBlockMap[block] = 0;
			continue;
		}

		ValueAnalysis::computeAnalysisForBlock(block);
		++ValueAnalysis::tripCountOfBlockMap[block];
		MemoryPlaceToRangeMap newResult = ValueAnalysis::getRanges(block,
			blockToOutputRangesMap);

		if (newResult != oldResult) {
			// If this block was analysed many times and still does
			// not converge, we will help it a little.
			if (blockToCounterMap[block] > ValueAnalysis::NumberOfPassesBeforeExpand) {
				expandChangingRanges(block, oldResult, newResult);
			}
		}

		newResult = ValueAnalysis::getRanges(block, blockToOutputRangesMap);
		if ((newResult != oldResult) || (ValueAnalysis::containOnlyGotoInsn(block))) {
			// Gets the successors of the processed block.
			const TTargetList &succs = block->targets();
			BOOST_FOREACH(const TTargetList::value_type &succ, succs) {
				ValueAnalysis::scheduleBlock(succ);
			}
		}
	}
}

/**
* @brief Computes value-range analysis for the given @a block.
*/
void ValueAnalysis::computeAnalysisForBlock(const Block *block)
{
	computeInputRanges(block);

	MemoryPlaceToRangeMap outputFromBlock;
	outputFromBlock = ValueAnalysis::getRanges(block, blockToInputRangesMap);

	// Starts to analyze the given block.
	const Insn *prevInsn = NULL;
	BOOST_FOREACH(const Insn *insn, *block) {
		ValueAnalysis::computeAnalysisForInsn(insn, prevInsn, outputFromBlock);
		prevInsn = insn;
	}

	// Assigns the output ranges to the currently processed block.
	blockToOutputRangesMap[block] = outputFromBlock;

	// Increments counter.
	++blockToCounterMap[block];
}

/**
* @brief Computes value-range analysis for the given @a insn that represents
*        call instruction. Results are stored in @a output.
*/
void ValueAnalysis::computeAnalysisForCall(const Insn* insn,
	MemoryPlaceToRangeMap &output)
{
	const TOperandList &opList = insn->operands;
	const struct cl_operand &ret = opList[0];   // [0] - destination

	if (ret.code == CL_OPERAND_VAR) {
		// Without this condition, the problem with functions whose return values
		// are not stored in the program occurred.
		const MemoryPlace *retVar = OperandToMemoryPlace::convert(&ret);
		Range retRange = ValueAnalysis::getRange(ret, output);
		output[retVar] = retRange;
	}
}

/**
* @brief Computes value-range analysis for the given @a insn and computed result
*        joins to @a output.
*/
void ValueAnalysis::computeAnalysisForInsn(const Insn *insn, const Insn *prevInsn,
										   MemoryPlaceToRangeMap &output)
{
	const enum cl_insn_e code = insn->code;

	switch (code) {
		case CL_INSN_NOP:
			// No operation.
			break;

		case CL_INSN_JMP:
			// Jump.
			break;

		case CL_INSN_COND:
			// Condition.
			ValueAnalysis::computeAnalysisForCond(insn, prevInsn, output);
			break;

		case CL_INSN_RET:
			//Return.
			break;

		case CL_INSN_ABORT:
			// Abort.
			break;

		case CL_INSN_UNOP:
			// Unary operation.
			ValueAnalysis::computeAnalysisForUnop(insn, output);
			break;

		case CL_INSN_BINOP:
			// Binary operation.
			ValueAnalysis::computeAnalysisForBinop(insn, output);
			break;

		case CL_INSN_CALL:
			// Call.
			ValueAnalysis::computeAnalysisForCall(insn, output);
			break;

		case CL_INSN_SWITCH:
			// Switch.
			break;

		case CL_INSN_LABEL:
			// Label.
			break;

		default:
			assert(false && "Invalid insn->code in computeAnalysisForInsn()!");
			return;
	}
}

/**
* @brief Evaluates condition specified by the @a code for ranges @a r1 and @a r2.
*/
bool ValueAnalysis::evaluateCond(const Range &r1, const Range &r2,
	const enum cl_binop_e code)
{
	Range result;
	switch (code) {
		case CL_BINOP_EQ:
			// Equal.
			result = logicalEq(r1, r2);
			break;

		case CL_BINOP_NE:
			// Unequal.
			result = logicalNeq(r1, r2);
			break;

		case CL_BINOP_LT:
			// Less than.
			result = logicalLt(r1, r2);
			break;

		case CL_BINOP_GT:
			// Greater than.
			result = logicalGt(r1, r2);
			break;

		case CL_BINOP_LE:
			// Less or equal than.
			result = logicalLtEq(r1, r2);
			break;

		case CL_BINOP_GE:
			// Greater or equal than
			result = logicalGtEq(r1, r2);
			break;

		default:
			assert(false);
			break;
	}

	if ((result.size() == 1) && (result.containsZero())) {
		// Condition is evaluated as false.
		return false;
	}

	// Condition can be evaluated as true.
	return true;
}


/**
* @brief Computes value-range analysis for the condition instruction @a insn,
*        where @a prevInsn is previous instruction to the @a insn instruction.
*        This function is responsible for computing trimmed ranges.
*/
void ValueAnalysis::computeAnalysisForCond(const Insn *insn, const Insn *prevInsn,
										   MemoryPlaceToRangeMap &output)
{
	if (prevInsn == NULL) {
		// If we do not have previous instruction, we cannot compute trimmed ranges.
		return;
	}

	// Gets the codes of both instruction.
	const enum cl_insn_e prevCode = prevInsn->code;
	if (prevCode != CL_INSN_BINOP) {
		// The previous instruction must be binary operation: !=, ==, <=, >=, <, >.
		return;
	}

	// Gets the necessary information about previous instruction.
	const TOperandList &opListPrev = prevInsn->operands;

	// There are three operands for binary operation.
	const struct cl_operand &dst = opListPrev[0];   // [0] - destination
	const struct cl_operand &src1 = opListPrev[1];  // [1] - source 1
	const struct cl_operand &src2 = opListPrev[2];  // [2] - source 2

	// Gets the memory place of the destination.
	MemoryPlace *dstVar = OperandToMemoryPlace::convert(&dst);

	// Gets the memory places of the sources and sets the source ranges.
	const MemoryPlace *srcVar1 = NULL;
	Range srcRange1;
	if (CL_OPERAND_VAR == src1.code) {
		srcVar1 = OperandToMemoryPlace::convert(&src1);
		srcRange1 = ValueAnalysis::getRange(src1, output);
	}

	const MemoryPlace *srcVar2 = NULL;
	Range srcRange2;
	if (CL_OPERAND_VAR == src2.code) {
		srcVar2 = OperandToMemoryPlace::convert(&src2);
		srcRange2 = ValueAnalysis::getRange(src2, output);
	}

	// Sets the source ranges for constants.
	if (CL_OPERAND_CST == src1.code) {
		Number bound = Utility::convertOperandToNumber(&src1);
		srcRange1 = Range(bound);
	}

	if (CL_OPERAND_CST == src2.code) {
		Number bound = Utility::convertOperandToNumber(&src2);
		srcRange2 = Range(bound);
	}

	// Gets the sub-code of previous instruction.
	const enum cl_binop_e prevSubcode = static_cast<enum cl_binop_e>(prevInsn->subCode);

	// Get the necessary information about current instruction (condition).
	const TOperandList &opListCurr = insn->operands;

	// There is one operand for condition operation.
	const struct cl_operand &cond = opListCurr[0];  // [0] - condition

	// Gets the memory place of the condition (it is an artificial variable).
	const MemoryPlace *condVar = OperandToMemoryPlace::convert(&cond);

	// Gets the targets.
	const TTargetList &targets = insn->targets;
	const Block *thenTarget = targets[0];
	const Block *elseTarget = targets[1];

	if (condVar != dstVar) {
		// Checks if in the previous instruction was assignment to the condition
		// variable.
		return;
	}

	// Set types of resulted ranges.
	Range srcRange1True, srcRange1False;
	Range srcRange2True, srcRange2False;
	srcRange1True = srcRange1False = srcRange1;
	srcRange2True = srcRange2False = srcRange2;

	switch (prevSubcode) {
		case CL_BINOP_EQ:
			// Equal.
			srcRange1True = srcRange1True.assign(
							computeRangeForEq(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForEq(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForNeq(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForNeq(srcRange2, srcRange1));
			break;

		case CL_BINOP_NE:
			// Unequal.
			srcRange1True = srcRange1True.assign(
							computeRangeForNeq(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForNeq(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForEq(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForEq(srcRange2, srcRange1));
			break;

		case CL_BINOP_LT:
			// Less than.
			srcRange1True = srcRange1True.assign(
							computeRangeForLt(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForGt(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForGtEq(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForLtEq(srcRange2, srcRange1));
			break;

		case CL_BINOP_GT:
			// Greater than.
			srcRange1True = srcRange1True.assign(
							computeRangeForGt(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForLt(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForLtEq(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForGtEq(srcRange2, srcRange1));
			break;

		case CL_BINOP_LE:
			// Less or equal than.
			srcRange1True = srcRange1True.assign(
							computeRangeForLtEq(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForGtEq(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForGt(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForLt(srcRange2, srcRange1));
			break;

		case CL_BINOP_GE:
			// Greater or equal than
			srcRange1True = srcRange1True.assign(
							computeRangeForGtEq(srcRange1,srcRange2));
			srcRange2True = srcRange2True.assign(
							computeRangeForLtEq(srcRange2, srcRange1));
			srcRange1False = srcRange1False.assign(
							computeRangeForLt(srcRange1, srcRange2));
			srcRange2False = srcRange2False.assign(
							computeRangeForGt(srcRange2, srcRange1));
			break;

		default:
			assert(false);
			return;
	}

	// Compute if there is the possibility that condition is evaluated as true.
	// bool condEval = ValueAnalysis::evaluateCond(srcRange1, srcRange2, prevSubcode);

	if (CL_OPERAND_VAR == src1.code) {
		// We store the result for the first operand of the condition.
		struct TrimmedKey key1(condVar, thenTarget, srcVar1);
		struct TrimmedKey key2(condVar, elseTarget, srcVar1);
		TrimmedRangesMap trimmedRangesMap;
		trimmedRangesMap[key1] = srcRange1True;
		trimmedRangesMap[key2] = srcRange1False;
		blockToTrimmedRangesMap[insn->bb] = trimmedRangesMap;
	}

	if (CL_OPERAND_VAR == src2.code) {
		// We store the result for the second operand of the condition.
		struct TrimmedKey key1(condVar, thenTarget, srcVar2);
		struct TrimmedKey key2(condVar, elseTarget, srcVar2);
		TrimmedRangesMap trimmedRangesMap = blockToTrimmedRangesMap[insn->bb];
		trimmedRangesMap[key1] = srcRange2True;
		trimmedRangesMap[key2] = srcRange2False;
		blockToTrimmedRangesMap[insn->bb] = trimmedRangesMap;
	}
}

/**
* @brief Computes value-range analysis for the given @a insn that represents
*        an unary operation and computed result joins to @a output.
*/
void ValueAnalysis::computeAnalysisForUnop(const Insn *insn,
				    					   MemoryPlaceToRangeMap &output)
{
	// There are two operands for unary operations.
	const TOperandList &opList = insn->operands;
	const struct cl_operand &dst = opList[0]; // [0] - destination
	const struct cl_operand &src = opList[1]; // [1] - source

	// Checks if left operand is valid.
	assert(dst.code == CL_OPERAND_VAR);

	// Checks if right operand is valid.
	assert(src.code == CL_OPERAND_CST || src.code == CL_OPERAND_VAR);

	const enum cl_unop_e code = static_cast<enum cl_unop_e>(insn->subCode);

	// We do not need to get memory place or source range for assignment
	// because operands can be structures.
	const MemoryPlace *dstVar = NULL;
	Range srcRange, resultRange;
	if (code != CL_UNOP_ASSIGN) {
		// Gets the destination memory place.
		dstVar = OperandToMemoryPlace::convert(&dst);
		// Gets the source range.
		srcRange = ValueAnalysis::getRange(src, output);
		// This is used only to set the type of the result range.
		// Necessary for correct behaviour of assign function from the Range class.
		resultRange = Utility::getMaxRange(dst);
	}

	switch (code) {
		case CL_UNOP_ASSIGN:
			// Assignment must be solved extra because of the assignment of
			// the structure to another structure.
			ValueAnalysis::assign(dst, src, output);
			return;

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
			assert(false && "Invalid insn->subCode in computeAnalysisForUnop()!");
			break;
	}

	// Setting the new range for destination.
	output[dstVar] = resultRange;
}

/**
* @brief Computes value-range analysis for the given @a insn that represents
*        a binary operation and computed result joins to @a output.
*/
void ValueAnalysis::computeAnalysisForBinop(const Insn *insn,
	 										MemoryPlaceToRangeMap &output)
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
	assert(src1.code == CL_OPERAND_CST || src1.code == CL_OPERAND_VAR);
	assert(src2.code == CL_OPERAND_CST || src2.code == CL_OPERAND_VAR);

	// Gets the source ranges.
	Range srcRange1 = ValueAnalysis::getRange(src1, output);
	Range srcRange2 = ValueAnalysis::getRange(src2, output);

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
	output[dstVar] = resultRange;
}

/**
* @brief Emits the result of analysis for the analyzed program that is
*        represented by @a stor into @a os.
*/
ostream& ValueAnalysis::printRanges(ostream &os, const Storage &stor)
{
	BOOST_FOREACH(const Fnc* pFnc, stor.callGraph.topOrder) {
		// Iterates over all functions.
		const Fnc &fnc = *pFnc;
		if (!isDefined(fnc))
			continue;

		string delimeter(10, '-');
		os << delimeter << " Function " << nameOf(fnc) << "() ";
		os << delimeter << endl;

		BOOST_FOREACH(const Block* pBlock, fnc.cfg) {
			// Iterates over all blocks.
			const Block &block = *pBlock;
			int firstLine = ((block.front())->loc).line;
			int lastLine = ((block.back())->loc).line;

			if (firstLine > lastLine) {
				std::swap(firstLine, lastLine);
			}

			// Prints input ranges.
			os << "Block " << block.name() << "[IN]" << " at lines from ";
			os << firstLine << " to ";
			os << lastLine << ":" << endl;

			// Gets the result of analysis for the currently processed block.
			MemoryPlaceToRangeMap &blockInfo = blockToInputRangesMap[pBlock];
			vector<MemoryPlaceRangePair> sortedBlockInfo(
				blockInfo.begin(), blockInfo.end());

			sort(sortedBlockInfo.begin(), sortedBlockInfo.end(),
				sortBlockInfo);

			BOOST_FOREACH(MemoryPlaceRangePair &mem, sortedBlockInfo) {
				// Iterates over all memory places in the block.
				if ((mem.first)->isArtificial())
					continue;

				// User variables and corresponding ranges in block are printed.
				os << "\t" << (mem.first)->asString();
				os << " = " << mem.second;
			}

			// Prints output ranges.
			os << "Block " << block.name() << "[OUT]:" << endl;

			// Gets the result of analysis for the currently processed block.
			MemoryPlaceToRangeMap &blockInfoOut = blockToOutputRangesMap[pBlock];
			vector<MemoryPlaceRangePair> sortedBlockInfoOut(
				blockInfoOut.begin(), blockInfoOut.end());

			sort(sortedBlockInfoOut.begin(), sortedBlockInfoOut.end(),
				sortBlockInfo);

			BOOST_FOREACH(MemoryPlaceRangePair &mem, sortedBlockInfoOut) {
				// Iterates over all memory places in the block.
				if ((mem.first)->isArtificial())
					continue;

				// User variables and corresponding ranges in block are printed.
				os << "\t" << (mem.first)->asString();
				os << " = " << mem.second;
			}
		}
	}
	return os;
}

/**
* @brief Joins data that was gained from the analysis of several blocks.
*
* @param[in] vec Data gained from the analysis of several blocks.
*
* @return United data from @a vec.
*
* Value-range analysis of every block is represented by @c MemoryPlaceToRangeMap
* that contains all memory places and corresponding ranges for them. During
* value-range analysis it is necessary to join data of different blocks
* together. For example, join data of all predecessor blocks that will be
* used as an input to the next block.
*/
ValueAnalysis::MemoryPlaceToRangeMap ValueAnalysis::join(const
				             MemoryPlaceToRangeMapVector &vec)
{
	MemoryPlaceToRangeMap result;
	if (vec.empty())
		return result;

	result = vec[0];
	for (MemoryPlaceToRangeMap::size_type i = 1; i != vec.size(); ++i) {
		// Iterates over all data that was got from several blocks.
		BOOST_FOREACH(MemoryPlaceToRangeMap::value_type item, vec[i]) {
			// Iterates over all memory places.
			result[item.first] = unite(result[item.first], item.second);
		}
	}

	return result;
}

