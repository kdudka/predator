/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Number.cc
* @brief  Implementation of the class that represents a number that can be
*         integral or floating-point type.
* @date   2012
*/

#include <limits>
#include <climits>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

// Enable assertions.
#undef NDEBUG
#include <cassert>

#include "Number.h"

using std::numeric_limits;
using std::ostringstream;
using std::string;
using std::ostream;
using std::pair;
using std::swap;

namespace {

	/**
	* @brief Converts the given value into a string.
	*
	* @tparam T Type of @a value.
	*/
	template<typename T>
	string toString(const T &value) {
		std::ostringstream out;
		out << value;
		return out.str();
	}

}

/**
* @brief Constructs a new number from integral value.
*
* @param[in] value Instance of this class will represent this @a value.
* @param[in] width Bit width of the type that was used to store @a value.
* @param[in] sign Boolean flag specifies if the type is signed or unsigned.
*/
Number::Number(Int value, unsigned width, bool sign)
		:type(INT), intValue(value), sign(sign), bitWidth(width)
{
	setIntLimits();
	fitIntoBitWidth();
}

/**
* @brief Constructs a new number from floating-point value.
*
* @param[in] value Instance of this class will represent this @a value.
* @param[in] width Bit width of the type that was used to store @a value.
*/
Number::Number(Float value, unsigned width)
		:type(FLOAT), floatValue(value), bitWidth(width)
{
	setFloatLimits();
	fitIntoBitWidth();
}

/**
* @brief Returns a number that would resulted if @a n was assigned into the
*        current number in C.
*/
Number Number::assign(const Number &n) const
{
	Number result(*this);
	if (result.isIntegral()) {
		if (n.isIntegral()) {
			result.intValue = n.intValue;
		} else if (n.isFloatingPoint()) {
			// The following C code
			//
			// double fs[] = {-INFINITY, -1e10, -100.0, 0.0, 100.0, 1e10,
			//                INFINITY, NAN};
			// for (size_t i = 0, e = sizeof(fs)/sizeof(fs[0]); i < e; ++i) {
			//     int n = fs[i];
			//     printf("%le %d\n", fs[i], n);
			// }
			//
			// emits (disregarding whitespace)
			//
			// -inf      -2147483648
			// -1.0e+10  -2147483648
			// -1.0e+02         -100
			//  0.0e+00            0
			//  1.0e+02          100
			//  1.0e+10  -2147483648
			//  inf      -2147483648
			//  nan      -2147483648
			if (n.isNotNumber() ||
					n.floatValue < intToFloat(minIntLimit, isSigned()) ||
					n.floatValue > intToFloat(maxIntLimit, isSigned())) {
				result.intValue = minIntLimit;
			} else {
				result.intValue = floatToInt(n.floatValue);
			}
		}
	} else if (result.isFloatingPoint()) {
		if (n.isIntegral()) {
			result.floatValue = intToFloat(n.intValue, n.isSigned());
		} else if (n.isFloatingPoint()) {
			result.floatValue = n.floatValue;
		}
	}
	result.fitIntoBitWidth();
	return result;
}

/**
* @brief Sets limits of an integral number.
*/
void Number::setIntLimits()
{
	// According to the bitWidth finds out which type was used to store
	// the value and computes the suitable limits.
	if (sizeof(char) == bitWidth) {
		// Signed char.
		if (isSigned()) {
			minIntLimit = numeric_limits<signed char>::min();
			maxIntLimit = numeric_limits<signed char>::max();
		}
		// Unsigned char.
		else {
			minIntLimit = numeric_limits<unsigned char>::min();
			maxIntLimit = numeric_limits<unsigned char>::max();
		}
	} else if (sizeof(short) == bitWidth) {
		// Signed short int.
		if (isSigned()) {
			minIntLimit = numeric_limits<signed short>::min();
			maxIntLimit = numeric_limits<signed short>::max();
		}
		// Unsigned short int.
		else {
			minIntLimit = numeric_limits<unsigned short>::min();
			maxIntLimit = numeric_limits<unsigned short>::max();
		}
	} else if (sizeof(int) == bitWidth) {
		// Signed int.
		if (isSigned()) {
			minIntLimit = numeric_limits<signed int>::min();
			maxIntLimit = numeric_limits<signed int>::max();
		}
		// Unsigned int.
		else {
			minIntLimit = numeric_limits<unsigned int>::min();
			maxIntLimit = numeric_limits<unsigned int>::max();
		}
	} else if (sizeof(long) == bitWidth) {
		// Signed long int.
		if (isSigned()) {
			minIntLimit = numeric_limits<signed long>::min();
			maxIntLimit = numeric_limits<signed long>::max();
		}
		// Unsigned long int.
		else {
			minIntLimit = numeric_limits<unsigned long>::min();
			maxIntLimit = numeric_limits<unsigned long>::max();
		}
	} else {
		std::cerr << bitWidth << "\n";
		// This should never happen.
		assert(!"Provided bit width of the number does not correspond to bit"
			   "width of any integral type.");
	}
}

