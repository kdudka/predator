/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   RangeTest.cc
* @brief  Test class for class Range.
* @date   2012
*/

#include <limits>
#include <cmath>
#include "Range.h"
#include "Number.h"
#include "gtest/gtest.h"

using namespace std;
typedef Range::Interval Interval;

// Minimal values.
template <typename T> T vmin() { return numeric_limits<T>::min(); }
template <> float vmin() { return -numeric_limits<float>::max(); }
template <> double vmin() { return -numeric_limits<double>::max(); }
template <> long double vmin() { return -numeric_limits<long double>::max(); }

// Maximal values.
template <typename T> T vmax() { return numeric_limits<T>::max(); }

// Number (int or float).
typedef Number N;

 // Int
template <typename T>
Number I(T val) { return Number(val, sizeof(T), vmin<T>() != 0); }

// Float
template <typename T>
Number F(T val) { return Number(val, sizeof(T)); }

class RangeTest : public ::testing::Test {
	protected:
		RangeTest() {
		}

		virtual ~RangeTest() {
		}

		virtual void SetUp() {
		}

		virtual void TearDown() {
		}

};

////////////////////////////////////////////////////////////////////////////////
// normalize()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IntervalsAreTransformedIfLowerBoundIsGreaterThanUpperBound)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(4)),
			  		Interval(I<int>(9), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(9), I<int>(4))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(4.9)),
			  		Interval(F<double>(89.5), F<double>(vmax<double>()))),
			  Range(Interval(F<double>(89.5), F<double>(4.9))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(4)),
					Interval(I<int>(6), I<int>(7)),
			  		Interval(I<int>(9), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(9), I<int>(4)),
			  	    Interval(I<int>(6), I<int>(7))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(4.9)),
					  Interval(F<double>(12.5), F<double>(14.3)),
					  Interval(F<double>(89.5), F<double>(vmax<double>()))),
			  Range(Interval(F<double>(89.5), F<double>(4.9)),
					  Interval(F<double>(12.5), F<double>(14.3))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(4)),
					Interval(I<int>(6), I<int>(7)),
					Interval(I<int>(12), I<int>(14)),
			  		Interval(I<int>(19), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(19), I<int>(4)),
			  	    Interval(I<int>(6), I<int>(7)),
			  	    Interval(I<int>(12), I<int>(14))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(4)),
					Interval(F<double>(6), F<double>(7)),
					Interval(F<double>(12), F<double>(14)),
					Interval(F<double>(19), F<double>(vmax<double>()))),
			  Range(Interval(F<double>(19), F<double>(4)),
					Interval(F<double>(6), F<double>(7)),
					Interval(F<double>(12), F<double>(14))));
}

TEST_F(RangeTest,
IntervalsAreSortedCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(9)),
					Interval(I<int>(89), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(89), I<int>(vmax<int>())),
					Interval(I<int>(vmin<int>()), I<int>(9))));

	EXPECT_EQ(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(21), I<int>(23)),
					Interval(I<int>(34), I<int>(38))),
			  Range(Interval(I<int>(34), I<int>(38)),
					Interval(I<int>(21), I<int>(23)),
					Interval(I<int>(4), I<int>(9))));

	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(2)),
					Interval(I<int>(4), I<int>(5)),
					Interval(I<int>(8), I<int>(10)),
					Interval(I<int>(15), I<int>(16))),
			  Range(Interval(I<int>(15), I<int>(16)),
					Interval(I<int>(1), I<int>(2)),
					Interval(I<int>(8), I<int>(10)),
					Interval(I<int>(4), I<int>(5))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(9.35)),
					Interval(F<double>(18.9), F<double>(vmax<double>()))),
			  Range(Interval(F<double>(18.9), F<double>(vmax<double>())),
			  		Interval(F<double>(vmin<double>()), F<double>(9.35))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(9.5)),
					Interval(F<double>(23.7), F<double>(23.9)),
					Interval(F<double>(34.5), F<double>(38.1))),
			  Range(Interval(F<double>(34.5), F<double>(38.1)),
					Interval(F<double>(23.7), F<double>(23.9)),
					Interval(F<double>(vmin<double>()), F<double>(9.5))));

	EXPECT_EQ(Range(Interval(F<double>(1.2), F<double>(2.2)),
					Interval(F<double>(4.5), F<double>(5.5)),
					Interval(F<double>(8.9), F<double>(10.2)),
					Interval(F<double>(15.4), F<double>(vmax<double>()))),
			  Range(Interval(F<double>(15.4), F<double>(vmax<double>())),
					Interval(F<double>(1.2), F<double>(2.2)),
					Interval(F<double>(8.9), F<double>(10.2)),
					Interval(F<double>(4.5), F<double>(5.5))));

}

TEST_F(RangeTest,
SameIntervalsAreJoinedCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(28))),
			  Range(Interval(I<int>(1), I<int>(28)),
					Interval(I<int>(1), I<int>(28)),
					Interval(I<int>(1), I<int>(28)),
					Interval(I<int>(1), I<int>(28))));

	EXPECT_EQ(Range(Interval(F<double>(1), F<double>(28))),
			  Range(Interval(F<double>(1), F<double>(28)),
					Interval(F<double>(1), F<double>(28)),
					Interval(F<double>(1), F<double>(28)),
					Interval(F<double>(1), F<double>(28))));
}

TEST_F(RangeTest,
NestedIntervalsAreJoinedCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(28))),
			  Range(Interval(I<int>(6), I<int>(23)),
					Interval(I<int>(5), I<int>(19)),
					Interval(I<int>(2), I<int>(20)),
					Interval(I<int>(vmin<int>()), I<int>(28))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(5)),
					Interval(I<int>(9), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(9), I<int>(4)),
					Interval(I<int>(10), I<int>(5)),
					Interval(I<int>(2), I<int>(4)),
					Interval(I<int>(58), I<int>(60))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(28.5))),
			  Range(Interval(F<double>(6.5), F<double>(23.5)),
					Interval(F<double>(5.1), F<double>(19.8)),
					Interval(F<double>(2.3), F<double>(20.7)),
					Interval(F<double>(vmin<double>()), F<double>(28.5))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(5.5)),
					Interval(F<double>(9.7), F<double>(vmax<double>())),
					Interval(F<double>(7.1), F<double>(7.3))),
			  Range(Interval(F<double>(9.7), F<double>(4.5)),
					Interval(F<double>(10.9), F<double>(5.5)),
					Interval(F<double>(2.4), F<double>(4.8)),
					Interval(F<double>(7.1), F<double>(7.3))));
}

TEST_F(RangeTest,
OverlappingIntervalsAreJoinedCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(36), I<int>(52)),
					Interval(I<int>(7), I<int>(40)),
					Interval(I<int>(3), I<int>(10)),
					Interval(I<int>(50), I<int>(5))));
}

TEST_F(RangeTest,
NestedAndOverlappingIntervalsAreJoinedCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<double>()), F<double>(8956.23))),
			  Range(Interval(F<double>(4327.3), F<double>(4350.5)),
					Interval(F<double>(4326.8), F<double>(8956.23)),
					Interval(F<double>(4256.98), F<double>(7896.36)),
					Interval(F<double>(vmin<double>()), F<double>(4356.85))));
}

TEST_F(RangeTest,
NeigbouringIntervalsAreJoinedCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10))),
			  Range(Interval(I<int>(8), I<int>(10)),
			  		Interval(I<int>(1), I<int>(8))));

	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10))),
			  Range(Interval(I<int>(1), I<int>(7)),
			  		Interval(I<int>(8), I<int>(10))));

	EXPECT_EQ(Range(Interval(F<double>(1.0), F<double>(10.5))),
			  Range(Interval(F<double>(8.5), F<double>(10.5)),
			  		Interval(F<double>(1.0), F<double>(8.5))));

	EXPECT_EQ(Range(Interval(F<double>(1.1), F<double>(10.8))),
			  Range(Interval(F<double>(1.1), F<double>(8.4)),
			  		Interval(F<double>(8.5), F<double>(10.8))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(9), I<int>(4)),
			  		Interval(I<int>(5), I<int>(8))));
}

TEST_F(RangeTest,
NotNeighbouringIntervalsAreNotJoined)
{
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(6)),
					Interval(I<int>(8), I<int>(10))),
			  Range(Interval(I<int>(8), I<int>(10)),
			  		Interval(I<int>(1), I<int>(6))));

	EXPECT_EQ(Range(Interval(F<double>(1.1), F<double>(8.3)),
					Interval(F<double>(8.5), F<double>(10.8))),
			  Range(Interval(F<double>(1.1), F<double>(8.3)),
			  		Interval(F<double>(8.5), F<double>(10.8))));
}

TEST_F(RangeTest,
OnlyOneNanInRangeIsKept)
{
	Range r = Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-NAN), F<double>(-NAN)),
			  		Interval(F<double>(NAN), F<double>(-NAN)),
			  		Interval(F<double>(-NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r.size());
}

////////////////////////////////////////////////////////////////////////////////
// containsNan()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsNanOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsNan(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsNanOfRangeThatContainsNanWorksCorrectly)
{
	Range r1 = Range(Interval(F<double>(NAN), F<double>(NAN)),
					 Interval(F<double>(-NAN), F<double>(-NAN)),
			  		 Interval(F<double>(NAN), F<double>(-NAN)),
			  		 Interval(F<double>(-NAN), F<double>(NAN)));
	EXPECT_TRUE(r1.containsNan());

	Range r2 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r2.containsNan());
}

TEST_F(RangeTest,
ContainsNanOfRangeThatDoesNotContainNanWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(vmin<int>()), I<int>(4)),
					 Interval(I<int>(6), I<int>(7)),
					 Interval(I<int>(12), I<int>(14)),
			  		 Interval(I<int>(19), I<int>(vmax<int>())));
	EXPECT_FALSE(r1.containsNan());

	Range r2 = Range(Interval(F<double>(1.2), F<double>(2.2)),
					 Interval(F<double>(4.5), F<double>(5.5)),
					 Interval(F<double>(8.9), F<double>(10.2)),
					 Interval(F<double>(15.4), F<double>(vmax<double>())));
	EXPECT_FALSE(r2.containsNan());

	Range r3 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)),
			 		 Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_FALSE(r3.containsNan());
}

////////////////////////////////////////////////////////////////////////////////
// containsPositiveInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsPositiveInfOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsPositiveInf(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsPositiveInfOfRangeThatContainsPositiveInfWorksCorrectly)
{
	Range r1 = Range(Interval(F<double>(NAN), F<double>(NAN)),
					 Interval(F<double>(-NAN), F<double>(-NAN)),
			  		 Interval(F<double>(NAN), F<double>(-NAN)),
			  		 Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_TRUE(r1.containsPositiveInf());

	Range r2 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r2.containsPositiveInf());

	Range r3 = Range(Interval(F<double>(-43.369), F<double>(INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r3.containsPositiveInf());

	Range r4 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r4.containsPositiveInf());
}

TEST_F(RangeTest,
ContainsPositiveInfOfRangeThatDoesNotContainPositiveInfWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(vmin<int>()), I<int>(4)),
					 Interval(I<int>(6), I<int>(7)),
					 Interval(I<int>(12), I<int>(14)),
			  		 Interval(I<int>(19), I<int>(vmax<int>())));
	EXPECT_FALSE(r1.containsPositiveInf());

	Range r2 = Range(Interval(F<double>(1.2), F<double>(2.2)),
					 Interval(F<double>(4.5), F<double>(5.5)),
					 Interval(F<double>(8.9), F<double>(10.2)),
					 Interval(F<double>(15.4), F<double>(vmax<double>())));
	EXPECT_FALSE(r2.containsPositiveInf());

	Range r3 = Range(Interval(F<double>(NAN), F<double>(NAN)),
			 		 Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_FALSE(r3.containsPositiveInf());
}

////////////////////////////////////////////////////////////////////////////////
// containsNegativeInf()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsNegativeInfOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsNegativeInf(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsNegativeInfOfRangeThatContainsNegativeInfWorksCorrectly)
{
	Range r1 = Range(Interval(F<double>(NAN), F<double>(NAN)),
					 Interval(F<double>(-NAN), F<double>(-NAN)),
			  		 Interval(F<double>(NAN), F<double>(-NAN)),
			  		 Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_TRUE(r1.containsNegativeInf());

	Range r2 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r2.containsNegativeInf());

	Range r3 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r3.containsNegativeInf());

	Range r4 = Range(Interval(F<double>(-INFINITY), F<double>(4.8)),
			 		 Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r4.containsNegativeInf());

	Range r5 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_TRUE(r5.containsNegativeInf());
}

TEST_F(RangeTest,
ContainsNegativeInfOfRangeThatDoesNotContainNegativeInfWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(vmin<int>()), I<int>(4)),
					 Interval(I<int>(6), I<int>(7)),
					 Interval(I<int>(12), I<int>(14)),
			  		 Interval(I<int>(19), I<int>(vmax<int>())));
	EXPECT_FALSE(r1.containsNegativeInf());

	Range r2 = Range(Interval(F<double>(1.2), F<double>(2.2)),
					 Interval(F<double>(4.5), F<double>(5.5)),
					 Interval(F<double>(8.9), F<double>(10.2)),
					 Interval(F<double>(15.4), F<double>(vmax<double>())));
	EXPECT_FALSE(r2.containsNegativeInf());

	Range r3 = Range(Interval(F<double>(NAN), F<double>(NAN)),
			 		 Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_FALSE(r3.containsNegativeInf());
}

////////////////////////////////////////////////////////////////////////////////
// containsTrue()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsTrueOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsTrue(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsTrueOfRangeThatContainsTrueWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(-10), I<int>(10)));
	EXPECT_TRUE(r1.containsTrue());
	Range r2 = Range(Interval(I<int>(-10), I<int>(0)));
	EXPECT_TRUE(r2.containsTrue());
	Range r3 = Range(Interval(I<int>(0), I<int>(10)));
	EXPECT_TRUE(r3.containsTrue());
	Range r4 = Range(Interval(I<int>(10), I<int>(50)));
	EXPECT_TRUE(r4.containsTrue());

	Range r5 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_TRUE(r5.containsTrue());
	Range r6 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_TRUE(r6.containsTrue());
	Range r7 = Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_TRUE(r7.containsTrue());
}

TEST_F(RangeTest,
ContainsTrueOfRangeThatDoesNotContainTrueWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(0), I<int>(0)));
	EXPECT_FALSE(r1.containsTrue());
	Range r2 = Range(Interval(F<double>(0.0), F<double>(0.0)));
	EXPECT_FALSE(r2.containsTrue());
}

////////////////////////////////////////////////////////////////////////////////
// containsFalse()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsFalseOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsFalse(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsFalseOfRangeThatContainsFalseWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(0), I<int>(0)));
	EXPECT_TRUE(r1.containsFalse());
	Range r2 = Range(Interval(F<double>(0.0), F<double>(0.0)));
	EXPECT_TRUE(r2.containsFalse());
	Range r3 = Range(Interval(I<int>(-10), I<int>(10)));
	EXPECT_TRUE(r3.containsFalse());
	Range r4 = Range(Interval(I<int>(-10), I<int>(0)));
	EXPECT_TRUE(r4.containsFalse());
	Range r5 = Range(Interval(I<int>(0), I<int>(10)));
	EXPECT_TRUE(r5.containsFalse());
}

TEST_F(RangeTest,
ContainsFalseOfRangeThatDoesNotContainFalseWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(10), I<int>(50)));
	EXPECT_FALSE(r1.containsFalse());
	Range r2 = Range(Interval(F<double>(vmin<double>()), F<double>(-0.1)),
					 Interval(F<double>(0.1), F<double>(vmax<double>())));
	EXPECT_FALSE(r2.containsFalse());

	Range r3 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_FALSE(r3.containsFalse());
	Range r4 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_FALSE(r4.containsFalse());
	Range r5 = Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_FALSE(r5.containsFalse());
}

////////////////////////////////////////////////////////////////////////////////
// containsZero()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsZeroOfEmptyRangeWorksCorrectly)
{
	Range r;
	EXPECT_DEATH(r.containsZero(), ".!empty().*");
}

TEST_F(RangeTest,
ContainsZeroOfRangeThatContainsZeroWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(0), I<int>(0)));
	EXPECT_TRUE(r1.containsZero());
	Range r2 = Range(Interval(F<double>(0.0), F<double>(0.0)));
	EXPECT_TRUE(r2.containsZero());
	Range r3 = Range(Interval(I<int>(-10), I<int>(10)));
	EXPECT_TRUE(r3.containsZero());
	Range r4 = Range(Interval(I<int>(-10), I<int>(0)));
	EXPECT_TRUE(r4.containsZero());
	Range r5 = Range(Interval(I<int>(0), I<int>(10)));
	EXPECT_TRUE(r5.containsZero());
}

TEST_F(RangeTest,
ContainsZeroOfRangeThatDoesNotContainZeroWorksCorrectly)
{
	Range r1 = Range(Interval(I<int>(10), I<int>(50)));
	EXPECT_FALSE(r1.containsZero());
	Range r2 = Range(Interval(F<double>(vmin<double>()), F<double>(-0.1)),
					 Interval(F<double>(0.1), F<double>(vmax<double>())));
	EXPECT_FALSE(r2.containsZero());

	Range r3 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_FALSE(r3.containsZero());
	Range r4 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_FALSE(r4.containsZero());
	Range r5 = Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_FALSE(r5.containsZero());
}

////////////////////////////////////////////////////////////////////////////////
// logicalNot()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalNotOfIntsWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalNot(Range(Interval(I<int>(-10), I<int>(10)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalNot(Range(Interval(I<int>(0), I<int>(0)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalNot(Range(Interval(I<int>(1), I<int>(1)))));
}

TEST_F(RangeTest,
LogicalNotOfFloatsWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalNot(Range(Interval(F<double>(-13.58), F<double>(10)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalNot(Range(Interval(F<double>(0.0), F<double>(0.0)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalNot(Range(Interval(F<double>(547.5), F<double>(588.8)))));
}

TEST_F(RangeTest,
LogicalNotOfSpecialFloatsWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalNot(Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_EQ(Range(Interval(I<int>(0), F<int>(0))),
			  logicalNot(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(I<int>(0), F<int>(0))),
			  logicalNot(Range(Interval(F<double>(NAN), F<double>(NAN)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalAnd()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalAndOfIntsWorksCorrectly)
{
	// [0,1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(0), I<int>(589)))));
	// [0,1] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [0,1] && [1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(1), I<int>(180)))));
	// [0] && [0,1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(-15), I<int>(180)))));
	// [0] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [0] && [1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(10), I<int>(10)))));
	// [1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(15), I<int>(258))),
			  			 Range(Interval(I<int>(-15), I<int>(180)))));
	// [1] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(15), I<int>(15))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [1] && [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(1), I<int>(1))),
			  			 Range(Interval(I<int>(10), I<int>(10)))));
}

TEST_F(RangeTest,
LogicalAndOfIntAndFloatWorksCorrectly)
{
	// [0,1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(0.0), F<double>(58.9)))));
	// [0,1] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0,1] && [1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(1.58), F<double>(18.0)))));
	// [0] && [0,1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(-15.36), F<double>(18.3)))));
	// [0] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0] && [1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(10.5), F<double>(10.8)))));
	// [1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(15), I<int>(258))),
			  			 Range(Interval(F<double>(-15.4), F<double>(18.7)))));
	// [1] && [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalAnd(Range(Interval(I<int>(15), I<int>(15))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [1] && [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(I<int>(1), I<int>(1))),
			  			 Range(Interval(F<double>(1.5), F<double>(10)))));

}

TEST_F(RangeTest,
LogicalAndOfFloatAndIntWorksCorrectly)
{
	// [1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(15.4), F<double>(25.8))),
			  			 Range(Interval(I<int>(-15), I<int>(18)))));
}

TEST_F(RangeTest,
LogicalAndOfFloatsWorksCorrectly)
{
	// [0,1] && [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-10), F<double>(10))),
			  			 Range(Interval(F<double>(0.0), F<double>(58.9)))));
}

TEST_F(RangeTest,
LogicalAndOfSpecialFloatsWorksCorrectly)
{
	// [NAN, NAN] && [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [NAN, NAN] && [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [NAN, NAN] && [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [NAN, NAN] && [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [INF, INF] && [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [INF, INF] && [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [INF, INF] && [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [INF, INF] && [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] && [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, -INF] && [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, -INF] && [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] && [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] && [NAN, NAN] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, INF] && [-INF, -INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, INF] && [INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] && [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalAnd(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalOr()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalOrOfIntsWorksCorrectly)
{
	// [0,1] || [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(I<int>(0), I<int>(589)))));
	// [0,1] || [0] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(I<int>(0), I<int>(0)))));
	// [0,1] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(I<int>(1), I<int>(180)))));
	// [0] || [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(I<int>(-15), I<int>(180)))));
	// [0] || [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(I<int>(0), I<int>(0)))));
	// [0] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(I<int>(10), I<int>(10)))));
	// [1] || [0,1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(15), I<int>(258))),
			  			Range(Interval(I<int>(-15), I<int>(180)))));
	// [1] || [0] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(15), I<int>(15))),
			  			Range(Interval(I<int>(0), I<int>(0)))));
	// [1] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(1), I<int>(1))),
			  			Range(Interval(I<int>(10), I<int>(10)))));
}

TEST_F(RangeTest,
LogicalOrOfIntAndFloatWorksCorrectly)
{
	// [0,1] || [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(F<double>(0.0), F<double>(5.89)))));
	// [0,1] || [0] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0,1] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(-10), I<int>(10))),
			  			Range(Interval(F<double>(19.5), F<double>(180.2)))));
	// [0] || [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(F<double>(-15.7), F<double>(18.0)))));
	// [0] || [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(0), I<int>(0))),
			  			Range(Interval(F<double>(10.5), F<double>(10.8)))));
	// [1] || [0,1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(15), I<int>(258))),
			  			Range(Interval(F<double>(-15.8), F<double>(18.9)))));
	// [1] || [0] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(15), I<int>(15))),
			  			Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [1] || [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(I<int>(1), I<int>(1))),
			  			Range(Interval(F<double>(10.5), F<double>(10.8)))));
}

TEST_F(RangeTest,
LogicalOrOfFloatAndIntWorksCorrectly)
{
	// [0] || [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(0.0), F<double>(0.0))),
			  			Range(Interval(I<int>(-15), I<int>(18)))));
}

TEST_F(RangeTest,
LogicalOrOfFloatsWorksCorrectly)
{
	// [1] || [0,1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(15), F<double>(258))),
			  			Range(Interval(F<double>(-15.8), F<double>(18.9)))));
}

TEST_F(RangeTest,
LogicalOrOfSpecialFloatsWorksCorrectly)
{
	// [NAN, NAN] || [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [NAN, NAN] || [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [NAN, NAN] || [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [NAN, NAN] || [-INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [INF, INF] || [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [INF, INF] || [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [INF, INF] || [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [INF, INF] || [-INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] || [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, -INF] || [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, -INF] || [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] || [-INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] || [NAN, NAN] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, INF] || [-INF, -INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, INF] || [INF, INF] = [1,1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] || [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalOr(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalXor()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalXorOfIntsWorksCorrectly)
{
	// [0,1] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(0), I<int>(589)))));
	// [0,1] xor [0] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [0,1] xor [1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(I<int>(1), I<int>(180)))));
	// [0] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(-15), I<int>(180)))));
	// [0] xor [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [0] xor [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(I<int>(10), I<int>(10)))));
	// [1] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(15), I<int>(258))),
			  			 Range(Interval(I<int>(-15), I<int>(180)))));
	// [1] xor [0] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(15), I<int>(15))),
			  			 Range(Interval(I<int>(0), I<int>(0)))));
	// [1] xor [1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(I<int>(1), I<int>(1))),
			  			 Range(Interval(I<int>(10), I<int>(10)))));
}

TEST_F(RangeTest,
LogicalXorOfIntAndFloatWorksCorrectly)
{
	// [0,1] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(0.0), F<double>(5.89)))));
	// [0,1] xor [0] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0,1] xor [1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(-10), I<int>(10))),
			  			 Range(Interval(F<double>(1.9), F<double>(18.0)))));
	// [0] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(-1.5), F<double>(1.80)))));
	// [0] xor [0] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [0] xor [1] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(0), I<int>(0))),
			  			 Range(Interval(F<double>(10.0), F<double>(10.5)))));
	// [1] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(15), I<int>(258))),
			  			 Range(Interval(F<double>(-15.8), F<double>(18.0)))));
	// [1] xor [0] = [1]
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(1))),
			  logicalXor(Range(Interval(I<int>(15), I<int>(15))),
			  			 Range(Interval(F<double>(0.0), F<double>(0.0)))));
	// [1] xor [1] = [0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(I<int>(1), I<int>(1))),
			  			 Range(Interval(F<double>(10.8), F<double>(10.99)))));
}

TEST_F(RangeTest,
LogicalXorOfFloatAndIntWorksCorrectly)
{
	// [0,1] xor [0,1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-10.5), F<double>(10.8))),
			  			 Range(Interval(I<int>(0), I<int>(589)))));
}

TEST_F(RangeTest,
LogicalXorOfFloatsWorksCorrectly)
{
	// [0,1] xor [1] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-10), F<double>(10))),
			  			 Range(Interval(F<double>(1.9), F<double>(18.0)))));
}

TEST_F(RangeTest,
LogicalXorOfSpecialFloatsWorksCorrectly)
{
	// [NAN, NAN] xor [NAN, NAN] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [NAN, NAN] xor [-INF, -INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [NAN, NAN] xor [INF, INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [NAN, NAN] xor [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [INF, INF] xor [NAN, NAN] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [INF, INF] xor [-INF, -INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [INF, INF] xor [INF, INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [INF, INF] xor [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] xor [NAN, NAN] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, -INF] xor [-INF, -INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, -INF] xor [INF, INF] = [0,0]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(0))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, -INF] xor [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] xor [NAN, NAN] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(NAN), F<double>(NAN)))));
	// [-INF, INF] xor [-INF, -INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	// [-INF, INF] xor [INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	// [-INF, INF] xor [-INF, INF] = [0,1]
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(1))),
			  logicalXor(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			 Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
}

