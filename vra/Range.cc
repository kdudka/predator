/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Range.cc
* @brief  Implementation of class that represents the value range of the variable.
* @date   2012
*/

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>

// Enable assertions.
#undef NDEBUG
#include <cassert>

#include "Range.h"

using std::vector;
using std::sort;
using std::max;
using std::min;
using std::cout;
using std::endl;
using std::pair;

typedef Range::Interval Interval;

namespace {

/**
* @brief Compares two intervals according to their lower bounds.
*
* @param[in] i1 The first interval.
* @param[in] i2 The second interval.
* @return @c true if the first interval is lower than the second one, @c false
*         otherwise.
*/
bool compareLowerBounds(const Range::Interval &i1, const Range::Interval &i2)
{
	if (i1.first.isNotNumber())
		return true;
	else if (i2.first.isNotNumber())
		return false;
	return (i1.first <  i2.first) ||
		   (i1.first == i2.first && i1.second < i2.second);
}

}

// Definition of static variables and constants.
const size_t Range::MAX_INTERVALS_IN_RANGE;

/**
* @brief Constructs an empty range.
*/
Range::Range()
{
}

/**
* @brief Constructs a range containing an interval [@a n, @a n] (a single point).
*
* @param[in] n Number that represents the lower bound and the upper bound of the
*              interval that will be added to the range.
*/
Range::Range(Number n)
{
	data.push_back(Interval(n,n));
}

/**
* @brief Constructs a range containing one interval.
*
* @param[in] i The first interval that will be added to the range.
*/
Range::Range(Interval i)
{
	data.push_back(i);
	normalize();
}

/**
* @brief Constructs a range containing two intervals.
*
* @param[in] i1 The first interval that will be added to the range.
* @param[in] i2 The second interval that will be added to the range.
*/
Range::Range(Interval i1, Interval i2)
{
	data.push_back(i1);
	data.push_back(i2);
	normalize();
}

/**
* @brief Constructs a range containing three intervals.
*
* @param[in] i1 The first interval that will be added to the range.
* @param[in] i2 The second interval that will be added to the range.
* @param[in] i3 The third interval that will be added to the range.
*/
Range::Range(Interval i1, Interval i2, Interval i3)
{
	data.push_back(i1);
	data.push_back(i2);
	data.push_back(i3);
	normalize();
}

/**
* @brief Constructs a range containing four intervals.
*
* @param[in] i1 The first interval that will be added to the range.
* @param[in] i2 The second interval that will be added to the range.
* @param[in] i3 The third interval that will be added to the range.
* @param[in] i4 The fourth interval that will be added to the range.
*/
Range::Range(Interval i1, Interval i2, Interval i3, Interval i4)
{
	data.push_back(i1);
	data.push_back(i2);
	data.push_back(i3);
	data.push_back(i4);
	normalize();
}

/**
* @brief Normalizes vector of intervals.
*
* It edits intervals, so the lower bounds are always lower or equal than upper
* bounds. It sorts intervals in the vector according to the numerical order and
* if intervals are overlapping or neighbouring it joins them together.
*/
void Range::normalize()
{
	// It does not make sense to normalize empty vector.
	if (data.size() == 0)
		return;

	// Only one NAN interval is kept.
	vector<Interval> tmp;
	bool isNanThere = false;
	for (iterator it = data.begin(); it != data.end(); ++it) {
		if (it->first.isNotNumber() || it->second.isNotNumber()) {
			if (!isNanThere) {
				// The first interval representing NAN is kept. Others are
				// removed.
				if (it->first.isNotNumber()) {
					tmp.push_back(Interval(it->first, it->first));
				} else {
					tmp.push_back(Interval(it->second, it->second));
				}
				isNanThere = true;
			}
		} else {
			// Everything except extra NAN intervals is kept.
			tmp.push_back(*it);
		}
	}
	swap(data, tmp);

	// Pre-processing intervals.
	tmp.clear();
	for (iterator it = data.begin(); it != data.end(); ++it) {
		if (it->first > it->second) {
			// The lower bound is greater than the upper bound.
			tmp.push_back(Interval(it->second.getMin(), it->second));
			tmp.push_back(Interval(it->first, it->first.getMax()));
		} else {
			// The lower bound is lower or equal than the upper bound.
			tmp.push_back(*it);
		}
	}
	swap(data, tmp);

	// Sorting intervals.
	sort(data.begin(), data.end(), compareLowerBounds);

	tmp.clear();
	tmp.push_back(data.front());

	// If the space between intervals is lower than or equal epsilon, they
	// will be joined together.
	Number epsilon(1, sizeof(int), true);
	if (data.front().first.isFloatingPoint()) {
		epsilon = Number(0.1, sizeof(double));
	}

	// Joining intervals.
	for (iterator current = data.begin() + 1; current != data.end(); current++) {
		Interval previous = tmp.back();
		// The second condition is necessary because of overflowing of the Number.
		if (current->first <= previous.second ||
			current->first <= previous.second + epsilon) {
			// The lower bound of currently processed interval is lower or equal
			// than upper bound of the previous processed interval (intervals are
			// neighbours or overlap).
			Number lowerBound = std::min(previous.first, current->first);
			Number upperBound = std::max(previous.second, current->second);
			tmp.back() = Interval(lowerBound, upperBound);
		} else {
			// If it is not possible to join the last processed interval with the
			// current one.
			tmp.push_back(*current);
		}
	}
	swap(data, tmp);

	// If there are too many intervals, merge them.
	if (data.size() > MAX_INTERVALS_IN_RANGE) {
		mergeIntervalsInPlace();
	}
}

/**
* @brief Merges the intervals in the range in place, meaning that the current
*        range is changed.
*
* See the description of mergeIntervals() for more information.
*/
void Range::mergeIntervalsInPlace() {
	// On ranges with no more than a single interval, there is nothing to do.
	if (data.size() < 2) {
		return;
	}

	if (isFloatingPoint() && containsNan()) {
		// We have to handle the situation when there is NAN separately.
		Number nan(data[0].first);
		Number min(data[1].first);
		Number max(data[data.size() - 1].second);
		data.clear();
		data.push_back(Interval(nan, nan));
		data.push_back(Interval(min, max));
	} else {
		Number min(data[0].first);
		Number max(data[data.size() - 1].second);
		data.clear();
		data.push_back(Interval(min, max));
	}
}

/**
* @brief Splits range into intervals where the intervals (INF, INF) and (-INF, -INF)
*        are always extra. Thus, there will be no intervals like (-INF, 5). This will
*        be always split into (-INF, -INF) and (MIN, 5). It also splits all intervals
*        that contains negative and positive numbers at the same time. For example,
*        interval (-5, 5) will be split into intervals (-5, -1) and (0, 5). Also, for
*        integral types (MIN, MIN) must be extra. Otherwise, it causes problems.
*        Because, MIN * (-1) = MIN.
*/
Range Range::splitBySpecialValues() const
{
	// I should always work with non-empty range.
	assert(!empty());

	Range result;
	if (isFloatingPoint()) {
		// Split floating-point range by special values.
		for (Range::const_iterator it = begin(); it != end(); ++it) {
			const Number &x = it->first;
			const Number &y = it->second;

			// In the next code, we need to have zero with the appropriate type.
			Number zero = x.assign(Number(0, sizeof(char), true));

			if (x.isNegativeInf() && y.isNumber() && y < zero) {
				// For intervals in the form of (-INF, -number).
				result.data.push_back(Interval(x, x));
				result.data.push_back(Interval(x.getMin(), y));
			} else if (x.isNegativeInf() && y.isNumber() && y >= zero) {
				// For intervals in the form of (-INF, number).
				result.data.push_back(Interval(x, x));
				result.data.push_back(Interval(x.getMin(), zero));
				result.data.push_back(Interval(zero, y));
			} else if (x.isNegativeInf() && y.isPositiveInf()) {
				// For intervals in the form of (-INF, INF).
				result.data.push_back(Interval(x, x));
				result.data.push_back(Interval(x.getMin(), zero));
				result.data.push_back(Interval(zero, x.getMax()));
				result.data.push_back(Interval(y, y));
			} else if (x.isNumber() && x < zero && y.isNumber() && y >= zero) {
				// For intervals in the form of (-number, number).
				result.data.push_back(Interval(x, zero));
				result.data.push_back(Interval(zero, y));
			} else if (x.isNumber() && x < zero && y.isPositiveInf()) {
				// For intervals in the form of (-number, INF)
				result.data.push_back(Interval(x, zero));
				result.data.push_back(Interval(zero, x.getMax()));
				result.data.push_back(Interval(y, y));
			} else if (x.isNumber() && x >= zero && y.isPositiveInf()) {
				// For intervals in the form of (number, INF).
				result.data.push_back(Interval(x, x.getMax()));
				result.data.push_back(Interval(y, y));
			} else {
				// Otherwise, for intervals in the forms (-INF, -INF),
				// (-number, -number), (number, number), (INF, INF), (NAN, NAN).
				result.data.push_back(Interval(x, y));
			}
		}
	} else if (isIntegral() && isSigned()) {
		// Split signed integral ranges by special values.
		for (Range::const_iterator it = begin(); it != end(); ++it) {
			const Number &x = it->first;
			const Number &y = it->second;

			// In the next code, we need to have zero with the appropriate type.
			Number zero = x.assign(Number(0, sizeof(char), true));

			if (x.isMin() && y.isMin()) {
				// For intervals in the form of (MIN, MIN).
				result.data.push_back(Interval(x, y));
			} else if (x.isMin() && y.isNumber() && y < zero) {
				// For intervals in the form of (MIN, - number).
				Number xPlusOne(x.getInt() + 1, x.getBitWidth(), x.getSign());
				result.data.push_back(Interval(x, x));
				result.data.push_back(Interval(xPlusOne, y));
			} else if (x.isMin() && y.isNumber() && y >= zero) {
				// For intervals in the form  of (MIN, number) and (MIN, MAX).
				Number xPlusOne(x.getInt() + 1, x.getBitWidth(), x.getSign());
				result.data.push_back(Interval(x, x));
				result.data.push_back(Interval(xPlusOne, zero));
				result.data.push_back(Interval(zero, y));
			} else if (x.isNumber() && x < zero && y.isNumber() && y >= zero) {
				// For intervals in the form of (-number, number) and (-number, MAX).
				result.data.push_back(Interval(x, zero));
				result.data.push_back(Interval(zero, y));
			} else {
				// Otherwise, for intervals in the forms (MIN, MIN),
				// (-number, -number), (number, number), (number, MAX), (MAX, MAX).
				result.data.push_back(Interval(x, y));
			}
		}
	} else if (isIntegral() && isUnsigned()) {
		// There are no problems for unsigned integral ranges.
		result = *this;
	}

	// Do not call the normalize() function here. It blends everything together
	// and the resulted range will be same as the given one.
	return result;
}