/**
* @brief Sets limits of a floating-point number.
*/
void Number::setFloatLimits()
{
	// According to the bitWidth finds out which type was used to store
	// the value and computes the suitable limits.
	if (sizeof(float) == bitWidth) {
		minFloatLimit = -numeric_limits<float>::max();
		maxFloatLimit = numeric_limits<float>::max();
	} else if (sizeof(double) == bitWidth) {
		minFloatLimit = -numeric_limits<double>::max();
		maxFloatLimit = numeric_limits<double>::max();
	} else if (sizeof(long double) == bitWidth) {
		minFloatLimit = -numeric_limits<long double>::max();
		maxFloatLimit = numeric_limits<long double>::max();
	} else {
		// This should never happen.
		assert(!"Provided bit width of the number does not correspond to bit"
			   "width of any floating-point type.");
	}
}

/**
* @brief Returns @c true if the number is an integer, @c false otherwise.
*
* Integers are @c char, @c short, @c int, and @c long.
*/
bool Number::isIntegral() const {
	return type == INT;
}

/**
* @brief Returns @c true if the number is a floating-point, @c false otherwise.
*
* Floats are @c float, @c double, and @c long @c double.
*/
bool Number::isFloatingPoint() const {
	return type == FLOAT;
}

/**
* @brief Returns @c true if the number is a signed integer, @c false otherwise.
*/
bool Number::isSigned() const {
	return isIntegral() && sign;
}

/**
* @brief Returns @c true if the number is an unsigned integer, @c false otherwise.
*/
bool Number::isUnsigned() const {
	return isIntegral() && !sign;
}

/**
* @brief Returns @c true for the floating-point number if this number represents
*        negative or positive infinity, @c false otherwise.
*/
bool Number::isInf() const
{
	return isFloatingPoint() && (isNegativeInf() || isPositiveInf());
}

/**
* @brief Returns @c true for the floating-point number if this number represents
*        negative infinity, @c false otherwise.
*/
bool Number::isNegativeInf() const
{
	// std::isinf() < 0 does not work as expected:
	//      long double minInf(-INFINITY);
	//      std::cout << std::isinf(minInf) << "\n"; // emits 1, not -1
	//
	// The following solution also does not work. For minimal long double
	// value, isinf() returns true but it should return false:
	// 		std::cout << floatValue << std::endl;
	// 		std::cout << isinf(floatValue) << std::endl;
	// 		std::cout << (floatValue == -INFINITY) << std::endl;
	// 		return isFloatingPoint() && isinf(floatValue) && floatValue < 0;
	//
	// The next solution works as expected.
	return isFloatingPoint() && toString(floatValue) == toString(-INFINITY);
}

/**
* @brief Returns @c true for the floating-point number if this number represents
*        positive infinity, @c false otherwise.
*/
bool Number::isPositiveInf() const
{
	// std::isinf() > 0 does not work as expected.
	//      long double minInf(-INFINITY);
	//      std::cout << std::isinf(minInf) << "\n"; // emits 1, not -1
	//
	// The following solution also does not work. For maximal long double
	// value, isinf() returns true but it should return false:
	// 		std::cout << floatValue << std::endl;
	// 		std::cout << isinf(floatValue) << std::endl;
	// 		std::cout << (floatValue == INFINITY) << std::endl;
	// 		return isFloatingPoint() && isinf(floatValue) && floatValue > 0;
	//
	// The next solution works as expected.
	return isFloatingPoint() && toString(floatValue) == toString(INFINITY);
}

/**
* @brief Returns @c true for the floating-point number if this number represents
*        not a number, @c false otherwise.
*/
bool Number::isNotNumber() const
{
	return isFloatingPoint() && isnan(floatValue);
}

/**
* @brief Returns @c true if the current number is not positive/negative INFINITY or
*        NAN, @c false otherwise.
*/
bool Number::isNumber() const
{
	return !isPositiveInf() && !isNegativeInf() && !isNotNumber();
}

/**
* @brief Returns @c true if the number equals the minimal representable number
*        on its bit width, @c false otherwise.
*/
bool Number::isMin() const {
	if (isIntegral()) {
		return intValue == minIntLimit;
	} else { // isFloatingPoint()
		return floatValue == minFloatLimit;
	}
}

/**
* @brief Returns @c true if the number equals the maximal representable number
*        on its bit width, @c false otherwise.
*/
bool Number::isMax() const {
	if (isIntegral()) {
		return intValue == maxIntLimit;
	} else { // isFloatingPoint()
		return floatValue == maxFloatLimit;
	}
}

