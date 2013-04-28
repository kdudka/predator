/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   ValueAnalysis.h
* @brief  Class performs the value-range analysis and stores the result.
* @date   2012
*/

#ifndef GUARD_VALUE_ANALYSIS_H
#define GUARD_VALUE_ANALYSIS_H

#include <cl/storage.hh>
#include <cl/code_listener.h>

#include <ostream>
#include <map>
#include <utility>
#include <vector>
#include <set>
#include <queue>

#include "Range.h"
#include "MemoryPlace.h"
#include "LoopFinder.h"

/**
* @brief Class performs the value-range analysis and stores the result.
*
* It implements functions for computing the value-range analysis for a function,
* for a block, for an instruction and so on. It stores the result of the
* analysis per each function of the program. For each memory place in every block,
* the final range is stored. Class is also responsible for printing tabular output.
*/
class ValueAnalysis {
	public:
		/// Type of the data stored per each block.
		typedef std::map<const MemoryPlace*, Range> MemoryPlaceToRangeMap;

		/// Type of the pair consisting of memory place and corresponding range.
		typedef std::pair<const MemoryPlace*, Range> MemoryPlaceRangePair;

	private:
		/// Just for assurance that nobody will try to use it.
		ValueAnalysis() { }

		/// Stores maximal number of passes through the block or zero if we do
		/// not know.
		static LoopFinder::BlockToUpperLimit tripCountOfBlockMap;

		/// Type for representing key into map that stores trimmed ranges.
		struct TrimmedKey {
			/// Pointer to the memory place for the condition variable.
			const MemoryPlace *conditionMp;

			/// Block for which the trimmed ranges are valid.
			const CodeStorage::Block *block;

			/// Pointer to the memory place representing variable for which trimmed
			/// range is computed.
			const MemoryPlace *varMp;

			/// Constructor for creating new structure.
			TrimmedKey(const MemoryPlace* cond, const CodeStorage::Block *block,
					   const MemoryPlace *var):
				conditionMp(cond), block(block), varMp(var) {}

			/// Comparison on less than for two structures. It will be used as a key
			/// to the map, so it is necessary to defined the comparison.
			bool operator<(const TrimmedKey &key) const {
				if (conditionMp < key.conditionMp) {
					return true;
				} else if (conditionMp > key.conditionMp) {
					return false;
				} else if (block < key.block) {
					return true;
				} else if (block > key.block) {
					return false;
				} else {
					return varMp < key.varMp;
				}
			}
		};

		/// Type for trimmed ranges stored for one block.
		typedef std::map<TrimmedKey, Range> TrimmedRangesMap;

		/// Type for trimmed ranges stored for every block in the analyzed program.
		typedef std::map<const CodeStorage::Block*, TrimmedRangesMap>
			BlockToTrimmedRangesMap;

		/// Type of data stored for several blocks.
		typedef std::vector<MemoryPlaceToRangeMap> MemoryPlaceToRangeMapVector;

		/// Type of data stored for the whole analyzed program.
		typedef std::map<const CodeStorage::Block*, MemoryPlaceToRangeMap>
			BlockToResultMap;

		/// Type for representing scheduler.
		typedef std::queue<const CodeStorage::Block *> SchedulerQueue;

		/// Type for representing scheduler.
		typedef std::set<const CodeStorage::Block *> SchedulerSet;

		/// Type for representing block counter.
		typedef std::map<const CodeStorage::Block *, unsigned> BlockToCounterMap;

		/// Mapping block to the trimmed ranges of this block.
		static BlockToTrimmedRangesMap blockToTrimmedRangesMap;

		/// Mapping block to the input ranges of this block.
		static BlockToResultMap blockToInputRangesMap;

		/// Mapping block to the output ranges of this block.
		static BlockToResultMap blockToOutputRangesMap;

		/// Block scheduler.
		static SchedulerQueue todoQueue;

		/// Block scheduler.
		static SchedulerSet todoSet;

		/// Specifies how many times the block is executed before the expansion
		/// of changing ranges will be performed.
		static const unsigned NumberOfPassesBeforeExpand;

		/// Stores how many times was the block executed.
		static BlockToCounterMap blockToCounterMap;

		static void scheduleBlock(const CodeStorage::Block *block);

		static MemoryPlaceToRangeMap getRanges(const CodeStorage::Block* block,
											   const BlockToResultMap &inputMap);

		static TrimmedRangesMap getTrimmedRanges(const CodeStorage::Block* block);

		static MemoryPlaceToRangeMap join(const MemoryPlaceToRangeMapVector &vec);

		static MemoryPlaceToRangeMap computePartialInputRanges(
											const CodeStorage::Block *current,
											const MemoryPlaceToRangeMap &out,
											const TrimmedRangesMap &trimmed);

		static void computeInputRanges(const CodeStorage::Block *current);

		static void expandChangingRanges(const CodeStorage::Block *block,
										 const MemoryPlaceToRangeMap &oldResult,
										 const MemoryPlaceToRangeMap &newResult);

		static void computeAnalysisForBlock(const CodeStorage::Block *block);

		static void computeAnalysisForInsn(const CodeStorage::Insn *insn,
										   const CodeStorage::Insn *prevInsn,
										   MemoryPlaceToRangeMap &output);

		static void computeAnalysisForCond(const CodeStorage::Insn *insn,
										   const CodeStorage::Insn *prevInsn,
										   MemoryPlaceToRangeMap &output);

		static void computeAnalysisForUnop(const CodeStorage::Insn *insn,
										   MemoryPlaceToRangeMap &output);

		static void computeAnalysisForBinop(const CodeStorage::Insn *insn,
										    MemoryPlaceToRangeMap &output);

		static void computeAnalysisForCall(const CodeStorage::Insn* insn,
										   MemoryPlaceToRangeMap &output);

		static Range getRange(const struct cl_operand &src,
							  MemoryPlaceToRangeMap &output,
							  std::deque<int> indexes = std::deque<int>());

		static void assign(const struct cl_operand &dst, const struct cl_operand &src,
						   MemoryPlaceToRangeMap &output);

		static void assignStructure(const struct cl_type *type,
									const struct cl_operand &dst,
									const struct cl_operand &src,
				    				MemoryPlaceToRangeMap &output,
									std::deque<int> &indexes);

		static void generateIndexes(const struct cl_type *type,
									std::deque<int> &ind,
									std::vector<std::deque<int> > &indVec);

		static void assignSimpleElement(const struct cl_operand &dst,
								 		const struct cl_operand &src,
				 				 		MemoryPlaceToRangeMap &output,
								 		std::deque<int> indDst = std::deque<int>(),
										std::deque<int> indSrc = std::deque<int>());

		static bool containOnlyGotoInsn(const CodeStorage::Block *block);

		static const struct cl_type *getType(const struct cl_operand &op);

		static bool evaluateCond(const Range &r1, const Range &r2,
			const enum cl_binop_e code);

	public:

		static std::ostream& printRanges(std::ostream &os,
										 const CodeStorage::Storage &stor);

		static void computeAnalysisForFnc(const CodeStorage::Fnc &fnc);
};

#endif
