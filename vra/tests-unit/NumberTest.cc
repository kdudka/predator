/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   NumberTest.cc
* @brief  Test class for class Number.
* @date   2012
*/

#include <utility>
#include <limits>
#include <climits>
#include <string>
#include <sstream>
#include <cmath>
#include <gmpxx.h>
#include "Number.h"
#include "gtest/gtest.h"

using namespace std;

// Minimal values
template <typename T> T vmin() { return numeric_limits<T>::min(); }
template <> float vmin() { return -numeric_limits<float>::max(); }
template <> double vmin() { return -numeric_limits<double>::max(); }
template <> long double vmin() { return -numeric_limits<long double>::max(); }

// Maximal values
template <typename T> T vmax() { return numeric_limits<T>::max(); }

// Number (int or float)
typedef Number N;

// Int
template <typename T>
Number I(T val) { return Number(val, sizeof(T), vmin<T>() != 0); }

// Float
template <typename T>
Number F(T val) { return Number(val, sizeof(T)); }

class NumberTest : public ::testing::Test
{
	protected:
		NumberTest() {}

		virtual ~NumberTest() {}

		virtual void SetUp() {}

		virtual void TearDown() {}
};

////////////////////////////////////////////////////////////////////////////////
// assign()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
AssignOfIntIntoIntOfSameTypeResultsIntoCorrectNumber)
{
	EXPECT_EQ(I<signed char>(1),
		I<signed char>(0).assign(I<signed char>(1)));
	EXPECT_EQ(I<unsigned char>(1),
		I<unsigned char>(0).assign(I<unsigned char>(1)));
	EXPECT_EQ(I<signed short>(1),
		I<signed short>(0).assign(I<signed short>(1)));
	EXPECT_EQ(I<unsigned short>(1),
		I<unsigned short>(0).assign(I<unsigned short>(1)));
	EXPECT_EQ(I<signed int>(1),
		I<signed int>(0).assign(I<signed int>(1)));
	EXPECT_EQ(I<unsigned int>(1),
		I<unsigned int>(0).assign(I<unsigned int>(1)));

	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(I<int>(vmin<int>())));
	EXPECT_EQ(I<int>(vmax<int>()),
		I<int>(0).assign(I<int>(vmax<int>())));
}

TEST_F(NumberTest,
AssignOfIntIntoIntOfDifferentTypeResultsIntoCorrectNumber)
{
	EXPECT_EQ(I<char>(vmin<int>()),
		I<char>(0).assign(I<int>(vmin<int>())));
	EXPECT_EQ(I<char>(vmin<char>()),
		I<char>(0).assign(I<int>(vmin<char>())));
	EXPECT_EQ(I<char>(-1),
		I<char>(0).assign(I<int>(-1)));
	EXPECT_EQ(I<char>(0),
		I<char>(0).assign(I<int>(0)));
	EXPECT_EQ(I<char>(1),
		I<char>(0).assign(I<int>(1)));
	EXPECT_EQ(I<char>(vmax<char>()),
		I<char>(0).assign(I<int>(vmax<char>())));
	EXPECT_EQ(I<char>(vmax<int>()),
		I<char>(0).assign(I<int>(vmax<int>())));
}

TEST_F(NumberTest,
AssignOfSignedIntoIntoUnsignedIntResultsIntoCorrectNumber)
{
	EXPECT_EQ(I<unsigned int>(-1),
		I<unsigned int>(0).assign(I<int>(-1)));
}

TEST_F(NumberTest,
AssignOfFloatIntoFloatOfSameTypeResultsIntoCorrectNumber)
{
	EXPECT_EQ(F<float>(1e30f),
		F<float>(0).assign(F<float>(1e30f)));
	EXPECT_EQ(F<double>(1e100),
		F<double>(0).assign(F<double>(1e100)));
	EXPECT_EQ(F<long double>(1e400L),
		F<long double>(0).assign(F<long double>(1e400L)));

	EXPECT_EQ(F<float>(INFINITY),
		F<float>(0).assign(F<float>(1e50)));
	EXPECT_EQ(F<double>(INFINITY),
		F<double>(0).assign(F<double>(1e400L)));

	EXPECT_EQ(F<float>(-INFINITY),
		F<float>(0).assign(F<float>(-INFINITY)));
	EXPECT_EQ(F<float>(INFINITY),
		F<float>(0).assign(F<float>(INFINITY)));
	EXPECT_TRUE(F<float>(0).assign(F<float>(NAN)).isNotNumber());

	EXPECT_EQ(F<double>(-INFINITY),
		F<double>(0).assign(F<double>(-INFINITY)));
	EXPECT_EQ(F<double>(INFINITY),
		F<double>(0).assign(F<double>(INFINITY)));
	EXPECT_TRUE(F<double>(0).assign(F<double>(NAN)).isNotNumber());

	EXPECT_EQ(F<long double>(-INFINITY),
		F<long double>(0).assign(F<long double>(-INFINITY)));
	EXPECT_EQ(F<long double>(INFINITY),
		F<long double>(0).assign(F<long double>(INFINITY)));
	EXPECT_TRUE(F<long double>(0).assign(F<long double>(NAN)).isNotNumber());
}

TEST_F(NumberTest,
AssignOfFloatIntoFloatOfDifferentTypeResultsIntoCorrectNumber)
{
	EXPECT_EQ(F<float>(-INFINITY),
		F<float>(0).assign(F<double>(-INFINITY)));
	EXPECT_EQ(F<float>(-INFINITY),
		F<float>(0).assign(F<double>(-1e200)));
	EXPECT_EQ(F<float>(-INFINITY),
		F<float>(0).assign(F<double>(-1e40)));
	EXPECT_EQ(F<float>(-1e20),
		F<float>(0).assign(F<double>(-1e20)));
	EXPECT_EQ(F<float>(0.0),
		F<float>(0).assign(F<double>(0.0)));
	EXPECT_EQ(F<float>(1e20),
		F<float>(0).assign(F<double>(1e20)));
	EXPECT_EQ(F<float>(INFINITY),
		F<float>(0).assign(F<double>(1e40)));
	EXPECT_EQ(F<float>(INFINITY),
		F<float>(0).assign(F<double>(1e200)));
	EXPECT_EQ(F<float>(INFINITY),
		F<float>(0).assign(F<double>(INFINITY)));
	EXPECT_TRUE(F<float>(0).assign(F<double>(NAN)).isNotNumber());

	EXPECT_EQ(F<double>(-INFINITY),
		F<double>(0).assign(F<long double>(-INFINITY)));
	EXPECT_EQ(F<double>(-INFINITY),
		F<double>(0).assign(F<long double>(-1e400L)));
	EXPECT_EQ(F<double>(-1e200),
		F<double>(0).assign(F<long double>(-1e200L)));
	EXPECT_EQ(F<double>(0.0),
		F<double>(0).assign(F<long double>(0.0)));
	EXPECT_EQ(F<double>(1e200),
		F<double>(0).assign(F<long double>(1e200L)));
	EXPECT_EQ(F<double>(INFINITY),
		F<double>(0).assign(F<long double>(1e400L)));
	EXPECT_EQ(F<double>(INFINITY),
		F<double>(0).assign(F<long double>(INFINITY)));
	EXPECT_TRUE(F<double>(0).assign(F<long double>(NAN)).isNotNumber());
}

TEST_F(NumberTest,
AssignOfFloatIntoIntResultsIntoCorrectNumber)
{
	// All floating-point numbers outside the range <INT_MIN, INT_MAX> get
	// converted to INT_MIN (see the comment in Number::assign()).
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(-INFINITY)));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(vmin<double>())));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(-1e20)));
	EXPECT_EQ(I<int>(-10),
		I<int>(0).assign(F<double>(-10.4)));
	EXPECT_EQ(I<int>(0),
		I<int>(0).assign(F<double>(0.0)));
	EXPECT_EQ(I<int>(10),
		I<int>(0).assign(F<double>(10.4)));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(1e20)));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(vmax<double>())));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(INFINITY)));
	EXPECT_EQ(I<int>(vmin<int>()),
		I<int>(0).assign(F<double>(NAN)));
}

TEST_F(NumberTest,
AssignOfIntIntoFloatResultsIntoCorrectNumber)
{
	// static_cast<> is used to suppress a warning
	EXPECT_EQ(F<double>(0.0),
		I<double>(0).assign(F<int>(0)));
}

////////////////////////////////////////////////////////////////////////////////
// isIntegral()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntegralNumberIsIntegral)
{
	EXPECT_TRUE(I<signed char>(0).isIntegral());
	EXPECT_TRUE(I<unsigned char>(0).isIntegral());
	EXPECT_TRUE(I<signed short>(0).isIntegral());
	EXPECT_TRUE(I<unsigned short>(0).isIntegral());
	EXPECT_TRUE(I<signed int>(0).isIntegral());
	EXPECT_TRUE(I<unsigned int>(0).isIntegral());
	EXPECT_TRUE(I<signed long>(0).isIntegral());
	EXPECT_TRUE(I<unsigned long>(0).isIntegral());
}

TEST_F(NumberTest,
FloatingPointNumberIsNotIntegral)
{
	EXPECT_FALSE(F<float>(0).isIntegral());
	EXPECT_FALSE(F<double>(0).isIntegral());
	EXPECT_FALSE(F<long double>(0).isIntegral());
}

////////////////////////////////////////////////////////////////////////////////
// isFloatingPoint()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntegralNumberIsNotFloatingPoint)
{
	EXPECT_FALSE(I<signed char>(0).isFloatingPoint());
	EXPECT_FALSE(I<unsigned char>(0).isFloatingPoint());
	EXPECT_FALSE(I<signed short>(0).isFloatingPoint());
	EXPECT_FALSE(I<unsigned short>(0).isFloatingPoint());
	EXPECT_FALSE(I<signed int>(0).isFloatingPoint());
	EXPECT_FALSE(I<unsigned int>(0).isFloatingPoint());
	EXPECT_FALSE(I<signed long>(0).isFloatingPoint());
	EXPECT_FALSE(I<unsigned long>(0).isFloatingPoint());
}

TEST_F(NumberTest,
FloatingPointNumberIsFloatingPoint)
{
	EXPECT_TRUE(F<float>(0).isFloatingPoint());
	EXPECT_TRUE(F<double>(0).isFloatingPoint());
	EXPECT_TRUE(F<long double>(0).isFloatingPoint());
}

////////////////////////////////////////////////////////////////////////////////
// isSigned()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
SignedIntegralNumberIsSigned)
{
	EXPECT_TRUE(I<signed char>(0).isSigned());
	EXPECT_TRUE(I<signed short>(0).isSigned());
	EXPECT_TRUE(I<signed int>(0).isSigned());
	EXPECT_TRUE(I<signed long>(0).isSigned());
}

TEST_F(NumberTest,
UnsignedIntegralNumberIsNotSigned)
{
	EXPECT_FALSE(I<unsigned char>(0).isSigned());
	EXPECT_FALSE(I<unsigned short>(0).isSigned());
	EXPECT_FALSE(I<unsigned int>(0).isSigned());
	EXPECT_FALSE(I<unsigned long>(0).isSigned());
}

TEST_F(NumberTest,
FloatingPointNumberIsNotSigned)
{
	EXPECT_FALSE(F<float>(0).isSigned());
	EXPECT_FALSE(F<double>(0).isSigned());
	EXPECT_FALSE(F<long double>(0).isSigned());
}

////////////////////////////////////////////////////////////////////////////////
// isUnsigned()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
UnsignedIntegralNumberIsUnsigned)
{
	EXPECT_TRUE(I<unsigned char>(0).isUnsigned());
	EXPECT_TRUE(I<unsigned short>(0).isUnsigned());
	EXPECT_TRUE(I<unsigned int>(0).isUnsigned());
	EXPECT_TRUE(I<unsigned long>(0).isUnsigned());
}

TEST_F(NumberTest,
SignedIntegralNumberIsNotUnsigned)
{
	EXPECT_FALSE(I<signed char>(0).isUnsigned());
	EXPECT_FALSE(I<signed short>(0).isUnsigned());
	EXPECT_FALSE(I<signed int>(0).isUnsigned());
	EXPECT_FALSE(I<signed long>(0).isUnsigned());
}

TEST_F(NumberTest,
FloatingPointNumberIsNotUnsigned)
{
	EXPECT_FALSE(F<float>(0).isUnsigned());
	EXPECT_FALSE(F<double>(0).isUnsigned());
	EXPECT_FALSE(F<long double>(0).isUnsigned());
}

////////////////////////////////////////////////////////////////////////////////
// isInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
FloatingPointInfinityIsInfinity)
{
	EXPECT_TRUE(F<float>(INFINITY).isInf());
	EXPECT_TRUE(F<float>(-INFINITY).isInf());
	EXPECT_TRUE(F<double>(INFINITY).isInf());
	EXPECT_TRUE(F<double>(-INFINITY).isInf());
	EXPECT_TRUE(F<long double>(INFINITY).isInf());
	EXPECT_TRUE(F<long double>(-INFINITY).isInf());
}

TEST_F(NumberTest,
NonInfinityFloatingPointIsNotInfinity)
{
	EXPECT_FALSE(F<float>(NAN).isInf());
	EXPECT_FALSE(F<double>(vmin<double>()).isInf());
	EXPECT_FALSE(F<long double>(vmax<double>()).isInf());
}

TEST_F(NumberTest,
IntegralNumberIsNotInfinity)
{
	EXPECT_FALSE(I<signed char>(vmin<signed char>()).isInf());
	EXPECT_FALSE(I<unsigned char>(vmax<unsigned char>()).isInf());
	EXPECT_FALSE(I<signed long>(vmin<signed long>()).isInf());
	EXPECT_FALSE(I<unsigned long>(vmax<unsigned long>()).isInf());
}

////////////////////////////////////////////////////////////////////////////////
// isNegativeInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
NegativeFloatingPointInfinityIsNegativeInfinity)
{
	EXPECT_TRUE(F<float>(-INFINITY).isNegativeInf());
	EXPECT_TRUE(F<double>(-INFINITY).isNegativeInf());
	EXPECT_TRUE(F<long double>(-INFINITY).isNegativeInf());
}

TEST_F(NumberTest,
PositiveFloatingPointInfinityIsNotNegativeInfinity)
{
	EXPECT_FALSE(F<float>(INFINITY).isNegativeInf());
	EXPECT_FALSE(F<double>(INFINITY).isNegativeInf());
	EXPECT_FALSE(F<long double>(INFINITY).isNegativeInf());
}

TEST_F(NumberTest,
NonInfinityFloatingPointIsNotNegativeInfinity)
{
	EXPECT_FALSE(F<float>(NAN).isNegativeInf());
	EXPECT_FALSE(F<double>(vmin<double>()).isNegativeInf());
	EXPECT_FALSE(F<long double>(vmax<double>()).isNegativeInf());
}

TEST_F(NumberTest,
IntegralNumberIsNotNegativeInfinity)
{
	EXPECT_FALSE(I<signed char>(vmin<signed char>()).isNegativeInf());
	EXPECT_FALSE(I<unsigned char>(vmax<unsigned char>()).isNegativeInf());
	EXPECT_FALSE(I<signed long>(vmin<signed long>()).isNegativeInf());
	EXPECT_FALSE(I<unsigned long>(vmax<unsigned long>()).isNegativeInf());
}

////////////////////////////////////////////////////////////////////////////////
// isPositiveInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
PositiveFloatingPointInfinityIsPositiveInfinity)
{
	EXPECT_TRUE(F<float>(INFINITY).isPositiveInf());
	EXPECT_TRUE(F<double>(INFINITY).isPositiveInf());
	EXPECT_TRUE(F<long double>(INFINITY).isPositiveInf());
}

TEST_F(NumberTest,
NegativeFloatingPointInfinityIsNotPositiveInfinity)
{
	EXPECT_FALSE(F<float>(-INFINITY).isPositiveInf());
	EXPECT_FALSE(F<double>(-INFINITY).isPositiveInf());
	EXPECT_FALSE(F<long double>(-INFINITY).isPositiveInf());
}

TEST_F(NumberTest,
NonInfinityFloatingPointIsNotPositiveInfinity)
{
	EXPECT_FALSE(F<float>(NAN).isPositiveInf());
	EXPECT_FALSE(F<double>(vmin<double>()).isPositiveInf());
	EXPECT_FALSE(F<long double>(vmax<double>()).isPositiveInf());
}