/**
* @brief Returns the smallest representable value for the current type.
*
* For integral numbers, it returns 1. For floating-point numbers, it returns
* @c std::numeric_limits<>::min(), e.g. 1.17549e-038 for @c float.
*/
Number Number::getEpsilon() const {
	if (isIntegral()) {
		return Number(1, bitWidth, isSigned());
	} else { // isFloatingPoint()
		if (bitWidth == sizeof(float)) {
			return Number(std::numeric_limits<float>::min(), bitWidth);
		} else if (bitWidth == sizeof(double)) {
			return Number(std::numeric_limits<double>::min(), bitWidth);
		} else { // bitWidth == sizeof(long double)
			return Number(std::numeric_limits<long double>::min(), bitWidth);
		}
	}
}

/**
* @brief Returns minimal limit as a number.
*/
Number Number::getMin() const
{
	if (isIntegral()) {
		return Number(minIntLimit, bitWidth, isSigned());
	} else { // isFloatingPoint()
		return Number(minFloatLimit, bitWidth);
	}
}

/**
* @brief Returns maximal limit as a number.
*/
Number Number::getMax() const
{
	if (isIntegral()) {
		return Number(maxIntLimit, bitWidth, isSigned());
	} else { // isFloatingPoint()
		return Number(maxFloatLimit, bitWidth);
	}
}

/**
* @brief Returns NAN as a number.
*/
Number Number::getNan() const
{
	assert(isFloatingPoint());
	return Number(NAN, bitWidth);
}

/**
* @brief Returns INFINITY as a number.
*/
Number Number::getPositiveInf() const
{
	assert(isFloatingPoint());
	return Number(INFINITY, bitWidth);
}

/**
* @brief Returns -INFINITY as a number.
*/
Number Number::getNegativeInf() const
{
	assert(isFloatingPoint());
	return Number(-INFINITY, bitWidth);
}

/**
* @brief Returns stored number as @c Int.
*/
Number::Int Number::getInt() const
{
	assert(isIntegral());
	return intValue;
}

/**
* @brief Returns stored number as @c Float.
*/
Number::Float Number::getFloat() const
{
	assert(isFloatingPoint());
	return floatValue;
}

/**
* @brief Returns the number of bytes of the current number's type.
*/
unsigned Number::getBitWidth() const
{
	return bitWidth;
}

/**
* @brief Returns the number of bits of the current number's type.
*/
unsigned Number::getNumOfBits() const
{
	return bitWidth * CHAR_BIT;
}

/**
* @brief Returns sign of the current number.
*/
bool Number::getSign() const
{
	assert(isIntegral());
	return sign;
}

/**
* @brief Performs integral promotion according to the C99 standard.
*/
void Number::integralPromotion()
{
	if (isFloatingPoint())
		return;

	// It is guaranteed that the type of the number is INT.
	if (bitWidth < sizeof(int)) {
		bitWidth = sizeof(int);
		sign = true;
		setIntLimits();
	}
}

/**
* @brief Sets new integral value. It is used after the conversion from signed
*        integral type to the unsigned integral type. Setting value is performed
*        according to the C99 standard.
*/
void Number::convertSignedToUnsigned()
{
	if (intValue < 0) {
		Int max = maxIntLimit + 1;
		Int tmp = -(intValue / max) + 1;
		intValue = intValue + tmp * max;
	}
}

