/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Range.h
* @brief  Class that represents the value range of the variable.
* @date   2012
*/

#ifndef GUARD_RANGE_H
#define GUARD_RANGE_H

#include <vector>
#include <utility>

#include "Number.h"

/**
* @brief Class that represents the value range of the variable.
*
* This class represents the value range that variable can acquire during the
* program execution and can be used for variables of all types in the C language.
* Also all reasonable operations over ranges are supplied by this class. However,
* the results of some operations are over-approximated.
*
* When a range has more than MAX_INTERVALS_IN_RANGE, mergeIntervals() is called.
*/
class Range {
	public:
		/// Definition of interval.
		typedef std::pair<Number, Number> Interval;

		/// Definition of the iterator for the Range class.
		typedef std::vector<Interval>::iterator iterator;

		/// Definition of the constant iterator for the Range class.
		typedef std::vector<Interval>::const_iterator const_iterator;

		/// Definition of the reverse iterator for the Range class.
		typedef std::vector<Interval>::reverse_iterator reverse_iterator;

		/// Definition of the constant iterator for the Range class.
		typedef std::vector<Interval>::const_reverse_iterator const_reverse_iterator;

		/// Definition of the type for expressing size.
		typedef size_t size_type;

		/// Returns the iterator to the first interval.
		iterator begin()             { return data.begin(); }

		/// Returns the constant iterator to the first interval.
		const_iterator begin() const { return data.begin(); }

		/// Returns the iterator behind the last interval.
		iterator end()				 { return data.end(); }

		/// Returns the constant iterator behind the last interval.
		const_iterator end() const   { return data.end(); }

		/// Returns the iterator to the last interval.
		reverse_iterator rbegin()             { return data.rbegin(); }

		/// Returns the constant iterator to the last interval.
		const_reverse_iterator rbegin() const { return data.rbegin(); }

		/// Returns the iterator before the first interval.
		reverse_iterator rend()				 { return data.rend(); }

		/// Returns the constant iterator before the first interval.
		const_reverse_iterator rend() const   { return data.rend(); }

		/// Returns the number of stored intervals.
		size_type size() const       { return data.size(); }

		/// Returns @c true if range is empty.
		bool empty() const           { return size() == 0; }

		/// Returns the interval on the given position.
		Interval& operator[](size_type i)             { return data[i]; }

		/// Returns the constant interval on the given position.
		const Interval& operator[](size_type i) const { return data[i]; }

		Range();
		explicit Range(Number n);
		explicit Range(Interval i);
		Range(Interval i1, Interval i2);
		Range(Interval i1, Interval i2, Interval i3);
		Range(Interval i1, Interval i2, Interval i3, Interval i4);

		Range assign(const Range &r) const;
		Range expand() const;
		Range mergeIntervals() const;

		bool containsNan() const;
		bool containsPositiveInf() const;
		bool containsNegativeInf() const;
		bool containsTrue() const;
		bool containsFalse() const;
		bool containsZero() const;
		bool isIntegral() const;
		bool isFloatingPoint() const;
		bool isSigned() const;
		bool isUnsigned() const;
		bool hasSameTypeAs(const Range &r) const;
		bool containsOnlySingleNumber() const;
		bool containsIntegralMin() const;
		bool containsIntegralMinusOne() const;
		Number getMax() const;
		Number getMin() const;

		static Range getMaxRange(const Number &n);

		friend bool operator==(const Range &r1, const Range &r2);
		friend bool operator!=(const Range &r1, const Range &r2);

		friend Range logicalEq(const Range &r1, const Range &r2);
		friend Range logicalNeq(const Range &r1, const Range &r2);
		friend Range logicalLt(const Range &r1, const Range &r2);
		friend Range logicalGt(const Range &r1, const Range &r2);
		friend Range logicalLtEq(const Range &r1, const Range &r2);
		friend Range logicalGtEq(const Range &r1, const Range &r2);

		friend Range operator-(const Range &r);
		friend Range operator+(const Range &r1, const Range &r2);
		friend Range operator-(const Range &r1, const Range &r2);
		friend Range operator*(const Range &r1, const Range &r2);
		friend Range rdiv(const Range &r1, const Range &r2);
		friend Range exact_div(const Range &r1, const Range &r2);
		friend Range trunc_div(const Range &r1, const Range &r2);
		friend Range trunc_mod(const Range &r1, const Range &r2);

		friend Range logicalNot(const Range &r);
		friend Range logicalAnd(const Range &r1, const Range &r2);
		friend Range logicalOr(const Range &r1, const Range &r2);
		friend Range logicalXor(const Range &r1, const Range &r2);

		friend Range overApproximateUnaryOp(const Range &r);
		friend Range overApproximateBinaryOp(const Range &r1, const Range &r2);

		friend Range bitNot(const Range &r);
		friend Range bitAnd(const Range &r1, const Range &r2);
		friend Range bitOr(const Range &r1, const Range &r2);
		friend Range bitXor(const Range &r1, const Range &r2);
		friend Range bitLeftShift(const Range &r1, const Range &r2);
		friend Range bitRightShift(const Range &r1, const Range &r2);
		friend Range bitLeftRotate(const Range &r1, const Range &r2);
		friend Range bitRightRotate(const Range &r1, const Range &r2);

		friend Range abs(const Range &r);
		friend Range intToFloat(const Range &r);
		friend Range min(const Range &r1, const Range &r2);
		friend Range max(const Range &r1, const Range &r2);

		friend Range unite(const Range &r1, const Range &r2);
		friend Range intersect(const Range& r1, const Range& r2);

		friend Range computeRangeForEq(const Range &r1, const Range &r2);
		friend Range computeRangeForNeq(const Range &r1, const Range &r2);
		friend Range computeRangeForGt(const Range &r1, const Range &r2);
		friend Range computeRangeForLt(const Range &r1, const Range &r2);
		friend Range computeRangeForGtEq(const Range &r1, const Range &r2);
		friend Range computeRangeForLtEq(const Range &r1, const Range &r2);

		friend std::ostream& operator<<(std::ostream&, const Range&);

	private:
		static Range performIntegralDivOp(const Range &r1, const Range &r2,
			bool exact);

		void normalize();
		void mergeIntervalsInPlace();
		Range splitBySpecialValues() const;
		Range splitByZero() const;
		static std::pair<Range, Range>
			extensionByCRules(const Range &r1, const Range &r2);

		/// Stores the intervals.
		std::vector<Interval> data;

		/// Maximal number of intervals in a range (after that,
		/// mergeIntervals() is automatically called).
		static const size_t MAX_INTERVALS_IN_RANGE = 15;
};

#endif