/**
* @brief Splits the range so that if it contains 0 or 0.0, then this value will
*        be in a separate interval (0, 0) or (0.0, 0.0)
*
* @return Splitted range.
*/
Range Range::splitByZero() const {
	// I should always work with non-empty range.
	assert(!empty());

	// In the next code, we need to have zero with the appropriate type.
	Number zero = data[0].first.assign(Number(0, sizeof(char), true));

	Range result;

	for (Range::const_iterator it = begin(); it != end(); ++it) {
		const Number &x = it->first;
		const Number &y = it->second;

		if (isFloatingPoint()) {
			// Split floating-point range.

			if (x == zero && y == zero) {
				result.data.push_back(Interval(zero, zero));
			} else if (x == zero) {
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(x.getEpsilon(), y));
			} else if (y == zero) {
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(x, -y.getEpsilon()));
			} else if (x < zero && y > zero) {
				result.data.push_back(Interval(x, -y.getEpsilon()));
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(x.getEpsilon(), y));
			} else {
				result.data.push_back(Interval(x, y));
			}
		} else if (isIntegral() && isSigned()) {
			// Split a signed integral range.
			if (x == zero && y == zero) {
				result.data.push_back(Interval(zero, zero));
			} else if (x == zero) {
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(Number(1, x.getBitWidth(),
					true), y));
			} else if (y == zero) {
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(x, Number(-1, x.getBitWidth(),
					true)));
			} else if (x < zero && y > zero) {
				result.data.push_back(Interval(x, Number(-1, x.getBitWidth(),
					true)));
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(Number(1, x.getBitWidth(),
					true), y));
			} else {
				result.data.push_back(Interval(x, y));
			}
		} else { // isIntegral() && isUnsigned()
			assert(isIntegral() && isUnsigned());

			// Split an unsigned integral range.
			if (x == zero && y == zero) {
				result.data.push_back(Interval(zero, zero));
			} else if (x == zero) {
				result.data.push_back(Interval(zero, zero));
				result.data.push_back(Interval(Number(1, x.getBitWidth(),
					false), y));
			} else {
				result.data.push_back(Interval(x, y));
			}
		}
	}

	// Do not call the normalize() function here. It blends everything together
	// and the resulted range will be same as the given one.
	return result;
}

/**
* @brief Returns @c true if current range has the same type as the range @a r, @c
*        false otherwise.
*/
bool Range::hasSameTypeAs(const Range &r) const
{
	assert(!empty() && !r.empty());

	return (((isIntegral() && r.isIntegral()) &&
			(isSigned() == r.isSigned()) &&
		    (data[0].first.getBitWidth() == r.data[0].first.getBitWidth())) ||
			((isFloatingPoint() && r.isFloatingPoint()) &&
			(data[0].first.getBitWidth() == r.data[0].first.getBitWidth())));
}

/**
* @brief Returns @c true if the current range contains just a single number, @c
*        false otherwise.
*
* For example, the ranges [1, 1] and [4.5, 4.5] both contain a single number
* while the ranges [1, 2] and [3, 3][5, 7] contains more than a single number.
*/
bool Range::containsOnlySingleNumber() const
{
	return size() == 1 && data[0].first == data[0].second;
}

/**
* @brief Returns @c true if the current range contains the minimal integral
*        value, @c false otherwise.
*
* If the range is composed of floating-point numbers, this function returns @c
* false.
*/
bool Range::containsIntegralMin() const
{
	if (empty() || !isIntegral()) {
		return false;
	}

	// The minimal integral value should be in the first interval (if any).
	return data[0].first.isMin();
}

/**
* @brief Returns @c true if the current range contains the intergral value @c
*        -1, @c false otherwise.
*
* If the range is composed of floating-point numbers, this function
* returns @c false.
*/
bool Range::containsIntegralMinusOne() const
{
	if (empty() || !isIntegral()) {
		return false;
	}

	// Create a -1 value of a proper type.
	Number minOne(-1, data[0].first.getBitWidth(), data[0].first.isSigned());

	// Go through the range and try to find -1.
	for (const_iterator it = begin(); it != end(); ++it) {
		// Notice that the case when it->fist == -1 or it-second == -1 is also
		// included in the following check.
		if (it->first <= minOne && it->second >= minOne) {
			return true;
		}
	}

	return false;
}

/**
* @brief Returns the maximal number stored in the current range.
*/
Number Range::getMax() const
{
	assert(!empty());
	return data[size() - 1].second;
}

/**
* @brief Returns the minimal number stored in the current range.
*/
Number Range::getMin() const
{
	assert(!empty());
	if (data[0].first.isNotNumber() && size() > 1) {
		return data[1].first;
	}
	else {
		return data[0].first;
	}
}

/**
* @brief Extends ranges according to the C99 standard.
*
* @param[in] r1 The first range that will be extended.
* @param[in] r2 The second range that will be extended.
*
* @return The pair of extended ranges.
*/
std::pair<Range, Range> Range::extensionByCRules(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	// This is done in order to get the result that contains the correct type.
	const Number &firstOp = r1.data[0].first;
	const Number &secondOp = r2.data[0].first;
	Number res = firstOp * secondOp;

	Range result1;
	for (Range::const_iterator it = r1.begin(); it != r1.end(); ++it) {
		// Converts the interval according to the type stored in res.
		Interval tmp = Interval(res.assign(it->first), res.assign(it->second));
		result1.data.push_back(tmp);
	}
	result1.normalize();

	Range result2;
	for (Range::const_iterator it = r2.begin(); it != r2.end(); ++it) {
		// Converts the interval according to the type stored in res.
		Interval tmp = Interval(res.assign(it->first), res.assign(it->second));
		result2.data.push_back(tmp);
	}
	result2.normalize();

	return std::pair<Range, Range>(result1, result2);
}

/**
* @brief Returns @c true if the current range contains @c NAN, @c false otherwise.
*/
bool Range::containsNan() const
{
	// I should always work with non-empty range.
	assert(!empty());

	Interval i = data[0];
	if (i.first.isNotNumber()) {
		return true;
	} else {
		return false;
	}
}

/**
* @brief Returns @c true if the current range contains @c INF, @c false otherwise.
*/
bool Range::containsPositiveInf() const
{
	// I should always work with non-empty range.
	assert(!empty());

	// Ranges are sorted. So, if INF is present in the range, it is its last element.
	Interval i = data[size() - 1];
	if (i.second.isPositiveInf()) {
		 // We use second, because (number, INF) is correct interval.
		return true;
	} else {
		return false;
	}
}

/**
* @brief Returns @c true if the current range contains @c -INF, @c false otherwise.
*/
bool Range::containsNegativeInf() const
{
	// I should always work with non-empty range.
	assert(!empty());

	// Ranges are sorted. So, if -INF is present in the range, it is its
	// first or second element. It is second if (NAN, NAN) interval is present.
	// Otherwise, it is first.
	if (data[0].first.isNegativeInf() ||
	   (size() > 1 && data[1].first.isNegativeInf())) {
		 // We use second, because (number, INF) is correct interval.
		return true;
	} else {
		return false;
	}
}

/**
* @brief Returns @c true if the current range contains value convertible to @c
*        true, @c false otherwise.
*/
bool Range::containsTrue() const
{
	// I should always work with non-empty range.
	assert(!empty());

	for (Range::const_iterator it = begin(); it != end(); ++it) {
		if (it->first.toBool() || it->second.toBool()) {
			return true;
		}
	}

	return false;
}

/**
* @brief Returns @c true if the current range contains value convertible to @c
*        false, @c false otherwise.
*/
bool Range::containsFalse() const
{
	// I should always work with non-empty range.
	assert(!empty());

	// Only zero is convertible to false.
	return containsZero();
}

/**
* @brief Returns @c true if the current range contains a zero (@c 0 or @c 0.0),
*        @c false otherwise.
*/
bool Range::containsZero() const
{
	// I should always work with non-empty range.
	assert(!empty());

	Number zeroInt(0, sizeof(int), true);
	Number zero(data[0].first.assign(zeroInt));
	for (Range::const_iterator it = begin(); it != end(); ++it) {
		if (!it->first.toBool() || !it->second.toBool() ||
			((it->first < zero) && (zero < it->second))) {
			return true;
		}
	}

	return false;
}