////////////////////////////////////////////////////////////////////////////////
// ==, !=
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
EmptyRangesAreEqual)
{
	EXPECT_EQ(Range(), Range());
}

TEST_F(RangeTest,
TwoRangesContainingSamePointAreEqual)
{
	EXPECT_EQ(Range(I<int>(vmin<int>())), Range(I<int>(vmin<int>())));
	EXPECT_EQ(Range(F<double>(1789.35)), Range(F<double>(1789.35)));
	EXPECT_EQ(Range(I<int>(4589)),
			  Range(Interval(I<int>(4589), I<int>(4589))));
	EXPECT_EQ(Range(F<double>(4589.5689)),
			  Range(Interval(F<double>(4589.5689), F<double>(4589.5689))));
}

TEST_F(RangeTest,
TwoRangesContainingDifferentPointAreNotEqual)
{
	EXPECT_NE(Range(I<int>(vmin<int>())), Range(I<int>(vmax<int>())));
	EXPECT_NE(Range(F<double>(1.35)), Range(F<double>(1789.35)));
	EXPECT_NE(Range(I<int>(4589)),
			  Range(Interval(I<int>(5889), I<int>(5889))));
	EXPECT_NE(Range(F<double>(4589.5689)),
			  Range(Interval(F<double>(4587.5689), F<double>(4587.5689))));
}

TEST_F(RangeTest,
TwoRangesContainingSameIntervalAreEqual)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(98))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(98))));
	EXPECT_EQ(Range(Interval(F<double>(436.5), F<double>(9887.8))),
			  Range(Interval(F<double>(436.5), F<double>(9887.8))));
}

TEST_F(RangeTest,
TwoRangesContainingDifferentIntervalAreNotEqual)
{
	EXPECT_NE(Range(Interval(I<int>(vmin<int>()), I<int>(97))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))));
	EXPECT_NE(Range(Interval(F<double>(4.5), F<double>(9.8))),
			  Range(Interval(F<double>(4.49), F<double>(9.8))));
}

TEST_F(RangeTest,
TwoRangesContainingTwoSameIntervalsAreEqual)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(9)),
					Interval(I<int>(11), I<int>(13))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(9)),
			  		Interval(I<int>(11), I<int>(13))));

	EXPECT_EQ(Range(Interval(F<double>(4.5), F<double>(5.8)),
					Interval(F<double>(10.0), F<double>(vmax<int>()))),
			  Range(Interval(F<double>(4.5), F<double>(5.8)),
			  		Interval(F<double>(10.0), F<double>(vmax<int>()))));
}

TEST_F(RangeTest,
TwoRangesContainingTwoDifferentIntervalsAreNotEqual)
{
	EXPECT_NE(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(11), I<int>(13))),
			  Range(Interval(I<int>(4), I<int>(9)),
			  		Interval(I<int>(11), I<int>(vmax<int>()))));

	EXPECT_NE(Range(Interval(F<double>(4.5), F<double>(5.8)),
					Interval(F<double>(11.0), F<double>(12.5))),
			  Range(Interval(F<double>(vmin<int>()), F<double>(5.8)),
			  		Interval(F<double>(11.0), F<double>(12.5))));
}

TEST_F(RangeTest,
TwoRangesContainingThreeSameIntervalsAreEqual)
{
	EXPECT_EQ(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(15), I<int>(17)),
					Interval(I<int>(19), I<int>(25))),
			  Range(Interval(I<int>(4), I<int>(9)),
			  		Interval(I<int>(15), I<int>(17)),
			  		Interval(I<int>(19), I<int>(25))));

	EXPECT_EQ(Range(Interval(F<double>(4.5), F<double>(6.8)),
					Interval(F<double>(8.5), F<double>(9.8)),
					Interval(F<double>(15.3), F<double>(15.9))),
			  Range(Interval(F<double>(4.5), F<double>(6.8)),
			  		Interval(F<double>(8.5), F<double>(9.8)),
			  		Interval(F<double>(15.3), F<double>(15.9))));
}

TEST_F(RangeTest,
TwoRangesContainingThreeDifferentIntervalsAreNotEqual)
{
	EXPECT_NE(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(15), I<int>(17)),
					Interval(I<int>(19), I<int>(25))),
			  Range(Interval(I<int>(4), I<int>(9)),
			  		Interval(I<int>(15), I<int>(17)),
			  		Interval(I<int>(19), I<int>(24))));

	EXPECT_NE(Range(Interval(F<double>(4.5), F<double>(6.8)),
					Interval(F<double>(8.5), F<double>(9.8)),
					Interval(F<double>(15.3), F<double>(15.9))),
			  Range(Interval(F<double>(4.5), F<double>(6.8)),
			  		Interval(F<double>(8.5), F<double>(9.8)),
			  		Interval(F<double>(15.3), F<double>(15.85))));
}

TEST_F(RangeTest,
TwoRangesContainingFourSameIntervalsAreEqual)
{
	EXPECT_EQ(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(15), I<int>(17)),
					Interval(I<int>(19), I<int>(25)),
					Interval(I<int>(29), I<int>(35))),
			  Range(Interval(I<int>(4), I<int>(9)),
			  		Interval(I<int>(15), I<int>(17)),
			  		Interval(I<int>(19), I<int>(25)),
					Interval(I<int>(29), I<int>(35))));

	EXPECT_EQ(Range(Interval(F<double>(4.5), F<double>(6.8)),
					Interval(F<double>(8.5), F<double>(9.8)),
					Interval(F<double>(15.3), F<double>(15.9)),
					Interval(F<double>(21.3), F<double>(25.9))),
			  Range(Interval(F<double>(4.5), F<double>(6.8)),
			  		Interval(F<double>(8.5), F<double>(9.8)),
			  		Interval(F<double>(15.3), F<double>(15.9)),
					Interval(F<double>(21.3), F<double>(25.9))));
}

TEST_F(RangeTest,
TwoRangesContainingFourDifferentIntervalsAreNotEqual)
{
	EXPECT_NE(Range(Interval(I<int>(4), I<int>(9)),
					Interval(I<int>(15), I<int>(17)),
					Interval(I<int>(19), I<int>(25)),
					Interval(I<int>(29), I<int>(35))),
			  Range(Interval(I<int>(4), I<int>(9)),
			  		Interval(I<int>(15), I<int>(17)),
			  		Interval(I<int>(19), I<int>(25)),
					Interval(I<int>(29), I<int>(vmax<int>()))));

	EXPECT_NE(Range(Interval(F<double>(4.5), F<double>(6.8)),
					Interval(F<double>(8.5), F<double>(9.8)),
					Interval(F<double>(15.3), F<double>(15.9)),
					Interval(F<double>(21.3), F<double>(25.9))),
			  Range(Interval(F<double>(4.5), F<double>(6.8)),
			  		Interval(F<double>(8.5), F<double>(9.8)),
			  		Interval(F<double>(15.3), F<double>(15.9)),
					Interval(F<double>(21.2), F<double>(25.9))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalEqOfAtLeastOneEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(logicalEq(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalEq(Range(), Range(Interval(I<int>(0), I<int>(10)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalEq(Range(Interval(I<int>(-145), I<int>(18))), Range()),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalEq(Range(), Range(Interval(F<double>(1.1), F<double>(1.35)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalEq(Range(Interval(F<double>(-14.5), F<double>(1.8))), Range()),
				 ".*r1.empty().*!r2.empty().*");
}

TEST_F(RangeTest,
LogicalEqOfTwoEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(1)) == logicalEq(Range(I<int>(89)), Range(I<int>(89))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalEq(Range(F<double>(14.5)), Range(F<double>(14.5))));

	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(4), I<int>(6))),
						  Range(Interval(I<int>(4), I<int>(6)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9))),
						  Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9)))));
}

TEST_F(RangeTest,
LogicalEqOfTwoNonEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalEq(Range(Interval(I<int>(-89), I<int>(-89))),
						  Range(Interval(I<int>(90), I<int>(90)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalEq(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(96), I<int>(98)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalEq(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(0)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalEq(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalEq(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(-0.8)))));
}

TEST_F(RangeTest,
LogicalEqOfTwoNeighbouringRangesWorksCorretly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(95), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(12)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(15), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalEqOfTwoOverlappingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(13)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(10.5), F<double>(15))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalEqOfTwoNestingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(89), I<int>(100))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(23)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(10.5), F<double>(35.8))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(28.3)))));
}

TEST_F(RangeTest,
LogicalEqOfTwoSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(99.9))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalEq(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(-INFINITY), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(INFINITY))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalNeq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalNeqOfAtLeastOneEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(logicalNeq(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalNeq(Range(), Range(Interval(I<int>(0), I<int>(10)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalNeq(Range(Interval(I<int>(-145), I<int>(18))), Range()),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalNeq(Range(), Range(Interval(F<double>(1.1), F<double>(1.35)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalNeq(Range(Interval(F<double>(-14.5), F<double>(1.8))), Range()),
				 ".*r1.empty().*!r2.empty().*");
}

TEST_F(RangeTest,
LogicalNeqOfTwoEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(0)) == logicalNeq(Range(I<int>(89)), Range(I<int>(89))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalNeq(Range(F<double>(14.5)), Range(F<double>(14.5))));

	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(4), I<int>(6))),
						   Range(Interval(I<int>(4), I<int>(6)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(4.5), F<double>(6.8)),
						 		 Interval(F<double>(8.5), F<double>(9.8)),
								 Interval(F<double>(15.3), F<double>(15.9)),
								 Interval(F<double>(21.3), F<double>(25.9))),
						   Range(Interval(F<double>(4.5), F<double>(6.8)),
								 Interval(F<double>(8.5), F<double>(9.8)),
								 Interval(F<double>(15.3), F<double>(15.9)),
								 Interval(F<double>(21.3), F<double>(25.9)))));
}

TEST_F(RangeTest,
LogicalNeqOfTwoNonEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalNeq(Range(Interval(I<int>(89), I<int>(95))),
						   Range(Interval(I<int>(96), I<int>(98)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalNeq(Range(Interval(I<int>(12), I<int>(15))),
						   Range(Interval(I<int>(-14), I<int>(0)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalNeq(Range(Interval(F<double>(14.5), F<double>(15))),
						   Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalNeq(Range(Interval(I<double>(12.3), I<double>(15.7))),
						   Range(Interval(I<double>(-14.8), I<double>(-0.8)))));
}

TEST_F(RangeTest,
LogicalNeqOfTwoNeighbouringRangesWorksCorretly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(89), I<int>(95))),
						   Range(Interval(I<int>(95), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(12), I<int>(15))),
						   Range(Interval(I<int>(-14), I<int>(12)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(14.5), F<double>(15))),
						   Range(Interval(F<double>(15), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<double>(12.3), I<double>(15.7))),
						   Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalNeqOfTwoOverlappingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(89), I<int>(95))),
						   Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(12), I<int>(15))),
						   Range(Interval(I<int>(-14), I<int>(13)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(10.5), F<double>(15))),
						   Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<double>(0.35), I<double>(15.7))),
						   Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalNeqOfTwoNestingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(89), I<int>(100))),
						   Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<int>(12), I<int>(15))),
						   Range(Interval(I<int>(-14), I<int>(23)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(10.5), F<double>(35.8))),
						   Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(I<double>(0.35), I<double>(15.7))),
						   Range(Interval(I<double>(-14.8), I<double>(28.3)))));
}

TEST_F(RangeTest,
LogicalNeqOfTwoSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						   Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8))),
						   Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						   Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8))),
						   Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)),
								 Interval(F<double>(14.3), F<double>(19.5)),
								 Interval(F<double>(59.8), F<double>(99.9))),
						   Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalNeq(Range(Interval(F<double>(INFINITY),
										  F<double>(INFINITY))),
						   Range(Interval(F<double>(INFINITY),
						   				  F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalNeq(Range(Interval(F<double>(-INFINITY),
										  F<double>(-INFINITY))),
						   Range(Interval(F<double>(-INFINITY),
						   				  F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(-INFINITY),
										  F<double>(-INFINITY))),
						   Range(Interval(F<double>(INFINITY),
						   				  F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(-INFINITY),
										  F<double>(INFINITY))),
						   Range(Interval(F<double>(-INFINITY),
						   				  F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalNeq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(-INFINITY), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(INFINITY))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalLt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalLtOfAtLeastOneEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(logicalLt(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalLt(Range(), Range(Interval(I<int>(0), I<int>(10)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalLt(Range(Interval(I<int>(-145), I<int>(18))), Range()),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalLt(Range(), Range(Interval(F<double>(1.1), F<double>(1.35)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalLt(Range(Interval(F<double>(-14.5), F<double>(1.8))), Range()),
				 ".*r1.empty().*!r2.empty().*");
}

TEST_F(RangeTest,
LogicalLtOfTwoEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(0)) == logicalLt(Range(I<int>(89)), Range(I<int>(89))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalLt(Range(F<double>(14.5)), Range(F<double>(14.5))));

	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(4), I<int>(6))),
						  Range(Interval(I<int>(4), I<int>(6)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9))),
						  Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9)))));
}

TEST_F(RangeTest,
LogicalLtOfTwoNonEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalLt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(96), I<int>(98)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalLt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(0)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalLt(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalLt(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(-0.8)))));
}

TEST_F(RangeTest,
LogicalLtOfTwoNeighbouringRangesWorksCorretly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(95), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(12)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(15), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalLtOfTwoOverlappingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(13)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(10.5), F<double>(15))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalLtOfTwoNestingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(89), I<int>(100))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(23)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(10.5), F<double>(35.8))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(28.3)))));
}

TEST_F(RangeTest,
LogicalLtOfTwoSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(99.9))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLt(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(-INFINITY), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(INFINITY))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalGt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalGtOfAtLeastOneEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(logicalGt(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalGt(Range(), Range(Interval(I<int>(0), I<int>(10)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalGt(Range(Interval(I<int>(-145), I<int>(18))), Range()),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalGt(Range(), Range(Interval(F<double>(1.1), F<double>(1.35)))),
				 ".*r1.empty().*!r2.empty().*");
	EXPECT_DEATH(logicalGt(Range(Interval(F<double>(-14.5), F<double>(1.8))), Range()),
				 ".*r1.empty().*!r2.empty().*");
}

TEST_F(RangeTest,
LogicalGtOfTwoEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(0)) == logicalGt(Range(I<int>(89)), Range(I<int>(89))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalGt(Range(F<double>(14.5)), Range(F<double>(14.5))));

	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(4), I<int>(6))),
						  Range(Interval(I<int>(4), I<int>(6)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9))),
						  Range(Interval(F<double>(4.5), F<double>(6.8)),
								Interval(F<double>(8.5), F<double>(9.8)),
								Interval(F<double>(15.3), F<double>(15.9)),
								Interval(F<double>(21.3), F<double>(25.9)))));
}

TEST_F(RangeTest,
LogicalGtOfTwoNonEqualRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalGt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(96), I<int>(98)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalGt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(0)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalGt(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalGt(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(-0.8)))));
}

TEST_F(RangeTest,
LogicalGtOfTwoNeighbouringRangesWorksCorretly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(95), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(12)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(14.5), F<double>(15))),
						  Range(Interval(F<double>(15), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<double>(12.3), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalGtOfTwoOverlappingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(89), I<int>(95))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(13)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(10.5), F<double>(15))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(12.3)))));
}

TEST_F(RangeTest,
LogicalGtOfTwoNestingRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(89), I<int>(100))),
						  Range(Interval(I<int>(93), I<int>(98)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<int>(12), I<int>(15))),
						  Range(Interval(I<int>(-14), I<int>(23)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(10.5), F<double>(35.8))),
						  Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(I<double>(0.35), I<double>(15.7))),
						  Range(Interval(I<double>(-14.8), I<double>(28.3)))));
}

TEST_F(RangeTest,
LogicalGtOfTwoSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(1.3), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(99.9))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGt(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
						  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
						  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGt(Range(Interval(F<double>(NAN), F<double>(NAN)),
								Interval(F<double>(-INFINITY), F<double>(4.8)),
								Interval(F<double>(14.3), F<double>(19.5)),
								Interval(F<double>(59.8), F<double>(INFINITY))),
						  Range(Interval(F<double>(NAN), F<double>(NAN)),
								 Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalLtEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalLtEqOfNonSpecialRangesWorksCorrectly)
{
	EXPECT_DEATH(logicalLtEq(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalLtEq(Range(Interval(F<double>(18), F<double>(28.9))),
						    Range(Interval(F<double>(16), F<double>(17)))));
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalLtEq(Range(Interval(F<double>(14.5), F<double>(16))),
						    Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(14.5), F<double>(17))),
						    Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(10.5), F<double>(15))),
						    Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(10.5), F<double>(35.8))),
						    Range(Interval(F<double>(12.8), F<double>(18.9)))));
}

TEST_F(RangeTest,
LogicalLtEqOfSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)),
								  Interval(F<double>(14.3), F<double>(19.5)),
								  Interval(F<double>(59.8), F<double>(99.9))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(INFINITY),
										   F<double>(INFINITY))),
						    Range(Interval(F<double>(INFINITY),
						    			   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(-INFINITY))),
						    Range(Interval(F<double>(-INFINITY),
						  				   F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(-INFINITY))),
						    Range(Interval(F<double>(INFINITY),
						  				   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(INFINITY))),
						    Range(Interval(F<double>(-INFINITY),
						  				   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalLtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(-INFINITY), F<double>(4.8)),
								  Interval(F<double>(14.3), F<double>(19.5)),
								  Interval(F<double>(59.8), F<double>(INFINITY))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// logicalGtEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
LogicalGtEqOfNonSpecialRangesWorksCorrectly)
{
	EXPECT_DEATH(logicalGtEq(Range(), Range()), ".*r1.empty().*!r2.empty().*");
	EXPECT_TRUE(Range(I<int>(1)) ==
				logicalGtEq(Range(Interval(F<double>(18), F<double>(28.9))),
						    Range(Interval(F<double>(16), F<double>(17)))));
	EXPECT_TRUE(Range(I<int>(0)) ==
				logicalGtEq(Range(Interval(F<double>(18), F<double>(28.9))),
						    Range(Interval(F<double>(30), F<double>(37.5)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(14.5), F<double>(16))),
						    Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(14.5), F<double>(17))),
						    Range(Interval(F<double>(16), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(10.5), F<double>(15))),
						    Range(Interval(F<double>(12.8), F<double>(18.9)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(10.5), F<double>(35.8))),
						    Range(Interval(F<double>(12.8), F<double>(18.9)))));
}

TEST_F(RangeTest,
LogicalGtEqOfSpecialRangesWorksCorrectly)
{
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)),
								  Interval(F<double>(14.3), F<double>(19.5)),
								  Interval(F<double>(59.8), F<double>(99.9))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(INFINITY),
										   F<double>(INFINITY))),
						    Range(Interval(F<double>(INFINITY),
						    			   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(1), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(-INFINITY))),
						    Range(Interval(F<double>(-INFINITY),
						  				   F<double>(-INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(0))) ==
				logicalGtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(-INFINITY))),
						    Range(Interval(F<double>(INFINITY),
						  				   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(-INFINITY),
										   F<double>(INFINITY))),
						    Range(Interval(F<double>(-INFINITY),
						  				   F<double>(INFINITY)))));
	EXPECT_TRUE(Range(Interval(I<int>(0), I<int>(1))) ==
				logicalGtEq(Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(-INFINITY), F<double>(4.8)),
								  Interval(F<double>(14.3), F<double>(19.5)),
								  Interval(F<double>(59.8), F<double>(INFINITY))),
						    Range(Interval(F<double>(NAN), F<double>(NAN)),
								  Interval(F<double>(1.3), F<double>(4.8)))));
}

////////////////////////////////////////////////////////////////////////////////
// isIntegral()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IsIntegralOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH((Range()).isIntegral(), ".*!empty().*");
}

TEST_F(RangeTest,
IsIntegralOfIntegralRangeWorksCorrectly)
{
	EXPECT_TRUE((Range(I<int>(15))).isIntegral());
	EXPECT_TRUE((Range(Interval(I<int>(-125), I<int>(58)))).isIntegral());
}

TEST_F(RangeTest,
IsIntegralOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_FALSE((Range(F<double>(15.58))).isIntegral());
	EXPECT_FALSE((Range(Interval(F<double>(-12.5), I<double>(5.8)))).isIntegral());
}


////////////////////////////////////////////////////////////////////////////////
// isFloatingPoint()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IsFloatingPointOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH((Range()).isFloatingPoint(), ".*!empty().*");
}

TEST_F(RangeTest,
IsFloatingPointOfIntegralRangeWorksCorrectly)
{
	EXPECT_FALSE((Range(I<int>(15))).isFloatingPoint());
	EXPECT_FALSE((Range(Interval(I<int>(-125), I<int>(58)))).isFloatingPoint());
}

TEST_F(RangeTest,
IsFloatingPointOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_TRUE((Range(F<double>(15.58))).isFloatingPoint());
	EXPECT_TRUE((Range(Interval(F<double>(2.5), I<double>(5)))).isFloatingPoint());
}

////////////////////////////////////////////////////////////////////////////////
// isSigned()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IsSignedOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH((Range()).isSigned(), ".*!empty().*");
}

TEST_F(RangeTest,
IsSignedOfIntegralRangeWorksCorrectly)
{
	EXPECT_TRUE((Range(I<int>(15))).isSigned());
	EXPECT_TRUE((Range(Interval(I<int>(-125), I<int>(58)))).isSigned());
	EXPECT_FALSE((Range(I<unsigned>(15))).isSigned());
	EXPECT_FALSE((Range(Interval(I<unsigned>(2), I<unsigned>(58)))).isSigned());
}

TEST_F(RangeTest,
IsSignedOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_FALSE((Range(F<double>(15.58))).isSigned());
	EXPECT_FALSE((Range(Interval(F<double>(2.5), I<double>(5)))).isSigned());
}

////////////////////////////////////////////////////////////////////////////////
// isUnsigned()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IsUnsignedOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH((Range()).isUnsigned(), ".*!empty().*");
}

TEST_F(RangeTest,
IsUnsignedOfIntegralRangeWorksCorrectly)
{
	EXPECT_FALSE((Range(I<int>(15))).isUnsigned());
	EXPECT_FALSE((Range(Interval(I<int>(-125), I<int>(58)))).isUnsigned());
	EXPECT_TRUE((Range(I<unsigned>(15))).isUnsigned());
	EXPECT_TRUE((Range(Interval(I<unsigned>(2), I<unsigned>(58)))).isUnsigned());
}

TEST_F(RangeTest,
IsUnsignedOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_FALSE((Range(F<double>(15.58))).isUnsigned());
	EXPECT_FALSE((Range(Interval(F<double>(2.5), I<double>(5)))).isUnsigned());
}

////////////////////////////////////////////////////////////////////////////////
// hasSameTypeAs()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
HasSameTypeAsOfSameTypeRangesWorksCorrectly)
{
	EXPECT_TRUE((Range(I<unsigned>(1))).hasSameTypeAs(Range(I<unsigned>(1))));
	EXPECT_TRUE((Range(I<int>(1))).hasSameTypeAs(Range(I<int>(1))));
	EXPECT_TRUE((Range(F<float>(1))).hasSameTypeAs(Range(F<float>(1))));
}

TEST_F(RangeTest,
HasSameTypeAsOfDifferentTypeRangesWorkscCorrectly)
{
	EXPECT_FALSE((Range(I<int>(1))).hasSameTypeAs(Range(I<unsigned>(1))));
	EXPECT_FALSE((Range(I<int>(1))).hasSameTypeAs(Range(I<signed char>(1))));
	EXPECT_FALSE((Range(I<int>(1))).hasSameTypeAs(Range(F<float>(1))));
}

TEST_F(RangeTest,
HasSameTypeAsOfEmptyRangesWorksCorretly)
{
	EXPECT_DEATH((Range()).hasSameTypeAs(Range()),".*!empty().*!r.empty().*");
	EXPECT_DEATH((Range()).hasSameTypeAs(Range(I<int>(0))),
				 ".*!empty().*!r.empty().*");
	EXPECT_DEATH((Range(F<float>(3.6))).hasSameTypeAs(Range()),
				 ".*!empty().*!r.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// containsOnlySingleNumber()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
RangesWithSingleNumberContainsOnlySingleNumber)
{
	EXPECT_TRUE(Range(I<int>(1)).containsOnlySingleNumber());
	EXPECT_TRUE(Range(F<double>(4.5)).containsOnlySingleNumber());
}

TEST_F(RangeTest,
RangesWithMoreNumbersDoNotContainOnlySingleNumber)
{
	EXPECT_FALSE(Range(Interval(I<int>(1), I<int>(2))).containsOnlySingleNumber());
	EXPECT_FALSE(Range(Interval(F<double>(4.5), I<int>(6.7))).containsOnlySingleNumber());
}

////////////////////////////////////////////////////////////////////////////////
// containsIntegralMin()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsIntegralMinRangeContainsIntegralMin)
{
	EXPECT_TRUE(Range(I<int>(vmin<int>())).containsIntegralMin());
	EXPECT_TRUE(Range(Interval(I<int>(vmin<int>()), I<int>(0))).containsIntegralMin());

	EXPECT_TRUE(Range(I<long>(vmin<long>())).containsIntegralMin());
	EXPECT_TRUE(Range(Interval(I<long>(vmin<long>()), I<long>(0))).containsIntegralMin());

	EXPECT_TRUE(Range(I<unsigned>(vmin<unsigned>())).containsIntegralMin());
	EXPECT_TRUE(Range(Interval(I<unsigned>(vmin<unsigned>()), I<unsigned>(10))).containsIntegralMin());
}

TEST_F(RangeTest,
ContainsIntegralMinRangeDoesNotContainIntegralMin)
{
	EXPECT_FALSE(Range(I<int>(vmin<int>() + 1)).containsIntegralMin());
	EXPECT_FALSE(Range(Interval(I<int>(vmin<int>() + 1), I<int>(-1))).containsIntegralMin());

	EXPECT_FALSE(Range(F<float>(vmin<float>())).containsIntegralMin());
	EXPECT_FALSE(Range(F<float>(-INFINITY)).containsIntegralMin());
}

////////////////////////////////////////////////////////////////////////////////
// containsIntegralMinusOne()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ContainsIntegralMinusOneRangeContainsIntegralMinusOne)
{
	EXPECT_TRUE(Range(I<int>(-1)).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<int>(-5), I<int>(-1))).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<int>(-5), I<int>(0))).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<int>(-1), I<int>(1))).containsIntegralMinusOne());

	EXPECT_TRUE(Range(I<unsigned>(-1)).containsIntegralMinusOne());

	EXPECT_TRUE(Range(I<long>(-1)).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<long>(-5), I<long>(-1))).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<long>(-5), I<long>(0))).containsIntegralMinusOne());
	EXPECT_TRUE(Range(Interval(I<long>(-1), I<long>(1))).containsIntegralMinusOne());
}

