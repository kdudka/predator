/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   LoopFinder.h
* @brief  Class computes for every block the upper limit that represents how many
*         times loops will be entered.
* @date   2013
*/

#ifndef GUARD_LOOP_FOUNDER_H
#define GUARD_LOOP_FOUNDER_H

#include <cl/storage.hh>
#include <cl/code_listener.h>

#include <ostream>
#include <map>
#include <vector>

#include "Number.h"

/**
* @brief Class computes the upper limit of loop evaluations.
*
* It means that for some types of loops, this class computes the number of loop
* repetitions. The computed upper limit is safe. In the case, that class is not able
* to compute this information or the block does not represent inner block the
* limit is set to zero. So, zero represents that we do not know how many times the
* block will be traversed or the block is not part of any loops.
*/
class LoopFinder {
	public:
		/// Type for storing upper limits for each block;
		typedef std::map<const CodeStorage::Block *, unsigned long> BlockToUpperLimit;

		static unsigned long getUpperLimit(const CodeStorage::Block *block);

		static std::ostream& printLoopAnalysis(std::ostream &os);

		static void computeLoopAnalysis(const CodeStorage::Storage &stor);

	private:
		static const unsigned long MaxTripCountOfTheLoopBeforeStop;

		/// Just for assurance that nobody will try to use it.
		LoopFinder() { }

		/// For every block stores the upper limit.
		static BlockToUpperLimit blockToUpperLimit;

		static void computeLoopAnalysisForFnc(const CodeStorage::Fnc &fnc);

		static void computeLoopAnalysisForBlock(const CodeStorage::Block *block);

		static const CodeStorage::Block* getThenBlock(const
			CodeStorage::Block *block);

		static bool isPatternWhile(const CodeStorage::Block *block,
			const CodeStorage::Block *thenBlock);

		static bool isPatternWhileIf(const CodeStorage::Block *block,
			const CodeStorage::Block *thenBlock);

		static bool isPatternWhileIfElse(const CodeStorage::Block *block,
			const CodeStorage::Block *thenBlock);

		static bool isPatternIf(const CodeStorage::Block *block,
			const CodeStorage::Block **gotoBlock);

		static bool isPatternIfElse(const CodeStorage::Block *block,
			const CodeStorage::Block **gotoBlock);

		static void computeLoopAnalysisForPatternWhile(
			const CodeStorage::Block *block,
			const CodeStorage::Block *thenBlock);

		static void computeLoopAnalysisForPatternWhileIf(
			const CodeStorage::Block *block,
			const CodeStorage::Block *ifBlock);

		static void computeLoopAnalysisForPatternWhileIfElse(
			const CodeStorage::Block *block,
			const CodeStorage::Block *thenBlock);

		static bool checkCondition(const CodeStorage::Block *block,
			const cl_operand **variable, const cl_operand **constant,
			enum cl_binop_e &type);

		static bool findInitialValueForCondVar(const CodeStorage::Block *block,
			const cl_operand *variable, const cl_operand **initial);

		static bool processPreviousInsn(const CodeStorage::Insn *insn,
			const CodeStorage::Insn *prevInsn, const cl_operand **variable,
			const cl_operand **constant, enum cl_binop_e &type);

		static bool findRelevantInsns(const CodeStorage::Block *block,
			const cl_operand *variable, std::vector<const CodeStorage::Insn*> &insns);

		static unsigned long computeTripCountOfLoop(const cl_operand *variable,
			const cl_operand *constant, const cl_operand *initial,
			const enum cl_binop_e type,
			const std::vector<const CodeStorage::Insn*> &insns);

		static bool addUnopInsn(const CodeStorage::Insn *insn,
			const cl_operand *variable, std::vector<const CodeStorage::Insn*> &insns);

		static bool addBinopInsn(const CodeStorage::Insn *insn,
			const cl_operand *variable, std::vector<const CodeStorage::Insn*> &insns);

		static bool addCallInsn(const CodeStorage::Insn *insn,
			const cl_operand *variable);

		static void setValueForBlock(const CodeStorage::Block *block,
			unsigned long value);

		static void processUnop(const CodeStorage::Insn *insn, Number &initialNum);

		static bool processBinop(const CodeStorage::Insn *insn, Number &initialNum);

		static bool blockModifiesVar(const CodeStorage::Block *block,
			const cl_operand *variable);
};

#endif