/**
* @brief Extends operands of binary operation according to the C99 standard.
*
* @param[in] first The first operand that will be extended.
* @param[in] second The second operand that will be extended.
*
* @return The pair of extended operands.
*/
pair<Number,Number> Number::extensionByCRules(Number first, Number second)
{
	// It will be set to true if the numbers were swapped.
	bool swaped = false;

	// Setting the number with bigger bit width to be the first one. From float
	// and int number, the float number will be the first. If int numbers have
	// equal bit widths, the unsigned number will be the first.
	if (first.isFloatingPoint() && second.isFloatingPoint()) {
		if (second.bitWidth > first.bitWidth) {
			swap(first, second);
			swaped = true;
		}
	} else if (first.isIntegral() && second.isIntegral()) {
		if (second.bitWidth > first.bitWidth) {
			swap(first, second);
			swaped = true;
		} else if (second.bitWidth == first.bitWidth) {
			if (first.isSigned() && second.isUnsigned()) {
				swap(first, second);
				swaped = true;
			}
		}
	} else if (second.isFloatingPoint()) {
		swap(first, second);
		swaped = true;
	}

	if (first.isFloatingPoint()) {
		// First, if the corresponding real type of either operand is long double,
		// the other operand is converted, without change of type domain, to a type
		// whose corresponding real type is long double.
		// Otherwise, if the corresponding real type of either operand is double,
		// the other operand is converted, without change of type domain, to a type
		// whose corresponding real type is double.
		// Otherwise, if the corresponding real type of either operand is float,
		// the other operand is converted, without change of type domain, to a type
		// whose corresponding real type is float.
		if (second.isIntegral())
			second.floatValue = intToFloat(second.intValue, second.isSigned());
		second.type = first.type;
		second.bitWidth = first.bitWidth;
		second.setFloatLimits();
	} else if (first.isIntegral()) {
		// Otherwise, the integer promotions are performed on both operands.
		first.integralPromotion();
		second.integralPromotion();
		if (first.sign == second.sign) {
			// Otherwise, if both operands have signed integer types or both
			// have unsigned integer types, the operand with the type of lesser
			// integer conversion rank is converted to the type of the operand
			// with greater rank.
			if (first.bitWidth > second.bitWidth) {
				second.bitWidth = first.bitWidth;
				second.setIntLimits();
			}
		} else if (first.isUnsigned() && second.isSigned()) {
			// Otherwise, if the operand that has unsigned integer type has rank
			// greater or equal to the rank of the type of the other operand,
			// then the operand with signed integer type is converted to the
			// type of the operand with unsigned integer type.
			second.bitWidth = first.bitWidth;
			second.sign = false;
			second.setIntLimits();
			second.convertSignedToUnsigned();
		} else if (first.isSigned() && second.isUnsigned()) {
			// Otherwise, if the type of the operand with signed integer type
			// can represent all of the values of the type of the operand with
			// unsigned integer type, then the operand with unsigned integer
			// type is converted to the type of the operand with signed integer
			// type.
			if (first.bitWidth > second.bitWidth) {
				second.bitWidth = first.bitWidth;
				second.sign = true;
				second.setIntLimits();
			} else if (first.bitWidth == second.bitWidth) {
				// Otherwise, both operands are converted to the unsigned
				// integer type corresponding to the type of the operand with
				// signed integer type.
				first.sign = false;
				first.setIntLimits();
				first.convertSignedToUnsigned();
			}
		}
	}

	// Check if extension went off without a hitch.
	assert(first.type == second.type);
	if (Number::INT == first.type) {
		assert(first.bitWidth == second.bitWidth && first.sign == second.sign);
	} else if (Number::FLOAT == first.type){
		assert(first.bitWidth == second.bitWidth);
	}

	// According to the swapping, the pair of extended operands will be returned.
	if (!swaped)
		return pair<Number, Number>(first, second);
	else
		return pair<Number, Number>(second, first);
}

/**
* @brief Converts the given floating-point number into an integer.
*/
Number::Int Number::floatToInt(const Float &n) {
	Int intNum;
	mpf_class floatNum = mpf_class(toString(n));
	mpz_set_f(intNum.get_mpz_t(), floatNum.get_mpf_t());
	return intNum;
}

/**
* @brief Converts the given integer into a floating-point number.
*
* @param n Integer to be converted.
* @param isSigned @c true if the integer is signed, @c false otherwise.
*/
Number::Float Number::intToFloat(const Int &n, bool isSigned) {
	if (isSigned) {
		return Float(n.get_si());
	} else {
		return Float(n.get_ui());
	}
}

/**
* @brief According to the type of the number, converts its value to the predefined
*        limits.
*/
void Number::fitIntoBitWidth()
{
	if (isIntegral()) {
		if (isSigned()) {
			Int valuesInBitWidth = 2 * maxIntLimit + 2;
			intValue -= minIntLimit;
			if (intValue < 0) {
				intValue += (-intValue / valuesInBitWidth + 1) * valuesInBitWidth;
			}
			intValue %= valuesInBitWidth;
			intValue += minIntLimit;
		} else {
			convertSignedToUnsigned();
			intValue = intValue % (maxIntLimit + 1);
		}
	} else if (isFloatingPoint()) {
		if (floatValue > maxFloatLimit)
			floatValue = INFINITY;
		else if (floatValue < minFloatLimit)
			floatValue = -INFINITY;
	}
}

/**
* @brief Checks whether operands are equal.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if operands are equal, @c false otherwise
*/
bool operator==(const Number& num1, const Number& num2)
{
	if (num1.isNotNumber() || num2.isNotNumber()) {
		// If one operand represents NAN, then equality comparison
		// gives false.
		return false;
	}

	pair<Number, Number> result = Number::extensionByCRules(num1, num2);
	Number &n1 = result.first;
	Number &n2 = result.second;

	switch (n1.type) {
		case Number::INT:
			return n1.intValue == n2.intValue;

		case Number::FLOAT:
			if (n1.isNegativeInf()) {
				return n2.isNegativeInf();
			} else if (n1.isPositiveInf()) {
				return n2.isPositiveInf();
			} else {
				// Testing two floats should not be done simply by using ==.
				// Instead, we say that two floats are equal if and only if
				// their value differ at most by an epsilon (a small number).
				//
				// The inspiration for the below code is from
				// http://www.parashift.com/c++-faq-lite/newbie.html#faq-29.17.
				//
				// See also Section 4.2 in
				// [D. Knuth, The Art of Computer Programming, Volume II].
				return std::abs(n1.floatValue - n2.floatValue) <=
					1e-5 * std::abs(n1.floatValue);
			}

		default:
			// This should never happen.
			assert(!"Unsupported type.");
			return false;
	}
}