TEST_F(RangeTest,
ContainsIntegralMinusOneRangeDoesNotContainIntegralMinusOne)
{
	EXPECT_FALSE(Range(I<int>(-2)).containsIntegralMinusOne());
	EXPECT_FALSE(Range(I<int>(0)).containsIntegralMinusOne());
	EXPECT_FALSE(Range(Interval(I<int>(0), I<int>(5))).containsIntegralMinusOne());

	EXPECT_FALSE(Range(F<float>(-1.0)).containsIntegralMinusOne());
	EXPECT_FALSE(Range(Interval(F<float>(-5.0), F<float>(5.0))).containsIntegralMinusOne());
}

////////////////////////////////////////////////////////////////////////////////
// getMax()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
GetMaxOfEmptyRange)
{
	EXPECT_DEATH((Range()).getMax(),".*!empty().*");
}

TEST_F(RangeTest,
GetMaxOfIntRange)
{
	EXPECT_EQ(I<int>(130),
			  (Range(Interval(I<int>(-125), I<int>(-120)),
			         Interval(I<int>(12), I<int>(130)))).getMax());
}

TEST_F(RangeTest,
GetMaxOfFloatRange)
{
	EXPECT_EQ(F<double>(1.30),
			  (Range(Interval(F<double>(-12.5), F<double>(-1.20)),
			         Interval(F<double>(0.12), F<double>(1.30)))).getMax());
}

TEST_F(RangeTest,
GetMaxOfSpecialFloatRange)
{
	EXPECT_EQ(F<double>(INFINITY),
			  (Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
			         Interval(F<double>(INFINITY), F<double>(INFINITY)))).getMax());
	EXPECT_EQ(F<double>(INFINITY),
			  (Range(Interval(F<double>(NAN), F<double>(NAN)),
			         Interval(F<double>(INFINITY), F<double>(INFINITY)))).getMax());
	EXPECT_EQ(F<double>(-INFINITY),
			  (Range(Interval(F<double>(NAN), F<double>(NAN)),
			         Interval(F<double>(-INFINITY), F<double>(-INFINITY)))).getMax());
}

////////////////////////////////////////////////////////////////////////////////
// getMin()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
GetMinOfEmptyRange)
{
	EXPECT_DEATH((Range()).getMin(),".*!empty().*");
}

TEST_F(RangeTest,
GetMinOfIntRange)
{
	EXPECT_EQ(I<int>(-125),
			  (Range(Interval(I<int>(-125), I<int>(-120)),
			         Interval(I<int>(12), I<int>(130)))).getMin());
}

TEST_F(RangeTest,
GetMinOfFloatRange)
{
	EXPECT_EQ(F<double>(-12.5),
			  (Range(Interval(F<double>(-12.5), F<double>(-1.20)),
					 Interval(F<double>(0.12), F<double>(1.30)))).getMin());
}

TEST_F(RangeTest,
GetMinOfSpecialFloatRange)
{
	EXPECT_EQ(F<double>(-INFINITY),
			  (Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
			         Interval(F<double>(INFINITY), F<double>(INFINITY)))).getMin());
	EXPECT_EQ(F<double>(INFINITY),
			  (Range(Interval(F<double>(NAN), F<double>(NAN)),
			         Interval(F<double>(INFINITY), F<double>(INFINITY)))).getMin());
	EXPECT_EQ(F<double>(-INFINITY),
			  (Range(Interval(F<double>(NAN), F<double>(NAN)),
			         Interval(F<double>(-INFINITY), F<double>(-INFINITY)))).getMin());
}

////////////////////////////////////////////////////////////////////////////////
// intToFloat()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IntToFloatOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(intToFloat(Range()), ".*!r.empty().*");
}

TEST_F(RangeTest,
IntToFloatOfIntegralRangeWorksCorrectly)
{
	EXPECT_EQ(Range(F<float>(15.0)), intToFloat(Range(I<int>(15))));
	EXPECT_EQ(Range(Interval(F<float>(15.0), F<float>(18.0))),
			  intToFloat(Range(Interval(I<int>(15), I<int>(18)))));
}

TEST_F(RangeTest,
IntToFloatOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_DEATH(intToFloat(Range(F<double>(15))), ".*r.isIntegral().*");
	EXPECT_DEATH(intToFloat(Range(Interval(F<double>(15), F<double>(18)))),
				 ".*r.isIntegral().*");
}

////////////////////////////////////////////////////////////////////////////////
// getMaxRange()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
GetMaxRangeOfIntegralNumber)
{
	EXPECT_EQ(Range(Interval(I<signed char>(vmin<signed char>()),
							 I<signed char>(vmax<signed char>()))),
			  Range::getMaxRange(I<signed char>(15)));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
			  Range::getMaxRange(I<int>(258)));
}

TEST_F(RangeTest,
GetMaxRangeOfFloatingPointNumber)
{
	Range r = Range::getMaxRange(F<double>(159.369));
	EXPECT_TRUE(r[0].first.isNotNumber());
	EXPECT_TRUE(r[0].second.isNotNumber());
	EXPECT_TRUE(r[1].first.isNegativeInf());
	EXPECT_TRUE(r[1].second.isPositiveInf());

	Range r1 = Range::getMaxRange(F<double>(INFINITY));
	EXPECT_TRUE(r1[0].first.isNotNumber());
	EXPECT_TRUE(r1[0].second.isNotNumber());
	EXPECT_TRUE(r1[1].first.isNegativeInf());
	EXPECT_TRUE(r1[1].second.isPositiveInf());

	Range r2 = Range::getMaxRange(F<double>(NAN));
	EXPECT_TRUE(r2[0].first.isNotNumber());
	EXPECT_TRUE(r2[0].second.isNotNumber());
	EXPECT_TRUE(r2[1].first.isNegativeInf());
	EXPECT_TRUE(r2[1].second.isPositiveInf());
}

////////////////////////////////////////////////////////////////////////////////
// assign()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
AssignmentOfUnsinedRangeToUnsignedRangeWorksCorrectly)
{
	// Assignment to the same type.
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(2))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned char>(0), I<unsigned char>(2)))));

	// Assignment of the smaller type to the bigger one.
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(2))),
			 (Range(Interval(I<unsigned>(15), I<unsigned>(15)))).assign(Range(Interval(I<unsigned char>(0), I<unsigned char>(2)))));

	// Assignment of the bigger type to the smaller one.
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(2))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(0), I<unsigned>(2)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(2))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(258)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(513)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(512)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(511)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(254))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(510)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(0)),
				    Interval(I<unsigned char>(2), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(258), I<unsigned>(512)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(1)),
				    Interval(I<unsigned char>(14), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<unsigned>(270), I<unsigned>(513)))));
}

TEST_F(RangeTest,
AssignmentOfSignedRangeToSignedRangeWorksCorrectly)
{
	// Assignment to the same type.
	EXPECT_EQ(Range(Interval(I<char>(0), I<char>(2))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<char>(0), I<char>(2)))));

	// Assignment of the smaller type to the bigger one.
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(2))),
			 (Range(Interval(I<int>(15), I<int>(15)))).assign(Range(Interval(I<char>(0), I<char>(2)))));

	// Assignment of the bigger type to the smaller one.
	EXPECT_EQ(Range(Interval(I<char>(126), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-130), I<int>(-129)))));

	EXPECT_EQ(Range(Interval(I<char>(106), I<char>(126))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-150), I<int>(-130)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(-128)),
				    Interval(I<char>(92), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-164), I<int>(-128)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(-100)),
				    Interval(I<char>(126), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-130), I<int>(-100)))));

	EXPECT_EQ(Range(Interval(I<char>(0), I<char>(127)),
				    Interval(I<char>(-128), I<char>(-128))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-256), I<int>(-128)))));

	EXPECT_EQ(Range(Interval(I<char>(-1), I<char>(127)),
				    Interval(I<char>(-128), I<char>(-128))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-257), I<int>(-128)))));

	EXPECT_EQ(Range(Interval(I<char>(-2), I<char>(127)),
				    Interval(I<char>(-128), I<char>(-128))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-258), I<int>(-128)))));

	EXPECT_EQ(Range(Interval(I<char>(-127), I<char>(-126))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(129), I<int>(130)))));

	EXPECT_EQ(Range(Interval(I<char>(-126), I<char>(-106))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(130), I<int>(150)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(-92))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(128), I<int>(164)))));

	EXPECT_EQ(Range(Interval(I<char>(100), I<char>(127)),
				    Interval(I<char>(-128), I<char>(-126))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(100), I<int>(130)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(0))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(128), I<int>(256)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(1))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(128), I<int>(257)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(2))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(128), I<int>(258)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-513), I<int>(-257)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(257), I<int>(514)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(-513), I<int>(-258)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(257), I<int>(512)))));

	EXPECT_EQ(Range(Interval(I<char>(-128), I<char>(-1)),
					Interval(I<char>(1), I<char>(127))),
			 (Range(Interval(I<char>(15), I<char>(15)))).assign(Range(Interval(I<int>(257), I<int>(511)))));
}

TEST_F(RangeTest,
AssignmentOfSignedRangeToUnsignedRangeWorksCorrectly)
{
	// Assignment of the smaller type to the bigger one.
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(2))),
			 (Range(Interval(I<unsigned>(15), I<unsigned>(15)))).assign(Range(Interval(I<signed char>(0), I<signed char>(2)))));

	// Assignment of the bigger type to the smaller one.
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-128), I<int>(127)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-512), I<int>(0)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(0)),
					Interval(I<unsigned char>(128), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-128), I<int>(0)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(52)),
					Interval(I<unsigned char>(203), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-53), I<int>(52)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(250), I<int>(512)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(2))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(256), I<int>(258)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-512), I<int>(250)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(1)),
					Interval(I<unsigned char>(255), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-1), I<int>(1)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-128), I<int>(128)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(127)),
					Interval(I<unsigned char>(129), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(15), I<unsigned char>(15)))).assign(Range(Interval(I<int>(-127), I<int>(127)))));
}

TEST_F(RangeTest,
AssignmentOfUnsignedRangeToSignedRangeWorksCorrectly)
{
	// Assignment of the smaller type to the bigger one.
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(2))),
			 (Range(Interval(I<int>(15), I<int>(15)))).assign(Range(Interval(I<unsigned char>(0), I<unsigned char>(2)))));

	// Assignment of the bigger type to the smaller one.
	EXPECT_EQ(Range(Interval(I<signed char>(126), I<signed char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(126), I<unsigned>(127)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(-127)),
					Interval(I<signed char>(126), I<unsigned char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(126), I<unsigned>(129)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(-126)),
					Interval(I<signed char>(127), I<unsigned char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(127), I<unsigned>(130)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(-106))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(128), I<unsigned>(150)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-6), I<signed char>(-1))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(250), I<unsigned>(255)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-1), I<signed char>(34))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(255), I<unsigned>(290)))));

	EXPECT_EQ(Range(Interval(I<signed char>(0), I<signed char>(18))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(512), I<unsigned>(530)))));

	EXPECT_EQ(Range(Interval(I<signed char>(0), I<signed char>(2))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(258)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(512)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(0)),
					Interval(I<signed char>(2), I<unsigned char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(258), I<unsigned>(512)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(256), I<unsigned>(513)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(258), I<unsigned>(513)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(1)),
					Interval(I<signed char>(14), I<signed char>(127))),
			 (Range(Interval(I<signed char>(15), I<signed char>(15)))).assign(Range(Interval(I<unsigned>(270), I<unsigned>(513)))));
}

TEST_F(RangeTest,
AssignmentOfFloatingPointRangeToFloatingPointRangeWorksCorrectly)
{
	// Assignment to the same type.
	EXPECT_EQ(Range(Interval(F<double>(0.5), F<double>(2.8))),
			 (Range(Interval(F<double>(1.5), F<double>(1.5)))).assign(Range(Interval(F<double>(0.5), F<double>(2.8)))));

	// Assignment of the smaller type to the bigger one.
	EXPECT_EQ(Range(Interval(F<double>(0), F<double>(2))),
			 (Range(Interval(F<double>(15), F<double>(15)))).assign(Range(Interval(F<float>(0), F<float>(2)))));

	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(INFINITY))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmin<double>())))));

	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(vmin<float>()))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmin<float>()-1)))));

	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-153.69))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(-153.69)))));

	EXPECT_EQ(Range(Interval(F<float>(-153.69), F<float>(153.69))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(-153.69), F<double>(153.69)))));

	EXPECT_EQ(Range(Interval(F<float>(-153.69), F<float>(INFINITY))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(-153.69), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(Interval(F<double>(vmax<double>()), F<double>(vmax<double>())))));
}

TEST_F(RangeTest,
AssignmentOfFloatingPointRangeToSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(vmax<double>()), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmin<double>())))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(-190), F<double>(-129)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(0), F<double>(256)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(-128), F<double>(127)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-1), I<signed char>(12))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(-1), F<double>(12)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
			 (Range(Interval(I<signed char>(1), I<signed char>(1)))).assign(Range(Interval(F<double>(NAN), F<double>(NAN)))));
}

TEST_F(RangeTest,
AssignmentOfFloatingPointRangeToUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(vmax<double>()), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmin<double>())))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(vmin<double>()), F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(-190), F<double>(-129)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(0), F<double>(256)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(-128), F<double>(127)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(12))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(0), F<double>(12)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(255))),
			 (Range(Interval(I<unsigned char>(1), I<unsigned char>(1)))).assign(Range(Interval(F<double>(NAN), F<double>(NAN)))));
}

TEST_F(RangeTest,
AssignmentOfUnsignedRangeToFloatingPointRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(48), F<float>(55)),
					Interval(F<float>(89), F<float>(100)),
					Interval(F<float>(103), F<float>(108))),
			 (Range(Interval(F<float>(1), F<float>(1)))).assign(Range(
			 		Interval(I<unsigned>(48), I<unsigned>(55)),
			 		Interval(I<unsigned>(89), I<unsigned>(100)),
			 		Interval(I<unsigned>(103), I<unsigned>(108)))));
}

TEST_F(RangeTest,
AssignmentOfSignedRangeToFloatingPointRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-48), F<double>(55)),
					Interval(F<double>(89), F<double>(100)),
					Interval(F<double>(103), F<double>(vmax<int>()))),
			 (Range(Interval(F<double>(1), F<double>(1)))).assign(Range(
			 		Interval(I<int>(-48), I<int>(55)),
			 		Interval(I<int>(89), I<int>(100)),
			 		Interval(I<int>(103), I<int>(vmax<int>())))));
}

TEST_F(RangeTest,
AssignmentBetweenEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH((Range()).assign(Range()), ".*!empty().*!r.empty().*");

	EXPECT_DEATH((Range(I<int>(10))).assign(Range()), ".*!empty().*!r.empty().*");

	EXPECT_DEATH((Range()).assign(Range(Interval(F<double>(vmax<double>()),
				  F<double>(vmax<double>())))), ".*!empty().*!r.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// expand()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ExpandForSignedIntegralIntRangesWorksCorrectly)
{
	// (0, 0) -> (-1, 1)
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(1))),
		Range(I<int>(0)).expand());

	// (1, 2) -> (-1, 4)
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(4))),
		Range(Interval(I<int>(1), I<int>(2))).expand());

	// (4, 8) -> (2, 16)
	EXPECT_EQ(Range(Interval(I<int>(2), I<int>(16))),
		Range(Interval(I<int>(4), I<int>(8))).expand());

	// (-8, -4) -> (-16, -2)
	EXPECT_EQ(Range(Interval(I<int>(-16), I<int>(-2))),
		Range(Interval(I<int>(-8), I<int>(-4))).expand());

	// (-63, 63) -> (-126, 126)
	EXPECT_EQ(Range(Interval(I<int>(-126), I<int>(126))),
		Range(Interval(I<int>(-63), I<int>(63))).expand());

	// (-65, 65) -> (-130, 130)
	EXPECT_EQ(Range(Interval(I<int>(-130), I<int>(130))),
		Range(Interval(I<int>(-65), I<int>(65))).expand());

	// (-2, 4000) -> (-4, 8000)
	EXPECT_EQ(Range(Interval(I<int>(-4), I<int>(8000))),
		Range(Interval(I<int>(-2), I<int>(4000))).expand());

	// (MAX, MAX) -> (MAX / 2, MAX)
	EXPECT_EQ(Range(Interval(I<int>(vmax<int>() / 2),
			I<int>(vmax<int>()))),
		Range(Interval(I<int>(vmax<int>()),
			I<int>(vmax<int>()))).expand());

	// (-5, -4) + (4, 8) -> (-10, -2) + (2, 16)
	EXPECT_EQ(Range(Interval(I<int>(-10), I<int>(-2)),
			Interval(I<int>(2), I<int>(16))),
		Range(Interval(I<int>(-5), I<int>(-4)),
			Interval(I<int>(4), I<int>(8))).expand());
}

TEST_F(RangeTest,
ExpandForUnsignedIntegralRangesWorksCorrectly)
{
	// (0, 0) -> (0, 1)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(1))),
		Range(I<unsigned>(0)).expand());

	// (1, 2) -> (0, 4)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(4))),
		Range(Interval(I<unsigned>(1), I<unsigned>(2))).expand());

	// (0, 4000) -> (0, 8000)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(8000))),
		Range(Interval(I<unsigned>(0), I<unsigned>(4000))).expand());

	// (MAX, MAX) -> (MAX / 2, MAX)
	EXPECT_EQ(Range(Interval(I<unsigned>(vmax<unsigned>() / 2),
			I<unsigned>(vmax<unsigned>()))),
		Range(Interval(I<unsigned>(vmax<unsigned>()),
			I<unsigned>(vmax<unsigned>()))).expand());
}

TEST_F(RangeTest,
ExpandForSignedIntegralCharRangesWorksCorrectly)
{
	// (0, 64) -> (-1, 127)
	EXPECT_EQ(Range(Interval(I<signed char>(-1), I<signed char>(127))),
		Range(Interval(I<signed char>(0), I<signed char>(64))).expand());

	// (-63, 63) -> (-126, 126)
	EXPECT_EQ(Range(Interval(I<signed char>(-126), I<signed char>(126))),
		Range(Interval(I<signed char>(-63), I<signed char>(63))).expand());

	// (-64, 64) -> (-128, 127)
	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
		Range(Interval(I<signed char>(-64), I<signed char>(64))).expand());

	// (-65, 65) -> (-128, 127)
	EXPECT_EQ(Range(Interval(I<signed char>(-128), I<signed char>(127))),
		Range(Interval(I<signed char>(-65), I<signed char>(65))).expand());
}

TEST_F(RangeTest,
ExpandForFloatingPointsWorksCorrectly)
{
	// Currently, expand() for floating-point ranges returns the maximal range.
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN)),
			Interval(F<float>(-INFINITY), F<float>(INFINITY))),
		Range(F<float>(1)).expand());
}

////////////////////////////////////////////////////////////////////////////////
// mergeIntervals()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
MergeIntervalsOnRangeWithSingleIntervalDoesNotChangeTheInterval)
{
	// (-1, 5) -> (-1, 5)
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(5))),
		Range(Interval(I<int>(-1), I<int>(5))).mergeIntervals());
	// (-INF, INF) -> (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
		Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))).mergeIntervals());
	// (NAN, NAN) -> (NAN, NAN)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN))),
		Range(Interval(F<double>(NAN), F<double>(NAN))).mergeIntervals());
}

TEST_F(RangeTest,
MergeIntervalsOnIntegralRangesWorksCorrectlyWhenThereAreMoreIntervals)
{
	// (-1, 5)(10, 20) -> (-1, 20)
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(20))),
		Range(Interval(I<int>(-1), I<int>(5)),
			Interval(I<int>(10), I<int>(20))).mergeIntervals());
	// (INT_MIN, INT_MAX)(-1, 5)(10, 20)(INT_MAX, INT_MAX) -> (-1, 20)
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>())),
			Interval(I<int>(-1), I<int>(5)),
			Interval(I<int>(10), I<int>(20)),
			Interval(I<int>(vmax<int>()), I<int>(vmax<int>()))).mergeIntervals());
}

TEST_F(RangeTest,
MergeIntervalsOnFloatingPointRangesWorksCorrectlyWhenThereAreMoreIntervalsButNoNAN)
{
	// (-5.0, -1.0)(6.7, 8.7)(10.0, INFINITY) -> (-5.0, INFINITY)
	EXPECT_EQ(Range(Interval(F<double>(-5.0), F<double>(INFINITY))),
		Range(Interval(F<double>(-5.0), F<double>(-1.0)),
			Interval(F<double>(6.7), F<double>(8.7)),
			Interval(F<double>(10.0), F<double>(INFINITY))).mergeIntervals());
}

TEST_F(RangeTest,
MergeIntervalsOnFloatingPointRangesWorksCorrectlyWhenThereAreMoreIntervalsAndNAN)
{
	// (NAN, NAN)(-5.0, -1.0)(6.7, 8.7)(10.0, INFINITY) -> (NAN, NAN)(-5.0, INFINITY)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
			Interval(F<double>(-5.0), F<double>(INFINITY))),
		Range(Interval(F<double>(NAN), F<double>(NAN)),
			Interval(F<double>(-5.0), F<double>(-1.0)),
			Interval(F<double>(6.7), F<double>(8.7)),
			Interval(F<double>(10.0), F<double>(INFINITY))).mergeIntervals());
}

////////////////////////////////////////////////////////////////////////////////
// - (unary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
UnaryMinusOfSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(I<int>(-vmin<int>())), -Range(I<int>(vmin<int>())));
	EXPECT_EQ(Range(I<int>(vmin<int>())), -Range(I<int>(vmin<int>())));
	EXPECT_EQ(Range(I<int>(-vmax<int>())), -Range(I<int>(vmax<int>())));
	EXPECT_EQ(Range(I<int>(0)), -Range(I<int>(0)));

	EXPECT_EQ(Range(Interval(I<int>(-10), I<int>(100))),
			 -Range(Interval(I<int>(-100), I<int>(10))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>())),
					Interval(I<int>(-5), I<int>(-(vmin<int>() + 1)))),
			 -Range(Interval(I<int>(vmin<int>()), I<int>(5))));
}

TEST_F(RangeTest,
UnaryMinusOfUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(I<unsigned>(-vmin<unsigned>())),
			 -Range(I<unsigned>(vmin<unsigned>())));
	EXPECT_EQ(Range(I<unsigned>(vmin<unsigned>())),
			 -Range(I<unsigned>(vmin<unsigned>())));
	EXPECT_EQ(Range(I<unsigned>(-vmax<unsigned>())),
			 -Range(I<unsigned>(vmax<unsigned>())));
	EXPECT_EQ(Range(I<unsigned>(0)), -Range(I<unsigned>(0)));

	unsigned maxUn = vmax<unsigned>();
	EXPECT_EQ(Range(Interval(I<unsigned>(maxUn + 1 - 100), I<unsigned>(maxUn))),
			 -Range(Interval(I<unsigned>(1), I<unsigned>(100))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmin<unsigned>())),
					Interval(I<unsigned>(maxUn + 1 - 10),
							 I<unsigned>(maxUn + 1))),
			 -Range(Interval(I<unsigned>(vmin<unsigned>()), I<unsigned>(10))));
}

TEST_F(RangeTest,
UnaryMinusOfFloatWorksCorrectly)
{
	EXPECT_EQ(Range(F<double>(vmin<double>())), -Range(F<double>(vmax<double>())));
	EXPECT_EQ(Range(F<double>(vmax<double>())), -Range(F<double>(vmin<double>())));

	EXPECT_EQ(Range(F<double>(INFINITY)), -Range(F<double>(-INFINITY)));
	EXPECT_EQ(Range(F<double>(-INFINITY)), -Range(F<double>(INFINITY)));

	Range r = -Range(F<double>(NAN));
	EXPECT_EQ(true, (r.begin()->first).isNotNumber());

	EXPECT_EQ(Range(F<double>(0)), -Range(F<double>(0)));
	EXPECT_EQ(Range(F<double>(-1789.35)), -Range(F<double>(1789.35)));
	EXPECT_EQ(Range(F<double>(1789.35)), -Range(F<double>(-1789.35)));

	EXPECT_EQ(Range(Interval(F<double>(-1789.58), F<double>(-1432)),
					Interval(F<double>(-1300), F<double>(1200))),
			 -Range(Interval(F<double>(1432), F<double>(1789.58)),
                    Interval(F<double>(-1200), F<double>(1300))));
}