TEST_F(NumberTest,
IntegralNumberIsNotPositiveInfinity)
{
	EXPECT_FALSE(I<signed char>(vmin<signed char>()).isPositiveInf());
	EXPECT_FALSE(I<unsigned char>(vmax<unsigned char>()).isPositiveInf());
	EXPECT_FALSE(I<signed long>(vmin<signed long>()).isPositiveInf());
	EXPECT_FALSE(I<unsigned long>(vmax<unsigned long>()).isPositiveInf());
}

////////////////////////////////////////////////////////////////////////////////
// isNotNumber()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
NANIsNotNumber)
{
	EXPECT_TRUE(F<float>(NAN).isNotNumber());
	EXPECT_TRUE(F<float>(-NAN).isNotNumber());
	EXPECT_TRUE(F<double>(NAN).isNotNumber());
	EXPECT_TRUE(F<double>(-NAN).isNotNumber());
	EXPECT_TRUE(F<long double>(NAN).isNotNumber());
	EXPECT_TRUE(F<long double>(-NAN).isNotNumber());
}

TEST_F(NumberTest,
NonNANNumberIsNotNotNumber)
{
	EXPECT_FALSE(F<float>(-INFINITY).isNotNumber());
	EXPECT_FALSE(F<double>(-INFINITY).isNotNumber());
	EXPECT_FALSE(F<long double>(-INFINITY).isNotNumber());
	EXPECT_FALSE(I<signed char>(vmin<signed char>()).isNotNumber());
	EXPECT_FALSE(I<unsigned char>(vmax<unsigned char>()).isNotNumber());
	EXPECT_FALSE(I<signed long>(vmin<signed long>()).isNotNumber());
	EXPECT_FALSE(I<unsigned long>(vmax<unsigned long>()).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// isNumber()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IsNumberWorksCorrectly)
{
	EXPECT_TRUE((Number(1, sizeof(char), true)).isNumber());
	EXPECT_TRUE((Number(1, sizeof(float))).isNumber());
	EXPECT_FALSE(Number(INFINITY, sizeof(double)).isNumber());
	EXPECT_FALSE(Number(-INFINITY, sizeof(double)).isNumber());
	EXPECT_FALSE(Number(NAN, sizeof(double)).isNumber());
}

////////////////////////////////////////////////////////////////////////////////
// isMin()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
MinimalNumberIsMin)
{
	EXPECT_TRUE(I<signed char>(vmin<signed char>()).isMin());
	EXPECT_TRUE(I<unsigned char>(vmin<unsigned char>()).isMin());
	EXPECT_TRUE(I<signed short>(vmin<signed short>()).isMin());
	EXPECT_TRUE(I<unsigned short>(vmin<unsigned short>()).isMin());
	EXPECT_TRUE(I<signed int>(vmin<signed int>()).isMin());
	EXPECT_TRUE(I<unsigned int>(vmin<unsigned int>()).isMin());
	EXPECT_TRUE(I<signed long>(vmin<signed long>()).isMin());
	EXPECT_TRUE(I<unsigned long>(vmin<unsigned long>()).isMin());
	EXPECT_TRUE(F<float>(vmin<float>()).isMin());
	EXPECT_TRUE(F<double>(vmin<double>()).isMin());
	EXPECT_TRUE(F<long double>(vmin<long double>()).isMin());
}

TEST_F(NumberTest,
NotMinimalNumberIsNotMin)
{
	EXPECT_FALSE(I<signed char>(-127).isMin());
	EXPECT_FALSE(I<unsigned char>(1).isMin());
	EXPECT_FALSE(I<signed short>(0).isMin());
	EXPECT_FALSE(I<unsigned short>(vmax<unsigned short>()).isMin());
	EXPECT_FALSE(F<float>(0.0).isMin());
	EXPECT_FALSE(F<double>(-NAN).isMin());
	EXPECT_FALSE(F<long double>(-INFINITY).isMin());
}

////////////////////////////////////////////////////////////////////////////////
// isMax()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
MaximalNumberIsMax)
{
	EXPECT_TRUE(I<signed char>(vmax<signed char>()).isMax());
	EXPECT_TRUE(I<unsigned char>(vmax<unsigned char>()).isMax());
	EXPECT_TRUE(I<signed short>(vmax<signed short>()).isMax());
	EXPECT_TRUE(I<unsigned short>(vmax<unsigned short>()).isMax());
	EXPECT_TRUE(I<signed int>(vmax<signed int>()).isMax());
	EXPECT_TRUE(I<unsigned int>(vmax<unsigned int>()).isMax());
	EXPECT_TRUE(I<signed long>(vmax<signed long>()).isMax());
	EXPECT_TRUE(I<unsigned long>(vmax<unsigned long>()).isMax());
	EXPECT_TRUE(F<float>(vmax<float>()).isMax());
	EXPECT_TRUE(F<double>(vmax<double>()).isMax());
	EXPECT_TRUE(F<long double>(vmax<long double>()).isMax());
}

TEST_F(NumberTest,
NotMaximalNumberIsNotMax)
{
	EXPECT_FALSE(I<signed char>(126).isMax());
	EXPECT_FALSE(I<unsigned char>(1).isMax());
	EXPECT_FALSE(I<signed short>(vmin<signed short>()).isMax());
	EXPECT_FALSE(I<unsigned short>(0).isMax());
	EXPECT_FALSE(F<float>(0.0).isMax());
	EXPECT_FALSE(F<double>(NAN).isMax());
	EXPECT_FALSE(F<long double>(INFINITY).isMax());
}

////////////////////////////////////////////////////////////////////////////////
// getEpsilon()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetLowestFloatingPointReturnsCorrectValue)
{
	EXPECT_EQ(I<char>(1), I<char>(0).getEpsilon());
	EXPECT_EQ(I<int>(1), I<int>(0).getEpsilon());
	EXPECT_EQ(I<unsigned>(1), I<unsigned>(0).getEpsilon());

	EXPECT_EQ(F<float>(std::numeric_limits<float>::min()),
		F<float>(0).getEpsilon());
	EXPECT_EQ(F<double>(std::numeric_limits<double>::min()),
		F<double>(0).getEpsilon());
	EXPECT_EQ(F<long double>(std::numeric_limits<long double>::min()),
		F<long double>(0).getEpsilon());
}

////////////////////////////////////////////////////////////////////////////////
// getMin()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetMinReturnsCorrectlyMinValue)
{
	EXPECT_EQ(I<char>(vmin<char>()), I<char>(0).getMin());
	EXPECT_EQ(I<short>(vmin<short>()), I<short>(0).getMin());
	EXPECT_EQ(I<int>(vmin<int>()), I<int>(0).getMin());
	EXPECT_EQ(I<long>(vmin<long>()), I<long>(0).getMin());
	EXPECT_EQ(F<float>(vmin<float>()), F<float>(0).getMin());
	EXPECT_EQ(F<double>(vmin<double>()), F<double>(0).getMin());
	EXPECT_EQ(F<long double>(vmin<long double>()),
		F<long double>(0).getMin());
}

////////////////////////////////////////////////////////////////////////////////
// getMax()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetMaxReturnsCorrectlyMaxValue)
{
	EXPECT_EQ(I<char>(vmax<char>()), I<char>(0).getMax());
	EXPECT_EQ(I<short>(vmax<short>()), I<short>(0).getMax());
	EXPECT_EQ(I<int>(vmax<int>()), I<int>(0).getMax());
	EXPECT_EQ(I<long>(vmax<long>()), I<long>(0).getMax());
	EXPECT_EQ(F<float>(vmax<float>()), F<float>(0).getMax());
	EXPECT_EQ(F<double>(vmax<double>()), F<double>(0).getMax());
	EXPECT_EQ(F<long double>(vmax<long double>()),
		F<long double>(0).getMax());
}

////////////////////////////////////////////////////////////////////////////////
// getNan()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetNanOfIntegralNumberReturnsNanCorrectly)
{
	EXPECT_DEATH((I<char>(vmax<char>())).getNan(), ".*isFloatingPoint().*");
}

TEST_F(NumberTest,
GetNanOfFloatingPointNumberReturnsNanCorrectly)
{
	EXPECT_TRUE(((F<double>(vmax<double>())).getNan()).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// getPositiveInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetPositiveInfOfIntegralNumberReturnsPositiveInfCorrectly)
{
	EXPECT_DEATH((I<int>(vmin<int>())).getPositiveInf(), ".*isFloatingPoint().*");
}

TEST_F(NumberTest,
GetPositiveInfOfFloatingPointNumberReturnsPositiveInfCorrectly)
{
	EXPECT_EQ(F<double>(INFINITY), (F<double>(vmin<double>())).getPositiveInf());
}

////////////////////////////////////////////////////////////////////////////////
// getNegativeInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetNegativeInfOfIntegralNumberReturnsNegativeInfCorrectly)
{
	EXPECT_DEATH((I<long>(vmax<long>())).getNegativeInf(), ".*isFloatingPoint().*");
}

TEST_F(NumberTest,
GetNegativeInfOfFloatingPointNumberReturnsNegativeInfCorrectly)
{
	EXPECT_EQ(F<double>(-INFINITY), (F<double>(vmin<double>())).getNegativeInf());
}

////////////////////////////////////////////////////////////////////////////////
// getInt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetIntOfIntegralNumberWorksCorrectly)
{
	EXPECT_EQ(mpz_class(vmin<int>()), (I<int>(vmin<int>())).getInt());
}

TEST_F(NumberTest,
GetIntOfFloatingPointNumberWorksCorrectly)
{
	EXPECT_DEATH((F<double>(vmin<double>())).getInt(), ".*isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// getFloat()
////////////////////////////////////////////////////////////////////////////////
TEST_F(NumberTest,
GetFloatOfIntegralNumberWorksCorrectly)
{
	EXPECT_DEATH((I<int>(vmin<int>())).getFloat(), ".*isFloatingPoint().*");
}

TEST_F(NumberTest,
GetFloatOfFloatingPointNumberWorksCorrectly)
{
	EXPECT_EQ(vmin<double>(), (F<double>(vmin<double>())).getFloat());
}

////////////////////////////////////////////////////////////////////////////////
// getBitWidth()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetBitWidthWorksCorrectly)
{
	EXPECT_EQ(sizeof(char), (Number(1, sizeof(char), true)).getBitWidth());
	EXPECT_EQ(sizeof(short), (Number(1, sizeof(short), false)).getBitWidth());
	EXPECT_EQ(sizeof(int), (Number(1, sizeof(int), true)).getBitWidth());
	EXPECT_EQ(sizeof(long), (Number(1, sizeof(long), false)).getBitWidth());

	EXPECT_EQ(sizeof(float), (Number(1.0, sizeof(float))).getBitWidth());
	EXPECT_EQ(sizeof(double), (Number(1.0, sizeof(double))).getBitWidth());
	EXPECT_EQ(sizeof(long double), (Number(1.0, sizeof(long double))).getBitWidth());
}

////////////////////////////////////////////////////////////////////////////////
// getNumOfBits()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetNumOfBitsWorksCorrectly)
{
	EXPECT_EQ(sizeof(char) * CHAR_BIT, (Number(1, sizeof(char), true)).getNumOfBits());
	EXPECT_EQ(sizeof(short) * CHAR_BIT, (Number(1, sizeof(short), false)).getNumOfBits());
	EXPECT_EQ(sizeof(int) * CHAR_BIT, (Number(1, sizeof(int), true)).getNumOfBits());
	EXPECT_EQ(sizeof(long) * CHAR_BIT, (Number(1, sizeof(long), false)).getNumOfBits());

	EXPECT_EQ(sizeof(float) * CHAR_BIT, (Number(1.0, sizeof(float))).getNumOfBits());
	EXPECT_EQ(sizeof(double) * CHAR_BIT, (Number(1.0, sizeof(double))).getNumOfBits());
	EXPECT_EQ(sizeof(long double) * CHAR_BIT, (Number(1.0, sizeof(long double))).getNumOfBits());
}

////////////////////////////////////////////////////////////////////////////////
// getSign()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GetSignOfIntegralNumberWorksCorrectly)
{
	EXPECT_TRUE((Number(1, sizeof(char), true)).getSign());
	EXPECT_FALSE((Number(1, sizeof(short), false)).getSign());
	EXPECT_TRUE((Number(1, sizeof(int), true)).getSign());
	EXPECT_FALSE((Number(1, sizeof(long), false)).getSign());

	EXPECT_DEATH((Number(1.0, sizeof(float))).getSign(), ".*isIntegral().*");
	EXPECT_DEATH((Number(1.0, sizeof(double))).getSign(), ".*isIntegral().*");
	EXPECT_DEATH((Number(1.0, sizeof(long double))).getSign(), ".*isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// floatToInt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
FloatToIntConversionWorksCorrectly)
{
	EXPECT_EQ(mpz_class(1), Number::floatToInt(1.35));
	EXPECT_EQ(mpz_class(1), Number::floatToInt(1.99));
	// TODO: append test for inf, -inf, nan, too large float
	// EXPECT_EQ(mpz_class(?), Number::floatToInt(INFINITY));
	// EXPECT_EQ(mpz_class(?), Number::floatToInt(-INFINITY));
	// EXPECT_EQ(mpz_class(?), Number::floatToInt(NAN));
	// EXPECT_EQ(mpz_class(?), Number::floatToInt(F<long double>(vmax<long double>())));
}

////////////////////////////////////////////////////////////////////////////////
// ==, !=
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntsWithSameValueAndBitWidthAndSignessAreEqual)
{
	EXPECT_EQ(I<signed char>(vmin<signed char>()),
		I<signed char>(vmin<signed char>()));
	EXPECT_EQ(I<short>(-1), I<short>(-1));
	EXPECT_EQ(I<int>(1), I<int>(1));
	EXPECT_EQ(I<long>(vmax<long>()), I<long>(vmax<long>()));
}

TEST_F(NumberTest,
IntsWithSameValueAndBitWidthButDifferentSignessAreEqual)
{
	EXPECT_EQ(I<char>(0), I<unsigned char>(0));
	EXPECT_EQ(I<short>(1), I<unsigned short>(1));
	EXPECT_EQ(I<int>(2), I<unsigned int>(2));
	EXPECT_EQ(I<long>(-1), I<unsigned long>(vmax<unsigned long>()));
	EXPECT_EQ(I<long>(vmax<long>()), I<long>(vmax<long>()));
}

TEST_F(NumberTest,
IntsWithSameValueAndSignessButDifferentBitWidthAreEqual)
{
	EXPECT_EQ(I<signed char>(vmin<signed char>()),
		I<short>(vmin<signed char>()));
	EXPECT_EQ(I<short>(-1), I<int>(-1));
	EXPECT_EQ(I<int>(1), I<long>(1));
}

TEST_F(NumberTest,
IntsWithSameValueButDifferentBitWidthAndSignessAreEqual)
{
	EXPECT_EQ(I<signed char>(0), I<unsigned short>(0));
	EXPECT_EQ(I<short>(1), I<unsigned int>(1));
	EXPECT_EQ(I<int>(vmax<int>()), I<unsigned long>(vmax<int>()));
}

TEST_F(NumberTest,
IntsWithDifferentValueAreNotEqual)
{
	EXPECT_NE(I<char>(-10), I<char>(-9));
	EXPECT_NE(I<short>(-1), I<short>(1));
	EXPECT_NE(I<int>(vmin<int>()), I<int>(vmax<int>()));
	EXPECT_NE(I<long>(10), I<long>(0));
}

TEST_F(NumberTest,
FloatsWithSameValueAndBitWidthAreEqual)
{
	EXPECT_EQ(F<float>(-1), F<float>(-1));
	EXPECT_EQ(F<float>(-INFINITY), F<float>(-INFINITY));
	EXPECT_EQ(F<float>(INFINITY), F<float>(INFINITY));

	EXPECT_EQ(F<double>(0), F<double>(0));
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY));
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY));

	EXPECT_EQ(F<long double>(1), F<long double>(1));
	EXPECT_EQ(F<long double>(-INFINITY), F<long double>(-INFINITY));
	EXPECT_EQ(F<long double>(INFINITY), F<long double>(INFINITY));
}

