/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   OperandToMemoryPlaceTest.cc
* @brief  Test class for class OperandToMemoryPlace.
* @date   2012
*/

#include "OperandToMemoryPlace.h"
#include "gtest/gtest.h"

using namespace std;

class OperandToMemoryPlaceTest : public ::testing::Test {
	protected:
		OperandToMemoryPlaceTest() {
		}

		virtual ~OperandToMemoryPlaceTest() {
		}

		virtual void SetUp() {
			OperandToMemoryPlace::init();
		}

		virtual void TearDown() {
		}

};

////////////////////////////////////////////////////////////////////////////////
// conversion of variables
////////////////////////////////////////////////////////////////////////////////

TEST_F(OperandToMemoryPlaceTest,
PreconditionViolated)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VOID;
	ASSERT_DEATH(OperandToMemoryPlace::convert(&op1),
				 "operand->code == CL_OPERAND_VAR");

	struct cl_operand op2;
	op2.code = CL_OPERAND_CST;
	ASSERT_DEATH(OperandToMemoryPlace::convert(&op2),
				 "operand->code == CL_OPERAND_VAR");
}

TEST_F(OperandToMemoryPlaceTest,
VariableIsConvertedToMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.accessor = NULL;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "variable_a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.type->item_cnt = 0;

	MemoryPlace *mp = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("variable_a", mp->asString());
	ASSERT_FALSE(mp->isArtificial());

	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
TwoIdenticalVariablesAreConvertedToTheSameMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.accessor = NULL;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "variable_a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.type->item_cnt = 0;

	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.accessor = NULL;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2569;
	op2.data.var->name = "variable_a";
	op2.data.var->artificial = false;
	op2.type = new struct cl_type;
	op2.type->item_cnt = 0;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_EQ(mp1, mp2);

	delete op1.type;
	delete op2.type;
	delete op1.data.var;
	delete op2.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
TwoNonIdenticalVariablesAreNotConvertedToTheSameMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.accessor = NULL;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "variable_a";
	op1.data.var->artificial = false;

	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.accessor = NULL;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2579;
	op2.data.var->name = "variable_a";
	op2.data.var->artificial = false;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_NE(mp1, mp2);

	delete op1.data.var;
	delete op2.data.var;
}

////////////////////////////////////////////////////////////////////////////////
// conversion of an item of a structure
////////////////////////////////////////////////////////////////////////////////