TEST_F(RangeTest,
UnaryMinusOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(-Range(), ".*!r.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// + (binary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BinaryPlusOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>())),
					Interval(I<int>(11), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(10), I<int>(vmax<int>())))
			+ Range(Interval(I<int>(1), I<int>(1))));

	EXPECT_EQ(Range(Interval(I<int>(6), I<int>(210))),
			  Range(Interval(I<int>(10), I<int>(200)))
			+ Range(Interval(I<int>(-4), I<int>(10))));

	EXPECT_EQ(Range(Interval(I<long>(vmin<long>()), I<long>(210)),
					Interval(I<long>(vmax<long>()), I<long>(vmax<long>()))),
			  Range(Interval(I<long>(vmin<long>()), I<long>(200)))
			+ Range(Interval(I<long>(-1), I<long>(10))));
}

TEST_F(RangeTest,
BinaryPlusOfSignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + vmax<unsigned>()),
							 I<unsigned>(-5 + vmax<unsigned>()))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			+ Range(Interval(I<unsigned>(vmax<unsigned>()),
				             I<unsigned>(vmax<unsigned>()))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + vmax<unsigned>() + 1),
							 I<unsigned>(-5 + vmax<unsigned>() + 1))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			+ Range(Interval(I<unsigned>(0),
				             I<unsigned>(0))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + vmax<unsigned>() + 1),
							 I<unsigned>(vmax<unsigned>() + 1))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(0)))
			+ Range(Interval(I<unsigned>(0),
				             I<unsigned>(0))));
}

TEST_F(RangeTest,
BinaryPlusOfSignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(11), F<float>(vmax<int>() + 1.0))),
			  Range(Interval(I<int>(10), I<int>(vmax<int>())))
			+ Range(Interval(F<float>(1), F<float>(1))));

	EXPECT_EQ(Range(Interval(F<float>(1 + 5.0), F<float>(1 + vmax<float>()))),
			  Range(Interval(I<int>(1), I<int>(1)))
			+ Range(Interval(F<float>(5.0), F<float>(vmax<float>()))));

	EXPECT_EQ(Range(Interval(F<float>(vmin<int>() - 105.35),
							 F<float>(vmax<int>() + 105.35))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>())))
			+ Range(Interval(F<float>(-105.35), F<float>(105.35))));
}

TEST_F(RangeTest,
BinaryPlusOfUnsignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
			  				 I<unsigned>(vmax<unsigned>())))
			+ Range(Interval(I<int>(-1), I<int>(-1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()), I<unsigned>(4535+100)),
					Interval(I<unsigned>(vmax<unsigned>() - 1),
							 I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(8), I<unsigned>(4535)))
			+ Range(Interval(I<int>(-10), I<int>(100))));
}

TEST_F(RangeTest,
BinaryPlusOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(0)),
					Interval(I<unsigned>(11), I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>())))
			+ Range(Interval(I<unsigned>(1), I<unsigned>(1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(11), I<unsigned>(17)),
					Interval(I<unsigned>(21), I<unsigned>(27)),
					Interval(I<unsigned>(101), I<unsigned>(127))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(15)),
			  		Interval(I<unsigned>(20), I<unsigned>(25)),
			  		Interval(I<unsigned>(100), I<unsigned>(125)))
			+ Range(Interval(I<unsigned>(1), I<unsigned>(1)),
					Interval(I<unsigned>(2), I<unsigned>(2))));

	EXPECT_EQ(Range(Interval(I<unsigned>(9), I<unsigned>(vmax<unsigned>() - 1))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>())))
			+ Range(Interval(I<unsigned>(vmax<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))));
}

TEST_F(RangeTest,
BinaryPlusOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<unsigned>() + vmin<double>()),
							 F<double>(vmax<unsigned>() + vmax<double>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
			  				 I<unsigned>(vmax<unsigned>())))
			+ Range(Interval(F<double>(vmin<double>()),
							 F<double>(vmax<double>()))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<unsigned>() - 1.0),
							 F<double>(vmax<unsigned>() - 1.0))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
			  				 I<unsigned>(vmax<unsigned>())))
			+ Range(Interval(F<double>(-1.0),
							 F<double>(-1.0))));

	EXPECT_EQ(Range(Interval(F<double>(25 - 27.35), F<double>(1438 + 128.0))),
			  Range(Interval(I<unsigned>(25), I<unsigned>(1438)))
			+ Range(Interval(F<double>(-27.35), F<double>(128.0))));
}

TEST_F(RangeTest,
BinaryPlusOfFloatAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<double>() - 1265.78),
							 F<double>(vmax<double>() + 128.35))),
			  Range(Interval(F<double>(vmin<double>()),
			  				 F<double>(vmax<double>())))
			+ Range(Interval(I<int>(-1265.78), I<int>(128.35))));
}

TEST_F(RangeTest,
BinaryPlusOfFloatAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-12589.6 + 437),
							 F<double>(12583.78 + 589))),
			  Range(Interval(F<double>(-12589.6), F<double>(12583.78)))
			+ Range(Interval(I<unsigned>(437), I<unsigned>(589))));

}

TEST_F(RangeTest,
BinaryPlusOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmin<double>())) + Range(F<double>(-1e308)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) + Range(F<double>(1e308)));

	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmin<double>())) + Range(F<double>(-1e300)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) + Range(F<double>(1e300)));

	EXPECT_EQ(Range(Interval(F<double>(7.8 + 14.35), F<double>(7898.0 + 14587.8))),
			  Range(Interval(F<double>(14.35), F<double>(14587.8)))
			+ Range(Interval(F<double>(7.8), F<double>(7898.0))));
}

TEST_F(RangeTest,
BinaryPlusOfSpecialFloatsWorksCorrectly)
{
	// (INF, INF) + (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			+ Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (INF, INF) + (-INF, -INF) = (NAN, NAN)
	Range r1 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 + Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(1), r1.size());
	EXPECT_EQ(true, (r1[0].first).isNotNumber());
	EXPECT_EQ(true, (r1[0].second).isNotNumber());

	// (INF, INF) + (-INF, 5) = (NAN, NAN) U (INF, INF)
	Range r2 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 + Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r2.size());
	EXPECT_EQ(true, (r2[0].first).isNotNumber());
	EXPECT_EQ(true, (r2[0].second).isNotNumber());
	EXPECT_EQ(true, (r2[1].first).isPositiveInf());
	EXPECT_EQ(true, (r2[1].second).isPositiveInf());

	// (INF, INF) + (-INF, INF) = (NAN, NAN) U (INF, INF)
	Range r3 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r3.size());
	EXPECT_EQ(true, (r3[0].first).isNotNumber());
	EXPECT_EQ(true, (r3[0].second).isNotNumber());
	EXPECT_EQ(true, (r3[1].first).isPositiveInf());
	EXPECT_EQ(true, (r3[1].second).isPositiveInf());

	// (INF, INF) + (NAN, NAN) = (NAN, NAN)
	Range r4 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r4.size());
	EXPECT_EQ(true, (r4[0].first).isNotNumber());
	EXPECT_EQ(true, (r4[0].second).isNotNumber());

	// (INF, INF) + (x, y) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			+ Range(Interval(F<double>(5.0), F<double>(8.0))));

	// (INF, INF) + (x, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			+ Range(Interval(F<double>(1.35), F<double>(INFINITY))));

	// (-INF, -INF) + (INF, INF) = (NAN, NAN)
	Range r5 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 + Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r5.size());
	EXPECT_EQ(true, (r5[0].first).isNotNumber());
	EXPECT_EQ(true, (r5[0].second).isNotNumber());

	// (-INF, -INF) + (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, -INF) + (-INF, 5) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, -INF) + (-INF, INF) = (-INF, -INF) U (NAN, NAN)
	Range r6 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r6.size());
	EXPECT_EQ(true, (r6[0].first).isNotNumber());
	EXPECT_EQ(true, (r6[0].second).isNotNumber());
	EXPECT_EQ(true, (r6[1].first).isNegativeInf());
	EXPECT_EQ(true, (r6[1].second).isNegativeInf());

	// (-INF, -INF) + (NAN, NAN) = (NAN, NAN)
	Range r7 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r7.size());
	EXPECT_EQ(true, (r7[0].first).isNotNumber());
	EXPECT_EQ(true, (r7[0].second).isNotNumber());

	// (-INF, -INF) + (1, 5) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(1), F<double>(5)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, -INF) + (1, INF) = (NAN, NAN) U (-INF, -INF)
	Range r8 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 + Range(Interval(F<double>(1.0), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r8.size());
	EXPECT_EQ(true, (r8[0].first).isNotNumber());
	EXPECT_EQ(true, (r8[0].second).isNotNumber());
	EXPECT_EQ(true, (r8[1].first).isNegativeInf());
	EXPECT_EQ(true, (r8[1].second).isNegativeInf());

	// (-INF, 5) + (INF, INF) = (NAN, NAN) U (INF, INF)
	Range r9 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			 + Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r9.size());
	EXPECT_EQ(true, (r9[0].first).isNotNumber());
	EXPECT_EQ(true, (r9[0].second).isNotNumber());
	EXPECT_EQ(true, (r9[1].first).isPositiveInf());
	EXPECT_EQ(true, (r9[1].second).isPositiveInf());

	// (-INF, 5) + (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, 5) + (-INF, 5) = (-INF, 10)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(10))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (-INF, 5) + (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r10 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r10.size());
	EXPECT_EQ(true, (r10[0].first).isNotNumber());
	EXPECT_EQ(true, (r10[0].second).isNotNumber());
	EXPECT_EQ(true, (r10[1].first).isNegativeInf());
	EXPECT_EQ(true, (r10[1].second).isPositiveInf());

	// (-INF, 5) + (NAN, NAN) = (NAN, NAN)
	Range r11 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			  + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r11.size());
	EXPECT_EQ(true, (r11[0].first).isNotNumber());
	EXPECT_EQ(true, (r11[0].second).isNotNumber());

	// (-INF, 5) + (5, 8) = (-INF, 5+8)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(5+8))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			+ Range(Interval(F<double>(5), F<double>(8))));

	// (-INF, 5) + (10, INF) = (NAN, NAN) U (-INF, INF)
	Range r = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			+ Range(Interval(F<double>(10), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r.size());
	EXPECT_EQ(true, (r[0].first).isNotNumber());
	EXPECT_EQ(true, (r[0].second).isNotNumber());
	EXPECT_EQ(true, (r[1].first).isNegativeInf());
	EXPECT_EQ(true, (r[1].second).isPositiveInf());

	// (-INF, INF) + (INF, INF) = (NAN, NAN) U (INF, INF)
	Range r13 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r13.size());
	EXPECT_EQ(true, (r13[0].first).isNotNumber());
	EXPECT_EQ(true, (r13[0].second).isNotNumber());
	EXPECT_EQ(true, (r13[1].first).isPositiveInf());
	EXPECT_EQ(true, (r13[1].second).isPositiveInf());

	// (-INF, INF) + (-INF, -INF) = (NAN, NAN) U (-INF, -INF)
	Range r14 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(2), r14.size());
	EXPECT_EQ(true, (r14[0].first).isNotNumber());
	EXPECT_EQ(true, (r14[0].second).isNotNumber());
	EXPECT_EQ(true, (r14[1].first).isNegativeInf());
	EXPECT_EQ(true, (r14[1].second).isNegativeInf());

	// (-INF, INF) + (-INF, 5) = (NAN, NAN) U (-INF, INF)
	Range r15 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r15.size());
	EXPECT_EQ(true, (r15[0].first).isNotNumber());
	EXPECT_EQ(true, (r15[0].second).isNotNumber());
	EXPECT_EQ(true, (r15[1].first).isNegativeInf());
	EXPECT_EQ(true, (r15[1].second).isPositiveInf());

	// (-INF, INF) + (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r16 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r16.size());
	EXPECT_EQ(true, (r16[0].first).isNotNumber());
	EXPECT_EQ(true, (r16[0].second).isNotNumber());
	EXPECT_EQ(true, (r16[1].first).isNegativeInf());
	EXPECT_EQ(true, (r16[1].second).isPositiveInf());

	// (-INF, INF) + (NAN, NAN) = (NAN, NAN)
	Range r17 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r17.size());
	EXPECT_EQ(true, (r17[0].first).isNotNumber());
	EXPECT_EQ(true, (r17[0].second).isNotNumber());

	// (-INF, INF) + (5, 8) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			+ Range(Interval(F<double>(5), F<double>(8))));

	// (-INF, INF) + (5, INF) = (NAN, NAN) U (-INF, INF)
	Range r18 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  + Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r18.size());
	EXPECT_EQ(true, (r18[0].first).isNotNumber());
	EXPECT_EQ(true, (r18[0].second).isNotNumber());
	EXPECT_EQ(true, (r18[1].first).isNegativeInf());
	EXPECT_EQ(true, (r18[1].second).isPositiveInf());

	// (NAN, NAN) + (INF, INF) = (NAN, NAN)
	Range r19 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r19.size());
	EXPECT_EQ(true, (r19[0].first).isNotNumber());
	EXPECT_EQ(true, (r19[0].second).isNotNumber());

	// (NAN, NAN) + (-INF, -INF) = (NAN, NAN)
	Range r20 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(1), r20.size());
	EXPECT_EQ(true, (r20[0].first).isNotNumber());
	EXPECT_EQ(true, (r20[0].second).isNotNumber());

	// (NAN, NAN) + (-INF, 5) = (NAN, NAN)
	Range r21 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(1), r21.size());
	EXPECT_EQ(true, (r21[0].first).isNotNumber());
	EXPECT_EQ(true, (r21[0].second).isNotNumber());

	// (NAN, NAN) + (-INF, INF) = (NAN, NAN)
	Range r22 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r22.size());
	EXPECT_EQ(true, (r22[0].first).isNotNumber());
	EXPECT_EQ(true, (r22[0].second).isNotNumber());

	// (NAN, NAN) + (NAN,  NAN) = (NAN, NAN)
	Range r23 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r23.size());
	EXPECT_EQ(true, (r23[0].first).isNotNumber());
	EXPECT_EQ(true, (r23[0].second).isNotNumber());

	// (NAN, NAN) + (5,  8) = (NAN, NAN)
	Range r24 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(5), F<double>(8)));
	EXPECT_EQ(size_t(1), r24.size());
	EXPECT_EQ(true, (r24[0].first).isNotNumber());
	EXPECT_EQ(true, (r24[0].second).isNotNumber());

	// (NAN, NAN) + (5,  INF) = (NAN, NAN)
	Range r25 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  + Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r25.size());
	EXPECT_EQ(true, (r25[0].first).isNotNumber());
	EXPECT_EQ(true, (r25[0].second).isNotNumber());

	// (5, 8) + (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (5, 8) + (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (5, 8) + (-INF, 5) = (-INF, 13)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(13))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (5, 8) + (-INF, INF) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (5,  8) + (NAN, NAN) = (NAN, NAN)
	Range r26 = Range(Interval(F<double>(5), F<double>(8)))
			  + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r26.size());
	EXPECT_EQ(true, (r26[0].first).isNotNumber());
	EXPECT_EQ(true, (r26[0].second).isNotNumber());

	// (5, 8) + (10, 20) = (15, 28)
	EXPECT_EQ(Range(Interval(F<double>(15), F<double>(28))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(10), F<double>(20))));

	// (5, 8) + (10, INF) = (15, INF)
	EXPECT_EQ(Range(Interval(F<double>(15), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			+ Range(Interval(F<double>(10), F<double>(INFINITY))));

	// (5, INF) + (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			+ Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (5,  INF) + (-INF, -INF) = (NAN, NAN) U (-INF, -INF)
	Range r27 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(2), r27.size());
	EXPECT_EQ(true, (r27[0].first).isNotNumber());
	EXPECT_EQ(true, (r27[0].second).isNotNumber());
	EXPECT_EQ(true, (r27[1].first).isNegativeInf());
	EXPECT_EQ(true, (r27[1].second).isNegativeInf());

	// (5,  INF) + (-INF, 5) = (NAN, NAN) U (-INF, INF)
	Range r28 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r28.size());
	EXPECT_EQ(true, (r28[0].first).isNotNumber());
	EXPECT_EQ(true, (r28[0].second).isNotNumber());
	EXPECT_EQ(true, (r28[1].first).isNegativeInf());
	EXPECT_EQ(true, (r28[1].second).isPositiveInf());

	// (5,  INF) + (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r29 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  + Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r29.size());
	EXPECT_EQ(true, (r29[0].first).isNotNumber());
	EXPECT_EQ(true, (r29[0].second).isNotNumber());
	EXPECT_EQ(true, (r29[1].first).isNegativeInf());
	EXPECT_EQ(true, (r29[1].second).isPositiveInf());

	// (5,  INF) + (NAN, NAN) = (NAN, NAN)
	Range r30 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  + Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r30.size());
	EXPECT_EQ(true, (r30[0].first).isNotNumber());
	EXPECT_EQ(true, (r30[0].second).isNotNumber());

	// (5, INF) + (10, 20) = (15, INFINITY)
	EXPECT_EQ(Range(Interval(F<double>(15), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			+ Range(Interval(F<double>(10), F<double>(20))));

	// (5, INF) + (10, INFINITY) = (15, INFINITY)
	EXPECT_EQ(Range(Interval(F<double>(15), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			+ Range(Interval(F<double>(10), F<double>(INFINITY))));
}

TEST_F(RangeTest,
BinaryPlusOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(Range() + Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range(I<int>(15)) + Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range() + Range(F<double>(1.5)), ".*!r1.empty().*!r2.empty().*");
}


////////////////////////////////////////////////////////////////////////////////
// - (binary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BinaryMinusOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(148)),
					Interval(I<int>(vmax<int>()), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(149)))
			- Range(Interval(I<int>(1), I<int>(1))));

	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(204))),
			  Range(Interval(I<int>(10), I<int>(200)))
			- Range(Interval(I<int>(-4), I<int>(10))));

	EXPECT_EQ(Range(Interval(I<long>(vmin<long>()), I<long>(199)),
					Interval(I<long>(vmax<long>()-9), I<long>(vmax<long>()))),
			  Range(Interval(I<long>(vmin<long>()), I<long>(200)))
			- Range(Interval(I<long>(1), I<long>(10))));
}

TEST_F(RangeTest,
BinaryMinusOfSignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + 2 + vmax<unsigned>()),
							 I<unsigned>(-5 + vmax<unsigned>() + 2))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			- Range(Interval(I<unsigned>(vmax<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + vmax<unsigned>() + 1),
							 I<unsigned>(-5 + vmax<unsigned>() + 1))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			- Range(Interval(I<unsigned>(0),
							 I<unsigned>(0))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<int>() + vmax<unsigned>() + 1),
							 I<unsigned>(0))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(0)))
			- Range(Interval(I<unsigned>(0),
				             I<unsigned>(0))));
}

TEST_F(RangeTest,
BinaryMinusOfSignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(9), F<float>(vmax<int>() - 1.0))),
			  Range(Interval(I<int>(10), I<int>(vmax<int>())))
			- Range(Interval(F<float>(1), F<float>(1))));

	EXPECT_EQ(Range(Interval(F<float>(vmin<float>()),
							 F<float>(-4))),
			  Range(Interval(I<int>(1), I<int>(1)))
			- Range(Interval(F<float>(5.0), F<float>(vmax<float>()))));

	EXPECT_EQ(Range(Interval(F<float>(vmin<int>() + 105.35),
							 F<float>(vmax<int>() - 105.35))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>())))
			- Range(Interval(F<float>(-105.35), F<float>(105.35))));
}

TEST_F(RangeTest,
BinaryMinusOfUnsignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
			  				 I<unsigned>(vmax<unsigned>())))
			- Range(Interval(I<int>(1), I<int>(1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(4535+100)),
					Interval(I<int>(vmax<unsigned>() - 1),
							 I<int>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(8), I<unsigned>(4535)))
			- Range(Interval(I<int>(-100), I<int>(10))));
}

TEST_F(RangeTest,
BinaryMinusOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(0)),
					Interval(I<unsigned>(11), I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>())))
			- Range(Interval(I<unsigned>(-1), I<unsigned>(-1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(8), I<unsigned>(14)),
					Interval(I<unsigned>(18), I<unsigned>(24)),
					Interval(I<unsigned>(98), I<unsigned>(124))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(15)),
			  		Interval(I<unsigned>(20), I<unsigned>(25)),
			  		Interval(I<unsigned>(100), I<unsigned>(125)))
			- Range(Interval(I<unsigned>(1), I<unsigned>(1)),
					Interval(I<unsigned>(2), I<unsigned>(2))));
}

TEST_F(RangeTest,
BinaryMinusOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<unsigned>() - vmax<double>()),
							 F<double>(vmax<unsigned>() - vmin<double>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>())))
			- Range(Interval(F<double>(vmin<double>()),
							 F<double>(vmax<double>()))));

	EXPECT_EQ(Range(Interval(F<double>(vmin<unsigned>() - 1.0),
							 F<double>(vmax<unsigned>() - 1.0))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>())))
			- Range(Interval(F<double>(1.0),
							 F<double>(1.0))));

	EXPECT_EQ(Range(Interval(F<double>(25 - 128.0), F<double>(1438 + 27.35))),
			  Range(Interval(I<unsigned>(25), I<unsigned>(1438)))
			- Range(Interval(F<double>(-27.35), F<double>(128.0))));
}

TEST_F(RangeTest,
BinaryMinusOfFloatAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<double>() - 128.35),
							 F<double>(vmax<double>() + 1265.78))),
			  Range(Interval(F<double>(vmin<double>()),
			  				 F<double>(vmax<double>())))
			- Range(Interval(I<int>(-1265.78), I<int>(128.35))));
}

TEST_F(RangeTest,
BinaryMinusOfFloatAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-12589.6 - 589),
							 F<double>(12583.78 - 437))),
			  Range(Interval(F<double>(-12589.6), F<double>(12583.78)))
			- Range(Interval(I<unsigned>(437), I<unsigned>(589))));
}

TEST_F(RangeTest,
BinaryMinusOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmin<double>())) - Range(F<double>(1e308)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) - Range(F<double>(-1e308)));

	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmin<double>())) - Range(F<double>(1e300)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) - Range(F<double>(-1e300)));

	EXPECT_EQ(Range(Interval(F<double>(14.35 - 7898.0),
							 F<double>(14587.8 - 7.8))),
			  Range(Interval(F<double>(14.35), F<double>(14587.8)))
			- Range(Interval(F<double>(7.8), F<double>(7898.0))));
}

