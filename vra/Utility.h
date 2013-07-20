/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Utility.h
* @brief  Class implements help functions for value-range analysis.
* @date   2012
*/

#ifndef GUARD_UTILITY_H
#define GUARD_UTILITY_H

#include <cl/code_listener.h>
#include <deque>
#include "Number.h"
#include "Range.h"

/**
* @brief Class implements help functions for value-range analysis.
*/
class Utility {
	private:
		/// Just for assurance that nobody will try to use it.
		Utility() { }

	public:
		static Number convertOperandToNumber(const cl_operand *operand);
		static Range getMaxRange(const cl_operand &operand,
								 std::deque<int> indexes = std::deque<int>());
};

#endif