/**
* @brief Checks whether operands are not equal.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if operands are not equal, @c false otherwise
*/
bool operator!=(const Number& num1, const Number& num2)
{
	return !(num1 == num2);
}

/**
* @brief Checks whether the first operand is lower or equal than the second one.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if the first operand is lower or equal than the second one,
*         @c false otherwise
*/
bool operator<=(const Number& num1, const Number& num2)
{
	return (num1 < num2) || (num1 == num2);
}

/**
* @brief Checks whether the first operand is greater or equal than the second one.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if the first operand is greater or equal than the second one,
*         @c false otherwise
*/
bool operator>=(const Number& num1, const Number& num2)
{
	return (num1 > num2) || (num1 == num2);
}

/**
* @brief Checks whether the first operand is lower than the second one.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if the first operand is lower than the second one, @c false
*         otherwise
*/
bool operator<(const Number& num1, const Number& num2)
{
	if (num1.isNotNumber() || num2.isNotNumber()) {
		// If one operand represents NAN, then equality comparison
		// gives false.
		return false;
	}

	pair<Number, Number> result = Number::extensionByCRules(num1, num2);
	Number &n1 = result.first;
	Number &n2 = result.second;

	switch (n1.type) {
		case Number::INT:
			return n1.intValue < n2.intValue;

		case Number::FLOAT:
			return n1.floatValue < n2.floatValue;

		default:
			// This should never happen.
			assert(!"Unsupported type.");
			return false;
	}
}

/**
* @brief Checks whether the first operand is greater than the second one.
*
* @param[in] num1 The first operand for comparison.
* @param[in] num2 The second operand for comparison.
*
* @return @c true if the first operand is greater than the second one, @c false
*         otherwise
*/
bool operator>(const Number& num1, const Number& num2)
{
	if (num1.isNotNumber() || num2.isNotNumber()) {
		// If one operand represents NAN, then equality comparison
		// gives false.
		return false;
	}

	pair<Number, Number> result = Number::extensionByCRules(num1, num2);
	Number &n1 = result.first;
	Number &n2 = result.second;

	switch (n1.type) {
		case Number::INT:
			return n1.intValue > n2.intValue;

		case Number::FLOAT:
			return n1.floatValue > n2.floatValue;

		default:
			// This should never happen.
			assert(!"Unsupported type.");
			return false;
	}
}

/**
* @brief Unary minus of the number.
*
* @param[in] op The first operand.
*
* @return Unary minus of @a op.
*
* Special cases that are handled: @code
*    -INF = -INF
*    -(-INF) = INF
*    -NAN = -NAN
*    -(-NAN) = NAN @endcode
*/
Number operator-(const Number& op)
{
	Number result = op;
	if (result.isIntegral()) {
		// For INT-type numbers.
		result.integralPromotion();
		result.intValue = -result.intValue;
		// If op is unsigned then after unary minus it is still unsigned.
		result.fitIntoBitWidth();
	} else if (result.isFloatingPoint()) {
		// For FLOAT-type numbers.
		result.floatValue = -result.floatValue;
	}
	return result;
}

/**
* @brief Adds two numbers.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The sum of @a op1 and @a op2.
*
* Special cases that are handled: @code
*    INF + INF = INF
*    INF + (-INF) = -NAN
*    INF + NAN = NAN
*    INF + number = INF
*   -INF + INF = -NAN
*   -INF + (-INF) = -INF
*   -INF + NAN = NAN
*   -INF + number = -INF
*    NAN + INF = NAN
*    NAN + (-INF) = NAN
*    NAN + NAN = NAN
*    NAN + number = NAN
*    number + INF = INF
*    number + (-INF) = -INF
*    number + NAN = NAN @endcode
*/
Number operator+(const Number& op1, const Number& op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	if (n1.isIntegral() && n2.isIntegral()) {
		Number::Int newValue = n1.intValue + n2.intValue;
		Number result(newValue, n1.bitWidth, n1.sign);
		result.fitIntoBitWidth();
		return result;
	} else if (n1.isFloatingPoint() && n2.isFloatingPoint()) {
		Number::Float newValue = n1.floatValue + n2.floatValue;
		Number result(newValue, n1.bitWidth);
		result.fitIntoBitWidth();
		return result;
	} else {
		// This should never happen.
		assert(!"Unsupported combination of types.");
		return Number(0, sizeof(char));
	}
}

/**
* @brief Subtracts two numbers.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The difference between @a op1 and @a op2.
*
* Special cases that are handled: @code
*    INF - INF = -NAN
*    INF - (-INF) = INF
*    INF - NAN = NAN
*    INF - number = INF
*   -INF - INF = -INF
*   -INF - (-INF) = -NAN
*   -INF - NAN = NAN
*   -INF - number = -INF
*    NAN - INF = NAN
*    NAN - (-INF) = NAN
*    NAN - NAN = NAN
*    NAN - number = NAN
*    number - INF = -INF
*    number - (-INF) = INF
*    number - NAN = NAN @endcode
*/
Number operator-(const Number& op1, const Number& op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	if (n1.isIntegral() && n2.isIntegral()) {
		Number::Int newValue = n1.intValue - n2.intValue;
		Number result(newValue, n1.bitWidth, n1.sign);
		result.fitIntoBitWidth();
		return result;
	} else if (n1.isFloatingPoint() && n2.isFloatingPoint()) {
		Number::Float newValue = n1.floatValue - n2.floatValue;
		Number result(newValue, n1.bitWidth);
		result.fitIntoBitWidth();
		return result;
	} else {
		// This should never happen.
		assert(!"Unsupported combination of types.");
		return Number(0, sizeof(char));
	}
}