TEST_F(RangeTest,
BinaryMinusOfSpecialFloatsWorksCorrectly)
{
	// (INF, INF) - (INF, INF) = (NAN, NAN)
	Range r1 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 - Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r1.size());
	EXPECT_EQ(true, (r1[0].first).isNotNumber());
	EXPECT_EQ(true, (r1[0].second).isNotNumber());

	// (INF, INF) - (-INF, -INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			- Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (INF, INF) - (-INF, 5) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			- Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (INF, INF) - (-INF, INF) = (NAN, NAN) U (INF, INF)
	Range r2 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r2.size());
	EXPECT_EQ(true, (r2[0].first).isNotNumber());
	EXPECT_EQ(true, (r2[0].second).isNotNumber());
	EXPECT_EQ(true, (r2[1].first).isPositiveInf());
	EXPECT_EQ(true, (r2[1].second).isPositiveInf());

	// (INF, INF) - (NAN, NAN) = (NAN, NAN)
	Range r3 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r3.size());
	EXPECT_EQ(true, (r3[0].first).isNotNumber());
	EXPECT_EQ(true, (r3[0].second).isNotNumber());

	// (INF, INF) - (x, y) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			- Range(Interval(F<double>(5), F<double>(10))));

	// (INF, INF) - (x, INF) = (NAN, NAN) U (INF, INF)
	Range r4 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 - Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r4.size());
	EXPECT_EQ(true, (r4[0].first).isNotNumber());
	EXPECT_EQ(true, (r4[0].second).isNotNumber());
	EXPECT_EQ(true, (r4[1].first).isPositiveInf());
	EXPECT_EQ(true, (r4[1].second).isPositiveInf());

	// (-INF, -INF) - (INF, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			- Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (-INF, -INF) - (-INF, -INF) = (NAN, NAN)
	Range r5 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 - Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(1), r5.size());
	EXPECT_EQ(true, (r5[0].first).isNotNumber());
	EXPECT_EQ(true, (r5[0].second).isNotNumber());

	// (-INF, -INF) - (-INF, 5) = (NAN,NAN) U (-INF, -INF)
	Range r6 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 - Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r6.size());
	EXPECT_EQ(true, (r6[0].first).isNotNumber());
	EXPECT_EQ(true, (r6[0].second).isNotNumber());
	EXPECT_EQ(true, (r6[1].first).isNegativeInf());
	EXPECT_EQ(true, (r6[1].second).isNegativeInf());

	// (-INF, -INF) - (-INF, INF) = (NAN, NAN) U (-INF, -INF)
	Range r7 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r7.size());
	EXPECT_EQ(true, (r7[0].first).isNotNumber());
	EXPECT_EQ(true, (r7[0].second).isNotNumber());
	EXPECT_EQ(true, (r7[1].first).isNegativeInf());
	EXPECT_EQ(true, (r7[1].second).isNegativeInf());

	// (-INF, -INF) - (NAN, NAN) = (NAN, NAN)
	Range r8 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r8.size());
	EXPECT_EQ(true, (r8[0].first).isNotNumber());
	EXPECT_EQ(true, (r8[0].second).isNotNumber());

	// (-INF, -INF) - (1, 5) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			- Range(Interval(F<double>(1), F<double>(5))));

	// (-INF, -INF) - (1, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			- Range(Interval(F<double>(1), F<double>(INFINITY))));

	// (-INF, 5) - (INF, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			- Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (-INF, 5) - (-INF, -INF) = (NAN, NAN)
	Range r9 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			 - Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(2), r9.size());
	EXPECT_EQ(true, (r9[0].first).isNotNumber());
	EXPECT_EQ(true, (r9[0].second).isNotNumber());
	EXPECT_EQ(true, (r9[1].first).isPositiveInf());
	EXPECT_EQ(true, (r9[1].second).isPositiveInf());

	// (-INF, 5) - (-INF, 5) = (NAN, NAN) U (-INF, INF)
	Range r10 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r10.size());
	EXPECT_EQ(true, (r10[0].first).isNotNumber());
	EXPECT_EQ(true, (r10[0].second).isNotNumber());
	EXPECT_EQ(true, (r10[1].first).isNegativeInf());
	EXPECT_EQ(true, (r10[1].second).isPositiveInf());

	// (-INF, 5) - (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r11 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r11.size());
	EXPECT_EQ(true, (r11[0].first).isNotNumber());
	EXPECT_EQ(true, (r11[0].second).isNotNumber());
	EXPECT_EQ(true, (r11[1].first).isNegativeInf());
	EXPECT_EQ(true, (r11[1].second).isPositiveInf());

	// (-INF, 5) - (NAN, NAN) = (NAN, NAN)
	Range r12 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			  - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r12.size());
	EXPECT_EQ(true, (r12[0].first).isNotNumber());
	EXPECT_EQ(true, (r12[0].second).isNotNumber());

	// (-INF, 5) - (10, 20) = (-INF, 5-10)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(5-10))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			- Range(Interval(F<double>(10), F<double>(20))));

	// (-INF, 5) - (10, INF) = (-INF, 5-10)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(5-10))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			- Range(Interval(F<double>(10), F<double>(INFINITY))));

	// (-INF, INF) - (INF, INF) = (NAN, NAN) U (-INF, -INF)
	Range r13 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r13.size());
	EXPECT_EQ(true, (r13[0].first).isNotNumber());
	EXPECT_EQ(true, (r13[0].second).isNotNumber());
	EXPECT_EQ(true, (r13[1].first).isNegativeInf());
	EXPECT_EQ(true, (r13[1].second).isNegativeInf());

	// (-INF, INF) - (-INF, -INF) = (NAN, NAN) U (INF, INF)
	Range r14 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(2), r14.size());
	EXPECT_EQ(true, (r14[0].first).isNotNumber());
	EXPECT_EQ(true, (r14[0].second).isNotNumber());
	EXPECT_EQ(true, (r14[1].first).isPositiveInf());
	EXPECT_EQ(true, (r14[1].second).isPositiveInf());

	// (-INF, INF) - (-INF, 5) = (NAN, NAN) U (-INF, INF)
	Range r15 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(2), r15.size());
	EXPECT_EQ(true, (r15[0].first).isNotNumber());
	EXPECT_EQ(true, (r15[0].second).isNotNumber());
	EXPECT_EQ(true, (r15[1].first).isNegativeInf());
	EXPECT_EQ(true, (r15[1].second).isPositiveInf());

	// (-INF, INF) - (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r16 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r16.size());
	EXPECT_EQ(true, (r16[0].first).isNotNumber());
	EXPECT_EQ(true, (r16[0].second).isNotNumber());
	EXPECT_EQ(true, (r16[1].first).isNegativeInf());
	EXPECT_EQ(true, (r16[1].second).isPositiveInf());

	// (-INF, INF) - (NAN, NAN) = (NAN, NAN)
	Range r17 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r17.size());
	EXPECT_EQ(true, (r17[0].first).isNotNumber());
	EXPECT_EQ(true, (r17[0].second).isNotNumber());

	// (-INF, INF) - (5, 8) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			- Range(Interval(F<double>(5), F<double>(8))));

	// (-INF, INF) - (5, INF) = (NAN, NAN) U (-INF, INF)
	Range r18 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			  - Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r18.size());
	EXPECT_EQ(true, (r18[0].first).isNotNumber());
	EXPECT_EQ(true, (r18[0].second).isNotNumber());
	EXPECT_EQ(true, (r18[1].first).isNegativeInf());
	EXPECT_EQ(true, (r18[1].second).isPositiveInf());

	// (NAN, NAN) - (INF, INF) = (NAN, NAN)
	Range r19 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r19.size());
	EXPECT_EQ(true, (r19[0].first).isNotNumber());
	EXPECT_EQ(true, (r19[0].second).isNotNumber());

	// (NAN, NAN) - (-INF, -INF) = (NAN, NAN)
	Range r20 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(1), r20.size());
	EXPECT_EQ(true, (r20[0].first).isNotNumber());
	EXPECT_EQ(true, (r20[0].second).isNotNumber());

	// (NAN, NAN) - (-INF, 5) = (NAN, NAN)
	Range r21 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(1), r21.size());
	EXPECT_EQ(true, (r21[0].first).isNotNumber());
	EXPECT_EQ(true, (r21[0].second).isNotNumber());

	// (NAN, NAN) - (-INF, INF) = (NAN, NAN)
	Range r22 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r22.size());
	EXPECT_EQ(true, (r22[0].first).isNotNumber());
	EXPECT_EQ(true, (r22[0].second).isNotNumber());

	// (NAN, NAN) - (NAN,  NAN) = (NAN, NAN)
	Range r23 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r23.size());
	EXPECT_EQ(true, (r23[0].first).isNotNumber());
	EXPECT_EQ(true, (r23[0].second).isNotNumber());

	// (NAN, NAN) - (5,  8) = (NAN, NAN)
	Range r24 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(5), F<double>(8)));
	EXPECT_EQ(size_t(1), r24.size());
	EXPECT_EQ(true, (r24[0].first).isNotNumber());
	EXPECT_EQ(true, (r24[0].second).isNotNumber());

	// (NAN, NAN) - (5,  INF) = (NAN, NAN)
	Range r25 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  - Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r25.size());
	EXPECT_EQ(true, (r25[0].first).isNotNumber());
	EXPECT_EQ(true, (r25[0].second).isNotNumber());

	// (5, 8) - (INF, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (5, 8) - (-INF, -INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (5, 8) - (-INF, 10) = (5-10, INF)
	EXPECT_EQ(Range(Interval(F<double>(5-10), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(-INFINITY), F<double>(10))));

	// (5, 8) - (-INF, INF) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (5,  8) - (NAN, NAN) = (NAN, NAN)
	Range r26 = Range(Interval(F<double>(5), F<double>(8)))
			  - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r26.size());
	EXPECT_EQ(true, (r26[0].first).isNotNumber());
	EXPECT_EQ(true, (r26[0].second).isNotNumber());

	// (5, 8) - (10, 20) = (-15, -2)
	EXPECT_EQ(Range(Interval(F<double>(5-20), F<double>(8-10))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(10), F<double>(20))));

	// (5, 8) - (10, INF) = (-INF, -2)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(8-10))),
			  Range(Interval(F<double>(5), F<double>(8)))
			- Range(Interval(F<double>(10), F<double>(INFINITY))));

	// (5, INF) - (INF, INF) = (NAN, NAN) U (-INF, -INF)
	Range r27 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  - Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r27.size());
	EXPECT_EQ(true, (r27[0].first).isNotNumber());
	EXPECT_EQ(true, (r27[0].second).isNotNumber());
	EXPECT_EQ(true, (r27[1].first).isNegativeInf());
	EXPECT_EQ(true, (r27[1].second).isNegativeInf());

	// (5,  INF) - (-INF, -INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			- Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (5,  INF) - (-INF, 10) = (5 - 10, INF)
	EXPECT_EQ(Range(Interval(F<double>(5-10), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			- Range(Interval(F<double>(-INFINITY), F<double>(10))));

	// (5,  INF) - (-INF, INF) = (NAN, NAN) U (-INF, INF)
	Range r28 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  - Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r28.size());
	EXPECT_EQ(true, (r28[0].first).isNotNumber());
	EXPECT_EQ(true, (r28[0].second).isNotNumber());
	EXPECT_EQ(true, (r28[1].first).isNegativeInf());
	EXPECT_EQ(true, (r28[1].second).isPositiveInf());

	// (5,  INF) - (NAN, NAN) = (NAN, NAN)
	Range r29 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  - Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r29.size());
	EXPECT_EQ(true, (r29[0].first).isNotNumber());
	EXPECT_EQ(true, (r29[0].second).isNotNumber());

	// (5, INF) - (10, 20) = (5-20, INF)
	EXPECT_EQ(Range(Interval(F<double>(5-20), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			- Range(Interval(F<double>(10), F<double>(20))));

	// (5, INF) - (10, INF) = (NAN, NAN) U (-INF, INF)
	Range r30 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  - Range(Interval(F<double>(10), F<double>(INFINITY)));
	EXPECT_EQ(size_t(2), r30.size());
	EXPECT_EQ(true, (r30[0].first).isNotNumber());
	EXPECT_EQ(true, (r30[0].second).isNotNumber());
	EXPECT_EQ(true, (r30[1].first).isNegativeInf());
	EXPECT_EQ(true, (r30[1].second).isPositiveInf());
}

TEST_F(RangeTest,
BinaryMinusOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(Range() - Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range(I<int>(15)) - Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range() - Range(F<double>(1.5)), ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// * (binary)
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
MultiplicationOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(-2)),
					Interval(I<int>(20), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(10), I<int>(vmax<int>())))
			* Range(Interval(I<int>(2), I<int>(2))));

	EXPECT_EQ(Range(Interval(I<int>(-800), I<int>(2000))),
			  Range(Interval(I<int>(10), I<int>(200)))
			* Range(Interval(I<int>(-4), I<int>(10))));

	EXPECT_EQ(Range(Interval(I<long>(vmin<long>()), I<long>(vmax<long>()))),
			  Range(Interval(I<long>(vmin<long>()), I<long>(200)))
			* Range(Interval(I<long>(-1), I<long>(10))));

	EXPECT_EQ(Range(Interval(I<long>(vmin<long>()), I<long>(vmin<long>())),
					Interval(I<long>(-200), I<long>(vmax<long>()))),
			  Range(Interval(I<long>(vmin<long>()), I<long>(200)))
			* Range(Interval(I<long>(-1), I<long>(-1))));

	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>((vmax<int>() / 2 + 5) * 2)),
					Interval(I<int>((vmax<int>() / 2 - 5) * 2), I<int>(vmax<int>()))),
			  Range(Interval(I<int>(vmax<int>() / 2 - 5), I<int>(vmax<int>() / 2 + 5)))
			* Range(Interval(I<int>(2), I<int>(2))));
}

TEST_F(RangeTest,
MultiplicationOfSignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			* Range(Interval(I<unsigned>(vmax<unsigned>()),
				             I<unsigned>(vmax<unsigned>()))));

	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(0))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(-5)))
			* Range(Interval(I<unsigned>(0),
							 I<unsigned>(0))));

	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(0))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(0)))
			* Range(Interval(I<unsigned>(0), I<unsigned>(0))));
}

TEST_F(RangeTest,
MultiplicationOfSignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(10), F<float>(vmax<int>() * 1.0))),
			  Range(Interval(I<int>(10), I<int>(vmax<int>())))
			* Range(Interval(F<float>(1), F<float>(1))));

	EXPECT_EQ(Range(Interval(F<float>(1 * 5.0), F<float>(1 * vmax<float>()))),
			  Range(Interval(I<int>(1), I<int>(1)))
			* Range(Interval(F<float>(5.0), F<float>(vmax<float>()))));

	EXPECT_EQ(Range(Interval(F<float>(vmin<int>() * 105.35),
							 F<float>(vmax<int>() * 105.35))),
			  Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>())))
			* Range(Interval(F<float>(-105.35), F<float>(105.35))));
}

TEST_F(RangeTest,
MultiplicationOfUnsignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>())))
			* Range(Interval(I<int>(-1), I<int>(-1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(vmin<unsigned>()), I<unsigned>(4535*100)),
					Interval(I<unsigned>(unsigned(-45350)),
							 I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(8), I<unsigned>(4535)))
			* Range(Interval(I<int>(-10), I<int>(100))));
}

TEST_F(RangeTest,
MultiplicationOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>())))
			* Range(Interval(I<unsigned>(1), I<unsigned>(1))));

	EXPECT_EQ(Range(Interval(I<unsigned>(20), I<unsigned>(30)),
					Interval(I<unsigned>(40), I<unsigned>(50)),
					Interval(I<unsigned>(200), I<unsigned>(250))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(15)),
					Interval(I<unsigned>(20), I<unsigned>(25)),
					Interval(I<unsigned>(100), I<unsigned>(125)))
			* Range(Interval(I<unsigned>(2), I<unsigned>(2))));

	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(vmax<unsigned>()))),
			  Range(Interval(I<unsigned>(10), I<unsigned>(vmax<unsigned>())))
			* Range(Interval(I<unsigned>(vmax<unsigned>()),
							 I<unsigned>(vmax<unsigned>()))));
}

TEST_F(RangeTest,
MultiplicationOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmax<unsigned>() * vmin<double>()),
							 F<double>(vmax<unsigned>() * vmax<double>()))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
							 I<unsigned>(vmax<unsigned>())))
			* Range(Interval(F<double>(vmin<double>()),
							 F<double>(vmax<double>()))));

	EXPECT_EQ(Range(Interval(F<double>(vmax<unsigned>() * (-1.0)),
							 F<double>(vmin<unsigned>() * (-1.0)))),
			  Range(Interval(I<unsigned>(vmin<unsigned>()),
							  I<unsigned>(vmax<unsigned>())))
			* Range(Interval(F<double>(-1.0), F<double>(-1.0))));

	EXPECT_EQ(Range(Interval(F<double>(1438 * (-27.35)), F<double>(1438 * 128.0))),
			  Range(Interval(I<unsigned>(25), I<unsigned>(1438)))
			* Range(Interval(F<double>(-27.35), F<double>(128.0))));
}

TEST_F(RangeTest,
MultiplicationOfFloatAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(vmin<double>() * 128.35),
							 F<double>(vmax<double>() * 128.35))),
			  Range(Interval(F<double>(vmin<double>()),
							 F<double>(vmax<double>())))
			* Range(Interval(I<int>(-1265.78), I<int>(128.35))));
}

TEST_F(RangeTest,
MultiplicationOfFloatAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-12589.6 * 589),
							 F<double>(12583.78 * 589))),
			  Range(Interval(F<double>(-12589.6), F<double>(12583.78)))
			* Range(Interval(I<unsigned>(437), I<unsigned>(589))));
}

TEST_F(RangeTest,
MultiplicationOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmin<double>())) * Range(F<double>(-1e308)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) * Range(F<double>(1e308)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmin<double>())) * Range(F<double>(-1e300)));

	EXPECT_EQ(Range(F<double>(INFINITY)),
			  Range(F<double>(vmax<double>())) * Range(F<double>(1e300)));

	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmin<double>())) * Range(F<double>(1e300)));

	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  Range(F<double>(vmax<double>())) * Range(F<double>(-1e300)));

	EXPECT_EQ(Range(Interval(F<double>(7.8 * 14.35), F<double>(7898.0 * 14587.8))),
			  Range(Interval(F<double>(14.35), F<double>(14587.8)))
			* Range(Interval(F<double>(7.8), F<double>(7898.0))));

	EXPECT_EQ(Range(Interval(F<double>(-50), F<double>(0))),
			  Range(Interval(F<double>(2), F<double>(5)))
			* Range(Interval(F<double>(-10), F<double>(0))));

	EXPECT_EQ(Range(Interval(F<double>(-50), F<double>(50))),
			  Range(Interval(F<double>(2), F<double>(5)))
			* Range(Interval(F<double>(-10), F<double>(10))));

	EXPECT_EQ(Range(Interval(F<double>(-40), F<double>(-6))),
			  Range(Interval(F<double>(2), F<double>(5)))
			* Range(Interval(F<double>(-8), F<double>(-3))));

	EXPECT_EQ(Range(Interval(F<double>(-50), F<double>(50))),
			  Range(Interval(F<double>(-10), F<double>(10)))
			* Range(Interval(F<double>(2), F<double>(5))));

	EXPECT_EQ(Range(Interval(F<double>(-100), F<double>(80))),
			  Range(Interval(F<double>(-5), F<double>(4)))
			* Range(Interval(F<double>(-10), F<double>(20))));

	EXPECT_EQ(Range(Interval(F<double>(-200), F<double>(400))),
			  Range(Interval(F<double>(-5), F<double>(20)))
			* Range(Interval(F<double>(-10), F<double>(20))));

	EXPECT_EQ(Range(Interval(F<double>(-50), F<double>(50))),
			  Range(Interval(F<double>(-5), F<double>(5)))
			* Range(Interval(F<double>(-10), F<double>(-1))));

	EXPECT_EQ(Range(Interval(F<double>(-505), F<double>(-10))),
			  Range(Interval(F<double>(-5), F<double>(-1)))
			* Range(Interval(F<double>(10), F<double>(101))));

	EXPECT_EQ(Range(Interval(F<double>(-50), F<double>(50))),
			  Range(Interval(F<double>(-10), F<double>(-1)))
			* Range(Interval(F<double>(-5), F<double>(5))));

	EXPECT_EQ(Range(Interval(F<double>(-40), F<double>(40))),
			  Range(Interval(F<double>(-8), F<double>(-3)))
			* Range(Interval(F<double>(-5), F<double>(5))));
}

TEST_F(RangeTest,
MultiplicationOfSpecialFloatsWorksCorrectly)
{
	// (INF, INF) * (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (INF, INF) * (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (INF, INF) * (-INF, 5) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (INF, INF) * (-INF, INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (INF, INF) * (NAN, NAN) = (NAN, NAN)
	Range r1 = Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			 * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r1.size());
	EXPECT_EQ(true, (r1[0].first).isNotNumber());
	EXPECT_EQ(true, (r1[0].second).isNotNumber());

	// (INF, INF) * (x, y) = (INF, INF), there is no zero in (x, y)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(5.0), F<double>(8.0))));

	// (INF, INF) * (x, y) = (NAN, NAN) U (INF, INF), there is  zero in (x, y)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-5.0), F<double>(8.0))));

	// (INF, INF) * (x, INF) = (INF, INF), there is no zero in (x, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(1.35), F<double>(INFINITY))));

	// (INF, INF) * (x, INF) = (NAN, NAN) U (INF, INF), there is zero in (x, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-1.35), F<double>(INFINITY))));

	// (-INF, -INF) * (INF, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (-INF, -INF) * (-INF, -INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, -INF) * (-INF, 5) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (-INF, -INF) * (-INF, INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (-INF, -INF) * (NAN, NAN) = (NAN, NAN)
	Range r2 = Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			 * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r2.size());
	EXPECT_EQ(true, (r2[0].first).isNotNumber());
	EXPECT_EQ(true, (r2[0].second).isNotNumber());

	// (-INF, -INF) * (1, 5) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(1), F<double>(5))));

	// (-INF, -INF) * (1, INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))
			* Range(Interval(F<double>(1), F<double>(INFINITY))));

	// (-INF, 5) * (INF, INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (-INF, 5) * (-INF, -INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, 5) * (-INF, 5) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (-INF, 5) * (-INF, INF) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (-INF, 5) * (NAN, NAN) = (NAN, NAN)
	Range r3 = Range(Interval(F<double>(-INFINITY), F<double>(5)))
			 * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r3.size());
	EXPECT_EQ(true, (r3[0].first).isNotNumber());
	EXPECT_EQ(true, (r3[0].second).isNotNumber());

	// (-INF, 5) * (5, 8) = (-INF, 5*8)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(40))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(5), F<double>(8))));

	// (-INF, 5) * (10, INF) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(5)))
			* Range(Interval(F<double>(10), F<double>(INFINITY))));

	// (-INF, INF) * (INF, INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (-INF, INF) * (-INF, -INF) = (NAN, NAN) U (-INF, -INF) U (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(-INFINITY)),
					Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (-INF, INF) * (-INF, 5) =  (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (-INF, INF) * (-INF, INF) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (-INF, INF) * (NAN, NAN) = (NAN, NAN)
	Range r4 = Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			 * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r4.size());
	EXPECT_EQ(true, (r4[0].first).isNotNumber());
	EXPECT_EQ(true, (r4[0].second).isNotNumber());

	// (-INF, INF) * (5, 8) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(5), F<double>(8))));

	// (-INF, INF) * (5, INF) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))
			* Range(Interval(F<double>(5), F<double>(INFINITY))));

	// (NAN, NAN) * (INF, INF) = (NAN, NAN)
	Range r5 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			 * Range(Interval(F<double>(INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r5.size());
	EXPECT_EQ(true, (r5[0].first).isNotNumber());
	EXPECT_EQ(true, (r5[0].second).isNotNumber());

	// (NAN, NAN) * (-INF, -INF) = (NAN, NAN)
	Range r6 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			 * Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)));
	EXPECT_EQ(size_t(1), r6.size());
	EXPECT_EQ(true, (r6[0].first).isNotNumber());
	EXPECT_EQ(true, (r6[0].second).isNotNumber());

	// (NAN, NAN) * (-INF, 5) = (NAN, NAN)
	Range r7 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			 * Range(Interval(F<double>(-INFINITY), F<double>(5)));
	EXPECT_EQ(size_t(1), r7.size());
	EXPECT_EQ(true, (r7[0].first).isNotNumber());
	EXPECT_EQ(true, (r7[0].second).isNotNumber());

	// (NAN, NAN) * (-INF, INF) = (NAN, NAN)
	Range r8 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			 * Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r8.size());
	EXPECT_EQ(true, (r8[0].first).isNotNumber());
	EXPECT_EQ(true, (r8[0].second).isNotNumber());

	// (NAN, NAN) * (NAN,  NAN) = (NAN, NAN)
	Range r9 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			 * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r9.size());
	EXPECT_EQ(true, (r9[0].first).isNotNumber());
	EXPECT_EQ(true, (r9[0].second).isNotNumber());

	// (NAN, NAN) * (5,  8) = (NAN, NAN)
	Range r10 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  * Range(Interval(F<double>(5), F<double>(8)));
	EXPECT_EQ(size_t(1), r10.size());
	EXPECT_EQ(true, (r10[0].first).isNotNumber());
	EXPECT_EQ(true, (r10[0].second).isNotNumber());

	// (NAN, NAN) * (5,  INF) = (NAN, NAN)
	Range r11 = Range(Interval(F<double>(NAN), F<double>(NAN)))
			  * Range(Interval(F<double>(5), F<double>(INFINITY)));
	EXPECT_EQ(size_t(1), r11.size());
	EXPECT_EQ(true, (r11[0].first).isNotNumber());
	EXPECT_EQ(true, (r11[0].second).isNotNumber());

	// (5, 8) * (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (5, 8) * (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (5, 8) * (-INF, 5) = (-INF, 40)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(40))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (5, 8) * (-INF, INF) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (5,  8) * (NAN, NAN) = (NAN, NAN)
	Range r12 = Range(Interval(F<double>(5), F<double>(8)))
			  * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r12.size());
	EXPECT_EQ(true, (r12[0].first).isNotNumber());
	EXPECT_EQ(true, (r12[0].second).isNotNumber());

	// (5, 8) * (10, 20) = (50, 160)
	EXPECT_EQ(Range(Interval(F<double>(50), F<double>(160))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(10), F<double>(20))));

	// (5, 8) * (10, INF) = (50, INF)
	EXPECT_EQ(Range(Interval(F<double>(50), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(8)))
			* Range(Interval(F<double>(10), F<double>(INFINITY))));

	// (5, INF) * (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(INFINITY), F<double>(INFINITY))));

	// (5,  INF) * (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))));

	// (5, INF) * (-INF, 5) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(5))));

	// (5, INF) * (-INF, INF) = (NAN, NAN) U (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN)),
					Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))));

	// (5,  INF) * (NAN, NAN) = (NAN, NAN)
	Range r13 = Range(Interval(F<double>(5), F<double>(INFINITY)))
			  * Range(Interval(F<double>(NAN), F<double>(NAN)));
	EXPECT_EQ(size_t(1), r13.size());
	EXPECT_EQ(true, (r13[0].first).isNotNumber());
	EXPECT_EQ(true, (r13[0].second).isNotNumber());

	// (5, INF) * (10, 20) = (50, INF)
	EXPECT_EQ(Range(Interval(F<double>(50), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(10), F<double>(20))));

	// (5, INF) * (10, INF) = (50, INF)
	EXPECT_EQ(Range(Interval(F<double>(50), F<double>(INFINITY))),
			  Range(Interval(F<double>(5), F<double>(INFINITY)))
			* Range(Interval(F<double>(10), F<double>(INFINITY))));
}

TEST_F(RangeTest,
MultiplicationOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(Range() * Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range(I<int>(15)) * Range(), ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(Range() * Range(F<double>(1.5)), ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// rdiv()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
RDivReturnsCorrectResult)
{
	// (0.0, 0.0) / (0.0, 0.0)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN))),
		rdiv(Range(Interval(F<double>(0.0), F<double>(0.0))),
			Range(Interval(F<double>(0.0), F<double>(0.0)))));

	// (0.0, 0.0) / (1.0, 1.0)
	EXPECT_EQ(Range(Interval(F<double>(0.0), F<double>(0.0))),
		rdiv(Range(Interval(F<double>(0.0), F<double>(0.0))),
			Range(Interval(F<double>(1.0), F<double>(1.0)))));

	// (1.0, 1.0) / (0.0, 0.0)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
		rdiv(Range(Interval(F<double>(1.0), F<double>(1.0))),
			Range(Interval(F<double>(0.0), F<double>(0.0)))));

	// (-1.0, -1.0) / (0.0, 0.0)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
		rdiv(Range(Interval(F<double>(-1.0), F<double>(-1.0))),
			Range(Interval(F<double>(0.0), F<double>(0.0)))));

	// (1.0, 5.0) / (2.0, 3.0)
	EXPECT_EQ(Range(Interval(F<double>(1.0 / 3.0), F<double>(1.0 / 2.0)),
			Interval(F<double>(5.0 / 3.0), F<double>(5.0 / 2.0))),
		rdiv(Range(Interval(F<double>(1.0), F<double>(5.0))),
			Range(Interval(F<double>(2.0), F<double>(3.0)))));

	// (INF, INF) / (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN))),
		rdiv(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));
}

