#include <iostream>
#include <gtest/gtest.h>
#include <cstring>

#include "DBFile.h"

DBFile dbFile;
const char *dbfile_dir = "../tpch-dbgen";
const char* fileToTest = "nation.bin";
const char *testTable = "../tpch-dbgen/nation.tbl";

int main(int argc, char **argv) {

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

//Test for DBFile Create funtion
TEST(Create_Test, Create_Test1) {

	int result = dbFile.Create(fileToTest, fType::heap, NULL);
	// Add some data to the DB File.
	Schema *rel = new Schema ("catalog", "nation");
	dbFile.Load (*rel, testTable);
	dbFile.Close();

	ASSERT_EQ(result, 1);
}

//Test for DBFile create function fail case
TEST(Create_Test, Create_Test2) {

	//int result = dbFile.Create("", fType::heap, NULL);
	EXPECT_EXIT( dbFile.Create("", fType::heap, NULL), testing::ExitedWithCode(1), "BAD!*");
}

//Test for DBFile Open funtion
TEST(Open_Test, Open_Test1) {

	int result = dbFile.Open(fileToTest);
	dbFile.Close();

	ASSERT_EQ(result, 1);
}

//Test for DBFile Open funtion fail case
TEST(Open_Test, Open_Test2) {

	int result = dbFile.Open("random_TestFile.bin");
	//dbFile.Close();

	ASSERT_EQ(result, 0);
}

//Test for DBFile Close funtion
TEST(Close_Test, Close_Test1) {

	dbFile.Open(fileToTest);
	int result = dbFile.Close();

	ASSERT_EQ(result, 1);
}