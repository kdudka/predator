/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Utility.cc
* @brief  Class implements help functions for value-range analysis.
* @date   2012
*/

#undef NDEBUG   // It is necessary for using assertions.

#include <cassert>
#include <iostream>
#include "Utility.h"

using std::deque;

/**
* @brief Converts @c cl_operand to the instance of the @c Number class.
*
* @param[in] operand It will be converted to the @c Number object.
*
* @return @c Number instance that was created from @a operand.
*
* Preconditions:
* - @code operand->code == CL_OPERAND_VAR || operand->type == CL_OEPRAND_CST @endcode
*/
Number Utility::convertOperandToNumber(const cl_operand *operand)
{
	// Is should be used only for variables or constant.
	assert(operand->code == CL_OPERAND_VAR || operand->code == CL_OPERAND_CST);

	// Bit size of the type of the constant.
	int bitWidth;

	if (operand->code == CL_OPERAND_CST) {
		switch ((operand->data).cst.code) {
			case CL_TYPE_INT:
				bitWidth = (operand->type)->size;
				if ((operand->type)->is_unsigned) {
					// Unsigned integral number.
					unsigned long value = (operand->data).cst.data.cst_uint.value;
					return Number(value, bitWidth, false);
				} else {
					// Signed integral value.
					long value = (operand->data).cst.data.cst_int.value;
					return Number(value, bitWidth, true);
				}

			case CL_TYPE_REAL: {
				bitWidth = (operand->type)->size;
				long double value = (operand->data).cst.data.cst_real.value;
				return Number(value, bitWidth);
			}

			default:
				// CL_TYPE_VOID, CL_TYPE_UNKNOWN, CL_TYPE_PTR, CL_TYPE_STRUCT,
				// CL_TYPE_UNION, CL_TYPE_ARRAY, CL_TYPE_FNC, CL_TYPE_CHAR,
				// CL_TYPE_BOOL, CL_TYPE_ENUM, CL_TYPE_STRING
				std::cerr << "Code of the constant: "
						<< ((operand->data).cst.code) << std::endl;
				assert(!"Unsupported!");
				return Number(0, sizeof(int), true);
		}
	} else if (operand->code == CL_OPERAND_VAR) {
		int bitWidth = operand->type->size;
		switch (operand->type->code) {
			case CL_TYPE_INT:
				if (operand->type->is_unsigned) {
					// Unsigned integral number.
					return Number(0, bitWidth, false);
				} else {
					// Signed integral value.
					return Number(0, bitWidth, true);
				}

			case CL_TYPE_BOOL:
				return Number(0, bitWidth, true);

			case CL_TYPE_REAL:
				return Number(0.0, bitWidth);

			default:
				// CL_TYPE_VOID, CL_TYPE_UNKNOWN, CL_TYPE_PTR,  CL_TYPE_UNION,
				// CL_TYPE_FNC, CL_TYPE_CHAR, CL_TYPE_ENUM, CL_TYPE_STRING
				std::cerr << "Type of variable: " << (operand->type->code) << std::endl;
				assert(!"Unsupported!");
				return Number(0, bitWidth, true);
		}
	}

	assert(!"Should not happen!");
	return Number(0, sizeof(int), true);
}

/**
* @brief According to the type of the @a operand, it returns the maximal
*        possible range.
*
* Preconditions:
* - @code operand->code == CL_OPERAND_VAR @endcode
*/
Range Utility::getMaxRange(const cl_operand &operand, deque<int> indexes)
{
	// Is should be used only for variables.
	assert(operand.code == CL_OPERAND_VAR);

	const struct cl_type *currentType;
	if (operand.accessor != NULL && !indexes.empty()) {
		currentType = operand.accessor->type;
	} else {
		currentType = operand.type;
	}

	// If the variable is nested in a structure or in an array.
	if (currentType->code == CL_TYPE_STRUCT || currentType->code == CL_TYPE_ARRAY) {
		while (currentType->code != CL_TYPE_INT &&
			   currentType->code != CL_TYPE_REAL &&
			   currentType->code != CL_TYPE_ENUM &&
			   currentType->code != CL_TYPE_BOOL) {
			int index = indexes.front();
			indexes.pop_front();
			currentType = ((currentType->items)[index]).type;
		}
	}

	// Size of the simple variable.
	int bitWidth = currentType->size;

	switch (currentType->code) {
		case CL_TYPE_INT:
			if (currentType->is_unsigned) {
				// Unsigned integral number.
				return Range::getMaxRange(Number(0, bitWidth, false));
			} else {
				// Signed integral value.
				return Range::getMaxRange(Number(0, bitWidth, true));
			}

		case CL_TYPE_BOOL:
			return Range(Range::Interval(Number(0, sizeof(int), true),
									     Number(1, sizeof(int), true)));

		case CL_TYPE_REAL:
			return Range::getMaxRange(Number(0.0, bitWidth));

		case CL_TYPE_ENUM:
			return Range::getMaxRange(Number(0, bitWidth, true));

		default:
			// CL_TYPE_VOID, CL_TYPE_UNKNOWN, CL_TYPE_PTR,  CL_TYPE_UNION,
			// CL_TYPE_FNC, CL_TYPE_CHAR, CL_TYPE_ENUM, CL_TYPE_STRING
			std::cerr << "Type of variable: " << (operand.type->code) << std::endl;
			assert(!"Unsupported!");
			return Range();
	}
}