TEST_F(NumberTest,
FloatsWithSameValueButDifferentBitWidthAreEqual)
{
	EXPECT_EQ(F<float>(-1), F<double>(-1));
	EXPECT_EQ(F<float>(-INFINITY), F<double>(-INFINITY));
	EXPECT_EQ(F<float>(INFINITY), F<long double>(INFINITY));

	EXPECT_EQ(F<double>(0), F<float>(0));
	EXPECT_EQ(F<double>(-INFINITY), F<float>(-INFINITY));
	EXPECT_EQ(F<double>(INFINITY), F<long double>(INFINITY));

	EXPECT_EQ(F<long double>(1), F<float>(1));
	EXPECT_EQ(F<long double>(-INFINITY), F<float>(-INFINITY));
	EXPECT_EQ(F<long double>(INFINITY), F<double>(INFINITY));
}

TEST_F(NumberTest,
FloatsWithDifferentValueAreNotEqual)
{
	EXPECT_NE(F<float>(-10.6), F<float>(0));
	EXPECT_NE(F<float>(vmin<float>()), F<float>(-INFINITY));
	EXPECT_NE(F<float>(vmax<float>()), F<float>(INFINITY));

	EXPECT_NE(F<double>(20.678), F<double>(-45));
	EXPECT_NE(F<double>(vmin<double>()), F<double>(-INFINITY));
	EXPECT_NE(F<double>(vmax<double>()), F<double>(INFINITY));

	EXPECT_NE(F<long double>(-14.125), F<long double>(14.125));
	EXPECT_NE(F<long double>(vmin<long double>()), F<long double>(-INFINITY));
	EXPECT_NE(F<long double>(vmax<long double>()), F<long double>(INFINITY));
}

TEST_F(NumberTest,
NanIsNotEqualToAnythingElse)
{
	EXPECT_NE(F<float>(NAN), F<float>(vmin<float>()));
	EXPECT_NE(F<float>(NAN), F<float>(vmax<float>()));
	EXPECT_NE(F<float>(NAN), F<float>(NAN));
	EXPECT_NE(F<float>(NAN), F<float>(-INFINITY));
	EXPECT_NE(F<float>(NAN), F<float>(INFINITY));

	EXPECT_NE(F<double>(NAN), F<double>(vmin<double>()));
	EXPECT_NE(F<double>(NAN), F<double>(vmax<double>()));
	EXPECT_NE(F<double>(NAN), F<double>(NAN));
	EXPECT_NE(F<double>(NAN), F<double>(-INFINITY));
	EXPECT_NE(F<double>(NAN), F<double>(INFINITY));

	EXPECT_NE(F<long double>(NAN), F<long double>(vmin<long double>()));
	EXPECT_NE(F<long double>(NAN), F<long double>(vmax<long double>()));
	EXPECT_NE(F<long double>(NAN), F<long double>(NAN));
	EXPECT_NE(F<long double>(NAN), F<long double>(-INFINITY));
	EXPECT_NE(F<long double>(NAN), F<long double>(INFINITY));
}

////////////////////////////////////////////////////////////////////////////////
// promotion
////////////////////////////////////////////////////////////////////////////////

// We use unary minus and getMax() to test the promotion.

TEST_F(NumberTest,
CharIsPromotedToInt)
{
	EXPECT_EQ(I<int>(vmax<int>()), (-I<signed char>(0)).getMax());
	EXPECT_EQ(I<int>(vmax<int>()), (-I<unsigned char>(0)).getMax());
}

TEST_F(NumberTest,
ShortIsPromotedToIntIfItIsOfLowerBitWidthThanInt)
{
	if (sizeof(short) < sizeof(int)) {
		EXPECT_EQ(I<int>(vmax<int>()), (-I<signed short>(0)).getMax());
		EXPECT_EQ(I<int>(vmax<int>()), (-I<unsigned short>(0)).getMax());
	} else {
		EXPECT_EQ(I<int>(vmax<int>()), (-I<signed short>(0)).getMax());
		EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()),
			(-I<unsigned short>(0)).getMax());
	}
}

TEST_F(NumberTest,
IntDoesNotGetPromotedToAnythingElse)
{
	EXPECT_EQ(I<int>(vmax<int>()), (-I<signed int>(0)).getMax());
	EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()),
		(-I<unsigned int>(0)).getMax());
}

TEST_F(NumberTest,
LongDoesNotGetPromotedToAnythingElse)
{
	EXPECT_EQ(I<long>(vmax<long>()), (-I<signed long>(0)).getMax());
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()),
		(-I<unsigned long>(0)).getMax());
}

TEST_F(NumberTest,
FloatsDoNotGetPromotedToAnything)
{
	EXPECT_EQ(F<float>(vmax<float>()), (-F<float>(0)).getMax());
	EXPECT_EQ(F<double>(vmax<double>()), (-F<double>(0)).getMax());
	EXPECT_EQ(F<long double>(vmax<long double>()),
		(-F<long double>(0)).getMax());
}

////////////////////////////////////////////////////////////////////////////////
// extension
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
CharOpCharResultsIntoSignedInt)
{
	EXPECT_EQ(I<int>(vmax<int>()),
		(I<signed char>(0) + I<signed char>(0)).getMax());

	EXPECT_EQ(I<int>(vmax<int>()),
		(I<unsigned char>(0) + I<unsigned char>(0)).getMax());
}

TEST_F(NumberTest,
SignedShortOpSignedShortResultsIntoSignedInt)
{
	EXPECT_EQ(I<int>(vmax<int>()),
		(I<short>(0) + I<short>(0)).getMax());
}

TEST_F(NumberTest,
UnsignedShortOpUnsignedShortResultsInEitherIntOrUnsignedInt)
{
	Number n((I<unsigned short>(0) + I<unsigned short>(0)).getMax());
	if (sizeof(short) < sizeof(int)) {
		EXPECT_EQ(I<int>(vmax<int>()), n);
	} else {
		EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()), n);
	}
}

TEST_F(NumberTest,
SignedIntOpUnsignedIntResultsIntoUnsignedInt)
{
	EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()),
		(I<int>(0) + I<unsigned int>(0)).getMax());
}

TEST_F(NumberTest,
SignedIntOpSignedIntResultsIntoSignedInt)
{
	EXPECT_EQ(I<int>(vmax<int>()),
		(I<int>(0) + I<int>(0)).getMax());
}

TEST_F(NumberTest,
UnsignedIntOpUnsignedIntResultsIntoUnsignedInt)
{
	EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()),
		(I<unsigned int>(0) + I<unsigned int>(0)).getMax());
}

TEST_F(NumberTest,
SignedIntOpSignedLongResultsIntoLong)
{
	EXPECT_EQ(I<long>(vmax<long>()),
		(I<int>(0) + I<long>(0)).getMax());
}

TEST_F(NumberTest,
UnsignedIntOpSignedLongResultsIntoEitherSignedOrUnsignedLong)
{
	Number n((I<unsigned int>(0) + I<long>(0)).getMax());
	if (sizeof(int) < sizeof(long)) {
		EXPECT_EQ(I<long>(vmax<long>()), n);
	} else {
		EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()), n);
	}
}

TEST_F(NumberTest,
SignedLongOpSignedLongResultsIntoSignedLong)
{
	EXPECT_EQ(I<long>(vmax<long>()),
		(I<long>(0) + I<long>(0)).getMax());
}

TEST_F(NumberTest,
SignedLongOpUnsignedLongResultsIntoUnsignedLong)
{
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()),
		(I<long>(0) + I<unsigned long>(0)).getMax());
}

TEST_F(NumberTest,
UnsignedLongOpUnsignedLongResultsIntoUnsignedLong)
{
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()),
		(I<unsigned long>(0) + I<unsigned long>(0)).getMax());
}

TEST_F(NumberTest,
IntOpFloatResultsIntoFloat)
{
	EXPECT_EQ(F<float>(vmax<float>()),
		(I<int>(0) + F<float>(0)).getMax());

	EXPECT_EQ(F<float>(vmax<float>()),
		(I<unsigned int>(0) + F<float>(0)).getMax());
}

TEST_F(NumberTest,
IntOpDoubleResultsIntoDouble)
{
	EXPECT_EQ(F<double>(vmax<double>()),
		(I<int>(0) + F<double>(0)).getMax());

	EXPECT_EQ(F<double>(vmax<double>()),
		(I<unsigned int>(0) + F<double>(0)).getMax());
}

TEST_F(NumberTest,
IntOpLongDoubleResultsIntoLongDouble)
{
	EXPECT_EQ(F<long double>(vmax<long double>()),
		(I<int>(0) + F<long double>(0)).getMax());

	EXPECT_EQ(F<long double>(vmax<long double>()),
		(I<unsigned int>(0) + F<long double>(0)).getMax());
}

TEST_F(NumberTest,
FloatOpFloatResultsIntoFloat)
{
	EXPECT_EQ(F<float>(vmax<float>()),
		(F<float>(0) + F<float>(0)).getMax());
}

TEST_F(NumberTest,
FloatOpDoubleResultsIntoDouble)
{
	EXPECT_EQ(F<double>(vmax<double>()),
		(F<float>(0) + F<double>(0)).getMax());
}

TEST_F(NumberTest,
FloatOpLongDoubleResultsIntoLongDouble)
{
	EXPECT_EQ(F<long double>(vmax<long double>()),
		(F<float>(0) + F<long double>(0)).getMax());
}

TEST_F(NumberTest,
DoubleOpDoubleResultsIntoDouble)
{
	EXPECT_EQ(F<double>(vmax<double>()),
		(F<double>(0) + F<double>(0)).getMax());
}

TEST_F(NumberTest,
DoubleOpLongDoubleResultsIntoLongDouble)
{
	EXPECT_EQ(F<long double>(vmax<long double>()),
		(F<double>(0) + F<long double>(0)).getMax());
}

TEST_F(NumberTest,
LongDoubleOpLongDoubleResultsIntoLongDouble)
{
	EXPECT_EQ(F<long double>(vmax<long double>()),
		(F<long double>(0) + F<long double>(0)).getMax());
}

////////////////////////////////////////////////////////////////////////////////
// - (unary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
NegatedZeroIsZero)
{
	EXPECT_EQ(I<int>(0), -I<signed char>(0));
	EXPECT_EQ(I<int>(0), -I<short>(0));
	EXPECT_EQ(I<int>(0), -I<int>(0));
	EXPECT_EQ(I<long>(0), -I<long>(0));
	EXPECT_EQ(F<float>(0), -F<float>(0));
	EXPECT_EQ(F<double>(0), -F<double>(0));
	EXPECT_EQ(F<long double>(0), -F<long double>(0));
}

TEST_F(NumberTest,
NegatedSignedIntGetsCorrectlyNegated)
{
	EXPECT_EQ(I<int>(-1), -I<signed char>(1));
	EXPECT_EQ(I<int>(-1), -I<short>(1));
	EXPECT_EQ(I<int>(-1), -I<int>(1));
	EXPECT_EQ(I<long>(-1), -I<long>(1));

	EXPECT_EQ(I<int>(-vmax<signed char>()),
		-I<signed char>(vmax<signed char>()));
	EXPECT_EQ(I<int>(-vmax<short>()), -I<short>(vmax<short>()));
	EXPECT_EQ(I<int>(-vmax<int>()), -I<int>(vmax<int>()));
	EXPECT_EQ(I<long>(-vmax<long>()), -I<long>(vmax<long>()));
}

TEST_F(NumberTest,
NegatedUnsignedIntIsStillUnsigned)
{
	// We do not test char and short because they are promoted to int.

	EXPECT_EQ(I<unsigned int>(vmax<unsigned int>()), -I<unsigned int>(1));
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()), -I<unsigned long>(1));

	EXPECT_EQ(I<unsigned int>(vmin<unsigned>()), -I<unsigned int>(vmin<unsigned>()));
}

TEST_F(NumberTest,
NegatedMinimalSignedIntIsStillMinimalSignedInt)
{
	// We do not test char and short because they are promoted to int.

	EXPECT_EQ(I(vmin<int>()), -I<int>(vmin<int>()));
	EXPECT_EQ(I(vmin<long>()), -I<long>(vmin<long>()));
}

TEST_F(NumberTest,
NegatedFloatGetsCorrectlyNegated)
{
	EXPECT_EQ(F<float>(-1), -F<float>(1));
	EXPECT_EQ(F<float>(1), -F<float>(-1));
	EXPECT_EQ(F<float>(-INFINITY), -F<float>(INFINITY));
	EXPECT_EQ(F<float>(INFINITY), -F<float>(-INFINITY));

	EXPECT_EQ(F<double>(-1), -F<double>(1));
	EXPECT_EQ(F<double>(1), -F<double>(-1));
	EXPECT_EQ(F<double>(-INFINITY), -F<double>(INFINITY));
	EXPECT_EQ(F<double>(INFINITY), -F<double>(-INFINITY));

	EXPECT_EQ(F<long double>(-1), -F<long double>(1));
	EXPECT_EQ(F<long double>(1), -F<long double>(-1));
	EXPECT_EQ(F<long double>(-INFINITY), -F<long double>(INFINITY));
	EXPECT_EQ(F<long double>(INFINITY), -F<long double>(-INFINITY));
}

TEST_F(NumberTest,
NegatedNanIsStilNan)
{
	// Since NAN differs from everything (even from another NAN), we have to
	// test this by checking that the result differs from any "reasonable"
	// number (there is no time to check all non-NAN numbers).

	EXPECT_NE(F<float>(-NAN), -F<float>(NAN));
	EXPECT_NE(F<float>(NAN), -F<float>(-NAN));
	EXPECT_NE(F<float>(INFINITY), -F<float>(NAN));
	EXPECT_NE(F<float>(-INFINITY), -F<float>(NAN));
	EXPECT_NE(F<float>(0), -F<float>(NAN));

	EXPECT_NE(F<double>(-NAN), -F<double>(NAN));
	EXPECT_NE(F<double>(NAN), -F<double>(-NAN));
	EXPECT_NE(F<double>(INFINITY), -F<double>(NAN));
	EXPECT_NE(F<double>(-INFINITY), -F<double>(NAN));
	EXPECT_NE(F<double>(0), -F<double>(NAN));

	EXPECT_NE(F<long double>(-NAN), -F<long double>(NAN));
	EXPECT_NE(F<long double>(NAN), -F<long double>(-NAN));
	EXPECT_NE(F<long double>(INFINITY), -F<long double>(NAN));
	EXPECT_NE(F<long double>(-INFINITY), -F<long double>(NAN));
	EXPECT_NE(F<long double>(0), -F<long double>(NAN));
}

////////////////////////////////////////////////////////////////////////////////
// <
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
LesserSignedIntIsLessThanGreaterSignedInt)
{
	EXPECT_LT(I<int>(vmin<int>()), I<int>(0));
	EXPECT_LT(I<int>(-100), I<int>(-40));
	EXPECT_LT(I<int>(-1), I<int>(1));
	EXPECT_LT(I<int>(4567), I<int>(vmax<int>()));
}

TEST_F(NumberTest,
UnsignedIntOneIsLessThanNegativeSignedIntOne)
{
	EXPECT_LT(I<unsigned int>(1), I<int>(-1));
}

TEST_F(NumberTest,
LesserFloatIsLessThanGreaterFloat)
{
	EXPECT_LT(F<double>(-INFINITY), F<double>(vmin<double>()));
	EXPECT_LT(F<double>(vmin<double>()), F<double>(-4356.58));
	EXPECT_LT(F<double>(-12), F<double>(0));
	EXPECT_LT(F<double>(10.0), F<double>(12.0));
	EXPECT_LT(F<double>(456.89), F<double>(10e10));
	EXPECT_LT(F<double>(21e10), F<double>(vmax<double>()));
	EXPECT_LT(F<double>(vmax<double>()), F<double>(INFINITY));
}

TEST_F(NumberTest,
NanIsNotLessThanAnyOtherFloat)
{
	EXPECT_FALSE(F<double>(NAN) < F<double>(-INFINITY));
	EXPECT_FALSE(F<double>(NAN) < F<double>(-45.7));
	EXPECT_FALSE(F<double>(NAN) < F<double>(0));
	EXPECT_FALSE(F<double>(NAN) < F<double>(10e10));
	EXPECT_FALSE(F<double>(NAN) < F<double>(INFINITY));
	EXPECT_FALSE(F<double>(NAN) < F<double>(NAN));
}