////////////////////////////////////////////////////////////////////////////////
// exact_div()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ExactDivDivisionByZeroResultsIntoOverApproximation)
{
	// x / 0 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		exact_div(Range(Interval(I<int>(1), I<int>(5))), Range(I<int>(0))));
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		exact_div(Range(Interval(I<int>(1), I<int>(5))),
			Range(Interval(I<int>(-5), I<int>(5)))));
}

TEST_F(RangeTest,
ExactDivDivisionIntMinByMinusOneResultsIntoOverApproximation)
{
	// INT_MIN / -1 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		exact_div(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
			Range(Interval(I<int>(-1), I<int>(-1)))));
}

TEST_F(RangeTest,
ExactDivWorksCorrectlyWhenDividingSignedIntWithSignedInt)
{
	// (4, 6) / (2, 3)
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(3))),
		exact_div(Range(Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(2), I<int>(3)))));
	// (4, 6) / (-3, -2)
	EXPECT_EQ(Range(Interval(I<int>(-3), I<int>(-1))),
		exact_div(Range(Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(-3), I<int>(-2)))));
	// (-2, -1) / (1, 3)
	EXPECT_EQ(Range(Interval(I<int>(-2), I<int>(0))),
		exact_div(Range(Interval(I<int>(-2), I<int>(-1))),
			Range(Interval(I<int>(1), I<int>(3)))));
	// (-2, -1) / (-3, -1)
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(2))),
		exact_div(Range(Interval(I<int>(-2), I<int>(-1))),
			Range(Interval(I<int>(-3), I<int>(-1)))));
	// (0, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(3))),
		exact_div(Range(Interval(I<int>(0), I<int>(3))),
			Range(Interval(I<int>(1), I<int>(4)))));
	// (-2, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<int>(-2), I<int>(3))),
		exact_div(Range(Interval(I<int>(-2), I<int>(3))),
			Range(Interval(I<int>(1), I<int>(4)))));
	// (INT_MIN, INT_MIN) / (1, 1)
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
		exact_div(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
			Range(Interval(I<int>(1), I<int>(1)))));
	// (-2, -1)(4, 6) / (-3, -1)(1, 3)
	EXPECT_EQ(Range(Interval(I<int>(-6), I<int>(6))),
		exact_div(Range(Interval(I<int>(-2), I<int>(-1)), Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(-3), I<int>(-1)), Interval(I<int>(1), I<int>(3)))));
}

TEST_F(RangeTest,
ExactDivWorksCorrectlyWhenDividingUnsignedIntWithUnsignedInt)
{
	// (4, 6) / (2, 3)
	EXPECT_EQ(Range(Interval(I<unsigned>(1), I<unsigned>(3))),
		exact_div(Range(Interval(I<unsigned>(4), I<unsigned>(6))),
			Range(Interval(I<unsigned>(2), I<unsigned>(3)))));
	// (0, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(3))),
		exact_div(Range(Interval(I<unsigned>(0), I<unsigned>(3))),
			Range(Interval(I<unsigned>(1), I<unsigned>(4)))));
}

TEST_F(RangeTest,
ExactDivWorksCorrectlyWhenDividingSignedIntWithUnsignedInt)
{
	// (-1, -1) / (1, 4)
	EXPECT_EQ(Range(Interval(I<unsigned>(vmax<unsigned>() / 4), I<unsigned>(vmax<unsigned>()))),
		exact_div(Range(Interval(I<int>(-1), I<int>(-1))),
			Range(Interval(I<unsigned>(1), I<unsigned>(4)))));
}

////////////////////////////////////////////////////////////////////////////////
// trunc_div()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
TruncDivDivisionByZeroResultsIntoOverApproximation)
{
	// x / 0 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_div(Range(Interval(I<int>(1), I<int>(5))), Range(I<int>(0))));
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_div(Range(Interval(I<int>(1), I<int>(5))),
			Range(Interval(I<int>(-5), I<int>(5)))));
}

TEST_F(RangeTest,
TruncDivDivisionIntMinByMinusOneResultsIntoOverApproximation)
{
	// INT_MIN / -1 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_div(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
			Range(Interval(I<int>(-1), I<int>(-1)))));
}

TEST_F(RangeTest,
TruncDivWorksCorrectlyWhenDividingSignedIntWithSignedInt)
{
	// (4, 6) / (2, 3)
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(3))),
		trunc_div(Range(Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(2), I<int>(3)))));
	// (4, 6) / (-3, -2)
	EXPECT_EQ(Range(Interval(I<int>(-3), I<int>(-1))),
		trunc_div(Range(Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(-3), I<int>(-2)))));
	// (-2, -1) / (1, 3)
	EXPECT_EQ(Range(Interval(I<int>(-2), I<int>(0))),
		trunc_div(Range(Interval(I<int>(-2), I<int>(-1))),
			Range(Interval(I<int>(1), I<int>(3)))));
	// (-2, -1) / (-3, -1)
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(2))),
		trunc_div(Range(Interval(I<int>(-2), I<int>(-1))),
			Range(Interval(I<int>(-3), I<int>(-1)))));
	// (0, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(3))),
		trunc_div(Range(Interval(I<int>(0), I<int>(3))),
			Range(Interval(I<int>(1), I<int>(4)))));
	// (-2, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<int>(-2), I<int>(3))),
		trunc_div(Range(Interval(I<int>(-2), I<int>(3))),
			Range(Interval(I<int>(1), I<int>(4)))));
	// (INT_MIN, INT_MIN) / (1, 1)
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
		trunc_div(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
			Range(Interval(I<int>(1), I<int>(1)))));
	// (-2, -1)(4, 6) / (-3, -1)(1, 3)
	EXPECT_EQ(Range(Interval(I<int>(-6), I<int>(6))),
		trunc_div(Range(Interval(I<int>(-2), I<int>(-1)), Interval(I<int>(4), I<int>(6))),
			Range(Interval(I<int>(-3), I<int>(-1)), Interval(I<int>(1), I<int>(3)))));
}

TEST_F(RangeTest,
TruncDivWorksCorrectlyWhenDividingUnsignedIntWithUnsignedInt)
{
	// (4, 6) / (2, 3)
	EXPECT_EQ(Range(Interval(I<unsigned>(1), I<unsigned>(3))),
		trunc_div(Range(Interval(I<unsigned>(4), I<unsigned>(6))),
			Range(Interval(I<unsigned>(2), I<unsigned>(3)))));
	// (0, 3) / (1, 4)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(3))),
		trunc_div(Range(Interval(I<unsigned>(0), I<unsigned>(3))),
			Range(Interval(I<unsigned>(1), I<unsigned>(4)))));
}

TEST_F(RangeTest,
TruncDivWorksCorrectlyWhenDividingSignedIntWithUnsignedInt)
{
	// (-1, -1) / (1, 4)
	EXPECT_EQ(Range(Interval(I<unsigned>(vmax<unsigned>() / 4), I<unsigned>(vmax<unsigned>()))),
		trunc_div(Range(Interval(I<int>(-1), I<int>(-1))),
			Range(Interval(I<unsigned>(1), I<unsigned>(4)))));
}

////////////////////////////////////////////////////////////////////////////////
// trunc_mod()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
TruncModModuloByZeroResultsIntoOverApproximation)
{
	// x % 0 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_mod(Range(Interval(I<int>(1), I<int>(5))), Range(I<int>(0))));
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_mod(Range(Interval(I<int>(1), I<int>(5))),
			Range(Interval(I<int>(-5), I<int>(5)))));
}

TEST_F(RangeTest,
TruncModModuloIntMinByMinusOneResultsIntoOverApproximation)
{
	// INT_MIN % -1 -> undefined behaviour in C
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_mod(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>()))),
			Range(Interval(I<int>(-1), I<int>(-1)))));
}

TEST_F(RangeTest,
TruncModWorksCorrectlyWhenModuloSignedIntWithSignedInt)
{
	// (-5, -5) / (-5, -1)
	//
	// Precise result: (-3, 0)
	EXPECT_EQ(Range(Interval(I<int>(-5), I<int>(0))),
		trunc_mod(Range(Interval(I<int>(-5), I<int>(-5))),
			Range(Interval(I<int>(-5), I<int>(-1)))));

	// (-10, -8)(-3, 2) / (1, 5)
	//
	// Precise result: (-4, 2)
	EXPECT_EQ(Range(Interval(I<int>(-10), I<int>(2))),
		trunc_mod(Range(Interval(I<int>(-10), I<int>(-8)),
				Interval(I<int>(-3), I<int>(2))),
			Range(Interval(I<int>(1), I<int>(5)))));

	// (INT_MIN, INT_MAX) / (1, 1)
	//
	// Precise result: (0, 0)
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		trunc_mod(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
			Range(Interval(I<int>(1), I<int>(1)))));
}

TEST_F(RangeTest,
TruncModWorksCorrectlyWhenModuloUnsignedIntWithUnsignedInt)
{
	// (5, 5) / (1, 5)
	//
	// Precise result: (0, 3)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(5))),
		trunc_mod(Range(Interval(I<unsigned>(5), I<unsigned>(5))),
			Range(Interval(I<unsigned>(1), I<unsigned>(5)))));
}

TEST_F(RangeTest,
TruncModWorksCorrectlyWhenModuloSignedIntWithUnsignedInt)
{
	// (-2, -1) % (1, 4)
	//
	// Precise result: (0, 3)
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(vmax<unsigned>()))),
		trunc_mod(Range(Interval(I<int>(-2), I<int>(-1))),
			Range(Interval(I<unsigned>(1), I<unsigned>(4)))));
}

////////////////////////////////////////////////////////////////////////////////
// bitNot()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitNotReturnsCorrectResultWhenRangeContainsSingleNumber)
{
	EXPECT_EQ(Range(I<signed char>(-2)),
		bitNot(Range(I<signed char>(1))));
	EXPECT_EQ(Range(I<int>(-2)),
		bitNot(Range(I<int>(1))));
	EXPECT_EQ(Range(I<unsigned>(vmax<unsigned>() - 1)),
		bitNot(Range(I<unsigned>(1))));
}

TEST_F(RangeTest,
BitNotReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitNot(Range(Interval(I<signed char>(1), I<signed char>(10)))));
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitNot(Range(Interval(I<int>(1), I<int>(10)))));
}

////////////////////////////////////////////////////////////////////////////////
// bitAnd()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitAndReturnsCorrectResultWhenBothRangesContainSingleNumber)
{
	EXPECT_EQ(Range(I<int>(1)),
		bitAnd(Range(I<signed char>(1)), Range(I<signed char>(3))));
	EXPECT_EQ(Range(I<int>(3)),
		bitAnd(Range(I<int>(7)), Range(I<int>(3))));
}

TEST_F(RangeTest,
BitAndReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitAnd(Range(Interval(I<int>(1), I<int>(10))),
			Range(Interval(I<int>(1), I<int>(10)))));
}

////////////////////////////////////////////////////////////////////////////////
// Bit Or
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitOrReturnsCorrectResultWhenBothRangesContainSingleNumber)
{
	EXPECT_EQ(Range(I<int>(3)),
		bitOr(Range(I<signed char>(1)), Range(I<signed char>(2))));
	EXPECT_EQ(Range(I<int>(7)),
		bitOr(Range(I<int>(7)), Range(I<int>(3))));
}

TEST_F(RangeTest,
BitOrReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitOr(Range(Interval(I<int>(1), I<int>(10))),
			Range(Interval(I<int>(1), I<int>(10)))));
}

////////////////////////////////////////////////////////////////////////////////
// bitXor()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitXorReturnsCorrectResultWhenBothRangesContainSingleNumber)
{
	EXPECT_EQ(Range(I<int>(3)),
		bitXor(Range(I<signed char>(1)), Range(I<signed char>(2))));
	EXPECT_EQ(Range(I<int>(4)),
		bitXor(Range(I<int>(7)), Range(I<int>(3))));
}

TEST_F(RangeTest,
BitXorReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitXor(Range(Interval(I<int>(1), I<int>(10))),
			Range(Interval(I<int>(1), I<int>(10)))));
}

////////////////////////////////////////////////////////////////////////////////
// bitLeftShift()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitLeftShiftReturnsCorrectResultWhenBothRangesContainSingleNumber)
{
	EXPECT_EQ(Range(I<int>(4)),
		bitLeftShift(Range(I<signed char>(2)), Range(I<signed char>(1))));
	EXPECT_EQ(Range(I<int>(16)),
		bitLeftShift(Range(I<int>(2)), Range(I<int>(3))));
}

TEST_F(RangeTest,
BitLeftShiftReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitLeftShift(Range(Interval(I<int>(1), I<int>(10))),
			Range(Interval(I<int>(1), I<int>(10)))));
}

TEST_F(RangeTest,
BitLeftShiftReturnsOverApproximationWhenRhsIsMoreThanBitWidthOfLhs)
{
	// If we are shifting more bits than the bit width of the left hand side,
	// like in `a << 33`, where `a` is a 32b integer, the result is undefined.
	// Therefore, we have to over approximate.
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitLeftShift(Range(I<int>(1)), Range(I<int>(sizeof(int) * 8 + 1))));
}

////////////////////////////////////////////////////////////////////////////////
// bitRightShift()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
BitRightShiftReturnsCorrectResultWhenBothRangesContainSingleNumber)
{
	EXPECT_EQ(Range(I<int>(8)),
		bitRightShift(Range(I<signed char>(16)), Range(I<signed char>(1))));
	EXPECT_EQ(Range(I<int>(2)),
		bitRightShift(Range(I<int>(4)), Range(I<int>(1))));
}

TEST_F(RangeTest,
BitRightShiftReturnsOverApproximationWhenRangeContainsMoreThanSingleNumber)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitRightShift(Range(Interval(I<int>(1), I<int>(10))),
			Range(Interval(I<int>(1), I<int>(10)))));
}

TEST_F(RangeTest,
BitRightShiftReturnsOverApproximationWhenRhsIsMoreThanBitWidthOfLhs)
{
	// If we are shifting more bits than the bit width of the left hand side,
	// like in `a >> 33`, where `a` is a 32b integer, the result is undefined.
	// Therefore, we have to over approximate.
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmax<int>()))),
		bitRightShift(Range(I<int>(1)), Range(I<int>(sizeof(int) * 8 + 1))));
}

////////////////////////////////////////////////////////////////////////////////
// overApproximateUnaryOp()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
OverApproximationOfUnaryBitOpWorksCorrectly)
{
	EXPECT_EQ(sizeof(int),
		overApproximateUnaryOp(Range(I<char>(1)))[0].first.getBitWidth());
}

////////////////////////////////////////////////////////////////////////////////
// overApproximateBinaryOp()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
OverApproximationOfBinaryBitOpWorksCorrectly)
{
	EXPECT_EQ(sizeof(int),
		overApproximateBinaryOp(Range(I<char>(1)),
			Range(I<char>(1)))[0].first.getBitWidth());
}

////////////////////////////////////////////////////////////////////////////////
// abs()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
AbsOfSignedIntegralRangeWorksCorrectly)
{
	EXPECT_EQ(Range(I<int>(vmin<int>())), abs(Range(I<int>(vmin<int>()))));
	EXPECT_EQ(Range(Interval(I<int>(5), I<int>(8))),
			  abs(Range(Interval(I<int>(-8), I<int>(-5)))));
	EXPECT_EQ(Range(Interval(I<int>(8), I<int>(15))),
			  abs(Range(Interval(I<int>(8), I<int>(15)))));
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>())),
					Interval(I<int>(0), I<int>(150)),
					Interval(I<int>(450), I<int>(600)),
					Interval(I<int>(vmax<int>()-9), I<int>(vmax<int>()))),
			  abs(Range(Interval(I<int>(vmin<int>()), I<int>(vmin<int>() + 10)),
			  			Interval(I<int>(-150), I<int>(1)),
			  			Interval(I<int>(450), I<int>(600)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(15))),
			  abs(Range(Interval(I<int>(-10), I<int>(15)))));
}

TEST_F(RangeTest,
AbsOfEmptyRangeWorksCorrectly)
{
	EXPECT_DEATH(abs(Range()), ".*!r.empty().*");
}

TEST_F(RangeTest,
AbsOfUnsignedIntegralRangeWorksCorrectly)
{
	EXPECT_DEATH(abs(Range(I<unsigned>(6))), ".*r.isIntegral().*r.isSigned().*");
}

TEST_F(RangeTest,
AbsOfFloatingPointRangeWorksCorrectly)
{
	EXPECT_DEATH(abs(Range(F<double>(-3.6))), ".*r.isIntegral().*r.isSigned().*");
}

////////////////////////////////////////////////////////////////////////////////
// min()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
MinOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  min(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(10), I<int>(15)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  min(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  min(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(4), I<int>(8)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  min(Range(Interval(I<int>(10), I<int>(15))),
			  	  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(4))),
			  min(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(1), I<int>(4)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(4))),
			  min(Range(Interval(I<int>(1), I<int>(4))),
			  	  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  min(Range(Interval(I<int>(4), I<int>(8))),
			  	  Range(Interval(I<int>(0), I<int>(5)))));
}

TEST_F(RangeTest,
MinOfSignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(0)),
					Interval(I<unsigned>(5), I<unsigned>(10))),
			  min(Range(Interval(I<int>(-1), I<int>(0))),
				  Range(Interval(I<unsigned>(5), I<unsigned>(10)))));
}

TEST_F(RangeTest,
MinOfSignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-4), F<double>(2.8))),
			  min(Range(Interval(I<int>(-4), I<int>(25))),
				  Range(Interval(F<double>(-3.2), F<double>(2.8)))));
}

TEST_F(RangeTest,
MinOfUnsignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(5)),
					Interval(I<unsigned>(vmax<unsigned>() - 10),
							 I<unsigned>(vmax<unsigned>() - 1))),
			  min(Range(Interval(I<unsigned>(vmax<unsigned>() - 10),
								 I<unsigned>(vmax<unsigned>() - 1))),
				  Range(Interval(I<signed>(-5), I<signed>(5)))));
}

TEST_F(RangeTest,
MinOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(0), I<unsigned>(10)),
					Interval(I<unsigned>(15), I<unsigned>(50))),
			  min(Range(Interval(I<unsigned>(0), I<unsigned>(10)),
			  			Interval(I<unsigned>(20), I<unsigned>(30)),
			  			Interval(I<unsigned>(40), I<unsigned>(50))),
			  	  Range(Interval(I<unsigned>(15), I<unsigned>(25)),
			  	  		Interval(I<unsigned>(28), I<unsigned>(42)),
			  	  		Interval(I<unsigned>(60), I<unsigned>(70)))));
}

TEST_F(RangeTest,
MinOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(1), F<double>(4)),
					Interval(F<double>(8.9), F<double>(9.3)),
					Interval(F<double>(10), F<double>(40)),
					Interval(F<double>(49.8), F<double>(58.3))),
			  min(Range(Interval(I<unsigned>(1), I<unsigned>(4)),
			  			Interval(I<unsigned>(10), I<unsigned>(40)),
			  			Interval(I<unsigned>(100), I<unsigned>(400))),
				  Range(Interval(F<double>(8.9), F<double>(9.3)),
				  		Interval(F<double>(49.8), F<double>(58.3)))));
}

TEST_F(RangeTest,
MinOfFloatAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-4.2), F<double>(1.0))),
			  min(Range(Interval(F<double>(-4.2), F<double>(28.9))),
				  Range(Interval(I<int>(-1), I<int>(1)))));
}

TEST_F(RangeTest,
MinOfFloatAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(8.9), F<double>(9.3)),
					Interval(F<double>(12), F<double>(50))),
			  min(Range(Interval(F<double>(8.9), F<double>(9.3)),
			  			Interval(F<double>(49.8), F<double>(58.3))),
			  	  Range(Interval(I<int>(12), I<int>(50)))));
}

TEST_F(RangeTest,
MinOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-45.3), F<double>(1.0)),
					Interval(F<double>(1.5), F<double>(4.2)),
					Interval(F<double>(4.8), F<double>(5.9))),
			  min(Range(Interval(F<double>(-45.3), F<double>(1.0)),
			  			Interval(F<double>(2.0), F<double>(3.3)),
			  			Interval(F<double>(4.8), F<double>(5.9))),
			  	  Range(Interval(F<double>(1.5), F<double>(2.5)),
			  	  		Interval(F<double>(2.8), F<double>(4.2)),
			  	  		Interval(F<double>(6.0), F<double>(7.0)))));
}

TEST_F(RangeTest,
MinOfSpecialFloatsWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
			  min(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
			  	  Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
			  min(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
			  	  Range(Interval(F<float>(INFINITY), F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(5.8))),
			  min(Range(Interval(F<float>(-INFINITY), F<float>(INFINITY))),
			  	  Range(Interval(F<float>(-4.3), F<float>(5.8)))));
}

TEST_F(RangeTest,
MinOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(min(Range(), Range()), ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// max()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
MaxOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(10), I<int>(15))),
			  max(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(10), I<int>(15)))));
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(5))),
			  max(Range(Interval(I<int>(0), I<int>(5))),
			  	  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(4), I<int>(8))),
			  max(Range(Interval(I<int>(0), I<int>(5))),
				  Range(Interval(I<int>(4), I<int>(8)))));
	EXPECT_EQ(Range(Interval(I<int>(10), I<int>(15))),
			  max(Range(Interval(I<int>(10), I<int>(15))),
				  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(5))),
			  max(Range(Interval(I<int>(0), I<int>(5))),
				  Range(Interval(I<int>(1), I<int>(4)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(5))),
			  max(Range(Interval(I<int>(1), I<int>(4))),
				  Range(Interval(I<int>(0), I<int>(5)))));
	EXPECT_EQ(Range(Interval(I<int>(4), I<int>(8))),
			  max(Range(Interval(I<int>(4), I<int>(8))),
				  Range(Interval(I<int>(0), I<int>(5)))));
}

TEST_F(RangeTest,
MaxOfSignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmax<unsigned>()),
							 I<unsigned>(vmax<unsigned>())),
					Interval(I<unsigned>(5), I<unsigned>(10))),
			  max(Range(Interval(I<int>(-1), I<int>(0))),
				  Range(Interval(I<unsigned>(5), I<unsigned>(10)))));
}

TEST_F(RangeTest,
MaxOfSignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-3.2), F<double>(25))),
			  max(Range(Interval(I<int>(-4), I<int>(25))),
				  Range(Interval(F<double>(-3.2), F<double>(2.8)))));
}

TEST_F(RangeTest,
MaxOfUnsignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(vmax<unsigned>() - 10),
							 I<unsigned>(vmax<unsigned>()))),
			  max(Range(Interval(I<unsigned>(vmax<unsigned>() - 10),
								 I<unsigned>(vmax<unsigned>() - 1))),
				  Range(Interval(I<signed>(-5), I<signed>(5)))));
}

TEST_F(RangeTest,
MaxOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(60), I<unsigned>(70)),
					Interval(I<unsigned>(15), I<unsigned>(50))),
			  max(Range(Interval(I<unsigned>(0), I<unsigned>(10)),
			  			Interval(I<unsigned>(20), I<unsigned>(30)),
			  			Interval(I<unsigned>(40), I<unsigned>(50))),
			  	  Range(Interval(I<unsigned>(15), I<unsigned>(25)),
			  	  		Interval(I<unsigned>(28), I<unsigned>(42)),
			  	  		Interval(I<unsigned>(60), I<unsigned>(70)))));
}

TEST_F(RangeTest,
MaxOfUnsignedIntAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(100), F<double>(400)),
					Interval(F<double>(8.9), F<double>(9.3)),
					Interval(F<double>(10), F<double>(40)),
					Interval(F<double>(49.8), F<double>(58.3))),
			  max(Range(Interval(I<unsigned>(1), I<unsigned>(4)),
			  			Interval(I<unsigned>(10), I<unsigned>(40)),
			  			Interval(I<unsigned>(100), I<unsigned>(400))),
				  Range(Interval(F<double>(8.9), F<double>(9.3)),
				  		Interval(F<double>(49.8), F<double>(58.3)))));
}

TEST_F(RangeTest,
MaxOfFloatAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(-1), F<double>(28.9))),
			  max(Range(Interval(F<double>(-4.2), F<double>(28.9))),
				  Range(Interval(I<int>(-1), I<int>(1)))));
}

TEST_F(RangeTest,
MaxOfFloatAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(49.8), F<double>(58.3)),
					Interval(F<double>(12), F<double>(50))),
			  max(Range(Interval(F<double>(8.9), F<double>(9.3)),
			  			Interval(F<double>(49.8), F<double>(58.3))),
			  	  Range(Interval(I<int>(12), I<int>(50)))));
}

TEST_F(RangeTest,
MaxOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(1.5), F<double>(4.2)),
					Interval(F<double>(4.8), F<double>(7.0))),
			  max(Range(Interval(F<double>(-45.3), F<double>(1.0)),
			  			Interval(F<double>(2.0), F<double>(3.3)),
			  			Interval(F<double>(4.8), F<double>(5.9))),
			  	  Range(Interval(F<double>(1.5), F<double>(2.5)),
			  	  		Interval(F<double>(2.8), F<double>(4.2)),
			  	  		Interval(F<double>(6.0), F<double>(7.0)))));
}

TEST_F(RangeTest,
MaxOfSpecialFloatsWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
			  max(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
			  	  Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
			  max(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
			  	  Range(Interval(F<float>(INFINITY), F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-4.3), F<float>(INFINITY))),
			  max(Range(Interval(F<float>(-INFINITY), F<float>(INFINITY))),
			  	  Range(Interval(F<float>(-4.3), F<float>(5.8)))));
}