/**
* @brief Returns the maximal range for the given number @a n according to the number
*        type.
*/
Range Range::getMaxRange(const Number &n)
{
	if (n.isIntegral()) {
		return Range(Range::Interval(n.getMin(), n.getMax()));
	} else { // n.isFloatingPoint()
		return Range(Range::Interval(n.getNan(), n.getNan()),
					 Range::Interval(n.getNegativeInf(), n.getPositiveInf()));
	}
}

/**
* @brief Computes logical @c not for range @a r.
*/
Range logicalNot(const Range &r)
{
	// I should always work with non-empty range.
	assert(!r.empty());

	Range result;
	Number zero = Number(0, sizeof(int), true);
	Number one = Number(1, sizeof(int), true);

	if (r.containsTrue() && r.containsFalse()) {
		// ! [0,1]
		result.data.push_back(Range::Interval(zero, one));
	} else if (r.containsTrue()) {
		// ! [1]
		result.data.push_back(Range::Interval(zero, zero));
	} else if (r.containsFalse()) {
		// ! [0]
		result.data.push_back(Range::Interval(one, one));
	}

	result.normalize();
	return result;
}

/**
* @brief Computes logical @c and for ranges @a r1 and @a r2.
*/
Range logicalAnd(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	Range result;

	// I use here int type because in C programs, integral promotion is applied.
	// Hence, by using int type instead boolean, I cannot break anything.
	Number zero = Number(0, sizeof(int), true);
	Number one = Number(1, sizeof(int), true);

	if (r1.containsFalse() || r2.containsFalse()) {
		result.data.push_back(Range::Interval(zero, zero));
	}

	if (r1.containsTrue() && r2.containsTrue()) {
		result.data.push_back(Range::Interval(one, one));
	}

	result.normalize();
	return result;
}

/**
* @brief Computes logical @c or for ranges @a r1 and @a r2.
*/
Range logicalOr(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	Range result;

	// I use here int type because in C programs, integral promotion is applied.
	// Hence, by using int type instead boolean, I cannot break anything.
	Number zero = Number(0, sizeof(int), true);
	Number one = Number(1, sizeof(int), true);

	if (r1.containsFalse() && r2.containsFalse()) {
		result.data.push_back(Range::Interval(zero, zero));
	}

	if (r1.containsTrue() || r2.containsTrue()) {
		result.data.push_back(Range::Interval(one, one));
	}

	result.normalize();
	return result;
}

/**
* @brief Computes logical @c xor for ranges @a r1 and @a r2.
*/
Range logicalXor(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	Range result;

	// I use here int type because in C programs, integral promotion is applied.
	// Hence, by using int type instead boolean, I cannot break anything.
	Number zero = Number(0, sizeof(int), true);
	Number one = Number(1, sizeof(int), true);

	if ((r1.containsTrue() && r2.containsTrue()) ||
		(r1.containsFalse() && r2.containsFalse())) {
		result.data.push_back(Range::Interval(zero, zero));
	}

	if ((r1.containsFalse() && r2.containsTrue()) ||
	    (r1.containsTrue() && r2.containsFalse())) {
		result.data.push_back(Range::Interval(one, one));
	}

	result.normalize();
	return result;
}

/**
* @brief Checks whether ranges are equal.
*
* @param[in] r1 The first range for comparison.
* @param[in] r2 The second range for comparison.
*
* @return @c true if ranges are equal, @c false otherwise.
*/
bool operator==(const Range &r1, const Range &r2)
{
	if (r1.size() != r2.size()) {
		// Different sizes of ranges. They are not equal.
		return false;
	}

	if (r1.size() == 0) {
		// Empty ranges must be treated separately because of assertions.
		return true;
	}

	if (!r1.containsNan() && !r1.containsNan()) {
		// If they do not contain NAN.
		return r1.data == r2.data;
	} else if (r1.containsNan() && r1.containsNan()) {
		// If they contain NAN.
		return std::equal(r1.begin() + 1, r1.end(), r2.begin() + 1);
	} else {
		// One range contain NAN. The other does not contain NAN.
		return false;
	}
}

/**
* @brief Checks whether ranges are not equal.
*
* @param[in] r1 The first range for comparison.
* @param[in] r2 The second range for comparison.
*
* @return @c true if ranges are not equal, @c false otherwise.
*/
bool operator!=(const Range &r1, const Range &r2)
{
	return r1.data != r2.data;
}

/**
* @brief Checks whether ranges @a r1 and @a r2 are equal and returns the
*        resulting range that can contain three types of intervals: [0,0],
*        [1,1] and [0,1]. [0,0] means that no two values from these two ranges
*        are equal. [0,1] means that there are values from these two ranges
*        that are equal and also values that are not equal. [1,1] means that
*        all values from these two rages are equal.
*/
Range logicalEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	// I will need this on different places to express interval [0,0].
	Number falseNum(false, sizeof(int), true);
	Range::Interval falseInt(falseNum, falseNum);

	Range result;
	if (r1.containsNan() || r2.containsNan()) {
		// If at least one range contains NAN, it is necessary to add [0,0].
		result.data.push_back(falseInt);
	}

	if ((r1.size() > 1 || r2.size() > 1) ||
	   (r1[0].second != r1[0].first) ||
	   (r2[0].second != r2[0].first)) {
		// If at least one range represents more than one point, we have to add [0,0].
		result.data.push_back(falseInt);
	}

	if ((r1.size() == 1 && r2.size() == 1) &&
	   (r1[0].second == r1[0].first) &&
	   (r2[0].second == r2[0].first) &&
	   (r1[0].first != r2[0].first)) {
		// If both ranges represent one point but these points are different.
		result.data.push_back(falseInt);
	}

	Range::const_iterator it = r1.begin();
	Range::const_iterator jt = r2.begin();
	while (it != r1.end() && jt != r2.end()) {
		// We have to find common number. If it exists, we have to add [1,1].
		if (it->first.isNotNumber()) {
			// We never compare NAN to something else because it is always false.
			++it;
			continue;
		}

		if (jt->first.isNotNumber()) {
			// We never compare NAN to something else because it is always false.
			++jt;
			continue;
		}

		if (it->second < jt->first) {
			// The first range is strictly lower than the second.
			++it;
			continue;
		}

		if (jt->second < it->first) {
			// The second interval is strictly lower than the first.
			++jt;
			continue;
		}

		// There is some overlapping of intervals in both ranges.
		// Hence, r1 and r2 have common number. We have to add [1,1].
		Number trueNum(true, sizeof(int), true);
		Range::Interval trueInt(trueNum, trueNum);
		result.data.push_back(trueInt);
		break;
	}

	result.normalize();
	return result;
}

/**
* @brief Checks whether ranges @a r1 and @a r2 are not equal and returns the
*        resulting range that can contain three types of intervals: [0,0],
*        [1,1] and [0,1]. [0,0] means that all values from these two ranges
*        are equal. [0,1] means that there are values from these two ranges
*        that are equal and also values that are not equal. [1,1] means that
*        no values from these two rages are equal.
*/
Range logicalNeq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	return logicalNot(logicalEq(r1, r2));
}