////////////////////////////////////////////////////////////////////////////////
// <=
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
LesserSignedIntIsLessOrEqualToGreaterSignedInt)
{
	EXPECT_LE(I<int>(vmin<int>()), I<int>(0));
	EXPECT_LE(I<int>(-100), I<int>(-40));
	EXPECT_LE(I<int>(-1), I<int>(1));
	EXPECT_LE(I<int>(4567), I<int>(vmax<int>()));
}

TEST_F(NumberTest,
UnsignedIntOneIsLessThanOrEqualToNegativeSignedIntOne)
{
	EXPECT_LE(I<unsigned int>(1), I<int>(-1));
}

TEST_F(NumberTest,
LesserFloatIsLessThanOrEqualToGreaterFloat)
{
	EXPECT_LE(F<double>(-INFINITY), F<double>(vmin<double>()));
	EXPECT_LE(F<double>(vmin<double>()), F<double>(-4356.58));
	EXPECT_LE(F<double>(-12), F<double>(0));
	EXPECT_LE(F<double>(10.0), F<double>(12.0));
	EXPECT_LE(F<double>(456.89), F<double>(10e10));
	EXPECT_LE(F<double>(21e10), F<double>(vmax<double>()));
	EXPECT_LE(F<double>(vmax<double>()), F<double>(INFINITY));
}

TEST_F(NumberTest,
NanIsNotLessThanOrEqualToAnyOtherFloat)
{
	EXPECT_FALSE(F<double>(NAN) < F<double>(-INFINITY));
	EXPECT_FALSE(F<double>(NAN) < F<double>(-45.7));
	EXPECT_FALSE(F<double>(NAN) < F<double>(0));
	EXPECT_FALSE(F<double>(NAN) < F<double>(10e10));
	EXPECT_FALSE(F<double>(NAN) < F<double>(INFINITY));
	EXPECT_FALSE(F<double>(NAN) < F<double>(NAN));
}

TEST_F(NumberTest,
NumberIsLessThanOrEqualToTheSameNumber)
{
	EXPECT_LE(I<int>(vmin<int>()), I<int>(vmin<int>()));
	EXPECT_LE(I<int>(0), I<int>(0));
	EXPECT_LE(I<int>(vmax<int>()), I<int>(vmax<int>()));

	EXPECT_LE(F<double>(vmin<double>()), F<double>(vmin<double>()));
	EXPECT_LE(F<double>(0), F<double>(0));
	EXPECT_LE(F<double>(vmax<double>()), F<double>(vmax<double>()));
}

////////////////////////////////////////////////////////////////////////////////
// >
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GreaterSignedIntIsGreaterThanLesserSignedInt)
{
	EXPECT_GT(I<int>(0), I<int>(vmin<int>()));
	EXPECT_GT(I<int>(-40), I<int>(-100));
	EXPECT_GT(I<int>(1), I<int>(-1));
	EXPECT_GT(I<int>(vmax<int>()), I<int>(4567));
}

TEST_F(NumberTest,
NegativeSignedIntOneIsGreaterThanUnsignedIntOne)
{
	EXPECT_GT(I<int>(-1), I<unsigned int>(1));
}

TEST_F(NumberTest,
GreaterFloatIsGreaterThanLesserFloat)
{
	EXPECT_GT(F<double>(vmin<double>()), F<double>(-INFINITY));
	EXPECT_GT(F<double>(-4356.58), F<double>(vmin<double>()));
	EXPECT_GT(F<double>(0), F<double>(-12));
	EXPECT_GT(F<double>(12.0), F<double>(10.0));
	EXPECT_GT(F<double>(10e10), F<double>(456.89));
	EXPECT_GT(F<double>(vmax<double>()), F<double>(21e10));
	EXPECT_GT(F<double>(INFINITY), F<double>(vmax<double>()));
}

TEST_F(NumberTest,
NanIsNotGreaterThanAnyOtherFloat)
{
	EXPECT_FALSE(F<double>(NAN) > F<double>(-INFINITY));
	EXPECT_FALSE(F<double>(NAN) > F<double>(-45.7));
	EXPECT_FALSE(F<double>(NAN) > F<double>(0));
	EXPECT_FALSE(F<double>(NAN) > F<double>(10e10));
	EXPECT_FALSE(F<double>(NAN) > F<double>(INFINITY));
	EXPECT_FALSE(F<double>(NAN) > F<double>(NAN));
}

////////////////////////////////////////////////////////////////////////////////
// >=
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
GreaterSignedIntIsGreaterOrEqualToLesserSignedInt)
{
	EXPECT_GE(I<int>(0), I<int>(vmin<int>()));
	EXPECT_GE(I<int>(-40), I<int>(-100));
	EXPECT_GE(I<int>(1), I<int>(-1));
	EXPECT_GE(I<int>(vmax<int>()), I<int>(4567));
}

TEST_F(NumberTest,
NegativeSignedIntOneIsGreaterOrEqualToUnsignedIntOne)
{
	EXPECT_GE(I<int>(-1), I<unsigned int>(1));
}

TEST_F(NumberTest,
GreaterFloatIsGreaterOrEqualToGreaterFloat)
{
	EXPECT_GE(F<double>(vmin<double>()), F<double>(-INFINITY));
	EXPECT_GE(F<double>(-4356.58), F<double>(vmin<double>()));
	EXPECT_GE(F<double>(0), F<double>(-12));
	EXPECT_GE(F<double>(12.0), F<double>(10.0));
	EXPECT_GE(F<double>(10e10), F<double>(456.89));
	EXPECT_GE(F<double>(vmax<double>()), F<double>(21e10));
	EXPECT_GE(F<double>(INFINITY), F<double>(vmax<double>()));
}

TEST_F(NumberTest,
NanIsNotGreaterOrEqualToAnyOtherFloat)
{
	EXPECT_FALSE(F<double>(NAN) >= F<double>(-INFINITY));
	EXPECT_FALSE(F<double>(NAN) >= F<double>(-45.7));
	EXPECT_FALSE(F<double>(NAN) >= F<double>(0));
	EXPECT_FALSE(F<double>(NAN) >= F<double>(10e10));
	EXPECT_FALSE(F<double>(NAN) >= F<double>(INFINITY));
	EXPECT_FALSE(F<double>(NAN) >= F<double>(NAN));
}

TEST_F(NumberTest,
NumberIsGreaterOrEqualToTheSameNumber)
{
	EXPECT_GE(I<int>(vmin<int>()), I<int>(vmin<int>()));
	EXPECT_GE(I<int>(0), I<int>(0));
	EXPECT_GE(I<int>(vmax<int>()), I<int>(vmax<int>()));

	EXPECT_GE(F<double>(vmin<double>()), F<double>(vmin<double>()));
	EXPECT_GE(F<double>(0), F<double>(0));
	EXPECT_GE(F<double>(vmax<double>()), F<double>(vmax<double>()));
}

////////////////////////////////////////////////////////////////////////////////
// + (binary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
AdditionOfTwoNonLimitNumbersOfTheSameTypeIsCorrect)
{
	EXPECT_EQ(I<int>(-200), I<int>(-100) + I<int>(-100));
	EXPECT_EQ(I<int>(10), I<int>(0) + I<int>(10));
	EXPECT_EQ(I<int>(13568), I<int>(5678) + I<int>(7890));

	EXPECT_EQ(F<double>(-56.1 + -23.3), F<double>(-56.1) + F<double>(-23.3));
	EXPECT_EQ(F<double>(0), F<double>(0) + F<double>(0));
}

TEST_F(NumberTest,
AdditionOfTwoSignedIntsWorksCorrectlyWhenOverflowOccurs)
{
	EXPECT_EQ(I<int>(vmin<int>()), I<int>(vmax<int>()) + I<int>(1));
	EXPECT_EQ(I<int>(-2), I<int>(vmax<int>()) + I<int>(vmax<int>()));
}

TEST_F(NumberTest,
AdditionOfTwoUnsignedIntsWorksCorrectlyWhenOverflowOccurs)
{
	EXPECT_EQ(I<unsigned>(0), I<unsigned>(vmax<unsigned>()) + I<unsigned>(1));
	EXPECT_EQ(I<unsigned>(vmax<unsigned>() - 1),
		I<unsigned>(vmax<unsigned>()) + I<unsigned>(vmax<unsigned>()));
}