TEST_F(OperandToMemoryPlaceTest,
TwoIdenticalStructItemsAreConvertedToTheSameMemoryPlace)
{
	// struct {
	//		struct {
	//			struct {
	//            int d;
	//          } c;
	//      } b;
	// } a;

	// a.b.c.d
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.type->item_cnt = 0;
	op1.type->code = CL_TYPE_STRUCT;
	op1.type->items = new struct cl_type_item[1];
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_ITEM;
	op1.accessor->data.item.id = 0;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->item_cnt = 1;
	op1.accessor->type->code = CL_TYPE_STRUCT;
	op1.accessor->type->items = new struct cl_type_item[1];
	op1.accessor->type->items[0].name = "b";
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->data.item.id = 0;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->item_cnt = 1;
	op1.accessor->next->type->code = CL_TYPE_STRUCT;
	op1.accessor->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->type->items[0].name = "c";
	op1.accessor->next->next = new struct cl_accessor;
	op1.accessor->next->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->next->data.item.id = 0;
	op1.accessor->next->next->type = new struct cl_type;
	op1.accessor->next->next->type->item_cnt = 1;
	op1.accessor->next->next->type->code = CL_TYPE_STRUCT;
	op1.accessor->next->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->next->type->items[0].name = "d";
	op1.accessor->next->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a.b.c.d", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete [] op1.accessor->next->next->type->items;
	delete op1.accessor->next->next->type;
	delete op1.accessor->next->next;
	delete [] op1.accessor->next->type->items;
	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete [] op1.accessor->type->items;
	delete op1.accessor->type;
	delete op1.accessor;
	delete [] op1.type->items;
	delete op1.type;
	delete op1.data.var;

	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2569;
	op2.data.var->name = "a";
	op2.data.var->artificial = false;
	op2.type = new struct cl_type;
	op2.type->item_cnt = 0;
	op2.type->code = CL_TYPE_STRUCT;
	op2.type->items = new struct cl_type_item[1];
	op2.accessor = new struct cl_accessor;
	op2.accessor->code = CL_ACCESSOR_ITEM;
	op2.accessor->data.item.id = 0;
	op2.accessor->type = new struct cl_type;
	op2.accessor->type->item_cnt = 1;
	op2.accessor->type->code = CL_TYPE_STRUCT;
	op2.accessor->type->items = new struct cl_type_item[1];
	op2.accessor->type->items[0].name = "b";
	op2.accessor->next = new struct cl_accessor;
	op2.accessor->next->code = CL_ACCESSOR_ITEM;
	op2.accessor->next->data.item.id = 0;
	op2.accessor->next->type = new struct cl_type;
	op2.accessor->next->type->item_cnt = 1;
	op2.accessor->next->type->code = CL_TYPE_STRUCT;
	op2.accessor->next->type->items = new struct cl_type_item[1];
	op2.accessor->next->type->items[0].name = "c";
	op2.accessor->next->next = new struct cl_accessor;
	op2.accessor->next->next->code = CL_ACCESSOR_ITEM;
	op2.accessor->next->next->data.item.id = 0;
	op2.accessor->next->next->type = new struct cl_type;
	op2.accessor->next->next->type->item_cnt = 1;
	op2.accessor->next->next->type->code = CL_TYPE_STRUCT;
	op2.accessor->next->next->type->items = new struct cl_type_item[1];
	op2.accessor->next->next->type->items[0].name = "d";
	op2.accessor->next->next->next = NULL;

	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_EQ("a.b.c.d", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	EXPECT_EQ(mp1, mp2);

	delete [] op2.accessor->next->next->type->items;
	delete op2.accessor->next->next->type;
	delete op2.accessor->next->next;
	delete [] op2.accessor->next->type->items;
	delete op2.accessor->next->type;
	delete op2.accessor->next;
	delete [] op2.accessor->type->items;
	delete op2.accessor->type;
	delete op2.accessor;
	delete [] op2.type->items;
	delete op2.type;
	delete op2.data.var;

}

TEST_F(OperandToMemoryPlaceTest,
TwoNonIdenticalStructItemsAreNotConvertedToTheSameMemoryPlace)
{
	// struct {
	//		struct {
	//			int i;
	//			float x;
	//      } b;
	//      int q;
	// } a;

	// a.b.x
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.type->items = new struct cl_type_item[2];
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_ITEM;
	op1.accessor->data.item.id = 0;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->items = new struct cl_type_item[2];
	op1.accessor->type->items[0].name = "b";
	op1.accessor->type->items[1].name = "q";
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->data.item.id = 1;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->items = new struct cl_type_item[2];
	op1.accessor->next->type->items[0].name = "i";
	op1.accessor->next->type->items[1].name = "x";
	op1.accessor->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a.b.x", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete [] op1.accessor->next->type->items;
	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete [] op1.accessor->type->items;
	delete op1.accessor->type;
	delete op1.accessor;
	delete [] op1.type->items;
	delete op1.type;
	delete op1.data.var;

	// a.b.i
	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2569;
	op2.data.var->name = "a";
	op2.data.var->artificial = false;
	op2.type = new struct cl_type;
	op2.type->items = new struct cl_type_item[2];
	op2.accessor = new struct cl_accessor;
	op2.accessor->code = CL_ACCESSOR_ITEM;
	op2.accessor->data.item.id = 0;
	op2.accessor->type = new struct cl_type;
	op2.accessor->type->items = new struct cl_type_item[2];
	op2.accessor->type->items[0].name = "b";
	op2.accessor->type->items[1].name = "q";
	op2.accessor->next = new struct cl_accessor;
	op2.accessor->next->code = CL_ACCESSOR_ITEM;
	op2.accessor->next->data.item.id = 0;
	op2.accessor->next->type = new struct cl_type;
	op2.accessor->next->type->items = new struct cl_type_item[2];
	op2.accessor->next->type->items[0].name = "i";
	op2.accessor->next->type->items[1].name = "x";
	op2.accessor->next->next = NULL;

	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_EQ("a.b.i", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	delete [] op2.accessor->next->type->items;
	delete op2.accessor->next->type;
	delete op2.accessor->next;
	delete [] op2.accessor->type->items;
	delete op2.accessor->type;
	delete op2.accessor;
	delete [] op2.type->items;
	delete op2.type;
	delete op2.data.var;

	ASSERT_NE(mp1, mp2);
}

////////////////////////////////////////////////////////////////////////////////
// conversion of an element of an array
////////////////////////////////////////////////////////////////////////////////

TEST_F(OperandToMemoryPlaceTest,
ArrayElementIsConvertedToMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.accessor = new struct cl_accessor;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->code = CL_TYPE_ARRAY;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->code = CL_TYPE_ARRAY;
	op1.accessor->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a[][]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ArrayElementsFromTheSameArrayAreConvertedToTheSameMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.accessor = new struct cl_accessor;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->code = CL_TYPE_ARRAY;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a[]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2569;
	op2.data.var->name = "a";
	op2.data.var->artificial = false;
	op2.accessor = new struct cl_accessor;
	op2.accessor->type = new struct cl_type;
	op2.accessor->type->code = CL_TYPE_ARRAY;
	op2.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op2.accessor->next = NULL;

	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_EQ("a[]", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.data.var;
	delete op2.accessor->type;
	delete op2.accessor;
	delete op2.data.var;

	ASSERT_EQ(mp1, mp2);
}

TEST_F(OperandToMemoryPlaceTest,
ArrayElementsFromDifferentArrayAreNotConvertedToTheSameMemoryPlace)
{
	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.accessor = new struct cl_accessor;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->code = CL_TYPE_ARRAY;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a[]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	struct cl_operand op2;
	op2.code = CL_OPERAND_VAR;
	op2.data.var = new struct cl_var;
	op2.data.var->uid = 2579;
	op2.data.var->name = "a";
	op2.data.var->artificial = false;
	op2.accessor = new struct cl_accessor;
	op2.accessor->type = new struct cl_type;
	op2.accessor->type->code = CL_TYPE_ARRAY;
	op2.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op2.accessor->next = NULL;

	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op2);
	ASSERT_EQ("a[]", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.data.var;
	delete op2.accessor->type;
	delete op2.accessor;
	delete op2.data.var;

	ASSERT_NE(mp1, mp2);
}

////////////////////////////////////////////////////////////////////////////////
// conversion of mixed structure and arrays
////////////////////////////////////////////////////////////////////////////////

TEST_F(OperandToMemoryPlaceTest,
ConversionOfStructureWithArrayWorksCorrectly)
{
	// struct str {
	//     int b[10];
	// } a;
	// a.b[0] = 5;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_ITEM;
	op1.accessor->data.item.id = 0;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->items = new struct cl_type_item[1];
	op1.accessor->type->items[0].name = "b";
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("a.b[]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete op1.accessor->next;
	delete [] op1.accessor->type->items;
	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfArrayOfStructuresWorksCorrectly)
{
	// struct s {
	//		float a;
	// 		int b;
	// };
	// struct s c[10];
	// c[0].b = 0;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "c";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->data.item.id = 1;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->items = new struct cl_type_item[2];
	op1.accessor->next->type->items[0].name = "a";
	op1.accessor->next->type->items[1].name = "b";
	op1.accessor->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("c[].b", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete [] op1.accessor->next->type->items;
	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfArrayOfStructuresInWhichArrayOfStructuresIsWorksCorrectly)
{
	// struct a {
	//     int b;
	//     double q;
	// };
	// struct c {
	//     struct a d[10];
	// };
	// struct c e[10];
	//
	// e[5].d[8].q = 0.0

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "e";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->data.item.id = 0;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->type->items[0].name = "d";
	op1.accessor->next->next = new struct cl_accessor;
	op1.accessor->next->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next->next = new struct cl_accessor;
	op1.accessor->next->next->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->next->next->data.item.id = 1;
	op1.accessor->next->next->next->type = new struct cl_type;
	op1.accessor->next->next->next->type->items = new struct cl_type_item[2];
	op1.accessor->next->next->next->type->items[0].name = "b";
	op1.accessor->next->next->next->type->items[1].name = "q";
	op1.accessor->next->next->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("e[].d[].q", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	op1.accessor->next->next->next->data.item.id = 0;
	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("e[].d[].b", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	ASSERT_NE(mp1, mp2);

	delete [] op1.accessor->next->next->next->type->items;
	delete op1.accessor->next->next->next->type;
	delete op1.accessor->next->next->next;
	delete op1.accessor->next->next;
	delete [] op1.accessor->next->type->items;
	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfArrayOfStructuresInWhichStructureWithArrayIsWorksCorrectly)
{
	// struct a {
	//     int b[10];
	// };
	// struct c {
	//     float q;
	//     struct a d;
	// };
	// struct c e[10];
	//
	// e[0].d.b[0] = 0;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "e";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->data.item.id = 1;
	op1.accessor->next->type = new struct cl_type;
	op1.accessor->next->type->items = new struct cl_type_item[2];
	op1.accessor->next->type->items[0].name = "q";
	op1.accessor->next->type->items[1].name = "d";
	op1.accessor->next->next = new struct cl_accessor;
	op1.accessor->next->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->next->data.item.id = 0;
	op1.accessor->next->next->type = new struct cl_type;
	op1.accessor->next->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->next->type->items[0].name = "b";
	op1.accessor->next->next->next = new struct cl_accessor;
	op1.accessor->next->next->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("e[].d.b[]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete op1.accessor->next->next->next;
	delete [] op1.accessor->next->next->type->items;
	delete op1.accessor->next->next->type;
	delete op1.accessor->next->next;
	delete [] op1.accessor->next->type->items;
	delete op1.accessor->next->type;
	delete op1.accessor->next;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfStructureWithArrayOfStructuresWorksCorrectly)
{
	// struct a {
	//     int b;
	// };
	// struct c {
	//     struct a d[10];
	// } e;
	//
	// e.d[0].b = 0;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "e";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_ITEM;
	op1.accessor->data.item.id = 0;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->items = new struct cl_type_item[1];
	op1.accessor->type->items[0].name = "d";
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next = new struct cl_accessor;
	op1.accessor->next->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->next->data.item.id = 0;
	op1.accessor->next->next->type = new struct cl_type;
	op1.accessor->next->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->next->type->items[0].name = "b";
	op1.accessor->next->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("e.d[].b", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete [] op1.accessor->next->next->type->items;
	delete op1.accessor->next->next->type;
	delete op1.accessor->next->next;
	delete op1.accessor->next;
	delete [] op1.accessor->type->items;
	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfStructureWithArrayOfStructureWithArrayWorksCorrectly)
{
	// struct a {
	//     int b[10];
	// };
	// struct c {
	//     struct a d[10];
	// } e;
	//
	// e.d[0].b[0] = 0;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "e";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.accessor = new struct cl_accessor;
	op1.accessor->code = CL_ACCESSOR_ITEM;
	op1.accessor->data.item.id = 0;
	op1.accessor->type = new struct cl_type;
	op1.accessor->type->items = new struct cl_type_item[1];
	op1.accessor->type->items[0].name = "d";
	op1.accessor->next = new struct cl_accessor;
	op1.accessor->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next = new struct cl_accessor;
	op1.accessor->next->next->code = CL_ACCESSOR_ITEM;
	op1.accessor->next->next->data.item.id = 0;
	op1.accessor->next->next->type = new struct cl_type;
	op1.accessor->next->next->type->items = new struct cl_type_item[1];
	op1.accessor->next->next->type->items[0].name = "b";
	op1.accessor->next->next->next = new struct cl_accessor;
	op1.accessor->next->next->next->code = CL_ACCESSOR_DEREF_ARRAY;
	op1.accessor->next->next->next->next = NULL;

	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1);
	ASSERT_EQ("e.d[].b[]", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	delete op1.accessor->next->next->next;
	delete [] op1.accessor->next->next->type->items;
	delete op1.accessor->next->next->type;
	delete op1.accessor->next->next;
	delete op1.accessor->next;
	delete [] op1.accessor->type->items;
	delete op1.accessor->type;
	delete op1.accessor;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfStructuresWithSpecifiedIndexesWorksCorrectly)
{
	// struct s {
	//		float a;
	// 		struct {
	//			int c;
	//			float d;
	//		} b;
	// } a;

	struct cl_operand op1;
	op1.code = CL_OPERAND_VAR;
	op1.data.var = new struct cl_var;
	op1.data.var->uid = 2569;
	op1.data.var->name = "a";
	op1.data.var->artificial = false;
	op1.type = new struct cl_type;
	op1.type->code = CL_TYPE_STRUCT;
	op1.accessor = NULL;
	op1.type->items = new struct cl_type_item[2];
	struct cl_type *tmp1 = new struct cl_type;
	tmp1->code = CL_TYPE_REAL;
	op1.type->items[0].type = tmp1;
	op1.type->items[0].name = "a";
	struct cl_type *tmp2 = new struct cl_type;
	tmp2->code = CL_TYPE_STRUCT;
	tmp2->items = new struct cl_type_item[2];
	op1.type->items[1].type = tmp2;
	op1.type->items[1].name = "b";
	struct cl_type *tmp3 = new struct cl_type;
	tmp3->code = CL_TYPE_INT;
	op1.type->items[1].type->items[0].type = tmp3;
	op1.type->items[1].type->items[0].name = "c";
	struct cl_type *tmp4 = new struct cl_type;
	tmp4->code = CL_TYPE_REAL;
	op1.type->items[1].type->items[1].type = tmp4;
	op1.type->items[1].type->items[1].name = "d";

	std::deque<int> indexes;
	indexes.push_back(0);
	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1, indexes);
	ASSERT_EQ("a.a", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	indexes.clear();
	indexes.push_back(1);
	indexes.push_back(0);
	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op1, indexes);
	ASSERT_EQ("a.b.c", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

	indexes.clear();
	indexes.push_back(1);
	indexes.push_back(1);
	MemoryPlace *mp3 = OperandToMemoryPlace::convert(&op1, indexes);
	ASSERT_EQ("a.b.d", mp3->asString());
	ASSERT_FALSE(mp3->isArtificial());

	delete tmp4;
	delete tmp3;
	delete [] tmp2->items;
	delete tmp2;
	delete tmp1;
	delete [] op1.type->items;
	delete op1.type;
	delete op1.data.var;
}

TEST_F(OperandToMemoryPlaceTest,
ConversionOfStructuresWithArrayWithSpecifiedIndexesWorksCorrectly)
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
	op1.accessor = NULL;
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
	tmp4->code = CL_TYPE_REAL;
	tmp2->items[0].type = tmp3;
	tmp2->items[1].type = tmp4;
	op1.type->items[0].type->items[0].type = tmp2;

	std::deque<int> indexes;
	indexes.push_back(0);
	indexes.push_back(0);
	indexes.push_back(0);
	MemoryPlace *mp1 = OperandToMemoryPlace::convert(&op1, indexes);
	ASSERT_EQ("q.a[].e", mp1->asString());
	ASSERT_FALSE(mp1->isArtificial());

	indexes.clear();
	indexes.push_back(0);
	indexes.push_back(0);
	indexes.push_back(1);
	MemoryPlace *mp2 = OperandToMemoryPlace::convert(&op1, indexes);
	ASSERT_EQ("q.a[].f", mp2->asString());
	ASSERT_FALSE(mp2->isArtificial());

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

