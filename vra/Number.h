/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   Number.h
* @brief  Class that represents a number that can be integral or floating-point
*         type.
* @date   2012
*/

#ifndef GUARD_NUMBER_H
#define GUARD_NUMBER_H

#include <string>
#include <ostream>
#include <utility>
#include <gmpxx.h>

/**
* @brief Class that represents a number that can be integral or floating-point type.
*
* This class encapsulates an integral or floating-point number. It defines common
* arithmetic, logical and bitwise operations and according to the specified bit
* width of the number solves the overflowing. So, instance of this class can represent
* all C-language types and can simulate the behaviour of these types. It also takes
* care of integral promotions and type extensions according to the C rules.
*/
class Number {
	private:
		/// Enumeration is used to distinguish the type of the stored number.
		enum Type {
			INT,
			FLOAT
		};

		/// Biggest integer.
		typedef mpz_class Int;

		/// Biggest float.
		typedef long double Float;

		/// Type of the stored number.
		Type type;

		/// Value of the number if @c type of the number is @c INT.
		Int intValue;

		/// Value of the number if @c type of the number is @c FLOAT.
		Float floatValue;

		/// Is the integer signed? This data item makes sense only if @c type
		/// of the number is @c INT.
		bool sign;

		/// Bit width of the represented number.
		unsigned bitWidth;

		/// Minimal value that can be stored in the number. It is used only if
		/// @c type of the number is @c INT.
		Int minIntLimit;

		/// Maximal value that can be stored in the number. It is used only if
		/// @c type of the number is @c INT.
		Int maxIntLimit;

		/// Minimal value that can be stored in the number. It is used only if
		/// @c type of the number is @c FLOAT.
		Float minFloatLimit;

		/// Maximal value that can be stored in the number. It is used only if
		/// @c type of the number is @c FLOAT.
		Float maxFloatLimit;

		void setIntLimits();
		void setFloatLimits();
		void fitIntoBitWidth();
		void integralPromotion();
		void convertSignedToUnsigned();

		static Number performTrunc(const Number &op1, const Number &op2, bool isMod);
		static Number performBitOp(const Number &op1, const Number &op2, char mode);
		static Number performShift(Number op1, Number op2, bool isLeft);

	public:
		Number(Int value, unsigned width, bool sign);
		Number(Float value, unsigned width);

		Number assign(const Number &n) const;

		bool isIntegral() const;
		bool isFloatingPoint() const;
		bool isSigned() const;
		bool isUnsigned() const;
		bool isInf() const;
		bool isNegativeInf() const;
		bool isPositiveInf() const;
		bool isNotNumber() const;
		bool isNumber() const;
		bool isMin() const;
		bool isMax() const;
		bool toBool() const;
		Number getEpsilon() const;
		Number getMin() const;
		Number getMax() const;
		Number getNan() const;
		Number getPositiveInf() const;
		Number getNegativeInf() const;
		Int getInt() const;
		Float getFloat() const;
		unsigned getBitWidth() const;
		unsigned getNumOfBits() const;
		bool getSign() const;

		static Int floatToInt(const Float &n);
		static Float intToFloat(const Int &n, bool isSigned);
		static std::pair<Number, Number> extensionByCRules(Number n1, Number n2);

		friend bool operator==(const Number &num1, const Number &num2);
		friend bool operator!=(const Number &num1, const Number &num2);
		friend bool operator<=(const Number &num1, const Number &num2);
		friend bool operator>=(const Number &num1, const Number &num2);
		friend bool operator<(const Number &num1, const Number &num2);
		friend bool operator>(const Number &num1, const Number &num2);

		friend Number operator-(const Number &op);
		friend Number operator+(const Number &op1, const Number &op2);
		friend Number operator-(const Number &op1, const Number &op2);
		friend Number operator*(const Number &op1, const Number &op2);
		friend Number rdiv(const Number &op1, const Number &op2);
		friend Number exact_div(const Number &op1, const Number &op2);
		friend Number trunc_div(const Number &op1, const Number &op2);
		friend Number trunc_mod(const Number &op1, const Number &op2);

		friend bool logicalNot(const Number &op);
		friend bool logicalAnd(const Number &op1, const Number &op2);
		friend bool logicalOr(const Number &op1, const Number &op2);
		friend bool logicalXor(const Number &op1, const Number &op2);

		friend Number bitNot(const Number &op);
		friend Number bitAnd(const Number &op1, const Number &op2);
		friend Number bitOr(const Number &op1, const Number &op2);
		friend Number bitXor(const Number &op1, const Number &op2);
		friend Number bitLeftShift(const Number &op1, const Number &op2);
		friend Number bitRightShift(const Number &op1, const Number &op2);

		friend Number abs(const Number &op);
		friend Number intToFloat(const Number &op);
		friend Number min(const Number &op1, const Number &op2);
		friend Number max(const Number &op1, const Number &op2);

		/**
		* @brief Emits @a n into @a os.
		*/
		friend std::ostream& operator<<(std::ostream &os, const Number &n);
};

#endif