TEST_F(NumberTest,
AdditionOfFloatsWorksCorrectlyWhenAddingLimitNumbers)
{
	//  INF + INF = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) + F<double>(INFINITY));
	//  INF + (-INF) = -NAN
	EXPECT_TRUE((F<double>(INFINITY) + F<double>(-INFINITY)).isNotNumber());
	//  INF + NAN = NAN
	EXPECT_TRUE((F<double>(INFINITY) + F<double>(NAN)).isNotNumber());
	//  INF + number = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) + F<double>(1e30));
	// -INF + INF = -NAN
	EXPECT_TRUE((F<double>(-INFINITY) + F<double>(INFINITY)).isNotNumber());
	// -INF + (-INF) = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) + F<double>(-INFINITY));
	// -INF + NAN = NAN
	EXPECT_TRUE((F<double>(-INFINITY) + F<double>(NAN)).isNotNumber());
	// -INF + number = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) + F<double>(1e30));
	//  NAN + INF = NAN
	EXPECT_TRUE((F<double>(NAN) + F<double>(INFINITY)).isNotNumber());
	//  NAN + (-INF) = NAN
	EXPECT_TRUE((F<double>(NAN) + F<double>(-INFINITY)).isNotNumber());
	//  NAN + NAN = NAN
	EXPECT_TRUE((F<double>(NAN) + F<double>(NAN)).isNotNumber());
	//  NAN + number = NAN
	EXPECT_TRUE((F<double>(NAN) + F<double>(1e30)).isNotNumber());
	//  number + INF = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(1e30) + F<double>(INFINITY));
	//  number + (-INF) = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(1e30) + F<double>(-INFINITY));
	//  number + NAN = NAN
	EXPECT_TRUE((F<double>(1e30) + F<double>(NAN)).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// - (binary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
SubtractionOfTwoNonLimitNumbersOfTheSameTypeIsCorrect)
{
	EXPECT_EQ(I<int>(-200), I<int>(-100) - I<int>(100));
	EXPECT_EQ(I<int>(-10), I<int>(0) - I<int>(10));
	EXPECT_EQ(I<int>(5233), I<int>(5689) - I<int>(456));

	EXPECT_EQ(F<float>(vmax<int>() - 1.0), F<float>(vmax<int>()) - F<float>(1.0));

	EXPECT_EQ(F<double>(-56.4 - 23.5), F<double>(-56.4) - F<double>(23.5));
	EXPECT_EQ(F<double>(0), F<double>(0) - F<double>(0));
}

TEST_F(NumberTest,
SubtractionOfTwoSignedIntsWorksCorrectlyWhenUnderflowOccurs)
{
	EXPECT_EQ(I<int>(vmax<int>()), I<int>(vmin<int>()) - I<int>(1));
	EXPECT_EQ(I<int>(vmin<int>() - vmax<int>()),
		I<int>(vmin<int>()) - I<int>(vmax<int>()));
}

TEST_F(NumberTest,
SubtractionOfTwoUnsignedIntsWorksCorrectlyWhenUnderflowOccurs)
{
	EXPECT_EQ(I<unsigned>(vmax<unsigned>()), I<unsigned>(0) - I<unsigned>(1));
	EXPECT_EQ(I<unsigned>(1), I<unsigned>(0) - I<unsigned>(vmax<unsigned>()));
}

TEST_F(NumberTest,
SubtractionOfFloatsWorksCorrectlyWhenSubtractingLimitNumbers)
{
	//  INF - INF = -NAN
	EXPECT_TRUE((F<double>(INFINITY) - F<double>(INFINITY)).isNotNumber());
	//  INF - (-INF) = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) - F<double>(-INFINITY));
	//  INF - NAN = NAN
	EXPECT_TRUE((F<double>(INFINITY) - F<double>(NAN)).isNotNumber());
	//  INF - number = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) - F<double>(1e30));
	// -INF - INF = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) - F<double>(INFINITY));
	// -INF - (-INF) = -NAN
	EXPECT_TRUE((F<double>(-INFINITY),
		F<double>(-INFINITY) - F<double>(-INFINITY)).isNotNumber());
	// -INF - NAN = NAN
	EXPECT_TRUE((F<double>(-INFINITY) - F<double>(NAN)).isNotNumber());
	// -INF - number = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) - F<double>(1e30));
	//  NAN - INF = NAN
	EXPECT_TRUE((F<double>(NAN) - F<double>(INFINITY)).isNotNumber());
	//  NAN - (-INF) = NAN
	EXPECT_TRUE((F<double>(NAN) - F<double>(-INFINITY)).isNotNumber());
	//  NAN - NAN = NAN
	EXPECT_TRUE((F<double>(NAN) - F<double>(NAN)).isNotNumber());
	//  NAN - number = NAN
	EXPECT_TRUE((F<double>(NAN) - F<double>(1e30)).isNotNumber());
	//  number - INF = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(1e30) - F<double>(INFINITY));
	//  number - (-INF) = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(1e30) - F<double>(-INFINITY));
	//  number - NAN = NAN
	EXPECT_TRUE((F<double>(1e30) - F<double>(NAN)).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// *
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
MultiplicationOfTwoNonLimitNumbersOfTheSameTypeIsCorrect)
{
	EXPECT_EQ(I<int>(10000), I<int>(-100) * I<int>(-100));
	EXPECT_EQ(I<int>(0), I<int>(0) * I<int>(10));
	EXPECT_EQ(I<int>(44799420), I<int>(5678) * I<int>(7890));

	EXPECT_EQ(F<double>(-56.0 * -23.0), F<double>(-56.0) * F<double>(-23.0));
	EXPECT_EQ(F<double>(0), F<double>(0) * F<double>(0));
}

TEST_F(NumberTest,
MultiplicationOfTwoSignedIntsWorksCorrectlyWhenOverflowOccurs)
{
	EXPECT_EQ(I<int>(-2), I<int>(vmax<int>()) * I<int>(2));
	EXPECT_EQ(I<int>(1), I<int>(vmax<int>()) * I<int>(vmax<int>()));
}

TEST_F(NumberTest,
MultiplicationOfTwoUnsignedIntsWorksCorrectlyWhenOverflowOccurs)
{
	EXPECT_EQ(I<unsigned>(vmax<unsigned>() - 1),
		I<unsigned>(vmax<unsigned>()) * I<unsigned>(2));
	EXPECT_EQ(I<unsigned>(1),
		I<unsigned>(vmax<unsigned>()) * I<unsigned>(vmax<unsigned>()));
}

TEST_F(NumberTest,
MultiplicationOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(F<double>(vmin<unsigned>() * vmax<double>()),
		I<unsigned>(vmin<unsigned>()) * F<double>(vmax<double>()));
}

TEST_F(NumberTest,
MultiplicationOfFloatsWorksCorrectlyMultiplyingLimitNumbers)
{
	//  INF * INF = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) * F<double>(INFINITY));
	//  INF * (-INF) = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(INFINITY) * F<double>(-INFINITY));
	//  INF * NAN = NAN
	EXPECT_TRUE((F<double>(INFINITY) * F<double>(NAN)).isNotNumber());
	//  INF * number = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(INFINITY) * F<double>(1e30));
	// -INF * INF = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) * F<double>(INFINITY));
	// -INF * (-INF) = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(-INFINITY) * F<double>(-INFINITY));
	// -INF * NAN = NAN
	EXPECT_TRUE((F<double>(-INFINITY) * F<double>(NAN)).isNotNumber());
	// -INF * number = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(-INFINITY) * F<double>(1e30));
	//  NAN * INF = NAN
	EXPECT_TRUE((F<double>(NAN) * F<double>(INFINITY)).isNotNumber());
	//  NAN * (-INF) = NAN
	EXPECT_TRUE((F<double>(NAN) * F<double>(-INFINITY)).isNotNumber());
	//  NAN * NAN = NAN
	EXPECT_TRUE((F<double>(NAN) * F<double>(NAN)).isNotNumber());
	//  NAN * number = NAN
	EXPECT_TRUE((F<double>(NAN) * F<double>(1e30)).isNotNumber());
	//  number * INF = INF
	EXPECT_EQ(F<double>(INFINITY), F<double>(1e30) * F<double>(INFINITY));
	//  number * (-INF) = -INF
	EXPECT_EQ(F<double>(-INFINITY), F<double>(1e30) * F<double>(-INFINITY));
	//  number * NAN = NAN
	EXPECT_TRUE((F<double>(1e30) * F<double>(NAN)).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// rdiv
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
RealDivisionOfTwoFloatsWithTheSameTypeWorksCorrect)
{
	EXPECT_EQ(F<float>(-10.0f / -2.0f), rdiv(F<float>(-10.0), F<float>(-2.0)));
	EXPECT_EQ(F<float>(0.0f), rdiv(F<float>(0), F<float>(-1)));
	EXPECT_EQ(F<float>(-5.45f / 0.0f), rdiv(F<float>(-5.45), F<float>(0.0)));

	EXPECT_EQ(F<double>(-10.0 / -2.0), rdiv(F<double>(-10.0), F<double>(-2.0)));
	EXPECT_EQ(F<double>(0.0), rdiv(F<double>(0), F<double>(-1)));
	EXPECT_EQ(F<double>(-5.45 / 0.0), rdiv(F<double>(-5.45), F<double>(0.0)));

	EXPECT_EQ(F<long double>(-10.0L / -2.0L),
			  rdiv(F<long double>(-10.0), F<long double>(-2.0)));
	EXPECT_EQ(F<long double>(0.0L), rdiv(F<long double>(0), F<long double>(-1)));
	EXPECT_EQ(F<long double>(-5.45L / 0.0L),
			  rdiv(F<long double>(-5.45), F<long double>(0.0)));
}

TEST_F(NumberTest,
RealDivisionOfTwoFloatsWithTheDifferentTypesWorksCorrect)
{
	EXPECT_EQ(F<double>(-10.0f / -2.0), rdiv(F<float>(-10.0), F<double>(-2.0)));
	EXPECT_EQ(F<long double>(0.0f / -1L), rdiv(F<float>(0), F<long double>(-1)));

	EXPECT_EQ(F<double>(-10.0 / -2.0f), rdiv(F<double>(-10.0), F<float>(-2.0)));
	EXPECT_EQ(F<long double>(0.0), rdiv(F<double>(0), F<long double>(-1)));

	EXPECT_EQ(F<long double>(0.0L / -1.0f), rdiv(F<long double>(0), F<float>(-1.0)));
	EXPECT_EQ(F<long double>(-5.0L / 0.0), rdiv(F<long double>(-5), F<double>(0.0)));
}

TEST_F(NumberTest,
RealDivisionOfSpecialFloatsWorksCorrectly)
{
	//  INF / INF = -NAN
	EXPECT_TRUE(rdiv(F<double>(INFINITY), F<double>(INFINITY)).isNotNumber());
	//  INF / (-INF) = -NAN
	EXPECT_TRUE(rdiv(F<double>(INFINITY), F<double>(-INFINITY)).isNotNumber());
	//  INF / NAN = NAN
	EXPECT_TRUE(rdiv(F<double>(INFINITY), F<double>(NAN)).isNotNumber());
	//  INF / number = INF
	EXPECT_EQ(F<double>(INFINITY), rdiv(F<double>(INFINITY), F<double>(1e30)));
	// -INF / INF = -NAN
	EXPECT_TRUE(rdiv(F<double>(-INFINITY), F<double>(INFINITY)).isNotNumber());
	// -INF / (-INF) = -NAN
	EXPECT_TRUE(rdiv(F<double>(-INFINITY), F<double>(-INFINITY)).isNotNumber());
	// -INF / NAN = NAN
	EXPECT_TRUE(rdiv(F<double>(-INFINITY), F<double>(NAN)).isNotNumber());
	// -INF / number = -INF
	EXPECT_EQ(F<double>(-INFINITY), rdiv(F<double>(-INFINITY), F<double>(1e30)));
	//  NAN / INF = NAN
	EXPECT_TRUE(rdiv(F<double>(NAN), F<double>(INFINITY)).isNotNumber());
	//  NAN / (-INF) = NAN
	EXPECT_TRUE(rdiv(F<double>(NAN), F<double>(-INFINITY)).isNotNumber());
	//  NAN / NAN = NAN
	EXPECT_TRUE(rdiv(F<double>(NAN), F<double>(NAN)).isNotNumber());
	//  NAN / number = NAN
	EXPECT_TRUE(rdiv(F<double>(NAN), F<double>(1e30)).isNotNumber());
	//  number / INF = 0
	EXPECT_EQ(F<double>(0), rdiv(F<double>(1e30), F<double>(INFINITY)));
	//  number / (-INF) = -0
	EXPECT_EQ(F<double>(0), rdiv(F<double>(1e30), F<double>(-INFINITY)));
	//  number / NAN = NAN
	EXPECT_TRUE(rdiv(F<double>(1e30), F<double>(NAN)).isNotNumber());
	//  +number / 0 = INF
	EXPECT_EQ(F<double>(INFINITY), rdiv(F<double>(10.0), F<double>(0.0)));
	//  -number / 0 = -INF
	EXPECT_EQ(F<double>(-INFINITY), rdiv(F<double>(-10.0), F<double>(0.0)));
	//  +number / -0 = -INF
	EXPECT_EQ(F<double>(-INFINITY), rdiv(F<double>(10.0), F<double>(-0.0)));
	//  -number / -0 = INF
	EXPECT_EQ(F<double>(INFINITY), rdiv(F<double>(-10.0), F<double>(-0.0)));
	//  0 / 0 = NAN
	EXPECT_TRUE(rdiv(F<double>(NAN), F<double>(0.0)).isNotNumber());
}

////////////////////////////////////////////////////////////////////////////////
// exact_div
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
ExactDivisionOfIntsWithTheSameTypeWorksCorrectly)
{
	EXPECT_EQ(I<int>(100 / -10), exact_div(I<signed char>(100), I<signed char>(-10)));
	EXPECT_EQ(I<int>(0 / 10), exact_div(I<signed char>(0), I<signed char>(10)));
	EXPECT_EQ(I<int>(127 / -127), exact_div(I<signed char>(127), I<signed char>(-127)));
	EXPECT_EQ(I<int>(100 / 10), exact_div(I<unsigned char>(100), I<unsigned char>(10)));
	EXPECT_EQ(I<int>(10 / 2), exact_div(I<unsigned char>(10), I<unsigned char>(2)));
	EXPECT_EQ(I<int>(1), exact_div(I<unsigned char>(255), I<unsigned char>(255)));

	EXPECT_EQ(I<int>(1000 / -10), exact_div(I<short>(1000), I<short>(-10)));
	EXPECT_EQ(I<int>(0 / 130), exact_div(I<short>(0), I<short>(130)));
	EXPECT_EQ(I<int>(15678 / -15678), exact_div(I<short>(15678), I<short>(-15678)));

	if (sizeof(short) < sizeof(int)) {
		EXPECT_EQ(I<int>(100 / 50),
				  exact_div(I<unsigned short>(100), I<unsigned short>(50)));
	} else if (sizeof(short) == sizeof(int)) {
		EXPECT_EQ(I<unsigned>(100 / 2),
				  exact_div(I<unsigned short>(100), I<unsigned short>(2)));
	}

	EXPECT_EQ(I<int>(100 / -10), exact_div(I<int>(100), I<int>(-10)));
	EXPECT_EQ(I<int>(0 / 10), exact_div(I<int>(0), I<int>(10)));
	EXPECT_EQ(I<int>(15678 / -15678), exact_div(I<int>(15678), I<int>(-15678)));
	EXPECT_EQ(I<unsigned>(100 / 10), exact_div(I<unsigned>(100), I<unsigned>(10)));
	EXPECT_EQ(I<unsigned>(10 / 2), exact_div(I<unsigned>(10), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned>(1), exact_div(I<unsigned>(7890), I<unsigned>(7890)));

	EXPECT_EQ(I<long>(1000 / -10), exact_div(I<long>(1000), I<long>(-10)));
	EXPECT_EQ(I<long>(0 / 1), exact_div(I<long>(0), I<long>(1)));
	EXPECT_EQ(I<long>(vmax<long>() / -vmax<long>()),
		      exact_div(I<long>(vmax<long>()), I<long>(-vmax<long>())));
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>() / vmax<unsigned long>()),
			  exact_div(I<unsigned long>(vmax<unsigned long>()),
			  			I<unsigned long>(vmax<unsigned long>())));
	EXPECT_EQ(I<unsigned long>(5000 / 2),
			  exact_div(I<unsigned long>(5000), I<unsigned long>(2)));
	EXPECT_EQ(I<unsigned long>(1),
			  exact_div(I<unsigned long>(7890), I<unsigned long>(7890)));

	// Note: INT_MIN / -1 is not tested because its behaviour is undefined.
	// EXPECT_EQ(I<int>(vmin<int>() / -1), exact_div(I<int>(vmin<int>()), I<int>(-1)));
}

TEST_F(NumberTest,
ExactDivisionOfIntsWithTheDifferentTypesWorksCorrectly)
{
	EXPECT_EQ(I<int>(-4), exact_div(I<signed char>(-20), I<unsigned char>(5)));
	EXPECT_EQ(I<int>(-61), exact_div(I<signed char>(-122), I<short>(2)));
	EXPECT_EQ(I<unsigned>((unsigned)-28 / 2),
			  exact_div(I<signed char>(-28), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(32), exact_div(I<signed char>(32), I<unsigned long>(1)));
	EXPECT_EQ(I<long>(10), exact_div(I<signed int>(780), I<long>(78)));
}

TEST_F(NumberTest,
ExactDivisionOfFloatsWorksCorrectly)
{
	EXPECT_DEATH(exact_div(F<double>(78.0), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(exact_div(F<double>(78.0), F<int>(2)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(exact_div(F<int>(158), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// trunc_div
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
TruncDivisionOfIntsWithTheSameTypeWorksCorrectly)
{
	EXPECT_EQ(I<int>(101 / -10), trunc_div(I<signed char>(101), I<signed char>(-10)));
	EXPECT_EQ(I<int>(1 / 10), trunc_div(I<signed char>(1), I<signed char>(10)));
	EXPECT_EQ(I<int>(127 / -127), trunc_div(I<signed char>(127), I<signed char>(-127)));
	EXPECT_EQ(I<int>(100 / 11), trunc_div(I<unsigned char>(100), I<unsigned char>(11)));
	EXPECT_EQ(I<int>(10 / 3), trunc_div(I<unsigned char>(10), I<unsigned char>(3)));
	EXPECT_EQ(I<int>(254 / 255),
	          trunc_div(I<unsigned char>(254), I<unsigned char>(255)));

	EXPECT_EQ(I<int>(1000 / -11), trunc_div(I<short>(1000), I<short>(-11)));
	EXPECT_EQ(I<int>(1 / 130), trunc_div(I<short>(1), I<short>(130)));
	EXPECT_EQ(I<int>(15678 / -15678), trunc_div(I<short>(15678), I<short>(-15678)));

	if (sizeof(short) < sizeof(int)) {
		EXPECT_EQ(I<int>(101 / 10),
				  trunc_div(I<unsigned short>(101), I<unsigned short>(10)));
	} else if (sizeof(short) == sizeof(int)) {
		EXPECT_EQ(I<unsigned>(101 / 10),
				  trunc_div(I<unsigned short>(101), I<unsigned short>(10)));
	}

	EXPECT_EQ(I<int>(100 / -11), trunc_div(I<int>(100), I<int>(-11)));
	EXPECT_EQ(I<int>(0 / 10), trunc_div(I<int>(0), I<int>(10)));
	EXPECT_EQ(I<int>(15678 / -1), trunc_div(I<int>(15678), I<int>(-1)));
	EXPECT_EQ(I<unsigned>(100 / 12), trunc_div(I<unsigned>(100), I<unsigned>(12)));
	EXPECT_EQ(I<unsigned>(10 / 25), trunc_div(I<unsigned>(10), I<unsigned>(25)));
	EXPECT_EQ(I<unsigned>(7890 / 7), trunc_div(I<unsigned>(7890), I<unsigned>(7)));

	EXPECT_EQ(I<long>(1000 / -11), trunc_div(I<long>(1000), I<long>(-11)));
	EXPECT_EQ(I<long>(12 / 7), trunc_div(I<long>(12), I<long>(7)));
	EXPECT_EQ(I<unsigned long>(5000 / 6),
			  trunc_div(I<unsigned long>(5000), I<unsigned long>(6)));

	// Note: INT_MIN / -1 is not tested because its behaviour is undefined.
	// EXPECT_EQ(I<int>(vmin<int>() / -1), trunc_div(I<int>(vmin<int>()), I<int>(-1)));
}

TEST_F(NumberTest,
TruncDivisionOfIntsWithTheDifferentTypesWorksCorrectly)
{
	EXPECT_EQ(I<int>(-21 / 5), trunc_div(I<signed char>(-21), I<unsigned char>(5)));
	EXPECT_EQ(I<int>(-50 / 10), trunc_div(I<signed char>(-50), I<unsigned short>(10)));
	EXPECT_EQ(I<int>(-121 / 2), trunc_div(I<signed char>(-121), I<short>(2)));
	EXPECT_EQ(I<unsigned>((unsigned)-29 / 2),
			  trunc_div(I<signed char>(-29), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(32),
			  trunc_div(I<signed char>(32), I<unsigned long>(1)));
	EXPECT_EQ(I<long>(781 / 78), trunc_div(I<signed int>(781), I<long>(78)));
}

TEST_F(NumberTest,
TruncDivisionOfFloatsWorksCorrectly)
{
	EXPECT_DEATH(trunc_div(F<double>(78.0), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(trunc_div(F<double>(78.0), F<int>(2)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(trunc_div(F<int>(158), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// trunc_mod
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
TruncModuloOfIntsWithTheSameTypeWorksCorrectly)
{
	EXPECT_EQ(I<int>(101 % -10), trunc_mod(I<signed char>(101), I<signed char>(-10)));
	EXPECT_EQ(I<int>(1 % 10), trunc_mod(I<signed char>(1), I<signed char>(10)));
	EXPECT_EQ(I<int>(127 % -127), trunc_mod(I<signed char>(127), I<signed char>(-127)));
	EXPECT_EQ(I<int>(100 % 11), trunc_mod(I<unsigned char>(100), I<unsigned char>(11)));
	EXPECT_EQ(I<int>(10 % 3), trunc_mod(I<unsigned char>(10), I<unsigned char>(3)));
	EXPECT_EQ(I<int>(25 % 26), trunc_mod(I<unsigned char>(25), I<unsigned char>(26)));

	EXPECT_EQ(I<int>(1000 % -11), trunc_mod(I<short>(1000), I<short>(-11)));
	EXPECT_EQ(I<int>(1 % 130), trunc_mod(I<short>(1), I<short>(130)));
	EXPECT_EQ(I<int>(15678 % -15678), trunc_mod(I<short>(15678), I<short>(-15678)));

	if (sizeof(short) < sizeof(int)) {
		EXPECT_EQ(I<int>(101 % 10),
				  trunc_mod(I<unsigned short>(101), I<unsigned short>(10)));
	} else if (sizeof(short) == sizeof(int)) {
		EXPECT_EQ(I<unsigned>(101 % 10),
				  trunc_mod(I<unsigned short>(101), I<unsigned short>(10)));
	}

	EXPECT_EQ(I<int>(-5 % -4), trunc_mod(I<int>(-5), I<int>(-4)));
	EXPECT_EQ(I<int>(100 % -11), trunc_mod(I<int>(100), I<int>(-11)));
	EXPECT_EQ(I<int>(0 % 10), trunc_mod(I<int>(0), I<int>(10)));
	EXPECT_EQ(I<int>(15678 % -1), trunc_mod(I<int>(15678), I<int>(-1)));
	EXPECT_EQ(I<unsigned>(100 % 12), trunc_mod(I<unsigned>(100), I<unsigned>(12)));
	EXPECT_EQ(I<unsigned>(10 % 25), trunc_mod(I<unsigned>(10), I<unsigned>(25)));
	EXPECT_EQ(I<unsigned>(7890 % 7), trunc_mod(I<unsigned>(7890), I<unsigned>(7)));

	EXPECT_EQ(I<long>(-1000 % -11), trunc_mod(I<long>(-1000), I<long>(-11)));
	EXPECT_EQ(I<long>(-12 % 7), trunc_mod(I<long>(-12), I<long>(7)));
	EXPECT_EQ(I<unsigned long>(5000 % 6),
			  trunc_mod(I<unsigned long>(5000), I<unsigned long>(6)));
}

TEST_F(NumberTest,
TruncModuloOfIntsWithTheDifferentTypesWorksCorrectly)
{
	EXPECT_EQ(I<int>(-21 % 5), trunc_mod(I<signed char>(-21), I<unsigned char>(5)));
	EXPECT_EQ(I<int>(-50 % 10), trunc_mod(I<signed char>(-50), I<unsigned short>(10)));
	EXPECT_EQ(I<int>(-121 % 2), trunc_mod(I<signed char>(-121), I<short>(2)));
	EXPECT_EQ(I<unsigned>((unsigned)-29 % 2),
			  trunc_mod(I<signed char>(-29), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(32 % 1),
			  trunc_mod(I<signed char>(32), I<unsigned long>(1)));
	EXPECT_EQ(I<long>(781 % 78), trunc_mod(I<signed int>(781), I<long>(78)));
}

TEST_F(NumberTest,
TruncModuloOfFloatsWorksCorrectly)
{
	EXPECT_DEATH(trunc_mod(F<double>(78.0), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(trunc_mod(F<double>(78.0), F<int>(2)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(trunc_mod(F<int>(158), F<double>(2.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// logicalNot()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
LogicalNotOfIntWorksCorrectly)
{
	EXPECT_EQ(!0, logicalNot(I<int>(0)));
	EXPECT_TRUE(!1 == logicalNot(I<int>(1)));
	EXPECT_EQ(!vmin<int>(), logicalNot(I<int>(vmin<int>())));
	EXPECT_EQ(!vmax<int>(), logicalNot(I<int>(vmax<int>())));
}

TEST_F(NumberTest,
LogicalNotOfFloatWorksCorrectly)
{
	EXPECT_EQ(!0.00, logicalNot(F<double>(0.00)));
	EXPECT_TRUE(!1.35 == logicalNot(F<double>(1.35)));
	EXPECT_EQ(!vmin<double>(), logicalNot(F<double>(vmin<double>())));
	EXPECT_EQ(!vmax<double>(), logicalNot(F<double>(vmax<double>())));
}

TEST_F(NumberTest,
LogicalNotOfAllTypesWorksCorrectly)
{
	// Integral types.
	EXPECT_TRUE(!(unsigned char)0 == logicalNot(I<unsigned char>(0)));
	EXPECT_TRUE(!(signed char)1 == logicalNot(I<signed char>(1)));
	EXPECT_TRUE(!(unsigned short)0 == logicalNot(I<unsigned short>(0)));
	EXPECT_TRUE(!(signed short)10 == logicalNot(I<signed short>(10)));
	EXPECT_TRUE(!(unsigned int)0 == logicalNot(I<unsigned int>(0)));
	EXPECT_TRUE(!(signed int)(-12) == logicalNot(I<signed int>(-12)));
	EXPECT_TRUE(!(unsigned long)0 == logicalNot(I<unsigned long>(0)));
	EXPECT_TRUE(!(signed long)(-5) == logicalNot(I<signed long>(-5)));

	// Floating-point types.
	EXPECT_TRUE(!0.0f == logicalNot(F<float>(0.0f)));
	EXPECT_TRUE(!1.25 == logicalNot(F<double>(1.25)));
	EXPECT_TRUE(!1.35L == logicalNot(F<long double>(1.35L)));
}

TEST_F(NumberTest,
LogicalNotOfSpecialFloatWorksCorrectly)
{
	EXPECT_TRUE(!(NAN) == logicalNot(F<double>(NAN)));
	EXPECT_TRUE(!INFINITY == logicalNot(F<double>(INFINITY)));
	EXPECT_TRUE(!(-INFINITY) == logicalNot(F<double>(-INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// logicalAnd()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntLogicalAndIntWorksCorrectly)
{
	EXPECT_TRUE((0 && 0) == logicalAnd(I<int>(0), I<int>(0)));
	EXPECT_TRUE((0 && 1) == logicalAnd(I<int>(0), I<int>(1)));
	EXPECT_TRUE((1 && 0) == logicalAnd(I<int>(1), I<int>(0)));
	EXPECT_TRUE((1 && 1) == logicalAnd(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntLogicalAndFloatWorksCorrectly)
{
	EXPECT_TRUE((0 && 0.0) == logicalAnd(I<int>(0), F<double>(0.0)));
	EXPECT_TRUE((0 && 1.38) == logicalAnd(I<int>(0), F<double>(1.38)));
	EXPECT_TRUE((10 && 0.0) == logicalAnd(I<int>(10), F<double>(0.0)));
	EXPECT_TRUE((11 && 4.35) == logicalAnd(I<int>(11), F<double>(4.35)));
}

TEST_F(NumberTest,
FloatLogicalAndIntWorksCorrectly)
{
	EXPECT_TRUE((0.0 && 0) == logicalAnd(F<double>(0.0), I<int>(0)));
	EXPECT_TRUE((0.0 && 12) == logicalAnd(F<double>(0.0), I<int>(12)));
	EXPECT_TRUE((125.89 && 0) == logicalAnd(F<double>(125.89), I<int>(0)));
	EXPECT_TRUE((125.89 && 125) == logicalAnd(F<double>(125.89), I<int>(125)));
}

TEST_F(NumberTest,
FloatLogicalAndFloatWorksCorrectly)
{
	EXPECT_TRUE((0.0 && 0.0) == logicalAnd(F<double>(0.0), F<double>(0.0)));
	EXPECT_TRUE((0.0 && 12.35) == logicalAnd(F<double>(0.0), F<double>(12.35)));
	EXPECT_TRUE((258.458 && 0.0) == logicalAnd(F<double>(258.458), F<double>(0.0)));
	EXPECT_TRUE((18.4 && 458.2) == logicalAnd(F<double>(18.4), F<double>(458.2)));
}

TEST_F(NumberTest,
UnsignedCharLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((unsigned char)0 && (unsigned char)0) ==
				  logicalAnd(I<unsigned char>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((unsigned char)0 && (signed char)-1) ==
				  logicalAnd(I<unsigned char>(0), I<signed char>(-1)));
	EXPECT_TRUE(((unsigned char)1 && (unsigned short)0) ==
				  logicalAnd(I<unsigned char>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((unsigned char)1 && (signed short)1) ==
				  logicalAnd(I<unsigned char>(1), I<signed short>(1)));
	EXPECT_TRUE(((unsigned char)0 && (unsigned int)0) ==
				  logicalAnd(I<unsigned char>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((unsigned char)1 && (signed int)-100) ==
				  logicalAnd(I<unsigned char>(1), I<signed int>(-100)));
	EXPECT_TRUE(((unsigned char)10 && (unsigned long)0) ==
				  logicalAnd(I<unsigned char>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((unsigned char)0 && (signed long)0) ==
				  logicalAnd(I<unsigned char>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
SignedCharLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((signed char)0 && (unsigned char)0) ==
				  logicalAnd(I<signed char>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((signed char)0 && (signed char)-1) ==
				  logicalAnd(I<signed char>(0), I<signed char>(-1)));
	EXPECT_TRUE(((signed char)1 && (unsigned short)0) ==
				  logicalAnd(I<signed char>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((signed char)1 && (signed short)1) ==
				  logicalAnd(I<signed char>(1), I<signed short>(1)));
	EXPECT_TRUE(((signed char)0 && (unsigned int)0) ==
				  logicalAnd(I<signed char>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((signed char)1 && (signed int)-100) ==
				  logicalAnd(I<signed char>(1), I<signed int>(-100)));
	EXPECT_TRUE(((signed char)10 && (unsigned long)0) ==
				  logicalAnd(I<signed char>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((signed char)0 && (signed long)0) ==
				  logicalAnd(I<signed char>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
UnsignedShortLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((unsigned short)0 && (unsigned char)0) ==
				  logicalAnd(I<unsigned short>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((unsigned short)0 && (signed char)-1) ==
				  logicalAnd(I<unsigned short>(0), I<signed char>(-1)));
	EXPECT_TRUE(((unsigned short)1 && (unsigned short)0) ==
				  logicalAnd(I<unsigned short>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((unsigned short)1 && (signed short)1) ==
				  logicalAnd(I<unsigned short>(1), I<signed short>(1)));
	EXPECT_TRUE(((unsigned short)0 && (unsigned int)0) ==
				  logicalAnd(I<unsigned short>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((unsigned short)1 && (signed int)-100) ==
				  logicalAnd(I<unsigned short>(1), I<signed int>(-100)));
	EXPECT_TRUE(((unsigned short)10 && (unsigned long)0) ==
				  logicalAnd(I<unsigned short>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((unsigned short)0 && (signed long)0) ==
				  logicalAnd(I<unsigned short>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
SignedShortLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((signed short)0 && (unsigned char)0) ==
				  logicalAnd(I<signed short>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((signed short)0 && (signed char)-1) ==
				  logicalAnd(I<signed short>(0), I<signed char>(-1)));
	EXPECT_TRUE(((signed short)1 && (unsigned short)0) ==
				  logicalAnd(I<signed short>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((signed short)1 && (signed short)1) ==
				  logicalAnd(I<signed short>(1), I<signed short>(1)));
	EXPECT_TRUE(((signed short)0 && (unsigned int)0) ==
				  logicalAnd(I<signed short>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((signed short)1 && (signed int)-100) ==
				  logicalAnd(I<signed short>(1), I<signed int>(-100)));
	EXPECT_TRUE(((signed short)10 && (unsigned long)0) ==
				  logicalAnd(I<signed short>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((signed short)0 && (signed long)0) ==
				  logicalAnd(I<signed short>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
UnsignedIntLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((unsigned int)0 && (unsigned char)0) ==
				  logicalAnd(I<unsigned int>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((unsigned int)0 && (signed char)-1) ==
				  logicalAnd(I<unsigned int>(0), I<signed char>(-1)));
	EXPECT_TRUE(((unsigned int)1 && (unsigned short)0) ==
				  logicalAnd(I<unsigned int>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((unsigned int)1 && (signed short)1) ==
				  logicalAnd(I<unsigned int>(1), I<signed short>(1)));
	EXPECT_TRUE(((unsigned int)0 && (unsigned int)0) ==
				  logicalAnd(I<unsigned int>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((unsigned int)1 && (signed int)-100) ==
				  logicalAnd(I<unsigned int>(1), I<signed int>(-100)));
	EXPECT_TRUE(((unsigned int)10 && (unsigned long)0) ==
				  logicalAnd(I<unsigned int>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((unsigned int)0 && (signed long)0) ==
				  logicalAnd(I<unsigned int>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
SignedIntLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((signed int)0 && (unsigned char)0) ==
				  logicalAnd(I<signed int>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((signed int)0 && (signed char)-1) ==
				  logicalAnd(I<signed int>(0), I<signed char>(-1)));
	EXPECT_TRUE(((signed int)1 && (unsigned short)0) ==
				  logicalAnd(I<signed int>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((signed int)1 && (signed short)1) ==
				  logicalAnd(I<signed int>(1), I<signed short>(1)));
	EXPECT_TRUE(((signed int)0 && (unsigned int)0) ==
				  logicalAnd(I<signed int>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((signed int)1 && (signed int)-100) ==
				  logicalAnd(I<signed int>(1), I<signed int>(-100)));
	EXPECT_TRUE(((signed int)10 && (unsigned long)0) ==
				  logicalAnd(I<signed int>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((signed int)0 && (signed long)0) ==
				  logicalAnd(I<signed int>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
UnsignedLongLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((unsigned long)0 && (unsigned char)0) ==
				  logicalAnd(I<unsigned long>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((unsigned long)0 && (signed char)-1) ==
				  logicalAnd(I<unsigned long>(0), I<signed char>(-1)));
	EXPECT_TRUE(((unsigned long)1 && (unsigned short)0) ==
				  logicalAnd(I<unsigned long>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((unsigned long)1 && (signed short)1) ==
				  logicalAnd(I<unsigned long>(1), I<signed short>(1)));
	EXPECT_TRUE(((unsigned long)0 && (unsigned int)0) ==
				  logicalAnd(I<unsigned long>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((unsigned long)1 && (signed int)-100) ==
				  logicalAnd(I<unsigned long>(1), I<signed int>(-100)));
	EXPECT_TRUE(((unsigned long)10 && (unsigned long)0) ==
				  logicalAnd(I<unsigned long>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((unsigned long)0 && (signed long)0) ==
				  logicalAnd(I<unsigned long>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
SignedLongLogicalAndAllTypesWorksCorrectly)
{
	EXPECT_TRUE(((signed long)0 && (unsigned char)0) ==
				  logicalAnd(I<signed long>(0), I<unsigned char>(0)));
	EXPECT_TRUE(((signed long)0 && (signed char)-1) ==
				  logicalAnd(I<signed long>(0), I<signed char>(-1)));
	EXPECT_TRUE(((signed long)1 && (unsigned short)0) ==
				  logicalAnd(I<signed long>(1), I<unsigned short>(0)));
	EXPECT_TRUE(((signed long)1 && (signed short)1) ==
				  logicalAnd(I<signed long>(1), I<signed short>(1)));
	EXPECT_TRUE(((signed long)0 && (unsigned int)0) ==
				  logicalAnd(I<signed long>(0), I<unsigned int>(0)));
	EXPECT_TRUE(((signed long)1 && (signed int)-100) ==
				  logicalAnd(I<signed long>(1), I<signed int>(-100)));
	EXPECT_TRUE(((signed long)10 && (unsigned long)0) ==
				  logicalAnd(I<signed long>(10), I<unsigned long>(0)));
	EXPECT_TRUE(((signed long)0 && (signed long)0) ==
				  logicalAnd(I<signed long>(0), I<signed long>(0)));
}

TEST_F(NumberTest,
AllIntTypesLogicalAndAllFloatTypesWorksCorrectly)
{
	EXPECT_TRUE(((unsigned char)0 && 0.0f) ==
				  logicalAnd(I<unsigned char>(0), F<float>(0.0f)));
	EXPECT_TRUE(((unsigned char)1 && 1.3) ==
	              logicalAnd(I<unsigned char>(1), F<double>(1.3)));
	EXPECT_TRUE(((unsigned char)2 && 1.8L) ==
	              logicalAnd(I<unsigned char>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((signed char)0 && 0.0f) ==
				  logicalAnd(I<signed char>(0), F<float>(0.0f)));
	EXPECT_TRUE(((signed char)-1 && 1.3) ==
				  logicalAnd(I<signed char>(-1), F<double>(1.3)));
	EXPECT_TRUE(((signed char)-2 && 1.8L) ==
				  logicalAnd(I<signed char>(-2), F<long double>(1.8L)));
	EXPECT_TRUE(((unsigned short)0 && 0.0f) ==
				  logicalAnd(I<unsigned short>(0), F<float>(0.0f)));
	EXPECT_TRUE(((unsigned short)1 && 1.3) ==
				  logicalAnd(I<unsigned short>(1), F<double>(1.3)));
	EXPECT_TRUE(((unsigned short)2 && 1.8L) ==
				  logicalAnd(I<unsigned short>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((signed short)0 && 0.0f) ==
				  logicalAnd(I<signed short>(0), F<float>(0.0f)));
	EXPECT_TRUE(((signed short)0 && 1.0) ==
				  logicalAnd(I<signed short>(0), F<double>(1.0)));
	EXPECT_TRUE(((signed short)2 && 1.8L) ==
				  logicalAnd(I<signed short>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((unsigned int)0 && 0.0f) ==
				  logicalAnd(I<unsigned int>(0), F<float>(0.0f)));
	EXPECT_TRUE(((unsigned int)1 && 1.3) ==
				  logicalAnd(I<unsigned int>(1), F<double>(1.3)));
	EXPECT_TRUE(((unsigned int)2 && 1.8L) ==
				  logicalAnd(I<unsigned int>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((signed int)0 && 0.0f) ==
				  logicalAnd(I<signed int>(0), F<float>(0.0f)));
	EXPECT_TRUE(((signed int)0 && 1.0) ==
				  logicalAnd(I<signed int>(0), F<double>(1.0)));
	EXPECT_TRUE(((signed int)2 && 1.8L) ==
				  logicalAnd(I<signed int>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((unsigned long)0 && 0.0f) ==
				  logicalAnd(I<unsigned long>(0), F<float>(0.0f)));
	EXPECT_TRUE(((unsigned long)1 && 0.0) ==
				  logicalAnd(I<unsigned long>(1), F<double>(0.0)));
	EXPECT_TRUE(((unsigned long)2 && 1.8L) ==
				  logicalAnd(I<unsigned long>(2), F<long double>(1.8L)));
	EXPECT_TRUE(((signed long)0 && 0.0f) ==
				  logicalAnd(I<signed long>(0), F<float>(0.0f)));
	EXPECT_TRUE(((signed long)0 && 1.0) ==
				  logicalAnd(I<signed long>(0), F<double>(1.0)));
	EXPECT_TRUE(((signed long)2 && 1.8L) ==
				  logicalAnd(I<signed long>(2), F<long double>(1.8L)));
}

TEST_F(NumberTest,
AllFloatTypesLogicalAndAllFloatTypesWorksCorrectly)
{
	EXPECT_TRUE((0.0f && 0.0f) == logicalAnd(F<float>(0.0f), F<float>(0.0f)));
	EXPECT_TRUE((0.0f && 1.0) == logicalAnd(F<float>(0.0f), F<double>(1.0)));
	EXPECT_TRUE((0.0f && 0.0L) == logicalAnd(F<float>(0.0f), F<long double>(0.0L)));
	EXPECT_TRUE((1.0 && 0.0f) == logicalAnd(F<double>(1.0), F<float>(0.0f)));
	EXPECT_TRUE((1.0 && 1.0) == logicalAnd(F<double>(1.0), F<double>(1.0)));
	EXPECT_TRUE((1.0 && 0.0L) == logicalAnd(F<double>(1.0), F<long double>(0.0L)));
	EXPECT_TRUE((0.5L && 1.5f) == logicalAnd(F<long double>(0.5L), F<float>(1.5f)));
	EXPECT_TRUE((1.8L && 1.4) == logicalAnd(F<long double>(1.8L), F<double>(1.4)));
	EXPECT_TRUE((2.1L && 0.3L) == logicalAnd(F<long double>(2.1L),
											 F<long double>(0.3L)));
}

TEST_F(NumberTest,
SpecialFloatLogicalAndSpecialFloatWorksCorrectly)
{
	// NAN && NAN
	EXPECT_TRUE((NAN && NAN) ==
				 logicalAnd(F<double>(NAN), F<double>(NAN)));
	// NAN && INF
	EXPECT_TRUE((NAN && INFINITY) ==
				 logicalAnd(F<double>(NAN), F<double>(INFINITY)));
	// NAN && -INF
	EXPECT_TRUE((NAN && -INFINITY) ==
				 logicalAnd(F<double>(NAN), F<double>(-INFINITY)));
	// INF && NAN
	EXPECT_TRUE((INFINITY && NAN) ==
				 logicalAnd(F<double>(INFINITY), F<double>(NAN)));
	// INF && INF
	EXPECT_TRUE((INFINITY && INFINITY) ==
				 logicalAnd(F<double>(INFINITY), F<double>(INFINITY)));
	// INF && -INF
	EXPECT_TRUE((INFINITY && -INFINITY) ==
				 logicalAnd(F<double>(INFINITY), F<double>(-INFINITY)));
	// -INF && NAN
	EXPECT_TRUE((-INFINITY && NAN) ==
				 logicalAnd(F<double>(-INFINITY), F<double>(NAN)));
	// -INF && INF
	EXPECT_TRUE((-INFINITY && INFINITY) ==
				 logicalAnd(F<double>(-INFINITY), F<double>(INFINITY)));
	// -INF && -INF
	EXPECT_TRUE((-INFINITY && -INFINITY) ==
				 logicalAnd(F<double>(-INFINITY), F<double>(-INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// logicalOr()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntLogicalOrIntWorksCorrectly)
{
	EXPECT_TRUE((0 || 0) == logicalOr(I<int>(0), I<int>(0)));
	EXPECT_TRUE((0 || 1) == logicalOr(I<int>(0), I<int>(1)));
	EXPECT_TRUE((1 || 0) == logicalOr(I<int>(1), I<int>(0)));
	EXPECT_TRUE((1 || 1) == logicalOr(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntLogicalOrFloatWorksCorrectly)
{
	EXPECT_TRUE((0 || 0.0) == logicalOr(I<int>(0), F<double>(0.0)));
	EXPECT_TRUE((0 || 1.38) == logicalOr(I<int>(0), F<double>(1.38)));
	EXPECT_TRUE((10 || 0.0) == logicalOr(I<int>(10), F<double>(0.0)));
	EXPECT_TRUE((11 || 4.35) == logicalOr(I<int>(11), F<double>(4.35)));
}

TEST_F(NumberTest,
FloatLogicalOrIntWorksCorrectly)
{
	EXPECT_TRUE((0.0 || 0) == logicalOr(F<double>(0.0), I<int>(0)));
	EXPECT_TRUE((0.0 || 12) == logicalOr(F<double>(0.0), I<int>(12)));
	EXPECT_TRUE((125.89 || 0) == logicalOr(F<double>(125.89), I<int>(0)));
	EXPECT_TRUE((125.89 || 125) == logicalOr(F<double>(125.89), I<int>(125)));
}

TEST_F(NumberTest,
FloatLogicalOrFloatWorksCorrectly)
{
	EXPECT_TRUE((0.0 || 0.0) == logicalOr(F<double>(0.0), F<double>(0.0)));
	EXPECT_TRUE((0.0 || 12.35) == logicalOr(F<double>(0.0), F<double>(12.35)));
	EXPECT_TRUE((258.458 || 0.0) == logicalOr(F<double>(258.458), F<double>(0.0)));
	EXPECT_TRUE((18.4 || 458.2) == logicalOr(F<double>(18.4), F<double>(458.2)));
}

TEST_F(NumberTest,
SpecialFloatLogicalOrSpecialFloatWorksCorrectly)
{
	// NAN || NAN
	EXPECT_TRUE((NAN || NAN) ==
				 logicalOr(F<double>(NAN), F<double>(NAN)));
	// NAN || INF
	EXPECT_TRUE((NAN || INFINITY) ==
				 logicalOr(F<double>(NAN), F<double>(INFINITY)));
	// NAN || -INF
	EXPECT_TRUE((NAN || -INFINITY) ==
				 logicalOr(F<double>(NAN), F<double>(-INFINITY)));
	// INF || NAN
	EXPECT_TRUE((INFINITY || NAN) ==
				 logicalOr(F<double>(INFINITY), F<double>(NAN)));
	// INF || INF
	EXPECT_TRUE((INFINITY || INFINITY) ==
				 logicalOr(F<double>(INFINITY), F<double>(INFINITY)));
	// INF || -INF
	EXPECT_TRUE((INFINITY || -INFINITY) ==
				 logicalOr(F<double>(INFINITY), F<double>(-INFINITY)));
	// -INF || NAN
	EXPECT_TRUE((-INFINITY || NAN) ==
				 logicalOr(F<double>(-INFINITY), F<double>(NAN)));
	// -INF || INF
	EXPECT_TRUE((-INFINITY || INFINITY) ==
				 logicalOr(F<double>(-INFINITY), F<double>(INFINITY)));
	// -INF || -INF
	EXPECT_TRUE((-INFINITY || -INFINITY) ==
				 logicalOr(F<double>(-INFINITY), F<double>(-INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// logicalXor()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntLogicalXorIntWorksCorrectly)
{
	EXPECT_TRUE(false == logicalXor(I<int>(0), I<int>(0)));
	EXPECT_TRUE(true == logicalXor(I<int>(0), I<int>(1)));
	EXPECT_TRUE(true == logicalXor(I<int>(1), I<int>(0)));
	EXPECT_TRUE(false == logicalXor(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntLogicalXorFloatWorksCorrectly)
{
	EXPECT_TRUE(false == logicalXor(I<int>(0), F<double>(0.0)));
	EXPECT_TRUE(true == logicalXor(I<int>(0), F<double>(1.38)));
	EXPECT_TRUE(true == logicalXor(I<int>(10), F<double>(0.0)));
	EXPECT_TRUE(false == logicalXor(I<int>(11), F<double>(4.35)));
}

TEST_F(NumberTest,
FloatLogicalXorIntWorksCorrectly)
{
	EXPECT_TRUE(false == logicalXor(F<double>(0.0), I<int>(0)));
	EXPECT_TRUE(true == logicalXor(F<double>(0.0), I<int>(12)));
	EXPECT_TRUE(true == logicalXor(F<double>(125.89), I<int>(0)));
	EXPECT_TRUE(false == logicalXor(F<double>(125.89), I<int>(125)));
}

TEST_F(NumberTest,
FloatLogicalXorFloatWorksCorrectly)
{
	EXPECT_TRUE(false == logicalXor(F<double>(0.0), F<double>(0.0)));
	EXPECT_TRUE(true == logicalXor(F<double>(0.0), F<double>(12.35)));
	EXPECT_TRUE(true == logicalXor(F<double>(258.458), F<double>(0.0)));
	EXPECT_TRUE(false == logicalXor(F<double>(18.4), F<double>(458.2)));
}

TEST_F(NumberTest,
SpecialFloatLogicalXorSpecialFloatWorksCorrectly)
{
	// NAN xor NAN
	EXPECT_TRUE(false == logicalXor(F<double>(NAN), F<double>(NAN)));
	// NAN xor INF
	EXPECT_TRUE(false == logicalXor(F<double>(NAN), F<double>(INFINITY)));
	// NAN xor -INF
	EXPECT_TRUE(false == logicalXor(F<double>(NAN), F<double>(-INFINITY)));
	// INF xor NAN
	EXPECT_TRUE(false == logicalXor(F<double>(INFINITY), F<double>(NAN)));
	// INF xor INF
	EXPECT_TRUE(false == logicalXor(F<double>(INFINITY), F<double>(INFINITY)));
	// INF xor -INF
	EXPECT_TRUE(false == logicalXor(F<double>(INFINITY), F<double>(-INFINITY)));
	// -INF xor NAN
	EXPECT_TRUE(false == logicalXor(F<double>(-INFINITY), F<double>(NAN)));
	// -INF xor INF
	EXPECT_TRUE(false == logicalXor(F<double>(-INFINITY), F<double>(INFINITY)));
	// -INF xor -INF
	EXPECT_TRUE(false ==logicalXor(F<double>(-INFINITY), F<double>(-INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// bitNot()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
BitNotOfIntWorksCorrectly)
{
	unsigned char a = 0; int b = ~a;
	EXPECT_EQ(I<int>(b), bitNot(I<unsigned char>(a)));
	EXPECT_EQ(I<int>(~0), bitNot(I<int>(0)));
	EXPECT_EQ(I<int>(~vmin<int>()), bitNot(I<int>(vmin<int>())));
	EXPECT_EQ(I<int>(~vmax<int>()), bitNot(I<int>(vmax<int>())));
	EXPECT_EQ(I<int>(~12569), bitNot(I<int>(12569)));
	EXPECT_EQ(I<int>(~(-12569)), bitNot(I<int>(-12569)));
}

TEST_F(NumberTest,
BitNotOfFloatWorksCorrectly)
{
	EXPECT_DEATH(bitNot(F<double>(1.35)), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(NAN)), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(INFINITY)), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(-INFINITY)), ".*op.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// bitAnd()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntBitAndIntWorksCorrectly)
{
	EXPECT_TRUE(I<int>(0 & 0) == bitAnd(I<int>(0), I<int>(0)));
	EXPECT_TRUE(I<int>(0 & 1) == bitAnd(I<int>(0), I<int>(1)));
	EXPECT_TRUE(I<int>(1 & 0) == bitAnd(I<int>(1), I<int>(0)));
	EXPECT_TRUE(I<int>(1 & 1) == bitAnd(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntBitAndFloatWorksCorrectly)
{
	EXPECT_DEATH(bitAnd(I<int>(0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(I<int>(vmin<int>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(I<int>(0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(I<int>(4), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitAndIntWorksCorrectly)
{
	EXPECT_DEATH(bitAnd(F<double>(0.0), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(vmin<double>()), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(0.0), I<int>(vmax<int>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(4.536), I<int>(1789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitAndFloatWorksCorrectly)
{
	EXPECT_DEATH(bitAnd(F<double>(0.0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(vmin<double>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(0.0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitAnd(F<double>(4.536), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
SpecialFloatBitAndSpecialFloatWorksCorrectly)
{
	// NAN & NAN
	EXPECT_DEATH(bitAnd(F<double>(NAN), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & INF
	EXPECT_DEATH(bitAnd(F<double>(NAN), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & -INF
	EXPECT_DEATH(bitAnd(F<double>(NAN), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & NAN
	EXPECT_DEATH(bitAnd(F<double>(INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & INF
	EXPECT_DEATH(bitAnd(F<double>(INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & -INF
	EXPECT_DEATH(bitAnd(F<double>(INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & NAN
	EXPECT_DEATH(bitAnd(F<double>(-INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & INF
	EXPECT_DEATH(bitAnd(F<double>(-INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & -INF
	EXPECT_DEATH(bitAnd(F<double>(-INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// bitOr()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntBitOrIntWorksCorrectly)
{
	EXPECT_TRUE(I<int>(0 | 0) == bitOr(I<int>(0), I<int>(0)));
	EXPECT_TRUE(I<int>(0 | 1) == bitOr(I<int>(0), I<int>(1)));
	EXPECT_TRUE(I<int>(1 | 0) == bitOr(I<int>(1), I<int>(0)));
	EXPECT_TRUE(I<int>(1 | 1) == bitOr(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntBitOrFloatWorksCorrectly)
{
	EXPECT_DEATH(bitOr(I<int>(0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(I<int>(vmin<int>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(I<int>(0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(I<int>(4), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitOrIntWorksCorrectly)
{
	EXPECT_DEATH(bitOr(F<double>(0.0), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(vmin<double>()), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(0.0), I<int>(vmax<int>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(4.536), I<int>(1789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitOrFloatWorksCorrectly)
{
	EXPECT_DEATH(bitOr(F<double>(0.0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(vmin<double>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(0.0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitOr(F<double>(4.536), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
SpecialFloatBitOrSpecialFloatWorksCorrectly)
{
	// NAN & NAN
	EXPECT_DEATH(bitOr(F<double>(NAN), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & INF
	EXPECT_DEATH(bitOr(F<double>(NAN), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & -INF
	EXPECT_DEATH(bitOr(F<double>(NAN), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & NAN
	EXPECT_DEATH(bitOr(F<double>(INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & INF
	EXPECT_DEATH(bitOr(F<double>(INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & -INF
	EXPECT_DEATH(bitOr(F<double>(INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & NAN
	EXPECT_DEATH(bitOr(F<double>(-INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & INF
	EXPECT_DEATH(bitOr(F<double>(-INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & -INF
	EXPECT_DEATH(bitOr(F<double>(-INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// bitXor()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntBitXorIntWorksCorrectly)
{
	EXPECT_TRUE(I<int>(0) == bitXor(I<int>(0), I<int>(0)));
	EXPECT_TRUE(I<int>(1) == bitXor(I<int>(0), I<int>(1)));
	EXPECT_TRUE(I<int>(1) == bitXor(I<int>(1), I<int>(0)));
	EXPECT_TRUE(I<int>(0) == bitXor(I<int>(1), I<int>(1)));
}

TEST_F(NumberTest,
IntBitXorFloatWorksCorrectly)
{
	EXPECT_DEATH(bitXor(I<int>(0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(I<int>(vmin<int>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(I<int>(0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(I<int>(4), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitXorIntWorksCorrectly)
{
	EXPECT_DEATH(bitXor(F<double>(0.0), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(vmin<double>()), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(0.0), I<int>(vmax<int>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(4.536), I<int>(1789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitXorFloatWorksCorrectly)
{
	EXPECT_DEATH(bitXor(F<double>(0.0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(vmin<double>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(0.0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitXor(F<double>(4.536), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
SpecialFloatBitXorSpecialFloatWorksCorrectly)
{
	// NAN & NAN
	EXPECT_DEATH(bitXor(F<double>(NAN), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & INF
	EXPECT_DEATH(bitXor(F<double>(NAN), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & -INF
	EXPECT_DEATH(bitXor(F<double>(NAN), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & NAN
	EXPECT_DEATH(bitXor(F<double>(INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & INF
	EXPECT_DEATH(bitXor(F<double>(INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & -INF
	EXPECT_DEATH(bitXor(F<double>(INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & NAN
	EXPECT_DEATH(bitXor(F<double>(-INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & INF
	EXPECT_DEATH(bitXor(F<double>(-INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & -INF
	EXPECT_DEATH(bitXor(F<double>(-INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// bitLeftShift()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntBitLeftShiftIntWorksCorrectly)
{
	// Check correctness if at least one operand is zero.
	EXPECT_EQ(I<int>(0 << 0), bitLeftShift(I<int>(0), I<int>(0)));
	EXPECT_EQ(I<int>(110 << 0),
			  bitLeftShift(I<signed char>(110), I<signed char>(0)));
	EXPECT_EQ(I<int>(0 << 2), bitLeftShift(I<int>(0), I<int>(2)));

	// Check correctness if both operands are non-negative numbers.
	EXPECT_EQ(I<int>(1589 << 1), bitLeftShift(I<short>(1589), I<short>(1)));
	EXPECT_EQ(I<int>(148 << 2), bitLeftShift(I<int>(148), I<int>(2)));
	EXPECT_EQ(I<long>(1589 << 3), bitLeftShift(I<long>(1589), I<long>(3)));
	EXPECT_EQ(I<unsigned>(148 << 2),
			  bitLeftShift(I<unsigned>(148), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(1589 << 3),
			  bitLeftShift(I<unsigned long>(1589), I<unsigned long>(3)));

	// Check correctness if the first operand is negative and the second one is
	// positive.
	EXPECT_EQ(I<int>(-1589 << 1), bitLeftShift(I<short>(-1589), I<short>(1)));
	EXPECT_EQ(I<int>(-148 << 2), bitLeftShift(I<int>(-148), I<int>(2)));
	EXPECT_EQ(I<long>(-1589 << 3), bitLeftShift(I<long>(-1589), I<long>(3)));

	EXPECT_EQ(I<unsigned>(-148 << 2), bitLeftShift(I<int>(-148), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(-1589 << 3),
			  bitLeftShift(I<long>(-1589), I<unsigned long>(3)));

	// Check correctness if second operand is negative.
	EXPECT_DEATH(bitLeftShift(I<long>(-148), I<long>(-2)), ".*op2.intValue >= 0.*");
	EXPECT_DEATH(bitLeftShift(I<long>(148), I<long>(-2)), ".*op2.intValue >= 0.*");

	// Check correctness if the second operand specifies greater bit width that the
	// first one uses.
	EXPECT_DEATH(bitLeftShift(I<int>(148), I<int>(100)),
				 ".*op1.bitWidth.*op2.intValue.*");
}

TEST_F(NumberTest,
IntBitLeftShiftFloatWorksCorrectly)
{
	EXPECT_DEATH(bitLeftShift(I<int>(0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(I<int>(vmin<int>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(I<int>(0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(I<int>(4), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitLeftShiftIntWorksCorrectly)
{
	EXPECT_DEATH(bitLeftShift(F<double>(0.0), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(vmin<double>()), I<int>(0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(0.0), I<int>(vmax<int>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(4.536), I<int>(1789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
FloatBitLeftShiftFloatWorksCorrectly)
{
	EXPECT_DEATH(bitLeftShift(F<double>(0.0), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(vmin<double>()), F<double>(0.0)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(0.0), F<double>(vmax<double>())),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	EXPECT_DEATH(bitLeftShift(F<double>(4.536), F<double>(1.789)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

TEST_F(NumberTest,
SpecialFloatBitLeftShiftSpecialFloatWorksCorrectly)
{
	// NAN & NAN
	EXPECT_DEATH(bitLeftShift(F<double>(NAN), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & INF
	EXPECT_DEATH(bitLeftShift(F<double>(NAN), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// NAN & -INF
	EXPECT_DEATH(bitLeftShift(F<double>(NAN), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & NAN
	EXPECT_DEATH(bitLeftShift(F<double>(INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & INF
	EXPECT_DEATH(bitLeftShift(F<double>(INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// INF & -INF
	EXPECT_DEATH(bitLeftShift(F<double>(INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & NAN
	EXPECT_DEATH(bitLeftShift(F<double>(-INFINITY), F<double>(NAN)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & INF
	EXPECT_DEATH(bitLeftShift(F<double>(-INFINITY), F<double>(INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
	// -INF & -INF
	EXPECT_DEATH(bitLeftShift(F<double>(-INFINITY), F<double>(-INFINITY)),
				 ".*op1.isIntegral().*op2.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// bitRightShift()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntBitRightShiftIntWorksCorrectly)
{
	// Check correctness if at least one operand is zero.
	EXPECT_EQ(I<int>(0 >> 0), bitRightShift(I<int>(0), I<int>(0)));
	EXPECT_EQ(I<int>(110 >> 0),
			  bitRightShift(I<signed char>(110), I<signed char>(0)));
	EXPECT_EQ(I<int>(0 >> 2), bitRightShift(I<int>(0), I<int>(2)));

	// Check correctness if both operands are non-negative numbers.
	EXPECT_EQ(I<int>(1589 >> 1), bitRightShift(I<short>(1589), I<short>(1)));
	EXPECT_EQ(I<int>(148 >> 2), bitRightShift(I<int>(148), I<int>(2)));
	EXPECT_EQ(I<long>(1589 >> 3), bitRightShift(I<long>(1589), I<long>(3)));
	EXPECT_EQ(I<unsigned>(148 >> 2),
			  bitRightShift(I<unsigned>(148), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(1589 >> 3),
			  bitRightShift(I<unsigned long>(1589), I<unsigned long>(3)));

	// Check correctness if the first operand is negative and the second one is
	// positive.
	EXPECT_EQ(I<int>(-1589 >> 1), bitRightShift(I<short>(-1589), I<short>(1)));
	EXPECT_EQ(I<int>(-148 >> 2), bitRightShift(I<int>(-148), I<int>(2)));
	EXPECT_EQ(I<long>(-1589 >> 3), bitRightShift(I<long>(-1589), I<long>(3)));

	EXPECT_EQ(I<unsigned>(-148 >> 2), bitRightShift(I<int>(-148), I<unsigned>(2)));
	EXPECT_EQ(I<unsigned long>(-1589 >> 3),
			  bitRightShift(I<long>(-1589), I<unsigned long>(3)));

	// Check correctness if second operand is negative.
	EXPECT_DEATH(bitRightShift(I<long>(-148), I<long>(-2)), ".*op2.intValue >= 0.*");
	EXPECT_DEATH(bitRightShift(I<long>(148), I<long>(-2)), ".*op2.intValue >= 0.*");

	// Check correctness if the second operand specifies greater bit width that the
	// first one uses.
	EXPECT_DEATH(bitRightShift(I<int>(148), I<int>(100)),
				 ".*op1.bitWidth.*op2.intValue.*");
}

////////////////////////////////////////////////////////////////////////////////
// abs()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
AbsOfSignedIntWorksCorrectly)
{
	// Signed char.
	EXPECT_EQ(I<int>(abs(-1)), abs(I<signed char>(-1)));
	EXPECT_EQ(I<int>(abs(1)), abs(I<signed char>(1)));
	EXPECT_EQ(I<int>(128), abs(I<signed char>(vmin<signed char>())));
	EXPECT_EQ(I<signed char>(abs(vmax<signed char>())),
			  abs(I<signed char>(vmax<signed char>())));

	// Signed short.
	EXPECT_EQ(I<int>(abs(-11)), abs(I<short>(-11)));
	EXPECT_EQ(I<int>(abs(110)), abs(I<short>(110)));
	EXPECT_EQ(I<int>(abs(vmin<short>())), abs(I<short>(vmin<short>())));
	EXPECT_EQ(I<int>(abs(vmax<short>())), abs(I<short>(vmax<short>())));

	// Signed int.
	EXPECT_EQ(I<int>(abs(-1435)), abs(I<int>(-1435)));
	EXPECT_EQ(I<int>(abs(11025)), abs(I<int>(11025)));
	EXPECT_EQ(I<int>(abs(vmin<int>())), abs(I<int>(vmin<int>())));
	EXPECT_EQ(I<int>(abs(vmax<int>())), abs(I<int>(vmax<int>())));

	// Signed long.
	EXPECT_EQ(I<long>(labs(-143589)), abs(I<long>(-143589)));
	EXPECT_EQ(I<long>(labs(711025)), abs(I<long>(711025)));
	EXPECT_EQ(I<long>(labs(vmin<long>()+1)), abs(I<long>(vmin<long>()+1)));
	EXPECT_EQ(I<long>(labs(vmax<long>())), abs(I<long>(vmax<long>())));
	EXPECT_EQ(I<long>(labs(vmin<long>())), abs(I<long>(vmin<long>())));
}

TEST_F(NumberTest,
AbsOfUnsignedIntegralWorksCorrectly)
{
	EXPECT_DEATH(abs(I<unsigned>(3)), ".*isIntegral().*isSigned().*");
}

TEST_F(NumberTest,
AbsOfFloatWorksCorrectly)
{
	EXPECT_DEATH(abs(F<float>(-1.33)), ".*isIntegral().*isSigned().*");
}

////////////////////////////////////////////////////////////////////////////////
// intToFloat()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
IntToFloatOfInt)
{
	// Char.
	EXPECT_EQ(F<float>(-120.0), intToFloat(I<signed char>(-120)));
	EXPECT_EQ(F<float>(120.0), intToFloat(I<unsigned char>(120)));
	// Short.
	EXPECT_EQ(F<float>(vmin<signed short>()),
			  intToFloat(I<signed short>(vmin<signed short>())));
	EXPECT_EQ(F<float>(vmax<unsigned short>()),
			  intToFloat(I<unsigned short>(vmax<unsigned short>())));
	// Int.
	EXPECT_EQ(F<float>(-126589.0), intToFloat(I<signed int>(-126589)));
	EXPECT_EQ(F<float>(20589.0), intToFloat(I<unsigned int>(20589)));
	// Long.
	EXPECT_EQ(F<float>(-12658958.0), intToFloat(I<signed long>(-12658958)));
	EXPECT_EQ(F<float>(620589789.0), intToFloat(I<unsigned long>(620589789)));
}

TEST_F(NumberTest,
IntToFloatOfFloat)
{
	EXPECT_DEATH(bitNot(F<float>(vmin<float>())), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(vmax<double>())), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<long double>(1.35)), ".*op.isIntegral().*");
}

TEST_F(NumberTest,
IntToFloatOfSpecialFloat)
{
	EXPECT_DEATH(bitNot(F<double>(NAN)), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(INFINITY)), ".*op.isIntegral().*");
	EXPECT_DEATH(bitNot(F<double>(-INFINITY)), ".*op.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// min()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
MinOfTwoSignedIntsWorksCorrectly)
{
	// Minimum of operands of the same type.
	EXPECT_EQ(I<signed char>(-125), min(I<signed char>(-125), I<signed char>(-1)));
	EXPECT_EQ(I<short>(-12), min(I<short>(-12), I<short>(9)));
	EXPECT_EQ(I<int>(-1223697), min(I<int>(-12), I<int>(-1223697)));
	EXPECT_EQ(I<long>(vmin<long>()),
			  min(I<long>(vmax<long>()), I<long>(vmin<long>())));
}

TEST_F(NumberTest,
MinOfTwoUnsignedIntsWorksCorrectly)
{
	// Minimum of operands of the same type.
	EXPECT_EQ(I<unsigned char>(12),
			  min(I<unsigned char>(12), I<unsigned char>(16)));
	EXPECT_EQ(I<unsigned short>(9),
			  min(I<unsigned short>(259), I<unsigned short>(9)));
	EXPECT_EQ(I<unsigned int>(1223697),
			  min(I<unsigned int>(25888888), I<unsigned int>(1223697)));
	EXPECT_EQ(I<unsigned long>(vmin<unsigned long>()),
			  min(I<unsigned long>(vmax<unsigned long>()),
				  I<unsigned long>(vmin<unsigned long>())));
}

TEST_F(NumberTest,
MinOfFloatsWorksCorrectly)
{
	EXPECT_EQ(F<float>(-1.35), min(F<float>(-1.35), F<float>(-1.01)));
	EXPECT_EQ(F<double>(-12.58), min(F<double>(-12.58), F<double>(9.8)));
	EXPECT_EQ(F<long double>(-122.58),
			  min(F<long double>(-12.589), F<long double>(-122.58)));
}

TEST_F(NumberTest,
MinOfSpecialFloatsWorksCorrectly)
{
	EXPECT_EQ(F<float>(-INFINITY), min(F<float>(-INFINITY), F<float>(-INFINITY)));
	EXPECT_EQ(F<float>(-INFINITY), min(F<float>(-INFINITY), F<float>(-1.35)));
	EXPECT_EQ(F<float>(-INFINITY), min(F<float>(-INFINITY), F<float>(INFINITY)));

	EXPECT_EQ(F<float>(-INFINITY), min(F<float>(INFINITY), F<float>(-INFINITY)));
	EXPECT_EQ(F<float>(-1.35), min(F<float>(INFINITY), F<float>(-1.35)));
	EXPECT_EQ(F<float>(INFINITY), min(F<float>(INFINITY), F<float>(INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// max()
////////////////////////////////////////////////////////////////////////////////

TEST_F(NumberTest,
MaxOfTwoSignedIntsWorksCorrectly)
{
	// Maximum of operands of the same type.
	EXPECT_EQ(I<signed char>(-1), max(I<signed char>(-125), I<signed char>(-1)));
	EXPECT_EQ(I<short>(9), max(I<short>(-12), I<short>(9)));
	EXPECT_EQ(I<int>(-12), max(I<int>(-12), I<int>(-1223697)));
	EXPECT_EQ(I<long>(vmax<long>()),
			  max(I<long>(vmax<long>()), I<long>(vmin<long>())));
}

TEST_F(NumberTest,
MaxOfTwoUnsignedIntsWorksCorrectly)
{
	// Maximum of operands of the same type.
	EXPECT_EQ(I<unsigned char>(16),
			  max(I<unsigned char>(12), I<unsigned char>(16)));
	EXPECT_EQ(I<unsigned short>(259),
			  max(I<unsigned short>(259), I<unsigned short>(9)));
	EXPECT_EQ(I<unsigned int>(25888888),
			  max(I<unsigned int>(25888888), I<unsigned int>(1223697)));
	EXPECT_EQ(I<unsigned long>(vmax<unsigned long>()),
			  max(I<unsigned long>(vmax<unsigned long>()),
				  I<unsigned long>(vmin<unsigned long>())));
}

TEST_F(NumberTest,
MaxOfFloatsWorksCorrectly)
{
	EXPECT_EQ(F<float>(-1.01), max(F<float>(-1.35), F<float>(-1.01)));
	EXPECT_EQ(F<double>(9.8), max(F<double>(-12.58), F<double>(9.8)));
	EXPECT_EQ(F<long double>(-12.589),
			  max(F<long double>(-12.589), F<long double>(-122.58)));
}

TEST_F(NumberTest,
MaxOfSpecialFloatsWorksCorrectly)
{
	EXPECT_EQ(F<float>(-INFINITY), max(F<float>(-INFINITY), F<float>(-INFINITY)));
	EXPECT_EQ(F<float>(-1.35), max(F<float>(-INFINITY), F<float>(-1.35)));
	EXPECT_EQ(F<float>(INFINITY), max(F<float>(-INFINITY), F<float>(INFINITY)));

	EXPECT_EQ(F<float>(INFINITY), max(F<float>(INFINITY), F<float>(-INFINITY)));
	EXPECT_EQ(F<float>(INFINITY), max(F<float>(INFINITY), F<float>(-1.35)));
	EXPECT_EQ(F<float>(INFINITY), max(F<float>(INFINITY), F<float>(INFINITY)));
}

////////////////////////////////////////////////////////////////////////////////
// main()
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