/**
* @brief Multiplies two numbers.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The result of multiplication of @a op1 and @a op2.
*
* Special cases that are handled: @code
*    INF * INF = INF
*    INF * (-INF) = -INF
*    INF * NAN = NAN
*    INF * number = INF
*   -INF * INF = -INF
*   -INF * (-INF) = INF
*   -INF * NAN = NAN
*   -INF * number = -INF
*    NAN * INF = NAN
*    NAN * (-INF) = NAN
*    NAN * NAN = NAN
*    NAN * number = NAN
*    number * INF = INF
*    number * (-INF) = -INF
*    number * NAN = NAN @endcode
*/
Number operator*(const Number& op1, const Number& op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	if (n1.isIntegral() && n2.isIntegral()) {
		Number::Int newValue = n1.intValue * n2.intValue;
		Number result(newValue, n1.bitWidth, n1.sign);
		result.fitIntoBitWidth();
		return result;
	} else if (n1.isFloatingPoint() && n2.isFloatingPoint()) {
		Number::Float newValue = n1.floatValue * n2.floatValue;
		Number result(newValue, n1.bitWidth);
		result.fitIntoBitWidth();
		return result;
	} else {
		// This should never happen.
		assert(!"Unsupported combination of types.");
		return Number(0, sizeof(char));
	}
}

/**
* @brief Performs division of two real numbers.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The result of division of @a op1 and @a op2.
*
* Special cases that are handled: @code
*    INF / INF = -NAN
*    INF / (-INF) = -NAN
*    INF / NAN = NAN
*    INF / number = INF
*   -INF / INF = -NAN
*   -INF / (-INF) = -NAN
*   -INF / NAN = NAN
*   -INF / number = -INF
*    NAN / INF = NAN
*    NAN / (-INF) = NAN
*    NAN / NAN = NAN
*    NAN / number = NAN
*    number / INF = 0
*    number / (-INF) = -0
*    number / NAN = NAN
*    +number / 0 = INF
*    -number / 0 = -INF
*    +number / -0 = -INF
*    -number / -0 = INF
*    0 / 0 = NAN @endcode
*/
Number rdiv(const Number& op1, const Number& op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	// This division works only for floating-point numbers.
	assert(n1.isFloatingPoint() && n2.isFloatingPoint());

	Number::Float newValue = n1.floatValue / n2.floatValue;
	Number result(newValue, n1.bitWidth);
	result.fitIntoBitWidth();

	return result;
}

/**
* @brief Performs division of two numbers where there is no remainder.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The result of division of @a op1 and @a op2.
*/
Number exact_div(const Number& op1, const Number& op2)
{
	// This division works only for integral numbers.
	assert(op1.isIntegral() && op2.isIntegral());

	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	Number::Int newValue = n1.intValue / n2.intValue;
	Number result(newValue, n1.bitWidth, n1.sign);
	result.fitIntoBitWidth();

	return result;
}

/**
* @brief According to the @a isMod flag, it performs modulo or integral division.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
* @param[in] isMod @c true means performing modulo operation, @c false means
*                  performing integral division.
*
* @return If @a isMod flag is set to @c true then the result is modulo of @a op1
*         and @a op2; otherwise the result is integral division of @a op1 and
*         @a op2.
*/
Number Number::performTrunc(const Number &op1, const Number& op2, bool isMod)
{
	// This should work only for integral numbers.
	assert(op1.isIntegral() && op2.isIntegral());

	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	// Performs operation on the C integral type.
	Number::Int res;
	if ((sizeof(int) == n1.bitWidth)) {
		if (n1.isSigned()) {
			int oper1, oper2;
			oper1 = mpz_get_si(n1.intValue.get_mpz_t());
			oper2 = mpz_get_si(n2.intValue.get_mpz_t());
			if (isMod) {
				// Computes modulo.
				res = oper1 % oper2;
			} else {
				// Computes integral division.
				res = oper1 / oper2;
			}
		} else {
			unsigned oper1, oper2;
			oper1 = mpz_get_ui(n1.intValue.get_mpz_t());
			oper2 = mpz_get_ui(n2.intValue.get_mpz_t());
			if (isMod) {
				// Computes modulo.
				res = oper1 % oper2;
			} else {
				// Computes integral division.
				res = oper1 / oper2;
			}
		}
	} else if ((sizeof(long) == n1.bitWidth)) {
		if (n1.isSigned()) {
			long oper1, oper2;
			oper1 = mpz_get_si(n1.intValue.get_mpz_t());
			oper2 = mpz_get_si(n2.intValue.get_mpz_t());
			if (isMod) {
				// Computes modulo.
				res = oper1 % oper2;
			} else {
				// Computes integral division.
				res = oper1 / oper2;
			}
		} else {
			unsigned long oper1, oper2;
			oper1 = mpz_get_ui(n1.intValue.get_mpz_t());
			oper2 = mpz_get_ui(n2.intValue.get_mpz_t());
			if (isMod) {
				// Computes modulo.
				res = oper1 % oper2;
			} else {
				// Computes integral division.
				res = oper1 / oper2;
			}
		}
	}

	Number result = Number(res, n1.bitWidth, n1.sign);
	result.fitIntoBitWidth();
	return result;;
}

