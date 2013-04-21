/**
* @author Daniela Ďuričeková, xduric00@stud.fit.vutbr.cz
* @file   MemoryPlaceTest.cc
* @brief  Test class for class MemoryPlace.
* @date   2012
*/

#include "MemoryPlace.h"
#include "gtest/gtest.h"

using namespace std;

class MemoryPlaceTest : public ::testing::Test {
	protected:
		MemoryPlaceTest() {
		}

		virtual ~MemoryPlaceTest() {
		}

		virtual void SetUp() {
		}

		virtual void TearDown() {
		}

};

////////////////////////////////////////////////////////////////////////////////
// variable as a memory place
////////////////////////////////////////////////////////////////////////////////

TEST_F(MemoryPlaceTest,
ArtificialVariableTest)
{
	MemoryPlace m("var", true);
	EXPECT_EQ("var", m.asString());
	EXPECT_TRUE(m.isArtificial());
}

TEST_F(MemoryPlaceTest,
VariableTest)
{
	MemoryPlace m("var", false);
	EXPECT_EQ("var", m.asString());
	EXPECT_FALSE(m.isArtificial());
}

////////////////////////////////////////////////////////////////////////////////
// item of a structure as a memory place
////////////////////////////////////////////////////////////////////////////////

TEST_F(MemoryPlaceTest,
ArtificialStructItemTest)
{
	MemoryPlace m("structure.item", true);
	EXPECT_EQ("structure.item", m.asString());
	EXPECT_TRUE(m.isArtificial());
}

TEST_F(MemoryPlaceTest,
StructItemTest)
{
	MemoryPlace m("structure.item", false);
	EXPECT_EQ("structure.item", m.asString());
	EXPECT_FALSE(m.isArtificial());
}

////////////////////////////////////////////////////////////////////////////////
// item of an union as a memory place
////////////////////////////////////////////////////////////////////////////////

TEST_F(MemoryPlaceTest,
ArtificialUnionItemTest)
{
	MemoryPlace m("union.item", true);
	EXPECT_EQ("union.item", m.asString());
	EXPECT_TRUE(m.isArtificial());
}

TEST_F(MemoryPlaceTest,
UnionItemTest)
{
	MemoryPlace m("union.item", false);
	EXPECT_EQ("union.item", m.asString());
	EXPECT_FALSE(m.isArtificial());
}

////////////////////////////////////////////////////////////////////////////////
// element of an array as a memory place
////////////////////////////////////////////////////////////////////////////////

TEST_F(MemoryPlaceTest,
ArtificialArrayElementTest)
{
	MemoryPlace m("array[0]", true);
	EXPECT_EQ("array[0]", m.asString());
	EXPECT_TRUE(m.isArtificial());
}

TEST_F(MemoryPlaceTest,
ArrayElementTest)
{
	MemoryPlace m("array[0]", false);
	EXPECT_EQ("array[0]", m.asString());
	EXPECT_FALSE(m.isArtificial());
}

////////////////////////////////////////////////////////////////////////////////
// representsElementOfArray
////////////////////////////////////////////////////////////////////////////////

TEST_F(MemoryPlaceTest,
RepresentsElementOfArrayOfElementOfArrayWorksCorrectly)
{
	MemoryPlace m("array[]", false);
	EXPECT_TRUE(m.representsElementOfArray());
}

TEST_F(MemoryPlaceTest,
RepresentsElementOfArrayOfElementOfArrayNestedInStructureWorksCorrectly)
{
	MemoryPlace m("struct.array[].item", false);
	EXPECT_TRUE(m.representsElementOfArray());
}

TEST_F(MemoryPlaceTest,
RepresentsElementOfArrayOfVariableWorksCorrectly)
{
	MemoryPlace m("var", false);
	EXPECT_FALSE(m.representsElementOfArray());
}

TEST_F(MemoryPlaceTest,
RepresentsElementOfArrayOfStructureItemWorksCorrectly)
{
	MemoryPlace m("struct.var", false);
	EXPECT_FALSE(m.representsElementOfArray());
}


int main(int argc, char *argv[])
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