/**
* @brief Checks whether range @a r1 is lower than range @a r2 and returns the
*        resulting range that can contain three types of intervals: [0,0],
*        [1,1] and [0,1]. [0,0] means that no two values from these two
*        ranges accomplish the condition for this comparison. [0,1] means that
*        there are values from these two ranges that accomplish the condition for
*        this comparison and also values that do not accomplish the condition for
*        this comparison. [1,1] means that all values from these two ranges
*        accomplish the condition for this comparison.
*/
Range logicalLt(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	// I will need this on different places to express interval [0,0].
	Number falseNum(false, sizeof(int), true);
	Range::Interval falseInt(falseNum, falseNum);

	Range result;
	if (!r1.containsNan() && !r2.containsNan()) {
		// There is no NAN in r1 and r2.
		Number r1Min = r1[0].first;
		Number r1Max = r1[r1.size() - 1].second;
		Number r2Min = r2[0].first;
		Number r2Max = r2[r2.size() - 1].second;
		Number first = Number(r1Min < r2Max, sizeof(int), true);
		Number second = Number(r1Max < r2Min, sizeof(int), true);
		result.data.push_back(Range::Interval(first, first));
		result.data.push_back(Range::Interval(second, second));
	} else if (r1.containsNan() && r2.containsNan()) {
		// There is NAN in both Ranges. Since there is NAN, we have to add [0,0].
		result.data.push_back(falseInt);
		if ((r1.size() > 1) && (r2.size() > 1)) {
			// Both ranges have at least two intervals. There is no need to do
			// something with ranges where at least one range has only NAN because
			// if we compare NAN with anything the result is false.
			Number r1Min = r1[1].first;
			Number r1Max = r1[r1.size() - 1].second;
			Number r2Min = r2[1].first;
			Number r2Max = r2[r2.size() - 1].second;
			Number first = Number(r1Min < r2Max, sizeof(int), true);
			Number second = Number(r1Max < r2Min, sizeof(int), true);
			result.data.push_back(Range::Interval(first, first));
			result.data.push_back(Range::Interval(second, second));
		}
	} else if (r1.containsNan() && !r2.containsNan()) {
		// The first range contains NAN and the second does not contain NAN.
		// Since there is NAN, we have to add [0,0].
		result.data.push_back(falseInt);
		if (r1.size() > 1) {
			// The first range has at least one interval apart from the NAN interval.
			// For the second range, we have at least one interval - range cannot
			// be empty.
			Number r1Min = r1[1].first;
			Number r1Max = r1[r1.size() - 1].second;
			Number r2Min = r2[0].first;
			Number r2Max = r2[r2.size() - 1].second;
			Number first = Number(r1Min < r2Max, sizeof(int), true);
			Number second = Number(r1Max < r2Min, sizeof(int), true);
			result.data.push_back(Range::Interval(first, first));
			result.data.push_back(Range::Interval(second, second));
		}
	} else if (!r1.containsNan() && r2.containsNan()) {
		// The second range contains NAN and the first does not contain NAN.
		// Since there is NAN, we have to add [0,0].
		result.data.push_back(falseInt);
		if (r2.size() > 1) {
			// The second range has at least one interval apart from the NAN interval.
			// For the first range, we have at least one interval - range cannot
			// be empty.
			Number r1Min = r1[0].first;
			Number r1Max = r1[r1.size() - 1].second;
			Number r2Min = r2[1].first;
			Number r2Max = r2[r2.size() - 1].second;
			Number first = Number(r1Min < r2Max, sizeof(int), true);
			Number second = Number(r1Max < r2Min, sizeof(int), true);
			result.data.push_back(Range::Interval(first, first));
			result.data.push_back(Range::Interval(second, second));
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Checks whether range @a r1 is greater than range @a r2 and returns the
*        resulting range that can contain three types of intervals: [0,0],
*        [1,1] and [0,1]. [0,0] means that no two values from these two
*        ranges accomplish the condition for this comparison. [0,1] means that
*        there are values from these two ranges that accomplish the condition for
*        this comparison and also values that do not accomplish the condition for
*        this comparison. [1,1] means that all values from these two ranges
*        accomplish the condition for this comparison.
*/
Range logicalGt(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	return logicalLt(r2,r1);
}

/**
* @brief Checks whether range @a r1 is lower than or equal to range @a r2 and
*        returns the resulting range that can contain three types of intervals:
*        [0,0], [1,1] and [0,1]. [0,0] means that no two values from these two
*        ranges accomplish the condition for this comparison. [0,1] means that
*        there are values from these two ranges that accomplish the condition for
*        this comparison and also values that do not accomplish the condition for
*        this comparison. [1,1] means that all values from these two ranges
*        accomplish the condition for this comparison.
*/
Range logicalLtEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	Range eqRange = logicalEq(r1, r2);
	Range ltRange = logicalLt(r1, r2);
	Range result;

	if (Range(Number(0, sizeof(int), true)) == eqRange) {
		//    ltRange eqRange result
		// 1.) [0,0]  [0,0]    [0,0]
		// 2.) [0,1]  [0,0]    [0,1]
		// 3.) [1,1]  [0,0]    [1,1]
		result = ltRange;
	} else if (Range(Number(1, sizeof(int), true)) == eqRange) {
		//    ltRange eqRange result
		// 1.) [0,0]  [1,1]    [1,1]
		// 2.) [0,1]  [1,1]    [1,1]
		// 3.) [1,1]  [1,1]    [1,1]
		result = eqRange;
	} else {
		//    ltRange eqRange result
		// 1.) [0,0]  [0,1]    [0,1]
		// 2.) [0,1]  [0,1]    [0,1] or [1,1] according to situation
		// 3.) [1,1]  [0,1]    [1,1]
		if (Range(Number(0, sizeof(int), true)) == ltRange) {
			// Implements 1.
			result = eqRange;
		} else if (Range(Number(1, sizeof(int), true)) == ltRange) {
			// Implements 3.
			result = ltRange;
		} else {
			// Implements 2.
			if (r1[r1.size() - 1].second <= r2[0].first) {
				result = Range(Number(1, sizeof(int), true));
			} else {
				result = eqRange;
			}
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Checks whether range @a r1 is greater than or equal to range @a r2 and
*        returns the resulting range that can contain three types of intervals:
*        [0,0], [1,1] and [0,1]. [0,0] means that no two values from these two
*        ranges accomplish the condition for this comparison. [0,1] means that
*        there are values from these two ranges that accomplish the condition for
*        this comparison and also values that do not accomplish the condition for
*        this comparison. [1,1] means that all values from these two ranges
*        accomplish the condition for this comparison.
*/
Range logicalGtEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty range.
	assert(!r1.empty() && !r2.empty());

	Range eqRange = logicalEq(r1, r2);
	Range gtRange = logicalGt(r1, r2);
	Range result;

	if (Range(Number(0, sizeof(int), true)) == eqRange) {
		//    gtRange eqRange result
		// 1.) [0,0]  [0,0]    [0,0]
		// 2.) [0,1]  [0,0]    [0,1]
		// 3.) [1,1]  [0,0]    [1,1]
		result = gtRange;
	} else if (Range(Number(1, sizeof(int), true)) == eqRange) {
		//    gtRange eqRange result
		// 1.) [0,0]  [1,1]    [1,1]
		// 2.) [0,1]  [1,1]    [1,1]
		// 3.) [1,1]  [1,1]    [1,1]
		result = eqRange;
	} else {
		//    gtRange eqRange result
		// 1.) [0,0]  [0,1]    [0,1]
		// 2.) [0,1]  [0,1]    [0,1] or [1,1] according to situation
		// 3.) [1,1]  [0,1]    [1,1]
		if (Range(Number(0, sizeof(int), true)) == gtRange) {
			// Implements 1.
			result = eqRange;
		} else if (Range(Number(1, sizeof(int), true)) == gtRange) {
			// Implements 3.
			result = gtRange;
		} else {
			// Implements 2.
			if (r1[0].first >= r2[r2.size() - 1].second) {
				result = Range(Number(1, sizeof(int), true));
			} else {
				result = eqRange;
			}
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Returns @c true if range contains integral numbers, @c false otherwise.
*/
bool Range::isIntegral() const
{
	// I should always work with non-empty range.
	assert(!empty());
	return data[0].first.isIntegral();
}

/**
* @brief Returns @c true if range contains floating-point numbers, @c false otherwise.
*/
bool Range::isFloatingPoint() const
{
	// I should always work with non-empty range.
	assert(!empty());
	return data[0].first.isFloatingPoint();
}

/**
* @brief Returns @c true if range contains signed integral intervals, @c false
*        otherwise
*/
bool Range::isSigned() const
{
	// I should always work with non-empty range.
	assert(!empty());
	return isIntegral() && data[0].first.getSign();
}

/**
* @brief Returns @c true if range contains unsigned integral intervals, @c false
*        otherwise
*/
bool Range::isUnsigned() const
{
	// I should always work with non-empty range.
	assert(!empty());
	return isIntegral() && !data[0].first.getSign();
}

/**
* @brief Converts integral range @a r to floating-point range.
*/
Range intToFloat(const Range &r)
{
	// I should always work with non-empty range.
	assert(!r.empty());

	// I should always work with an integral range.
	assert(r.isIntegral());

	Range result;
	for (Range::const_iterator it = r.begin(); it !=r.end(); ++it) {
		Number first = intToFloat(it->first);
		Number second = intToFloat(it->second);
		result.data.push_back(Range::Interval(first, second));
	}

	result.normalize();
	return result;
}

/**
* @brief Returns a range that would resulted if @a r was assigned into the
*        current range with the respect to the type of current range and rules
*        in the C language.
*/
Range Range::assign(const Range &r) const
{
	// I should always work with non-empty ranges.
	assert(!empty() && !r.empty());

	// It is used in order to know the precise type of the range on the left side.
	// The assignment can have different properties according to this type, for example
	// solving overflows and so on.
	Number lowerOld = data[0].first;
	Number upperOld = data[0].first;

	Range result;
	if (isIntegral() && r.isIntegral()) {
		// An integral range is assigned to an integral range.
		// We need to compute scope of the type in mpz_class because of overflows.
		mpz_class scope = (lowerOld.getMax()).getInt() - (lowerOld.getMin()).getInt();
		scope = scope + 1;
		for (Range::const_iterator it = r.begin(); it != r.end(); ++it) {
			Number lowerNew = it->first;
			Number upperNew = it->second;
			mpz_class intervalScope = upperNew.getInt() - lowerNew.getInt();
			if (intervalScope >= scope) {
				// If the scope of the new interval is greater than or equal to the
				// scope of the type on the left side then maximal range is assigned.
				// We can return this maximal range. It is a correct result.
				return Range::getMaxRange(lowerOld);
			} else {
				Number l = lowerOld.assign(lowerNew);
				Number u = upperOld.assign(upperNew);
				result.data.push_back(Interval(l, u));
			}
		}
	} else if ((isFloatingPoint() && r.isFloatingPoint()) ||
			   (isFloatingPoint() && r.isIntegral())) {
		// A floating-point range is assigned to a floating-point range or
		// an integral range is assigned to a floating-point range.
		for (Range::const_iterator it = r.begin(); it != r.end(); ++it) {
			Number lowerNew = it->first;
			Number upperNew = it->second;
			Number l = lowerOld.assign(lowerNew);
			Number u = upperOld.assign(upperNew);
			result.data.push_back(Interval(l,u));
		}
	} else if (isIntegral() && r.isFloatingPoint()) {
		// A floating-point range is assigned to an integral range.
		for (Range::const_iterator it = r.begin(); it != r.end(); ++it) {
			Number lowerNew = it->first;
			Number upperNew = it->second;
			if ((lowerNew >= lowerOld.getMin()) &&
				(upperNew <= lowerOld.getMax())) {
				// Checks if new interval fits into the type of the variable on the
				// left side.
				Number l = lowerOld.assign(lowerNew);
				Number u = upperOld.assign(upperNew);
				result.data.push_back(Interval(l,u));
			} else {
				// This is not defined according to the C standard. If too big
				// floating-point number is assigned to integral number, we return
				// the maximal possible range.
				return Range::getMaxRange(lowerOld);
			}
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Expands the current range0*
* For integral ranges:
*  - Expands all the intervals in the range by 2. For example, (-5, -2)
*    becomes (-10, -1) and (6, 100) becomes (3, 200).
*  - If the range is signed and contains a zero, it introduces the
*    interval (-1, 1) to the range.
*  - If the range is unsigned and contains a zero, it introduces the interval
*    (0, 1) to the range.
*
* For floating-point ranges:
*  - Returns the maximal range.
*/
Range Range::expand() const {
	// I should always work with non-empty ranges.
	assert(!empty());

	// For floating-points, return the maximal range.
	if (isFloatingPoint()) {
		return getMaxRange(data[0].first);
	}

	// To simplify the implementation, we split the range by special values.
	Range r = splitBySpecialValues();

	Range result;

	// Expand the ranges.
	for (Range::const_iterator it = r.data.begin(); it != r.data.end(); it++) {
		const Number &x = it->first;
		const Number &y = it->second;

		Number zero = x.assign(Number(0, x.getBitWidth(), x.isSigned()));
		Number two = x.assign(Number(2, x.getBitWidth(), x.isSigned()));
		if (x < zero) {
			// The interval is composed of two negative numbers, so multiply x
			// with 2 and divide y with 2.
			Number newMin(x.assign(x * two));
			if (newMin > x) {
				// Make sure that the new minimum is not higher than the
				// original x.
				newMin = x.getMin();
			}
			Number newMax(x.assign(trunc_div(y, two)));
			result.data.push_back(Interval(newMin, newMax));
		} else {
			// The interval is composed of two positive numbers (or zero), so
			// divide x with 2 and multiply y with 2.
			Number newMin(x.assign(trunc_div(x, two)));
			Number newMax(x.assign(y * two));
			if (newMax < y) {
				// Make sure that the new maximum is not lower than the
				// original y.
				newMax = x.getMax();
			}
			result.data.push_back(Interval(newMin, newMax));
		}
	}

	// Handle the presence of 0.
	if (result.containsZero()) {
		if (result.isSigned()) {
			result.data.push_back(Interval(
				Number(-1, result[0].first.getBitWidth(), true),
				Number(1, result[0].first.getBitWidth(), true)));
		} else { // result.isUnsigned()
			result.data.push_back(Interval(
				Number(1, result[0].first.getBitWidth(), false),
				Number(1, result[0].first.getBitWidth(), false)));
		}
	}

	// We have to normalize the result before returning (there will be
	// intervals that can be merged).
	result.normalize();

	return result;
}

/**
* @brief Merges the intervals in the range.
*
* If the range is integral, the returned range is of the form
* @code
* (min, max)
* @endcode
* where @c min is the minimal number from the original range and @c max is the
* maximal number from the original range.
*
* If the range is floating-point, the returned range may have an additional
* range of the form
* @code
* (NAN, NAN)
* @endcode
* if the original range contained @c NAN.
*/
Range Range::mergeIntervals() const {
	Range copy(*this);
	copy.mergeIntervalsInPlace();
	return copy;
}

/**
* @brief Unary minus of the range.
*
* @param[in] r The first range.
*
* @return Unary minus of @a r.
*/
Range operator-(const Range &r)
{
	// I should always work with non-empty ranges.
	assert(!r.empty());

	Range result;

	for (Range::const_iterator it = r.data.begin(); it != r.data.end(); it++) {
		if (it->first.isIntegral()) {
			// Handles unary minus of signed and unsigned integral ranges.
			if (it->first.isMin()) {
				// Special care for minimal value because of asymmetry of
				// integral numbers.
				if (it->second.isMin()) {
					// -(min, min) = (min, min)
					result.data.push_back(Interval(it->first, it->second));
				} else {
					result.data.push_back(Interval(it->first, it->first));
					// I need to add 'one' to one border of the interval but it
					// must be the same type as the type of this border is.
					Number intOne(1, sizeof(int), true);
					Number one((it->first).assign(intOne));
					Interval tmp = Interval(-it->second, -(it->first + one));
					result.data.push_back(Interval(tmp));
				}
			} else {
				result.data.push_back(Interval(-it->second, -it->first));
			}
		} else {
			// Handles unary minus of floating-point ranges.
			result.data.push_back(Interval(-it->second, -it->first));
		}
	}

	result. normalize();
	return result;
}

/**
* @brief Performs an addition of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the addition of @a r1 a @a r2.
*/
Range operator+(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	Range result;

	for (Range::const_iterator it = r1.begin(); it != r1.end(); it++) {
		for (Range::const_iterator jt = r2.begin(); jt != r2.end(); jt++) {
			// Consider two intervals (x, y) + (z, w). Interesting computations
			// are (x+z, y+w), (x+z, x+w), (y+z, y+w), (z+x, z+y) and (w+x, w+y).
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;
			Interval tmp = Interval(x + z, y + w);
			result.data.push_back(tmp);
			tmp = Interval(x + z, x + w);
			result.data.push_back(tmp);
			tmp = Interval(y + z, y + w);
			result.data.push_back(tmp);
			tmp = Interval(z + x, z + y);
			result.data.push_back(tmp);
			tmp = Interval(w + x, w + y);
			result.data.push_back(tmp);
			// It is not necessary to include (x+w, y+z) because it is included in
			// (x+z, y+w).
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs a subtraction of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the subtraction of @a r1 a @a r2.
*/
Range operator-(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	Range result;

	for (Range::const_iterator it = r1.begin(); it != r1.end(); it++) {
		for (Range::const_iterator jt = r2.begin(); jt != r2.end(); jt++) {
			// Consider two intervals: (x, y) - (z, w): Interesting computations
			// are only (x-z, y-z), (x-w, y-w) and (x-w, y-z). A computation
			// (x-z, y-w) is contained in the (x-w, y-z) and it causes a problem
			// than lower bound can be in the position of the upper bound that
			// is not recognizable because of overflows (in some cases it could
			// be desirable).
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;
			Interval tmp = Interval(x - z, y - z);
			result.data.push_back(tmp);
			tmp = Interval(x - w, y - z);
			result.data.push_back(tmp);
			tmp = Interval(x - w, y - w);
			result.data.push_back(tmp);
			// Needed computations are also (x-z, x-z), (x-w, x-w), (y-z, y-z) and
			// (y-w, y-w). It really must be here, do not comment it. You can try it
			// and then execute tests for this module.
			tmp = Interval(x-z, x-z);
			result.data.push_back(tmp);
			tmp = Interval(x-w, x-w);
			result.data.push_back(tmp);
			tmp = Interval(y-z, y-z);
			result.data.push_back(tmp);
			tmp = Interval(y-w, y-w);
			result.data.push_back(tmp);
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs a multiplication of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the multiplication of @a r1 and @a r2.
*/
Range operator*(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// To make computation easier, we split the given ranges by special values.
	Range r1s = r1.splitBySpecialValues();
	Range r2s = r2.splitBySpecialValues();

	Range result;

	if (r1s.isIntegral() && r2s.isIntegral()) {
		// If both ranges are integral, a different approach must be used because
		// overflows can cause problems. Consider that we have two ranges: the
		// first is (2,10) and the second is (255,255). If we use standard
		// approach, we get (254,246). After normalizing, this result is (0,246)
		// and (254,255). However, the correct result should be (246,254).
		for (Range::const_iterator it = r1s.begin(); it != r1s.end(); it++) {
			for (Range::const_iterator jt = r2s.begin(); jt != r2s.end(); jt++) {
				const mpz_class &x = it->first.getInt();
				const mpz_class &y = it->second.getInt();
				const mpz_class &z = jt->first.getInt();
				const mpz_class &w = jt->second.getInt();

				mpz_class limit1, limit2;
				if ((x >= 0) && (y >= 0) && (z >= 0) && (w >= 0)) {
					// Multiplication of positive intervals.
					limit1 = x * z;
					limit2 = y * w;
				} else if ((x >= 0) && (y >= 0) && (z <= 0) && (w <= 0)) {
					// Multiplication of the positive interval with the negative one.
					limit1 = y * z;
					limit2 = x * w;
				} else if ((x <= 0) && (y <= 0) && (z >= 0) && (w >= 0)) {
					// Multiplication of the negative interval with the positive one.
					limit1 = x * w;
					limit2 = y * z;
				} else if ((x <= 0) && (y <= 0) && (z <= 0) && (w <= 0)) {
					// Multiplication of negative intervals.
					limit1 = y * w;
					limit2 = x * z;
				}

				// We need to know the actual type and scope of the result.
				Number info = it->first * jt->first;
				mpz_class scope = info.getMax().getInt() - info.getMin().getInt() + 1;
				if ((limit2 - limit1) >= scope) {
					// If the difference between computed limits is greater or equal
					// to the scope of the result's type, we return maximal range.
					return Range::getMaxRange(info);
				} else {
					// Otherwise, we have to convert mzp_class to Number and create
					// interval.
					Number l1 = Number(limit1, info.getBitWidth(), info.getSign());
					Number l2 = Number(limit2, info.getBitWidth(), info.getSign());
					result.data.push_back(Interval(l1, l2));
				}
			}
		}
	} else {
		// If at least one of the ranges is floating-point type, the result will
		// be also floating-point. After the extension, both ranges will be
		// floating-point type. So, no overflow problems will be here.
		for (Range::const_iterator it = r1s.begin(); it != r1s.end(); it++) {
			for (Range::const_iterator jt = r2s.begin(); jt != r2s.end(); jt++) {
				const Number &x = it->first;
				const Number &y = it->second;
				const Number &z = jt->first;
				const Number &w = jt->second;
				Number l1 = x * z;
				Number l2 = x * w;
				Number l3 = y * z;
				Number l4 = y * w;
				// Firstly, I thought that there is a problem because of usage
				// min() and max(). In C is weird behaviour when I compare
				// signed and unsigned variables. However, here it is used only
				// for floats. So, it is all right.
				Number lower = min(min(l1, l2), min(l3, l4));
				Number upper = max(max(l1, l2), max(l3, l4));
				result.data.push_back(Interval(lower, upper));
			}
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs a real division of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the real division of @a r1 and @a r2.
*/
Range rdiv(const Range &r1, const Range &r2)
{
	// This division works only for floating-point ranges.
	assert(r1.isFloatingPoint() && r2.isFloatingPoint());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// To simplify the computation, we
	// (1) extend them by C rules,
	std::pair<Range, Range> extR = Range::extensionByCRules(r1, r2);
	// (2) split them by special values, and
	Range r1s = extR.first.splitBySpecialValues();
	Range r2s = extR.second.splitBySpecialValues();
	// (3) split the ranges by 0.0.
	r1s = r1s.splitByZero();
	r2s = r2s.splitByZero();

	Range result;

	for (Range::const_iterator it = r1s.begin(); it != r1s.end(); it++) {
		for (Range::const_iterator jt = r2s.begin(); jt != r2s.end(); jt++) {
			// Consider two intervals (x, y) / (z, w).
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;

			Number xw = rdiv(x, w);
			Number xz = rdiv(x, z);
			result.data.push_back(Interval(std::min(xw, xz), std::max(xw, xz)));

			Number yw = rdiv(y, w);
			Number yz = rdiv(y, z);
			result.data.push_back(Interval(std::min(yw, yz), std::max(yw, yz)));
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs an integral division operation over @a r1 and @a r2.
*
* @param[in] r1 The first range.
* @param[in] r2 The second range.
* @param[in] exact If @c true, do @c exact_div; otherwise, do @c trunc_div.
*
* @return The range representing the division of @a r1 and @a r2.
*/
Range Range::performIntegralDivOp(const Range &r1, const Range &r2, bool exact)
{
	// This division works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// If there is a possibility of dividing by zero, over approximate.
	if (r2.containsZero()) {
		return overApproximateBinaryOp(r1, r2);
	}

	// To simplify the computation, we
	// (1) extend them by C rules, and
	std::pair<Range, Range> extR = Range::extensionByCRules(r1, r2);
	// (2) split the given ranges by special values.
	Range r1s = extR.first.splitBySpecialValues();
	Range r2s = extR.second.splitBySpecialValues();

	// If there is a possibility of dividing INT_MIN by -1, over approximate.
	// The reason is that in C, INT_MIN / -1 has undefined behaviour.
	if (r1s.isIntegral() && r1s.isSigned() && r1s.containsIntegralMin() &&
			r2s.containsIntegralMinusOne()) {
		return overApproximateBinaryOp(r1s, r2s);
	}

	Range result;

	for (Range::const_iterator it = r1s.begin(); it != r1s.end(); it++) {
		for (Range::const_iterator jt = r2s.begin(); jt != r2s.end(); jt++) {
			// Consider two intervals (x, y) / (z, w).
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;

			Number xw = exact ? exact_div(x, w) : trunc_div(x, w);
			Number xz = exact ? exact_div(x, z) : trunc_div(x, z);
			result.data.push_back(Interval(std::min(xw, xz), std::max(xw, xz)));

			Number yw = exact ? exact_div(y, w) : trunc_div(y, w);
			Number yz = exact ? exact_div(y, z) : trunc_div(y, z);
			result.data.push_back(Interval(std::min(yw, yz), std::max(yw, yz)));
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs an exact division of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the exact division of @a r1 and @a r2.
*/
Range exact_div(const Range &r1, const Range &r2)
{
	return Range::performIntegralDivOp(r1, r2, true);
}

/**
* @brief Performs a trunc division of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the trunc division of @a r1 and @a r2.
*/
Range trunc_div(const Range &r1, const Range &r2)
{
	return Range::performIntegralDivOp(r1, r2, false);
}

/**
* @brief Performs a modulo of two ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the modulo of @a r1 and @a r2.
*/
Range trunc_mod(const Range &r1, const Range &r2)
{
	// This modulo works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// If there is a possibility of modulo by zero, over approximate.
	if (r2.containsZero()) {
		return overApproximateBinaryOp(r1, r2);
	}

	// To simplify the computation, we
	// (1) extend them by C rules, and
	std::pair<Range, Range> extR = Range::extensionByCRules(r1, r2);
	// (2) split the given ranges by special values and by zero.
	Range r1s = extR.first.splitBySpecialValues().splitByZero();
	Range r2s = extR.second.splitBySpecialValues().splitByZero();

	// If there is a possibility of INT_MIN % -1, over approximate.
	// The reason is that in C, INT_MIN % -1 has undefined behaviour.
	if (r1s.isIntegral() && r1s.isSigned() && r1s.containsIntegralMin() &&
			r2s.containsIntegralMinusOne()) {
		return overApproximateBinaryOp(r1s, r2s);
	}

	Range result;

	// We cannot compute the result in the analogous way to other operations,
	// meaning that for (x, y) % (z, w), we comute x % y, x % z, y % z, and y %
	// w, and use these results. Indeed, for example, (-5, -5) % (-5, -1)
	// should be (-4, 0), but both -5 % -5 and -5 % -1 equal 0.
	//
	// Therefore, what we do, for (x, y) % (z, w), we use the interval (x, 0)
	// (if x < 0) or (0, y) (if x >= 0). This is a slight over approximation.
	// However, it is correct and easier to compute than the precise result.
	//
	// In this way, it suffices to go over just the first range. Moreover, we
	// use the fact that when computing x % y, the sign of the result is the
	// sign of x.
	for (Range::const_iterator it = r1s.begin(); it != r1s.end(); it++) {
		const Number &x = it->first;
		const Number &y = it->second;

		Number zero = x.assign(Number(0, x.getBitWidth(), x.isSigned()));
		if (x < zero) {
			// Use (x, 0).
			result.data.push_back(Interval(x, zero));
		} else { // x >= zero
			// Use (0, y).
			result.data.push_back(Interval(zero, y));
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Performs a not of a given range. Now, it only returns the maximal range. I
*        do not see any pattern how to perform this operation on range.
* @param[in] r The first range.
*
* @return The range representing the bit not of @a r.
*/
Range bitNot(const Range &r)
{
	// This operation works only for integral ranges.
	assert(r.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r.empty());

	// Currently, we compute a precise result only if the range contains just a
	// single number.
	if (!r.containsOnlySingleNumber()) {
		return overApproximateUnaryOp(r);
	}

	// Compute a precise result.
	return Range(bitNot(r[0].first));
}

/**
* @brief Performs an and of two ranges. Now, it only returns the maximal range. I
*        do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the and of @a r1 and @a r2.
*/
Range bitAnd(const Range &r1, const Range &r2)
{
	// This operation works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Currently, we compute a precise result only if the ranges contain just a
	// single number.
	if (!r1.containsOnlySingleNumber() || !r2.containsOnlySingleNumber()) {
		// Over approximate.
		return overApproximateBinaryOp(r1, r2);
	}

	// Compute a precise result.
	return Range(bitAnd(r1[0].first, r2[0].first));
}

/**
* @brief Performs an inclusive or of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the ior of @a r1 and @a r2.
*/
Range bitOr(const Range &r1, const Range &r2)
{
	// This operation works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Currently, we compute a precise result only if the ranges contain just a
	// single number.
	if (!r1.containsOnlySingleNumber() || !r2.containsOnlySingleNumber()) {
		// Over approximate.
		return overApproximateBinaryOp(r1, r2);
	}

	// Compute a precise result.
	return Range(bitOr(r1[0].first, r2[0].first));
}

/**
* @brief Performs an exclusive or of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the xor of @a r1 and @a r2.
*/
Range bitXor(const Range &r1, const Range &r2)
{
	// This operation works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Currently, we compute a precise result only if the ranges contain just a
	// single number.
	if (!r1.containsOnlySingleNumber() || !r2.containsOnlySingleNumber()) {
		// Over approximate.
		return overApproximateBinaryOp(r1, r2);
	}

	// Compute a precise result.
	return Range(bitXor(r1[0].first, r2[0].first));
}

/**
* @brief Performs a left shift of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the left shift of @a r1 and @a r2.
*/
Range bitLeftShift(const Range &r1, const Range &r2)
{
	// This operation works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Currently, we compute a precise result only if the ranges contain just a
	// single number.
	if (!r1.containsOnlySingleNumber() || !r2.containsOnlySingleNumber()) {
		// Over approximate.
		return overApproximateBinaryOp(r1, r2);
	}

	// If we are shifting more bits than the bit width of the left-hand side,
	// like in `a << 33`, where `a` is a 32b integer, the result is undefined.
	// Therefore, we have to over approximate in such a case.
	if (r2[0].first > Number(r1[0].first.getNumOfBits(), r2[0].first.getBitWidth(), true)) {
		return overApproximateBinaryOp(r1, r2);
	}

	// Compute a precise result.
	return Range(bitLeftShift(r1[0].first, r2[0].first));
}

/**
* @brief Performs a right shift of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the right shift of @a r1 and @a r2.
*/
Range bitRightShift(const Range &r1, const Range &r2)
{
	// This operation works only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Currently, we compute a precise result only if the ranges contain just a
	// single number.
	if (!r1.containsOnlySingleNumber() || !r2.containsOnlySingleNumber()) {
		// Over approximate.
		return overApproximateBinaryOp(r1, r2);
	}

	// If we are shifting more bits than the bit width of the left-hand side,
	// like in `a >> 33`, where `a` is a 32b integer, the result is undefined.
	// Therefore, we have to over approximate in such a case.
	if (r2[0].first > Number(r1[0].first.getNumOfBits(), r2[0].first.getBitWidth(), true)) {
		return overApproximateBinaryOp(r1, r2);
	}

	// Compute a precise result.
	return Range(bitRightShift(r1[0].first, r2[0].first));
}

/**
* @brief Performs a left rotate of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the left rotate of @a r1 and @a r2.
*/
Range bitLeftRotate(const Range &r1, const Range &r2)
{
	// Currently, we just over approximate the result.
	return overApproximateBinaryOp(r1, r2);
}

/**
* @brief Performs a right rotate of two ranges. Now, it only returns the maximal
*        range. I do not see any pattern how to perform this operation on ranges.
* @param[in] r1 The first range.
* @param[in] r2 The second range.
*
* @return The range representing the right rotate of @a r1 and @a r2.
*/
Range bitRightRotate(const Range &r1, const Range &r2)
{
	// Currently, we just over approximate the result.
	return overApproximateBinaryOp(r1, r2);
}

/**
* @brief Over-approximates the result of unary operations, like not.
*/
Range overApproximateUnaryOp(const Range &r)
{
	// I should always work with non-empty ranges.
	assert(!r.empty());

	// Bit operations work only for integral ranges.
	assert(r.isIntegral());

	// We need to promote the range to get a proper result. We do this by
	// extending the range r with itself.
	std::pair<Range, Range> promR = Range::extensionByCRules(r, r);
	return Range::getMaxRange(promR.first[0].first);
}

/**
* @brief Over-approximates the result of binary operations, like aditions,
*        shifts and so on.
*/
Range overApproximateBinaryOp(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// Bit operations work only for integral ranges.
	assert(r1.isIntegral() && r2.isIntegral());

	// We need to extend the ranges to get a proper result.
	std::pair<Range, Range> extR = Range::extensionByCRules(r1, r2);
	return Range::getMaxRange(extR.first[0].first);
}

/**
* @brief Computes the absolute value of the given range @a r.
*/
Range abs(const Range &r)
{
	// I should always work with non-empty range.
	assert(!r.empty());

	// Abs instruction is generated only for signed integral ranges.
	assert(r.isIntegral() && r.isSigned());

	Range rs = r.splitBySpecialValues();
	Range result;
	for (Range::const_iterator it = rs.begin(); it != rs.end(); ++it) {
		Number absFirst = abs(it->first);
		Number absSecond = abs(it->second);
		if (absFirst <= absSecond) {
			result.data.push_back(Range::Interval(absFirst, absSecond));
		} else {
			result.data.push_back(Range::Interval(absSecond, absFirst));
		}
	}

	result.normalize();
	return result;
}

/**
* @brief Computes the minimal range from the given ranges @a r1 and @a r2.
*
* Preconditions:
*  - Since Code Listener does not generate min instruction for floating-point types
*    this function does not cover the behaviour if the interval (NAN, NAN) is in
*    the range. However, otherwise it works correctly even for floating-point types.
*/
Range min(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// We want ranges to have the same type. Otherwise, we have problems with
	// signed and unsigned integral ranges.
	std::pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1s = r.first;
	Range r2s = r.second;

	Range result;
	Range::const_iterator it = r1s.begin();
	Range::const_iterator jt = r2s.begin();

	while (it != r1s.end() && jt != r2s.end()) {
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;

			if (x <= z && y <= w) {
				// The first interval is minimal. It also covers the possibility if
				// the first interval overlaps the second one or intervals are same.
				result.data.push_back(Range::Interval(x, y));
				++it;
			} else if (z <= x && w <= y) {
				// The second interval is minimal. It also covers the possibility if
				// the second interval overlaps the first one or intervals are same.
				result.data.push_back(Range::Interval(z, w));
				++jt;
			} else {
				// If one from intervals is nested in the other one.
				Number lower = min(x, z);
				Number upper = min(y, w);
				result.data.push_back(Range::Interval(lower, upper));
				if (y < w) {
					++it;
				} else {
					++jt;
				}
			}
	}
	result.normalize();
	return result;
}

/**
* @brief Computes the maximal range from the given ranges @a r1 and @a r2.
*
* Preconditions:
*  - Since Code Listener does not generate max instruction for floating-point types
*    this function does not cover the behaviour if the interval (NAN, NAN) is in
*    the range. However, otherwise it works correctly even for floating-point types.
*/
Range max(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// We want ranges to have the same type. Otherwise, we have problems with
	// signed and unsigned integral ranges.
	std::pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1s = r.first;
	Range r2s = r.second;

	Range result;
	Range::const_reverse_iterator it = r1s.rbegin();
	Range::const_reverse_iterator jt = r2s.rbegin();

	while (it != r1s.rend() && jt != r2s.rend()) {
			const Number &x = it->first;
			const Number &y = it->second;
			const Number &z = jt->first;
			const Number &w = jt->second;

			if (x >= z && y >= w) {
				// The first interval is maximal. It also covers the possibility if
				// the first interval overlaps the second one or intervals are same.
				result.data.push_back(Range::Interval(x, y));
				++it;
			} else if (z >= x && w >= y) {
				// The second interval is maximal. It also covers the possibility if
				// the second interval overlaps the first one or intervals are same.
				result.data.push_back(Range::Interval(z, w));
				++jt;
			} else {
				// If one from intervals is nested in the other one.
				Number lower = max(x, z);
				Number upper = max(y, w);
				result.data.push_back(Range::Interval(lower, upper));
				if (x < z) {
					++jt;
				} else {
					++it;
				}
			}
	}
	result.normalize();
	return result;
}

/**
* @brief Returns the union of the given ranges @a r1 and @a r2.
*
* Preconditions:
*  - This should be used only for @a r1 and @a r2 that have the same type.
*  - It could be used for empty ranges, too.
*/
Range unite(const Range& r1, const Range& r2)
{
	Range result;
	for (Range::const_iterator it = r1.begin(); it != r1.end(); ++it)
		result.data.push_back(*it);

	for (Range::const_iterator it = r2.begin(); it != r2.end(); ++it)
		result.data.push_back(*it);

	result.normalize();
	return result;
}

/**
* @brief Returns the range representing the intersection of @a r1 and @a r2. It is
*        worth to note that (NAN, NAN) is never covered in the result because NAN
*        is not equal to anything (even NAN).
*
* Preconditions:
*  - Both ranges have the same type.
*/
Range intersect(const Range& r1, const Range& r2)
{
	if (r1.empty() || r2.empty())
		return Range();

	assert(r1.hasSameTypeAs(r2));

	Range result;

	for (Range::const_iterator it = r1.begin(); it != r1.end(); ++it) {
		for (Range::const_iterator jt = r2.begin(); jt != r2.end(); ++jt) {
			if (it->first >= jt->first && it->second <= jt->second) {
				// Interval from the r1 range is nested in interval from r2.
				Interval tmp = Interval(it->first, it->second);
				result.data.push_back(tmp);
			} else if (jt->first >= it->first && jt->second <= it->second) {
				// Interval from the r2 range is nested in interval from r1.
				Interval tmp = Interval(jt->first, jt->second);
				result.data.push_back(tmp);
			} else if (it->second == jt->first) {
				// The upper bound of the interval from the range r1 is also the
				// lower bound for interval from the range r2.
				Interval tmp = Interval(it->second, it->second);
				result.data.push_back(tmp);
			} else if (jt->second == it->first) {
				// The upper bound of the interval from the range r2 is also the
				// lower bound for interval from the range r1.
				Interval tmp = Interval(jt->second, jt->second);
				result.data.push_back(tmp);
			} else if (jt->first < it->second && jt->second > it->second) {
				// The interval from the range r1 is overlapped by the interval from
				// the range r2.
				Interval tmp = Interval(jt->first, it->second);
				result.data.push_back(tmp);
			} else if (it->first < jt->second && it->second > jt->second) {
				// The interval from the range r2 is overlapped by the interval from
				// the range r1.
				Interval tmp = Interval(it->first, jt->second);
				result.data.push_back(tmp);
			}
		}
	}
	result.normalize();
	return result;
}

/**
* @brief Computes the new range that is valid after the comparison for equality
*        of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call this function with swapped
*        parameters. The result cannot be empty range. In this case,
*        over-approximation is done.
*/
Range computeRangeForEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1Extend = r.first;
	Range r2Extend = r.second;
	Range result = intersect(r1Extend, r2Extend);

	if (!result.empty()) {
		// We need to keep the right type of the range, so we use assign to do it
		// automatically for us.
		result = r1.assign(result);
		result.normalize();
		return result;
	} else {
		// The result is empty, so we have to over-approximate.
		return r1.assign(r2);
	}
}

/**
* @brief Computes the new range that is valid after the comparison for non-equality
*        of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call this function with swapped
*        parameters.
*/
Range computeRangeForNeq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// This is correct result in all cases. In the case that the range r1 and r2
	// contains only one same number, the result is over-approximation because
	// the result should be an empty range. However, this is not desirable.
	return r1;
}

/**
* @brief Computes the new range that is valid after the comparison (greater than)
*        of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call opposite function (less than)
*        with swapped parameters. The result cannot be empty range.
*/
Range computeRangeForGt(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// After extension, both ranges have same type.
	pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1E = r.first;
	Range r2E = r.second;

	Range result;
	Number minFromR2 = r2E.getMin();

	for (Range::const_iterator it = r1E.begin(); it != r1E.end(); ++it) {
		const Number &x = it->first;
		const Number &y = it->second;

		if (x.isNotNumber() && y.isNotNumber()) {
			// We have to take care of the (NAN, NAN) interval separately because
			// it always returns false when compared to other numbers (even NAN).
			continue;
		}

		if (x > minFromR2 && y > minFromR2) {
			// If the whole r1 interval is greater than than minimal value from
			// the range r2.
			result.data.push_back(Range::Interval(x, y));
		} else if (x <= minFromR2 && y > minFromR2) {
			// If the part of the r1 interval is less than than minimal value
			// from the range r2.
			result.data.push_back(Range::Interval(minFromR2 +
				minFromR2.getEpsilon(), y));
		}
	}

	if (!result.empty()) {
		// We need to keep the right type of the range, so we use assign to do it
		// automatically for us.
		result = r1.assign(result);
		result.normalize();
		return result;
	} else {
		// The result is empty, so we have to over-approximate. We return the
		// interval (min from r2 + epsilon, max of r1's type).
		Number r2Min = r1[0].first.assign(r2E[0].first);
		Number r1Epsilon = r1[0].first.getEpsilon();
		if (r1.isIntegral()) {
			// When the min from r2 is the maximum of r1, use it instead of
			// adding one.
			Number r1Max = r1[0].first.getMax();
			Number newMin = r2Min.isMax() ? r1Max : r2Min + r1Epsilon;
			return Range(Interval(newMin, r1Max));
		} else { // r1.isFloatingPoint()
			// The maximal value for floating-points is INFINITY.
			return Range(Interval(r2Min + r1Epsilon, r1[0].first.getPositiveInf()));
		}
	}
}

/**
* @brief Computes the new range that is valid after the comparison (less than)
*        of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call opposite function (greater than)
*        with swapped parameters. The result cannot be empty range.
*/
Range computeRangeForLt(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// After extension, both ranges have same type.
	pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1E = r.first;
	Range r2E = r.second;

	Range result;
	Number maxFromR2 = r2E.getMax();

	for (Range::const_iterator it = r1E.begin(); it != r1E.end(); ++it) {
		const Number &x = it->first;
		const Number &y = it->second;

		if (x.isNotNumber() && y.isNotNumber()) {
			// We have to take care of the (NAN, NAN) interval separately because
			// it always returns false when compared to other numbers (even NAN).
			continue;
		}

		if (x < maxFromR2 && y < maxFromR2) {
			// If the whole r1 interval is less than than maximal value from
			// the range r2.
			result.data.push_back(Range::Interval(x, y));
		} else if (x < maxFromR2 && y >= maxFromR2) {
			// If the part of the r1 interval is less than than maximal value
			// from the range r2.
			result.data.push_back(Range::Interval(x, maxFromR2 - y.getEpsilon()));
		} else {
			// The current r1 interval is bigger than maximal value from r2.
			break;
		}
	}

	if (!result.empty()) {
		// We need to keep the right type of the range, so we use assign to do it
		// automatically for us.
		result = r1.assign(result);
		result.normalize();
		return result;
	} else {
		// The result is empty, so we have to over-approximate. We return the
		// interval (min of r1's type, max from r2 - epsilon).
		Number r2Max = r1[0].first.assign(r2E[r2.size() - 1].second);
		Number r1Epsilon = r1[0].first.getEpsilon();
		if (r1.isIntegral()) {
			// When the max from r2 is the minimum of r1, use it instead of
			// subtracting one.
			Number r1Min = r1[0].first.getMin();
			Number newMax = r2Max.isMin() ? r1Min : r2Max - r1Epsilon;
			return Range(Interval(r1Min, newMax));
		} else { // r1.isFloatingPoint()
			// The minimal value for floating-points is -INFINITY.
			return Range(Interval(r1[0].first.getNegativeInf(),
				r2Max - r1Epsilon));
		}
	}
}

/**
* @brief Computes the new range that is valid after the comparison (greater than or
*        equal) of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call opposite function (less than or
*        equal) with swapped parameters. The result cannot be empty range.
*/
Range computeRangeForGtEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// After extension, both ranges have same type.
	pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1E = r.first;
	Range r2E = r.second;

	Range result;
	Number minFromR2 = r2E.getMin();

	for (Range::const_iterator it = r1E.begin(); it != r1E.end(); ++it) {
		const Number &x = it->first;
		const Number &y = it->second;

		if (x.isNotNumber() && y.isNotNumber()) {
			// We have to take care of the (NAN, NAN) interval separately because
			// it always returns false when compared to other numbers (even NAN).
			continue;
		}

		if (x >= minFromR2 && y >= minFromR2) {
			// If the whole r1 interval is greater than or equal than minimal value
			// from the range r2.
			result.data.push_back(Range::Interval(x, y));
		} else if (x < minFromR2 && y >= minFromR2) {
			// If the part of the r1 interval is less than or equal than maximal
			// value from the range r2.
			result.data.push_back(Range::Interval(minFromR2, y));
		}
	}

	if (!result.empty()) {
		// We need to keep the right type of the range, so we use assign to do it
		// automatically for us.
		result = r1.assign(result);
		result.normalize();
		return result;
	} else {
		// The result is empty, so we have to over-approximate. We return the
		// interval (min from r2, max of r1's type).
		Number r2Min = r1[0].first.assign(r2E[0].first);
		if (r1.isIntegral()) {
			return Range(Interval(r2Min, r1[0].first.getMax()));
		} else { // r1.isFloatingPoint()
			// The maximal value for floating-points is INFINITY.
			return Range(Interval(r2Min, r1[0].first.getPositiveInf()));
		}
	}
}

/**
* @brief Computes the new range that is valid after the comparison (less than or
*        equal) of the given two ranges (@a r1 and @a r2) is evaluated as true. The
*        resulted range is valid for the range @a r1. To compute new range for
*        the range @a r2, it is necessary to call opposite function (greater than
*        or equal) with swapped parameters. The result cannot be empty range.
*/
Range computeRangeForLtEq(const Range &r1, const Range &r2)
{
	// I should always work with non-empty ranges.
	assert(!r1.empty() && !r2.empty());

	// After extension, both ranges have same type.
	pair<Range, Range> r = Range::extensionByCRules(r1, r2);
	Range r1E = r.first;
	Range r2E = r.second;

	Range result;
	Number maxFromR2 = r2E.getMax();

	for (Range::const_iterator it = r1E.begin(); it != r1E.end(); ++it) {
		const Number &x = it->first;
		const Number &y = it->second;

		if (x.isNotNumber() && y.isNotNumber()) {
			// We have to take care of the (NAN, NAN) interval separately because
			// it always returns false when compared to other numbers (even NAN).
			continue;
		}

		if (x <= maxFromR2 && y <= maxFromR2) {
			// If the whole r1 interval is less than or equal than maximal value
			// from the range r2.
			result.data.push_back(Range::Interval(x, y));
		} else if (x <= maxFromR2 && y > maxFromR2) {
			// If the part of the r1 interval is less than or equal than maximal
			// value from the range r2.
			result.data.push_back(Range::Interval(x, maxFromR2));
		} else {
			// The current r1 interval is bigger than maximal value from r2.
			break;
		}
	}

	if (!result.empty()) {
		// We need to keep the right type of the range, so we use assign to do it
		// automatically for us.
		result = r1.assign(result);
		result.normalize();
		return result;
	} else {
		// The result is empty, so we have to over-approximate. We return the
		// interval (min of r1's type, max from r2).
		Number r2Max = r1[0].first.assign(r2E[r2.size() - 1].second);
		if (r1.isIntegral()) {
			return Range(Interval(r1[0].first.getMin(), r2Max));
		} else { // r1.isFloatingPoint()
			// The minimal value for floating-points is -INFINITY.
			return Range(Interval(r1[0].first.getNegativeInf(), r2Max));
		}
	}
}

/**
* @brief Emits @a r into @a os.
*/
std::ostream& operator<<(std::ostream& os, const Range& r)
{
	os << "{ ";
	for (Range::const_iterator it = r.begin(); it != r.end(); it++) {
		os << "(" << it->first << ", " << it->second << ")";
		if (it + 1 != r.end())
			os << ", ";
	}
	os << " }" << endl;

	return os;
}