/**
* @brief Performs integer division. The result is rounded to zero.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The result of division of @a op1 and @a op2.
*/
Number trunc_div(const Number& op1, const Number& op2)
{
	return Number::performTrunc(op1, op2, false);
}

/**
* @brief Performs modulo.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
*
* @return The result of modulo of @a op1 and @a op2.
*/
Number trunc_mod(const Number& op1, const Number& op2)
{
	return Number::performTrunc(op1, op2, true);
}

/**
* @brief Converts the instance of @c Number to @c bool.
*/
bool Number::toBool() const
{
	if (isIntegral()) {
		return intValue != 0;
	} else { // isFloatingPoint()
		// To prevent a warning saying that floats should not be compared by
		// using ==, convert it first to bool and return that bool.
		bool asBool = floatValue;
		return asBool;
	}
}

/**
* @brief Performs logical @c not on @a op.
*/
bool logicalNot(const Number& op)
{
	return !op.toBool();
}

/**
* @brief Performs logical @c and on @a op1 and @a op2.
*/
bool logicalAnd(const Number& op1, const Number& op2)
{
	return op1.toBool() && op2.toBool();
}

/**
* @brief Performs logical @c or on @a op1 and @a op2.
*/
bool logicalOr(const Number& op1, const Number& op2)
{
	return op1.toBool() || op2.toBool();
}

/**
* @brief Performs logical @c xor on @a op1 and @a op2.
*/
bool logicalXor(const Number& op1, const Number& op2)
{
	return logicalOr(op1, op2) && !logicalAnd(op1, op2);
}

/**
* @brief Performs bit @c not on @a op.
*/
Number bitNot(const Number& op)
{
	// Bitwise operations are performed only on integral numbers.
	assert(op.isIntegral());

	Number promotedOp = op;
	promotedOp.integralPromotion();
	Number::Int result;
	mpz_com(result.get_mpz_t(), promotedOp.intValue.get_mpz_t());

	return Number(result, promotedOp.bitWidth, promotedOp.sign);
}

/**
* @brief Performs binary bit operation. Parameter @a mode determines which one
*        will be performed.
*
* @param[in] op1 The first operand.
* @param[in] op2 The second operand.
* @param[in] mode @c 'A' means bit @c and, @c 'O' means bit @c or and @c 'X' means
*                 bit @c xor.
*
* @return Result of binary bit operation specified by @a mode performed on
*         @a op1 and @a op2.
*/
Number Number::performBitOp(const Number &op1, const Number &op2, char mode)
{
	// Bitwise operations are performed only on integral numbers.
	assert(op1.isIntegral() && op2.isIntegral());

	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	Number::Int res;
	switch (mode) {
		case 'A':
			// Performs bit and.
			mpz_and(res.get_mpz_t(), n1.intValue.get_mpz_t(),
					n2.intValue.get_mpz_t());
			break;

		case 'O':
			// Performs bit or.
			mpz_ior(res.get_mpz_t(), n1.intValue.get_mpz_t(),
					n2.intValue.get_mpz_t());
			break;

		case 'X':
			// Performs bit xor.
			mpz_xor(res.get_mpz_t(), n1.intValue.get_mpz_t(),
					n2.intValue.get_mpz_t());
			break;
	}

	return Number(res, n1.bitWidth, n1.sign);
}

/**
* @brief Performs bit @c and on @a op1 and @a op2.
*/
Number bitAnd(const Number& op1, const Number& op2)
{
	return Number::performBitOp(op1, op2, 'A');
}

/**
* @brief Performs bit @c or on @a op1 and @a op2.
*/
Number bitOr(const Number& op1, const Number& op2)
{
	return Number::performBitOp(op1, op2, 'O');
}

/**
* @brief Performs bit @c xor on @a op1 and @a op2.
*/
Number bitXor(const Number& op1, const Number& op2)
{
	return Number::performBitOp(op1, op2, 'X');
}

