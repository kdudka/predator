/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   OperandToMemoryPlace.h
* @brief  Class converts @c cl_operand to the instance of the @c MemoryPlace class.
* @date   2012
*/

#ifndef GUARD_OPERAND_TO_MEMORY_PLACE_H
#define GUARD_OPERAND_TO_MEMORY_PLACE_H

#include <vector>
#include <map>
#include <deque>
#include <cl/code_listener.h>
#include <gmpxx.h>
#include "MemoryPlace.h"

/**
* @brief Class converts @c cl_operand to the instance of the @c MemoryPlace class.
*
* This class converts the given @c cl_operand to the @c MemoryPlace class according
* to the type of @c cl_operand.
*/

class OperandToMemoryPlace {
	private:
		/// Just for assurance that nobody will try to use it.
		OperandToMemoryPlace() { }

		/// Represents integral type.
		typedef mpz_class Int;

		/// Type used to represent id for memory place.
		typedef std::vector<Int> UidVector;

		/// Map that for each @c UidVector stores corresponding @c MemoryPlace.
		static std::map<UidVector, MemoryPlace*> memoryPlaceMap;

		static MemoryPlace* convertSimpleOperand(const cl_operand *operand);

	public:
		static MemoryPlace* convert(const cl_operand *operand,
									std::deque<int> indexes = std::deque<int>());

		static void init();
};

#endif