TEST_F(RangeTest,
MaxOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(max(Range(), Range()), ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// unite()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
UniteOfSignedCharAndSignedCharWorksCorrectly)
{
	EXPECT_EQ(Range(),
			  unite(Range(), Range()));

	EXPECT_EQ(Range(Number(vmin<signed char>(), sizeof(char), true)),
			  unite(Range(Number(vmin<signed char>(), sizeof(char), true)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<signed char>(), sizeof(char), true)),
			  unite(Range(),
					Range(Number(vmax<signed char>(), sizeof(char), true))));

	EXPECT_EQ(Range(Interval(I<signed char>(-120), I<signed char>(-1)),
					Interval(I<signed char>(120), I<signed char>(125))),
			  unite(Range(Interval(I<signed char>(-120), I<signed char>(-1))),
					Range(Interval(I<signed char>(120), I<signed char>(125)))));

	EXPECT_EQ(Range(Interval(I<signed char>(-120), I<signed char>(-115)),
					Interval(I<signed char>(-100), I<signed char>(-70)),
					Interval(I<signed char>(-60), I<signed char>(-50)),
					Interval(I<signed char>(20), I<signed char>(30))),
			  unite(Range(Interval(I<signed char>(-120), I<signed char>(-118)),
						  Interval(I<signed char>(-100), I<signed char>(-80)),
						  Interval(I<signed char>(-60), I<signed char>(-50))),
					Range(Interval(I<signed char>(-117), I<signed char>(-115)),
						  Interval(I<signed char>(-90), I<signed char>(-70)),
						  Interval(I<signed char>(-58), I<signed char>(-56)),
						  Interval(I<signed char>(20), I<signed char>(30)))));
}

TEST_F(RangeTest,
UniteOfUnsignedCharAndUnsignedCharWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<unsigned char>(), sizeof(char), false)),
			  unite(Range(Number(vmin<unsigned char>(), sizeof(char), false)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<unsigned char>(), sizeof(char), false)),
			  unite(Range(),
					Range(Number(vmax<unsigned char>(), sizeof(char), false))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(2)),
					Interval(I<unsigned char>(120), I<unsigned char>(125))),
			  unite(Range(Interval(I<unsigned char>(0), I<unsigned char>(2))),
					Range(Interval(I<unsigned char>(120), I<unsigned char>(125)))));

	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(8)),
					Interval(I<unsigned char>(10), I<unsigned char>(30)),
					Interval(I<unsigned char>(80), I<unsigned char>(90)),
					Interval(I<unsigned char>(200), I<unsigned char>(255))),
			  unite(Range(Interval(I<unsigned char>(0), I<unsigned char>(5)),
						  Interval(I<unsigned char>(10), I<unsigned char>(20)),
						  Interval(I<unsigned char>(80), I<unsigned char>(90))),
					Range(Interval(I<unsigned char>(6), I<unsigned char>(8)),
						  Interval(I<unsigned char>(15), I<unsigned char>(30)),
						  Interval(I<unsigned char>(85), I<unsigned char>(86)),
						  Interval(I<unsigned char>(200), I<unsigned char>(255)))));
}

TEST_F(RangeTest,
UniteOfSignedShortAndSignedShortWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<signed short>(), sizeof(short), true)),
			  unite(Range(Number(vmin<signed short>(), sizeof(short), true)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<signed short>(), sizeof(short), true)),
			  unite(Range(),
					Range(Number(vmax<signed short>(), sizeof(short), true))));

	EXPECT_EQ(Range(Interval(I<signed short>(-120), I<signed short>(-1)),
					Interval(I<signed short>(120), I<signed short>(125))),
			  unite(Range(Interval(I<signed short>(-120), I<signed short>(-1))),
					Range(Interval(I<signed short>(120), I<signed short>(125)))));

	EXPECT_EQ(Range(Interval(I<signed short>(-1200), I<signed short>(-1150)),
					Interval(I<signed short>(-100), I<signed short>(-70)),
					Interval(I<signed short>(60), I<signed short>(70)),
					Interval(I<signed short>(20), I<signed short>(30))),
			  unite(Range(Interval(I<signed short>(-1200), I<signed short>(-1180)),
						  Interval(I<signed short>(-100), I<signed short>(-80)),
						  Interval(I<signed short>(60), I<signed short>(70))),
					Range(Interval(I<signed short>(-1179), I<signed short>(-1150)),
						  Interval(I<signed short>(-90), I<signed short>(-70)),
						  Interval(I<signed short>(68), I<signed short>(69)),
						  Interval(I<signed short>(20), I<signed short>(30)))));
}

TEST_F(RangeTest,
UniteOfUnsignedShortAndUnsignedShortWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<unsigned short>(), sizeof(short), false)),
			  unite(Range(Number(vmin<unsigned short>(), sizeof(short), false)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<unsigned short>(), sizeof(short), false)),
			  unite(Range(),
					Range(Number(vmax<unsigned short>(), sizeof(short), false))));

	EXPECT_EQ(Range(Interval(I<unsigned short>(0), I<unsigned short>(2)),
					Interval(I<unsigned short>(120), I<unsigned short>(125))),
			  unite(Range(Interval(I<unsigned short>(0), I<unsigned short>(2))),
					Range(Interval(I<unsigned short>(120),
								   I<unsigned short>(125)))));

	EXPECT_EQ(Range(Interval(I<unsigned short>(0), I<unsigned short>(8)),
					Interval(I<unsigned short>(10), I<unsigned short>(30)),
					Interval(I<unsigned short>(80), I<unsigned short>(90)),
					Interval(I<unsigned short>(200),
							 I<unsigned short>(vmax<unsigned short>()))),
			  unite(Range(Interval(I<unsigned short>(0), I<unsigned short>(5)),
						  Interval(I<unsigned short>(10), I<unsigned short>(20)),
						  Interval(I<unsigned short>(80), I<unsigned short>(90))),
					Range(Interval(I<unsigned short>(6), I<unsigned short>(8)),
						  Interval(I<unsigned short>(15), I<unsigned short>(30)),
						  Interval(I<unsigned short>(85), I<unsigned short>(86)),
						  Interval(I<unsigned short>(200),
								   I<unsigned short>(vmax<unsigned short>())))));
}

TEST_F(RangeTest,
UniteOfSignedIntAndSignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<signed int>(), sizeof(int), true)),
			  unite(Range(Number(vmin<signed int>(), sizeof(int), true)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<signed int>(), sizeof(int), true)),
			  unite(Range(),
					Range(Number(vmax<signed int>(), sizeof(int), true))));

	EXPECT_EQ(Range(Interval(I<signed int>(-120), I<signed int>(-1)),
					Interval(I<signed int>(120), I<signed int>(125))),
			  unite(Range(Interval(I<signed int>(-120), I<signed int>(-1))),
					Range(Interval(I<signed int>(120), I<signed int>(125)))));

	EXPECT_EQ(Range(Interval(I<signed int>(-1200), I<signed int>(-1150)),
					Interval(I<signed int>(-100), I<signed int>(-70)),
					Interval(I<signed int>(60), I<signed int>(70)),
					Interval(I<signed int>(20), I<signed int>(30))),
			  unite(Range(Interval(I<signed int>(-1200), I<signed int>(-1180)),
						  Interval(I<signed int>(-100), I<signed int>(-80)),
						  Interval(I<signed int>(60), I<signed int>(70))),
					Range(Interval(I<signed int>(-1179), I<signed int>(-1150)),
						  Interval(I<signed int>(-90), I<signed int>(-70)),
						  Interval(I<signed int>(68), I<signed int>(69)),
						  Interval(I<signed int>(20), I<signed int>(30)))));
}

TEST_F(RangeTest,
UniteOfUnsignedIntAndUnsignedIntWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<unsigned int>(), sizeof(int), false)),
			  unite(Range(Number(vmin<unsigned int>(), sizeof(int), false)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<unsigned int>(), sizeof(int), false)),
			  unite(Range(),
					Range(Number(vmax<unsigned int>(), sizeof(int), false))));

	EXPECT_EQ(Range(Interval(I<unsigned int>(0), I<unsigned int>(2)),
					Interval(I<unsigned int>(120), I<unsigned int>(125))),
			  unite(Range(Interval(I<unsigned int>(0), I<unsigned int>(2))),
					Range(Interval(I<unsigned int>(120),
								   I<unsigned int>(125)))));

	EXPECT_EQ(Range(Interval(I<unsigned int>(0), I<unsigned int>(8)),
					Interval(I<unsigned int>(10), I<unsigned int>(30)),
					Interval(I<unsigned int>(80), I<unsigned int>(90)),
					Interval(I<unsigned int>(200),
							 I<unsigned int>(vmax<unsigned int>()))),
			  unite(Range(Interval(I<unsigned int>(0), I<unsigned int>(5)),
						  Interval(I<unsigned int>(10), I<unsigned int>(20)),
						  Interval(I<unsigned int>(80), I<unsigned int>(90))),
					Range(Interval(I<unsigned int>(6), I<unsigned int>(8)),
						  Interval(I<unsigned int>(15), I<unsigned int>(30)),
						  Interval(I<unsigned int>(85), I<unsigned int>(86)),
						  Interval(I<unsigned int>(200),
								   I<unsigned int>(vmax<unsigned int>())))));
}

TEST_F(RangeTest,
UniteOfSignedLongAndSignedLongWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<signed long>(), sizeof(long), true)),
			  unite(Range(Number(vmin<signed long>(), sizeof(long), true)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<signed long>(), sizeof(long), true)),
			  unite(Range(),
					Range(Number(vmax<signed long>(), sizeof(long), true))));

	EXPECT_EQ(Range(Interval(I<signed long>(-120), I<signed long>(-1)),
					Interval(I<signed long>(120), I<signed long>(125))),
			  unite(Range(Interval(I<signed long>(-120), I<signed long>(-1))),
					Range(Interval(I<signed long>(120), I<signed long>(125)))));

	EXPECT_EQ(Range(Interval(I<signed long>(-1200), I<signed long>(-1150)),
					Interval(I<signed long>(-100), I<signed long>(-70)),
					Interval(I<signed long>(60), I<signed long>(70)),
					Interval(I<signed long>(20), I<signed long>(30))),
			  unite(Range(Interval(I<signed long>(-1200), I<signed long>(-1180)),
						  Interval(I<signed long>(-100), I<signed long>(-80)),
					      Interval(I<signed long>(60), I<signed long>(70))),
					Range(Interval(I<signed long>(-1179), I<signed long>(-1150)),
						  Interval(I<signed long>(-90), I<signed long>(-70)),
						  Interval(I<signed long>(68), I<signed long>(69)),
						  Interval(I<signed long>(20), I<signed long>(30)))));
}

TEST_F(RangeTest,
UniteOfUnsignedLongAndUnsignedLongWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<unsigned long>(), sizeof(long), true)),
			  unite(Range(Number(vmin<unsigned long>(), sizeof(long), true)),
					Range()));

	EXPECT_EQ(Range(Number(vmax<unsigned long>(), sizeof(long), true)),
			  unite(Range(),
					Range(Number(vmax<unsigned long>(), sizeof(long), true))));

	EXPECT_EQ(Range(Interval(I<unsigned long>(-120), I<unsigned long>(-1)),
					Interval(I<unsigned long>(120), I<unsigned long>(125))),
			  unite(Range(Interval(I<unsigned long>(-120), I<unsigned long>(-1))),
					Range(Interval(I<unsigned long>(120), I<unsigned long>(125)))));

	EXPECT_EQ(Range(Interval(I<unsigned long>(-1200), I<unsigned long>(-1150)),
					Interval(I<unsigned long>(-100), I<unsigned long>(-70)),
					Interval(I<unsigned long>(60), I<unsigned long>(70)),
					Interval(I<unsigned long>(20), I<unsigned long>(30))),
			  unite(Range(Interval(I<unsigned long>(-1200),
								   I<unsigned long>(-1180)),
						  Interval(I<unsigned long>(-100), I<unsigned long>(-80)),
						  Interval(I<unsigned long>(60), I<unsigned long>(70))),
					Range(Interval(I<unsigned long>(-1179),
								   I<unsigned long>(-1150)),
						  Interval(I<unsigned long>(-90), I<unsigned long>(-70)),
						  Interval(I<unsigned long>(68), I<unsigned long>(69)),
						  Interval(I<unsigned long>(20), I<unsigned long>(30)))));
}

TEST_F(RangeTest,
UniteOfFloatAndFloatWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<float>(), sizeof(float))),
			  unite(Range(Number(vmin<float>(), sizeof(float))),
					Range()));

	EXPECT_EQ(Range(Number(vmax<float>(), sizeof(float))),
			  unite(Range(),
					Range(Number(vmax<float>(), sizeof(float)))));

	EXPECT_EQ(Range(Interval(F<float>(-120.33), F<float>(-1.35)),
					Interval(F<float>(120.36), F<float>(125.67))),
			  unite(Range(Interval(F<float>(-120.33), F<float>(-1.35))),
					Range(Interval(F<float>(120.36), F<float>(125.67)))));

	EXPECT_EQ(Range(Interval(F<float>(-1200.35), F<float>(-1150.43)),
					Interval(F<float>(-100.23), F<float>(-70.5)),
					Interval(F<float>(60.36), F<float>(70.95)),
					Interval(F<float>(20.8), F<float>(30.4))),
			  unite(Range(Interval(F<float>(-1200.35),F<float>(-1180.0)),
						  Interval(F<float>(-100.23), F<float>(-80.58)),
						  Interval(F<float>(60.36), F<float>(70.95))),
					Range(Interval(F<float>(-1179.9), F<float>(-1150.43)),
						  Interval(F<float>(-90.45), F<float>(-70.5)),
						  Interval(F<float>(68.3), F<float>(69.0)),
						  Interval(F<float>(20.8), F<float>(30.4)))));
}

TEST_F(RangeTest,
UniteOfDoubleAndDoubleWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<double>(), sizeof(double))),
			  unite(Range(Number(vmin<double>(), sizeof(double))),
					Range()));

	EXPECT_EQ(Range(Number(vmax<double>(), sizeof(double))),
			  unite(Range(),
					Range(Number(vmax<double>(), sizeof(double)))));

	EXPECT_EQ(Range(Interval(F<double>(-120.33), F<double>(-1.35)),
					Interval(F<double>(120.36), F<double>(125.67))),
			  unite(Range(Interval(F<double>(-120.33), F<double>(-1.35))),
					Range(Interval(F<double>(120.36), F<double>(125.67)))));

	EXPECT_EQ(Range(Interval(F<double>(-1200.35), F<double>(-1150.43)),
					Interval(F<double>(-100.23), F<double>(-70.5)),
					Interval(F<double>(60.36), F<double>(70.95)),
					Interval(F<double>(20.8), F<double>(30.4))),
			  unite(Range(Interval(F<double>(-1200.35),F<double>(-1180.0)),
						  Interval(F<double>(-100.23), F<double>(-80.58)),
						  Interval(F<double>(60.36), F<double>(70.95))),
					Range(Interval(F<double>(-1179.9), F<double>(-1150.43)),
						  Interval(F<double>(-90.45), F<double>(-70.5)),
						  Interval(F<double>(68.3), F<double>(69.0)),
						  Interval(F<double>(20.8), F<double>(30.4)))));
}

TEST_F(RangeTest,
UniteOfLongDoubleAndLongDoubleWorksCorrectly)
{
	EXPECT_EQ(Range(Number(vmin<long double>(), sizeof(long double))),
			  unite(Range(Number(vmin<long double>(), sizeof(long double))),
					Range()));

	EXPECT_EQ(Range(Number(vmax<long double>(), sizeof(long double))),
			  unite(Range(),
					Range(Number(vmax<long double>(), sizeof(long double)))));

	EXPECT_EQ(Range(Interval(F<long double>(-120.33), F<long double>(-1.35)),
					Interval(F<long double>(120.36), F<long double>(125.67))),
			  unite(Range(Interval(F<long double>(-120.33), F<long double>(-1.35))),
					Range(Interval(F<long double>(120.36), F<long double>(125.67)))));

	EXPECT_EQ(Range(Interval(F<long double>(-1200.35), F<long double>(-1150.43)),
					Interval(F<long double>(-100.23), F<long double>(-70.5)),
					Interval(F<long double>(60.36), F<long double>(70.95)),
					Interval(F<long double>(20.8), F<long double>(30.4))),
			  unite(Range(Interval(F<long double>(-1200.35),F<long double>(-1180.0)),
						  Interval(F<long double>(-100.23), F<long double>(-80.58)),
						  Interval(F<long double>(60.36), F<long double>(70.95))),
					Range(Interval(F<long double>(-1179.9), F<long double>(-1150.43)),
						  Interval(F<long double>(-90.45), F<long double>(-70.5)),
						  Interval(F<long double>(68.3), F<long double>(69.0)),
						  Interval(F<long double>(20.8), F<long double>(30.4)))));
}

////////////////////////////////////////////////////////////////////////////////
// intersection()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
IntersectOfSignedIntRangeAndSignedIntRangeWorksCorrectly)
{
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<int>(15), I<int>(20))),
			            Range(Interval(I<int>(115), I<int>(220)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<int>(-220), I<int>(-115))),
			            Range(Interval(I<int>(-20), I<int>(-15)))));
	EXPECT_EQ(Range(),
			  intersect(Range(),
			            Range(Interval(I<int>(115), I<int>(220)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<int>(-20), I<int>(-15))),
			            Range()));

	EXPECT_EQ(Range(Interval(I<int>(22), I<int>(25))),
			  intersect(Range(Interval(I<int>(15), I<int>(25))),
			            Range(Interval(I<int>(22), I<int>(220)))));

	EXPECT_EQ(Range(Interval(I<int>(-25), I<int>(-22))),
			  intersect(Range(Interval(I<int>(-220), I<int>(-22))),
			            Range(Interval(I<int>(-25), I<int>(-15)))));

	EXPECT_EQ(Range(Interval(I<int>(20), I<int>(20))),
			  intersect(Range(Interval(I<int>(15), I<int>(20))),
			            Range(Interval(I<int>(20), I<int>(220)))));

	EXPECT_EQ(Range(Interval(I<int>(-20), I<int>(-20))),
			  intersect(Range(Interval(I<int>(-220), I<int>(-20))),
			            Range(Interval(I<int>(-20), I<int>(-15)))));

	EXPECT_EQ(Range(Interval(I<int>(15), I<int>(20))),
			  intersect(Range(Interval(I<int>(15), I<int>(20))),
			            Range(Interval(I<int>(10), I<int>(vmax<int>())))));

	EXPECT_EQ(Range(Interval(I<int>(-20), I<int>(-15))),
			  intersect(Range(Interval(I<int>(vmin<int>()), I<int>(-10))),
			            Range(Interval(I<int>(-20), I<int>(-15)))));

	EXPECT_EQ(Range(Interval(I<int>(15), I<int>(16)),
					Interval(I<int>(18), I<int>(20)),
					Interval(I<int>(25), I<int>(27))),
			  intersect(Range(Interval(I<int>(10), I<int>(16)),
			  				  Interval(I<int>(18), I<int>(20)),
			  				  Interval(I<int>(25), I<int>(30))),
			            Range(Interval(I<int>(15), I<int>(27)))));

	EXPECT_EQ(Range(Interval(I<int>(-16), I<int>(-15)),
					Interval(I<int>(-20), I<int>(-18)),
					Interval(I<int>(-27), I<int>(-25))),
			  intersect(Range(Interval(I<int>(-27), I<int>(-15))),
			 			Range(Interval(I<int>(-16), I<int>(-10)),
			  				  Interval(I<int>(-20), I<int>(-18)),
			  				  Interval(I<int>(-30), I<int>(-25)))));
}

TEST_F(RangeTest,
IntersectOfUnsignedIntRangeAndUnsignedIntRangeWorksCorrectly)
{
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<unsigned>(vmin<unsigned>()),
			  						   I<unsigned>(20))),
			            Range(Interval(I<unsigned>(115), I<unsigned>(220)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<unsigned>(115),
			  						   I<unsigned>(vmax<unsigned>()))),
			            Range(Interval(I<unsigned>(15), I<unsigned>(20)))));
	EXPECT_EQ(Range(),
			  intersect(Range(),
			            Range(Interval(I<unsigned>(115), I<unsigned>(220)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(I<unsigned>(15), I<unsigned>(20))),
			            Range()));

	EXPECT_EQ(Range(Interval(I<unsigned>(22), I<unsigned>(25))),
			  intersect(Range(Interval(I<unsigned>(15), I<unsigned>(25))),
			            Range(Interval(I<unsigned>(22), I<unsigned>(220)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(22), I<unsigned>(25))),
			  intersect(Range(Interval(I<unsigned>(22), I<unsigned>(220))),
			            Range(Interval(I<unsigned>(15), I<unsigned>(25)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(20), I<unsigned>(20))),
			  intersect(Range(Interval(I<unsigned>(15), I<unsigned>(20))),
			            Range(Interval(I<unsigned>(20), I<unsigned>(220)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(20), I<unsigned>(20))),
			  intersect(Range(Interval(I<unsigned>(20), I<unsigned>(220))),
			            Range(Interval(I<unsigned>(15), I<unsigned>(20)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(15), I<unsigned>(20))),
			  intersect(Range(Interval(I<unsigned>(15), I<unsigned>(20))),
			            Range(Interval(I<unsigned>(10),
			            			   I<unsigned>(vmax<unsigned>())))));

	EXPECT_EQ(Range(Interval(I<unsigned>(15), I<unsigned>(20))),
			  intersect(Range(Interval(I<unsigned>(10),
			  						   I<unsigned>(vmax<unsigned>()))),
			            Range(Interval(I<unsigned>(15), I<unsigned>(20)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(15), I<unsigned>(16)),
					Interval(I<unsigned>(18), I<unsigned>(20)),
					Interval(I<unsigned>(25), I<unsigned>(27))),
			  intersect(Range(Interval(I<unsigned>(10), I<unsigned>(16)),
			  				  Interval(I<unsigned>(18), I<unsigned>(20)),
			  				  Interval(I<unsigned>(25), I<unsigned>(30))),
			            Range(Interval(I<unsigned>(15), I<unsigned>(27)))));

	EXPECT_EQ(Range(Interval(I<unsigned>(15), I<unsigned>(16)),
					Interval(I<unsigned>(18), I<unsigned>(20)),
					Interval(I<unsigned>(25), I<unsigned>(27))),
			  intersect(Range(Interval(I<unsigned>(15), I<unsigned>(27))),
			 			Range(Interval(I<unsigned>(10), I<unsigned>(16)),
			  				  Interval(I<unsigned>(18), I<unsigned>(20)),
			  				  Interval(I<unsigned>(25), I<unsigned>(30)))));
}

TEST_F(RangeTest,
IntersectOfFloatRangeAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(F<double>(vmin<double>()),
			  						   F<double>(28.69))),
			            Range(Interval(F<double>(115.28), F<double>(2204.57)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(F<double>(115.35),
			  						   F<double>(vmax<double>()))),
			            Range(Interval(F<double>(15.68), F<double>(23.97)))));
	EXPECT_EQ(Range(),
			  intersect(Range(),
			            Range(Interval(F<double>(115.43), F<double>(220.58)))));
	EXPECT_EQ(Range(),
			  intersect(Range(Interval(F<double>(16.89), F<double>(23.87))),
			            Range()));

	EXPECT_EQ(Range(Interval(F<double>(22.78), F<double>(25.78))),
			  intersect(Range(Interval(F<double>(15.83), F<double>(25.78))),
			            Range(Interval(F<double>(22.78), F<double>(220.83)))));

	EXPECT_EQ(Range(Interval(F<double>(22.48), F<double>(25.48))),
			  intersect(Range(Interval(F<double>(22.48), F<double>(220.48))),
			            Range(Interval(F<double>(15.48), F<double>(25.48)))));

	EXPECT_EQ(Range(Interval(F<double>(20.589), F<double>(20.589))),
			  intersect(Range(Interval(F<double>(15.78), F<double>(20.589))),
			            Range(Interval(F<double>(20.589), F<double>(220.35)))));

	EXPECT_EQ(Range(Interval(F<double>(20.698), F<double>(20.698))),
			  intersect(Range(Interval(F<double>(20.698), F<double>(220.7))),
			            Range(Interval(F<double>(15.038), F<double>(20.698)))));

	EXPECT_EQ(Range(Interval(F<double>(15.35), F<double>(20.68))),
			  intersect(Range(Interval(F<double>(15.35), F<double>(20.68))),
			            Range(Interval(F<double>(10.25),
			            			   F<double>(vmax<double>())))));

	EXPECT_EQ(Range(Interval(F<double>(15.789), F<double>(20.789))),
			  intersect(Range(Interval(F<double>(10.36),
			  						   F<double>(vmax<double>()))),
			            Range(Interval(F<double>(15.789), F<double>(20.789)))));

	EXPECT_EQ(Range(Interval(F<double>(15.58), F<double>(16.35)),
					Interval(F<double>(18.68), F<double>(20.34)),
					Interval(F<double>(25.78), F<double>(27.33))),
			  intersect(Range(Interval(F<double>(10.56), F<double>(16.35)),
			  				  Interval(F<double>(18.68), F<double>(20.34)),
			  				  Interval(F<double>(25.78), F<double>(30.33))),
			            Range(Interval(F<double>(15.58), F<double>(27.33)))));

	EXPECT_EQ(Range(Interval(F<double>(15.58), F<double>(16.35)),
					Interval(F<double>(18.68), F<double>(20.34)),
					Interval(F<double>(25.78), F<double>(27.33))),
			  intersect(Range(Interval(F<double>(15.58), F<double>(27.33))),
			 			Range(Interval(F<double>(10.56), F<double>(16.35)),
			  				  Interval(F<double>(18.68), F<double>(20.34)),
			  				  Interval(F<double>(25.78), F<double>(30.34)))));

	EXPECT_EQ(Range(),
			  intersect(Range(Interval(F<double>(25.83), F<double>(25.99))),
			            Range(Interval(F<double>(22.78), F<double>(25.82)))));
}


TEST_F(RangeTest,
IntersectOfSpecialFloatRangeAndSpecialFloatRangeWorksCorrectly)
{
	// (NAN, NAN) \cap (NAN, NAN) = \emptyset
	EXPECT_EQ(Range(), intersect(Range(F<double>(NAN)), Range(F<double>(NAN))));

	// (INF, INF) \cap (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(F<double>(INFINITY)),
			  intersect(Range(F<double>(INFINITY)), Range(F<double>(INFINITY))));

	// (-INF, -INF) \cap (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(F<double>(-INFINITY)),
			  intersect(Range(F<double>(-INFINITY)), Range(F<double>(-INFINITY))));

	// (-INF, INF) \cap (-INF, INF) = (-INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  intersect(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			Range(Interval(F<double>(-INFINITY), F<double>(INFINITY)))));

	// (-INF, INF) \cap (INF, INF) = (INF, INF)
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  intersect(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			Range(Interval(F<double>(INFINITY), F<double>(INFINITY)))));

	// (-INF, INF) \cap (-INF, -INF) = (-INF, -INF)
	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  intersect(Range(Interval(F<double>(-INFINITY), F<double>(INFINITY))),
			  			Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY)))));
}