/**
* @brief According to @a isLeft, it performs left or right shift on @a op1 and @a op2.
*/
Number Number::performShift(Number op1, Number op2, bool isLeft)
{
	// Bit left shift is performed only on integral numbers.
	assert(op1.isIntegral() && op2.isIntegral());

	// Bit shifts are performed only if the second operand is non-negative.
	// This is not true for the C language. Thus, approximation must be used in
	// the Range class.
	assert(op2.intValue >= 0);

	// Extension of the operands.
	op1.integralPromotion();
	op2.integralPromotion();

	// Bit shift is performed only if op2 is lower than the number of bits of
	// the op1's type. This is not true for the C language. Thus, approximation
	// is used in the Range class. It must be after integralPromotion()!
	assert(op1.bitWidth * CHAR_BIT > op2.intValue);

	// Shift are not defined for Int, so we have to use left shift from C.
	// We have to store Int values into C types.
	Number::Int res;
	if ((sizeof(int) == op1.bitWidth)) {
		if (op1.isSigned()) {
			int signedOP1, signedOP2;
			signedOP1 = mpz_get_si(op1.intValue.get_mpz_t());
			signedOP2 = mpz_get_si(op2.intValue.get_mpz_t());
			res = isLeft ? (signedOP1 << signedOP2) : (signedOP1 >> signedOP2);
		} else {
			unsigned signedOP1, signedOP2;
			signedOP1 = mpz_get_ui(op1.intValue.get_mpz_t());
			signedOP2 = mpz_get_ui(op2.intValue.get_mpz_t());
			res = isLeft ? (signedOP1 << signedOP2) : (signedOP1 >> signedOP2);
		}
	} else if ((sizeof(long) == op1.bitWidth)) {
		if (op1.isSigned()) {
			long signedOP1, signedOP2;
			signedOP1 = mpz_get_si(op1.intValue.get_mpz_t());
			signedOP2 = mpz_get_si(op2.intValue.get_mpz_t());
			res = isLeft ? (signedOP1 << signedOP2) : (signedOP1 >> signedOP2);
		} else {
			unsigned long signedOP1, signedOP2;
			signedOP1 = mpz_get_ui(op1.intValue.get_mpz_t());
			signedOP2 = mpz_get_ui(op2.intValue.get_mpz_t());
			res = isLeft ? (signedOP1 << signedOP2) : (signedOP1 >> signedOP2);
		}
	}

	Number result = Number(res, op1.bitWidth, op1.sign);
	result.fitIntoBitWidth();
	return result;
}

/**
* @brief Performs bit left shift on @a op1 and @a op2.
*
* Preconditions:
*  - @a op1 and @a op2 are integral numbers
*  - @a op2 is non-negative number
*  - @a op2 is lower than @c op1.bitWidth
*/
Number bitLeftShift(const Number& op1, const Number& op2)
{
	return Number::performShift(op1, op2, true);
}

/**
* @brief Performs bit right shift on @a op1 and @a op2.
*
* Preconditions:
*  - @a op1 and @a op2 are integral numbers
*  - @a op2 is non-negative number
*  - @a op2 is lower than @c op1.bitWidth
*/
Number bitRightShift(const Number& op1, const Number& op2)
{
	return Number::performShift(op1, op2, false);
}

/**
* @brief Gets absolute value of @a op. The given @a op has to be signed integral
*        type. Code Listener does not generate @c abs instruction for unsigned
*        integral type or floating type. Code Listener also performs integral
*        promotion. So, we do not use @c integralPromotion() function.
*/
Number abs(const Number& op)
{
	assert(op.isIntegral() && op.isSigned());

	// Remember that in the C language holds abs(min) == min.
	return (op.intValue >= 0 ? op : -op);
}

/**
* @brief Converts @a op from integral type to floating-point type.
*
* Preconditions:
*  - @a op is integral type
*/
Number intToFloat(const Number& op)
{
	assert(op.isIntegral());
	if (op.sign) {
		return Number((float) mpz_get_si(op.intValue.get_mpz_t()), sizeof(float));
	} else {
		return Number((float) mpz_get_ui(op.intValue.get_mpz_t()), sizeof(float));
	}
}

/**
* @brief Gets the minimum from @a op1 and @a op2.
*/
Number min(const Number &op1, const Number &op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	if (n1 <= n2) {
		return n1;
	} else {
		return n2;
	}
}

/**
* @brief Gets the maximum number from @a op1 and @a op2.
*/
Number max(const Number &op1, const Number &op2)
{
	// Extension of the operands.
	pair<Number, Number> r = Number::extensionByCRules(op1, op2);
	Number &n1 = r.first;
	Number &n2 = r.second;

	if (n1 >= n2) {
		return n1;
	} else {
		return n2;
	}
}

/**
* @brief Emits @a n into @a os.
*/
ostream& operator<<(ostream &os, const Number &n)
{
	if (n.isIntegral())
		os << n.intValue;
	else if (n.isFloatingPoint()) {
		os << n.floatValue;
	}
	return os;
}

// TODO: pointer plus
