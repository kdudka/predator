/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   UtilityTest.cc
* @brief  Test class for class Utility.
* @date   2012
*/

#include "Utility.h"
#include "gtest/gtest.h"

using namespace std;

class UtilityTest : public ::testing::Test {
	protected:
		UtilityTest() {
		}

		virtual ~UtilityTest() {
		}

		virtual void SetUp() {
		}

		virtual void TearDown() {
		}

};

////////////////////////////////////////////////////////////////////////////////
// convertOperandToNumber()
////////////////////////////////////////////////////////////////////////////////

TEST_F(UtilityTest,
PreconditionViolated)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VOID;
	EXPECT_DEATH(Utility::convertOperandToNumber(&op1),
		".*operand->code == CL_OPERAND_VAR.*operand->code == CL_OPERAND_CST.*");
}

TEST_F(UtilityTest,
SignedCharOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = -1;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(char);
	op1.type->is_unsigned = false;

	EXPECT_EQ(Number(-1, sizeof(char), true),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
UnsignedCharOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = 100;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(char);
	op1.type->is_unsigned = true;

	EXPECT_EQ(Number(100, sizeof(char), false),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
SignedShortOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = -100;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(short);
	op1.type->is_unsigned = false;

	EXPECT_EQ(Number(-100, sizeof(short), true),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
UnsignedShortOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = 125;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(short);
	op1.type->is_unsigned = true;

	EXPECT_EQ(Number(125, sizeof(short), false),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
SignedIntOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = -123456;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(int);
	op1.type->is_unsigned = false;

	EXPECT_EQ(Number(-123456, sizeof(int), true),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
UnsignedIntOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = 1234567;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(int);
	op1.type->is_unsigned = true;

	EXPECT_EQ(Number(1234567, sizeof(int), false),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
SignedLongOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = -123456;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(long);
	op1.type->is_unsigned = false;

	EXPECT_EQ(Number(-123456, sizeof(long), true),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
UnsignedLongOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_INT;
	op1.data.cst.data.cst_int.value = 1234567;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(long);
	op1.type->is_unsigned = true;

	EXPECT_EQ(Number(1234567, sizeof(long), false),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
FloatOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_REAL;
	op1.data.cst.data.cst_real.value = 1.35;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(float);

	EXPECT_EQ(Number(1.35, sizeof(float)),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
DoubleOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_REAL;
	op1.data.cst.data.cst_real.value = 1.3507;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(double);

	EXPECT_EQ(Number(1.3507, sizeof(double)),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
LongDoubleOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_REAL;
	op1.data.cst.data.cst_real.value = 1.35789;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(long double);

	EXPECT_EQ(Number(1.35789, sizeof(long double)),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
VariableOperandIsConvertedToNumber)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->size = sizeof(long double);
	op1.type->code = CL_TYPE_REAL;

	EXPECT_EQ(Number(0, sizeof(long double)),
			  Utility::convertOperandToNumber(&op1));

	delete op1.type;
}

TEST_F(UtilityTest,
UnsupportedOperandConvertedToNumberReturnsError)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	op1.data.cst.code = CL_TYPE_VOID;

	EXPECT_DEATH(Utility::convertOperandToNumber(&op1), "Unsupported!");
}

////////////////////////////////////////////////////////////////////////////////
// getMaxRange()
////////////////////////////////////////////////////////////////////////////////

TEST_F(UtilityTest,
GetMaxRangeOfSignedIntegralNumberWorksCorrectly)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_INT;
	op1.type->size = sizeof(int);
	op1.type->is_unsigned = false;

	EXPECT_EQ(Range::getMaxRange(Number(1, sizeof(int), true)),
			  Utility::getMaxRange(op1));
	delete op1.type;
}

TEST_F(UtilityTest,
GetMaxRangeOfUnsignedIntegralNumberWorksCorrectly)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_INT;
	op1.type->size = sizeof(int);
	op1.type->is_unsigned = true;

	EXPECT_EQ(Range::getMaxRange(Number(1, sizeof(int), false)),
			  Utility::getMaxRange(op1));
	delete op1.type;
}

TEST_F(UtilityTest,
GetMaxRangeOfFloatingPointNumberWorksCorrecly)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_REAL;
	op1.type->size = sizeof(double);

	EXPECT_EQ(Range::getMaxRange(Number(1.0, sizeof(double))),
			  Utility::getMaxRange(op1));
	delete op1.type;
}

TEST_F(UtilityTest,
GetMaxRangeOfUnsupportedOperandWorksCorrectly)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_VOID;

	EXPECT_DEATH(Utility::getMaxRange(op1), "Unsupported!");
	delete op1.type;
}

TEST_F(UtilityTest,
GetMaxRangeWhenPreconditionViolatedWorksCorrectly)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_CST;
	EXPECT_DEATH(Utility::getMaxRange(op1), ".*operand.code == CL_OPERAND_VAR.*");
}

TEST_F(UtilityTest,
GetMaxRangeWithSpecifiedIndexesForNestedStructureWorksCorrecly)
{
	// struct t {
	// 		int e;
	//		double f;
	// }
	// struct {
	// 		struct t a[10];
	// } q;

	struct cl_operand op1;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 1709;
	op1.data.var->name = "q";
	op1.data.var->artificial = false;
	op1.code = CL_OPERAND_VAR;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_STRUCT;
	op1.type->items = new struct cl_type_item[1];
	struct cl_type *tmp1 = new struct cl_type;
	tmp1->code = CL_TYPE_ARRAY;
	tmp1->items = new struct cl_type_item[1];
	op1.type->items[0].type = tmp1;
	op1.type->items[0].name = "a";
	struct cl_type *tmp2 = new struct cl_type;
	tmp2->code = CL_TYPE_STRUCT;
	tmp2->items = new struct cl_type_item[2];
	tmp2->items[0].name = "e";
	tmp2->items[1].name = "f";
	struct cl_type *tmp3 = new struct cl_type;
	struct cl_type *tmp4 = new struct cl_type;
	tmp3->code = CL_TYPE_INT;
	tmp3->size = sizeof(int);
	tmp3->is_unsigned = false;
	tmp4->code = CL_TYPE_REAL;
	tmp4->size = sizeof(double);
	tmp2->items[0].type = tmp3;
	tmp2->items[1].type = tmp4;
	op1.type->items[0].type->items[0].type = tmp2;

	std::deque<int> indexes;
	indexes.push_back(0);
	indexes.push_back(0);
	indexes.push_back(0);
	EXPECT_EQ(Range::getMaxRange(Number(1, sizeof(int), true)),
			  Utility::getMaxRange(op1, indexes));

	indexes.clear();
	indexes.push_back(0);
	indexes.push_back(0);
	indexes.push_back(1);
	EXPECT_EQ(Range::getMaxRange(Number(1, sizeof(double))),
			  Utility::getMaxRange(op1, indexes));

	delete tmp4;
	delete tmp3;
	delete [] tmp2->items;
	delete tmp2;
	delete tmp1;
	delete [] op1.type->items;
	delete op1.type;
	delete op1.data.var;

}

int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