TEST_F(RangeTest,
IntersectOfRangesOfDifferentTypesWorksCorretly)
{
	EXPECT_DEATH(intersect(Range(I<int>(0)), Range(F<float>(0.0))),
				 ".*r1.hasSameTypeAs\\(r2\\).*");
	EXPECT_DEATH(intersect(Range(I<int>(0)), Range(I<unsigned>(0))),
				 ".*r1.hasSameTypeAs\\(r2\\).*");
	EXPECT_DEATH(intersect(Range(I<unsigned char>(0)), Range(I<unsigned long>(0))),
				 ".*r1.hasSameTypeAs\\(r2\\).*");
}

TEST_F(RangeTest,
IntersectOfEmptyRangesWorksCorrectly)
{
	EXPECT_EQ(Range(), intersect(Range(), Range()));
	EXPECT_EQ(Range(), intersect(Range(I<int>(0.0)), Range()));
	EXPECT_EQ(Range(), intersect(Range(), Range(F<float>(0.0))));
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForEqOfSignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(-3), I<int>(3))),
			  computeRangeForEq(Range(Interval(I<int>(-10), I<int>(10))),
			  					Range(Interval(I<int>(-3), I<int>(3)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfSignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Range(Interval(I<int>(1), I<int>(5)))),
			  computeRangeForEq(Range(Interval(I<int>(-1), I<int>(0))),
			  					Range(Interval(I<unsigned>(1), I<unsigned>(5)))));
	EXPECT_EQ(Range(Range(Interval(I<int>(-1), I<int>(-1)))),
			  computeRangeForEq(Range(Interval(I<int>(-1), I<int>(0))),
			  					Range(Interval(I<unsigned>(vmax<unsigned>()),
			  								   I<unsigned>(vmax<unsigned>())))));
	EXPECT_EQ(Range(Range(Interval(I<int>(-2), I<int>(0)))),
			  computeRangeForEq(Range(Interval(I<int>(-2), I<int>(0))),
			  					Range(Interval(I<unsigned>(vmin<unsigned>()),
			  								   I<unsigned>(vmax<unsigned>())))));
	EXPECT_EQ(Range(Range(Interval(I<int>(-5), I<int>(20)))),
			  computeRangeForEq(Range(Interval(I<int>(-5), I<int>(20))),
			  					Range(Interval(I<unsigned>(vmin<unsigned>()),
			  								   I<unsigned>(vmax<unsigned>())))));
	EXPECT_EQ(Range(Range(Interval(I<int>(10), I<int>(10)))),
			  computeRangeForEq(Range(Interval(I<int>(-1), I<int>(10))),
			  					Range(Interval(I<unsigned>(10),
			  								   I<unsigned>(20)))));
	EXPECT_EQ(Range(Range(Interval(I<int>(-10), I<int>(-2)))),
			  computeRangeForEq(Range(Interval(I<int>(-10), I<int>(-2))),
			  					Range(Interval(I<unsigned>(vmax<unsigned>()-100),
			  								   I<unsigned>(vmax<unsigned>())))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfSignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(39), I<int>(40))),
			  computeRangeForEq(Range(Interval(I<int>(-100), I<int>(-50)),
			  						  Interval(I<int>(-40), I<int>(40)),
			  						  Interval(I<int>(50), I<int>(75))),
			  					Range(Interval(F<double>(39.5), F<double>(47.8)),
			  						  Interval(F<double>(vmax<int>() - 3.5),
			  						  		   F<double>(vmax<int>() - 1.8)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfUnsignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(0), I<unsigned>(0)),
						  Interval(I<unsigned>(vmax<unsigned>()), I<unsigned>(vmax<unsigned>())))),
			  computeRangeForEq(Range(Interval(I<unsigned>(1), I<unsigned>(5))),
			  					Range(Interval(I<int>(-1), I<int>(0)))));
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(vmax<unsigned>()),
								   I<unsigned>(vmax<unsigned>())))),
			  computeRangeForEq(Range(Interval(I<unsigned>(vmax<unsigned>()),
											   I<unsigned>(vmax<unsigned>()))),
			  					Range(Interval(I<int>(-1), I<int>(0)))));
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(0), I<unsigned>(0)),
						  Interval(I<unsigned>(vmax<unsigned>() - 1),
						  		   I<unsigned>(vmax<unsigned>())))),
			  computeRangeForEq(Range(Interval(I<unsigned>(vmin<unsigned>()),
											   I<unsigned>(vmax<unsigned>()))),
			  					Range(Interval(I<int>(-2), I<int>(0)))));
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(0), I<unsigned>(20)),
						  Interval(I<unsigned>(vmax<unsigned>() - 4),
						  		   I<unsigned>(vmax<unsigned>())))),
			  computeRangeForEq(Range(Interval(I<unsigned>(vmin<unsigned>()),
											   I<unsigned>(vmax<unsigned>()))),
			  					Range(Interval(I<int>(-5), I<int>(20)))));
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(10), I<unsigned>(10)))),
			  computeRangeForEq(Range(Interval(I<unsigned>(10),
											   I<unsigned>(20))),
			  					Range(Interval(I<int>(-1), I<int>(10)))));
	EXPECT_EQ(Range(Range(Interval(I<unsigned>(vmax<unsigned>() - 9),
								   I<unsigned>(vmax<unsigned>() - 1)))),
			  computeRangeForEq(Range(Interval(I<unsigned>(vmax<unsigned>()-100),
											   I<unsigned>(vmax<unsigned>()))),
			  					Range(Interval(I<int>(-10), I<int>(-2)))));

}

TEST_F(RangeTest,
ComputeRangeForEqOfUnsignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(7), I<unsigned>(10))),
			  computeRangeForEq(Range(Interval(I<unsigned>(0), I<unsigned>(10))),
			  					Range(Interval(I<unsigned>(7), I<unsigned>(18)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfUnsignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(7), I<unsigned>(9))),
			  computeRangeForEq(Range(Interval(I<unsigned>(0), I<unsigned>(10))),
			  					Range(Interval(F<double>(7), F<double>(9)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfFloatAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(39.5), F<double>(40.0))),
			  computeRangeForEq(Range(Interval(F<double>(39.5), F<double>(47.8)),
									  Interval(F<double>(vmax<int>() - 3.5),
											  F<double>(vmax<int>() - 1.8))),
								Range(Interval(I<int>(-100), I<int>(-50)),
									  Interval(I<int>(-40), I<int>(40)),
									  Interval(I<int>(50), I<int>(75)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfFloatAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(39.5), F<double>(47.8))),
			  computeRangeForEq(Range(Interval(F<double>(39.5), F<double>(47.8)),
									  Interval(F<double>(vmax<double>() - 3.5),
											   F<double>(vmax<double>() - 1.8))),
								Range(Interval(I<unsigned>(20), I<unsigned>(50)),
									  Interval(I<unsigned>(89), I<unsigned>(140)),
									  Interval(I<unsigned>(150), I<unsigned>(175)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfFloatdAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(15.58), F<double>(25.9))),
			  computeRangeForEq(Range(Interval(F<double>(15.58), F<double>(183.9))),
			  					Range(Interval(F<double>(-25.6), F<double>(25.9)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfSpecialFloatAndSpecialFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(INFINITY),
			  								   F<double>(INFINITY))),
			  					Range(Interval(F<double>(INFINITY),
			  								   F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(-INFINITY),
											   F<double>(-INFINITY))),
								  Range(Interval(F<double>(-INFINITY),
												 F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(-INFINITY),
												 F<double>(INFINITY))),
								  Range(Interval(F<double>(-INFINITY),
												 F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(INFINITY), F<double>(INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(-INFINITY),
											   F<double>(-INFINITY))),
								Range(Interval(F<double>(INFINITY),
											   F<double>(INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(INFINITY),
											   F<double>(INFINITY))),
								Range(Interval(F<double>(-INFINITY),
											   F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(-INFINITY), F<double>(-INFINITY))),
			  computeRangeForEq(Range(Interval(F<double>(NAN),
											   F<double>(NAN))),
								Range(Interval(F<double>(-INFINITY),
											   F<double>(-INFINITY)))));

	EXPECT_EQ(Range(Interval(F<double>(NAN), F<double>(NAN))),
			  computeRangeForEq(Range(Interval(F<double>(INFINITY),
											   F<double>(INFINITY))),
								Range(Interval(F<double>(NAN),
											   F<double>(NAN)))));
}

TEST_F(RangeTest,
ComputeRangeForEqOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForEq(Range(), Range()),
				".*!r1.empty().*!r2.empty()");
	EXPECT_DEATH(computeRangeForEq(Range(F<double>(1)), Range()),
				".*!r1.empty().*!r2.empty()");
	EXPECT_DEATH(computeRangeForEq(Range(), Range(I<int>(1))),
				".*!r1.empty().*!r2.empty()");
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForNeq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForNeqOfCorrectRangesWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(4))),
			  computeRangeForNeq(Range(Interval(I<int>(1), I<int>(4))),
			  					 Range(Interval(F<double>(3), F<double>(6)))));

	EXPECT_EQ(Range(Interval(F<double>(3), F<double>(6))),
			  computeRangeForNeq(Range(Interval(F<double>(3), F<double>(6))),
			  				Range(Interval(I<int>(1), I<int>(4)))));
}

TEST_F(RangeTest,
ComputeRangeForNeqOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForNeq(Range(), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForNeq(Range(F<double>(1)), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForNeq(Range(), Range(F<double>(2))),
				 ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForLtEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForLtEqOfSignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(10))),
		      computeRangeForLtEq(Range(I<int>(15)),
		   					      Range(I<int>(10))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(100), I<int>(120))),
		      computeRangeForLtEq(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(100), I<int>(120))),
		   					      Range(Interval(I<int>(100), I<int>(125)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(99), I<int>(115))),
		      computeRangeForLtEq(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<int>(100), I<int>(115)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfSignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(10)),
				    Interval(I<int>(99), I<int>(115))),
		      computeRangeForLtEq(Range(Interval(I<int>(-1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<unsigned>(100), I<unsigned>(115)))));
	EXPECT_EQ(Range(Interval(I<signed char>(-1), I<signed char>(10)),
				    Interval(I<signed char>(99), I<signed char>(120))),
		      computeRangeForLtEq(Range(Interval(I<signed char>(-1),
		      									 I<signed char>(10)),
		   							    Interval(I<signed char>(99),
		   							    		 I<signed char>(120))),
		   					      Range(Interval(I<unsigned char>(100),
		   					      				 I<unsigned char>(254)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfSignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(15))),
		      computeRangeForLtEq(Range(Interval(I<int>(16),I<int>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(15.69)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(99), I<int>(99))),
		      computeRangeForLtEq(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(F<double>(-1.35), F<double>(99)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfUnsignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(vmax<unsigned char>()))),
			  computeRangeForLtEq(Range(Interval(I<unsigned char>(1),
			  									 I<unsigned char>(10)),
			  							Interval(I<unsigned char>(25),
			  									 I<unsigned char>(250))),
			  					  Range(Interval(I<signed char>(-10),
			  					  			     I<signed char>(-1)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfUnsignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(16), I<unsigned>(19))),
		      computeRangeForLtEq(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(I<int>(10), I<int>(19)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfUnsignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(16), I<unsigned>(19))),
		      computeRangeForLtEq(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(19.6)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfFloatAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(11.0))),
		      computeRangeForLtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<int>(-10), I<int>(11)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfFloatAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(1))),
		      computeRangeForLtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<unsigned>(0),
		   					      				 I<unsigned>(1)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfFloatdAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(10.5))),
		      computeRangeForLtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(F<float>(6.8),
		   					      				 F<float>(10.5)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfSpecialFloatAndSpecialFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLtEq(Range(Interval(F<float>(NAN), F<float>(NAN)),
		   							    Interval(F<float>(-INFINITY),
		   							    		 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForLtEq(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLtEq(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLtEq(Range(Interval(F<float>(-INFINITY),
		      									 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLtEq(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForLtEq(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(NAN),
		   					      				 F<float>(NAN)))));
}

TEST_F(RangeTest,
ComputeRangeForLtEqOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForLtEq(Range(), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForLtEq(Range(F<double>(1)), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForLtEq(Range(), Range(F<double>(2))),
				 ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForLt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForLtOfSignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(9))),
		      computeRangeForLt(Range(I<int>(15)),
		   					      Range(I<int>(10))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(100), I<int>(120))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(100), I<int>(120))),
		   					      Range(Interval(I<int>(100), I<int>(125)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(99), I<int>(114))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<int>(100), I<int>(115)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(9))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10))),
		   					      Range(Interval(I<int>(10), I<int>(10)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10))),
		   					      Range(Interval(I<int>(10), I<int>(20)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(9))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10))),
		   					      Range(Interval(I<int>(5), I<int>(10)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(8))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10))),
		   					      Range(Interval(I<int>(5), I<int>(9)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfSignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(0), I<int>(10)),
				    Interval(I<int>(99), I<int>(114))),
		      computeRangeForLt(Range(Interval(I<int>(-1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<unsigned>(100), I<unsigned>(115)))));
	EXPECT_EQ(Range(Interval(I<signed char>(-1), I<signed char>(10)),
				    Interval(I<signed char>(99), I<signed char>(120))),
		      computeRangeForLt(Range(Interval(I<signed char>(-1),
		      									 I<signed char>(10)),
		   							    Interval(I<signed char>(99),
		   							    		 I<signed char>(120))),
		   					      Range(Interval(I<unsigned char>(100),
		   					      				 I<unsigned char>(254)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfSignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(vmin<int>()), I<int>(14))),
		      computeRangeForLt(Range(Interval(I<int>(16),I<int>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(15.69)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10))),
		      computeRangeForLt(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(F<double>(-1.35), F<double>(99)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfUnsignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned char>(0), I<unsigned char>(vmax<unsigned char>() - 1))),
			  computeRangeForLt(Range(Interval(I<unsigned char>(1),
			  									 I<unsigned char>(10)),
			  							Interval(I<unsigned char>(25),
			  									 I<unsigned char>(250))),
			  					  Range(Interval(I<signed char>(-10),
			  					  			     I<signed char>(-1)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfUnsignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(16), I<unsigned>(18))),
		      computeRangeForLt(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(I<int>(10), I<int>(19)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfUnsignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(16), I<unsigned>(19))),
		      computeRangeForLt(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(19.6)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfFloatAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(11.0))),
		      computeRangeForLt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<int>(-10), I<int>(11)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfFloatAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(1))),
		      computeRangeForLt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<unsigned>(0),
		   					      				 I<unsigned>(1)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfFloatdAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87))),
		      computeRangeForLt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(F<float>(6.8),
		   					      				 F<float>(10.5)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfSpecialFloatAndSpecialFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLt(Range(Interval(F<float>(NAN), F<float>(NAN)),
		   							    Interval(F<float>(-INFINITY),
		   							    		 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForLt(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForLt(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLt(Range(Interval(F<float>(-INFINITY),
		      									 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(-INFINITY))),
		      computeRangeForLt(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForLt(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(NAN),
		   					      				 F<float>(NAN)))));
}

TEST_F(RangeTest,
ComputeRangeForLtOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForLt(Range(), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForLt(Range(F<double>(1)), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForLt(Range(), Range(F<double>(2))),
				 ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForGtEq()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForGtEqOfSignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(15), I<int>(vmax<int>()))),
		      computeRangeForGtEq(Range(I<int>(10)),
		   					      Range(I<int>(15))));
	EXPECT_EQ(Range(Interval(I<int>(100), I<int>(125))),
			  computeRangeForGtEq(Range(Interval(I<int>(100), I<int>(125))),
			  					  Range(Interval(I<int>(1), I<int>(10)),
										Interval(I<int>(100), I<int>(120)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(115))),
			  computeRangeForGtEq(Range(Interval(I<int>(-10), I<int>(115))),
			                      Range(Interval(I<int>(1), I<int>(10)),
										Interval(I<int>(99), I<int>(120)))));
	EXPECT_EQ(Range(Interval(I<int>(9), I<int>(vmax<int>()))),
			  computeRangeForGtEq(Range(Interval(I<int>(0), I<int>(0))),
			  					  Range(Interval(I<int>(9), I<int>(9)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfSignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(-1)),
				    Interval(I<int>(100), I<int>(120))),
		      computeRangeForGtEq(Range(Interval(I<int>(-1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<unsigned>(100), I<unsigned>(115)))));

	EXPECT_EQ(Range(Interval(I<signed char>(100), I<signed char>(120))),
		      computeRangeForGtEq(Range(Interval(I<signed char>(-1),
		      									 I<signed char>(10)),
		   							    Interval(I<signed char>(99),
		   							    		 I<signed char>(120))),
		   					      Range(Interval(I<unsigned char>(100),
		   					      				 I<unsigned char>(255)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfSignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(16), I<int>(25))),
		      computeRangeForGtEq(Range(Interval(I<int>(16),I<int>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(15.69)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(99), I<int>(120))),
		      computeRangeForGtEq(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(F<double>(-1.35), F<double>(99)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfUnsignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned char>(1), I<unsigned char>(10)),
					Interval(I<unsigned char>(25), I<unsigned char>(250))),
			  computeRangeForGtEq(Range(Interval(I<unsigned char>(1),
			  									 I<unsigned char>(10)),
			  							Interval(I<unsigned char>(25),
			  									 I<unsigned char>(250))),
			  					  Range(Interval(I<signed char>(-10),
			  					  			     I<signed char>(-1)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfUnsignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(30), I<unsigned>(vmax<unsigned>()))),
		      computeRangeForGtEq(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(I<int>(30), I<int>(39)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfUnsignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(10), I<unsigned>(25))),
		      computeRangeForGtEq(Range(Interval(I<unsigned>(9),I<unsigned>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(19.6)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfFloatAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<int>(-10), I<int>(11)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfFloatAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(4.0), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<unsigned>(4),
		   					      				 I<unsigned>(10)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfFloatdAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(6.8), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGtEq(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(F<float>(6.8),
		   					      				 F<float>(10.5)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfSpecialFloatAndSpecialFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
		      computeRangeForGtEq(Range(Interval(F<float>(NAN), F<float>(NAN)),
		   							    Interval(F<float>(-INFINITY),
		   							    		 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
		      computeRangeForGtEq(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForGtEq(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(INFINITY),F<float>(INFINITY))),
		      computeRangeForGtEq(Range(Interval(F<float>(-INFINITY),
		      									 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(-INFINITY), F<float>(INFINITY))),
		      computeRangeForGtEq(Range(Interval(F<float>(-INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForGtEq(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(NAN),
		   					      				 F<float>(NAN)))));
}

TEST_F(RangeTest,
ComputeRangeForGtEqOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForGtEq(Range(), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForGtEq(Range(F<double>(1)), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForGtEq(Range(), Range(F<double>(2))),
				 ".*!r1.empty().*!r2.empty().*");
}

////////////////////////////////////////////////////////////////////////////////
// computeRangeForGt()
////////////////////////////////////////////////////////////////////////////////

TEST_F(RangeTest,
ComputeRangeForGtOfSignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(16), I<int>(vmax<int>()))),
		      computeRangeForGt(Range(I<int>(10)),
		   					      Range(I<int>(15))));
	EXPECT_EQ(Range(Interval(I<int>(100), I<int>(125))),
			  computeRangeForGt(Range(Interval(I<int>(100), I<int>(125))),
			  					  Range(Interval(I<int>(1), I<int>(10)),
										Interval(I<int>(100), I<int>(120)))));
	EXPECT_EQ(Range(Interval(I<int>(101), I<int>(125))),
			  computeRangeForGt(Range(Interval(I<int>(100), I<int>(125))),
			  					  Range(Interval(I<int>(100), I<int>(120)))));
	EXPECT_EQ(Range(Interval(I<int>(2), I<int>(115))),
			  computeRangeForGt(Range(Interval(I<int>(-10), I<int>(115))),
			                      Range(Interval(I<int>(1), I<int>(10)),
										Interval(I<int>(99), I<int>(120)))));
	EXPECT_EQ(Range(Interval(I<int>(10), I<int>(vmax<int>()))),
			  computeRangeForGt(Range(Interval(I<int>(0), I<int>(0))),
			                      Range(Interval(I<int>(9), I<int>(9)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfSignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(-1), I<int>(-1)),
				    Interval(I<int>(101), I<int>(120))),
		      computeRangeForGt(Range(Interval(I<int>(-1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(I<unsigned>(100), I<unsigned>(115)))));

	EXPECT_EQ(Range(Interval(I<signed char>(101), I<signed char>(120))),
		      computeRangeForGt(Range(Interval(I<signed char>(-1),
		      									 I<signed char>(10)),
		   							    Interval(I<signed char>(99),
		   							    		 I<signed char>(120))),
		   					      Range(Interval(I<unsigned char>(100),
		   					      				 I<unsigned char>(255)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfSignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<int>(16), I<int>(25))),
		      computeRangeForGt(Range(Interval(I<int>(16),I<int>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(15.69)))));
	EXPECT_EQ(Range(Interval(I<int>(1), I<int>(10)),
				    Interval(I<int>(99), I<int>(120))),
		      computeRangeForGt(Range(Interval(I<int>(1), I<int>(10)),
		   							    Interval(I<int>(99), I<int>(120))),
		   					      Range(Interval(F<double>(-1.35), F<double>(99)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfUnsignedAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned char>(1), I<unsigned char>(10)),
					Interval(I<unsigned char>(25), I<unsigned char>(250))),
			  computeRangeForGt(Range(Interval(I<unsigned char>(1),
			  									 I<unsigned char>(10)),
			  							Interval(I<unsigned char>(25),
			  									 I<unsigned char>(250))),
			  					  Range(Interval(I<signed char>(-10),
			  					  			     I<signed char>(-1)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfUnsignedAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(31), I<unsigned>(vmax<unsigned>()))),
		      computeRangeForGt(Range(Interval(I<unsigned>(16),I<unsigned>(25))),
		   					      Range(Interval(I<int>(30), I<int>(39)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfUnsignedAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(I<unsigned>(10), I<unsigned>(25))),
		      computeRangeForGt(Range(Interval(I<unsigned>(9),I<unsigned>(25))),
		   					      Range(Interval(F<double>(10.25), F<double>(19.6)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfFloatAndSignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(1.1), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<int>(-10), I<int>(11)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfFloatAndUnsignedRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(4.0), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(I<unsigned>(4),
		   					      				 I<unsigned>(10)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfFloatdAndFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(6.8), F<float>(9.87)),
				    Interval(F<float>(10.5), F<float>(12.8))),
		      computeRangeForGt(Range(Interval(F<float>(1.1), F<float>(9.87)),
		   							    Interval(F<float>(10.5), F<float>(12.8))),
		   					      Range(Interval(F<float>(6.8),
		   					      				 F<float>(10.5)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfSpecialFloatAndSpecialFloatRangeWorksCorrectly)
{
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
		      computeRangeForGt(Range(Interval(F<float>(NAN), F<float>(NAN)),
		   							    Interval(F<float>(-INFINITY),
		   							    		 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForGt(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForGt(Range(Interval(F<float>(NAN), F<float>(NAN))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
		      computeRangeForGt(Range(Interval(F<float>(-INFINITY),
		      									 F<float>(-INFINITY))),
		   					      Range(Interval(F<float>(INFINITY),
		   					      				 F<float>(INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(INFINITY), F<float>(INFINITY))),
		      computeRangeForGt(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(-INFINITY),
		   					      				 F<float>(-INFINITY)))));
	EXPECT_EQ(Range(Interval(F<float>(NAN), F<float>(NAN))),
		      computeRangeForGt(Range(Interval(F<float>(INFINITY),
		      									 F<float>(INFINITY))),
		   					      Range(Interval(F<float>(NAN),
		   					      				 F<float>(NAN)))));
}

TEST_F(RangeTest,
ComputeRangeForGtOfEmptyRangesWorksCorrectly)
{
	EXPECT_DEATH(computeRangeForGt(Range(), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForGt(Range(F<double>(1)), Range()),
				 ".*!r1.empty().*!r2.empty().*");
	EXPECT_DEATH(computeRangeForGt(Range(), Range(F<double>(2))),
				 ".*!r1.empty().*!r2.empty().*");
}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
