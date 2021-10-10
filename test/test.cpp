#include <iostream>
#include <fstream>
#include <gtest/gtest.h>
#include "../inireader.hpp"

struct TestCtx;
inline TestCtx* g_testctx{};

struct TestCtx {
	TestCtx() {
		g_testctx = this;
	}

	~TestCtx() {
		g_testctx = nullptr;
	}

	ini::Parser ini_file;
};

TEST(ParseTest, NoError) {
	EXPECT_EQ(g_testctx->ini_file.HasParseError(), false);
}

TEST(ParseTest, DefaultSection) {
	EXPECT_STREQ(g_testctx->ini_file.GetDefault("default section value").c_str(), "test value");
}

TEST(ParseTest, Section1) {
	auto section = g_testctx->ini_file["Section 1"];
	EXPECT_STREQ(section["Option 1"].c_str(), "value 1");
	EXPECT_STREQ(section["Option 2"].c_str(), "value 2");
	EXPECT_STREQ(section["oPtion 1"].c_str(), "value 2\\ \\ \\");
}

TEST(ParseTest, Numbers) {
	auto section = g_testctx->ini_file["Numbers"];
	EXPECT_STREQ(section["num"].c_str(), "-1285");
	EXPECT_STREQ(section["num_bin"].c_str(), "0b01101001");
	EXPECT_STREQ(section["num_hex"].c_str(), "0x12ae,0xAc2B");
	EXPECT_STREQ(section["num_oct"].c_str(), "01754");
	EXPECT_STREQ(section["float1"].c_str(), "-124.45667356");
	EXPECT_STREQ(section["float2"].c_str(), "+4.1234565E+45");
	EXPECT_STREQ(section["float3"].c_str(), "412.34565e45");
	EXPECT_STREQ(section["float4"].c_str(), "-1.1245864E-6");
}

TEST(ParseTest, Other) {
	auto section = g_testctx->ini_file["Other"];
	EXPECT_STREQ(section["bool1"].c_str(), "1");
	EXPECT_STREQ(section["bool2"].c_str(), "on");
	EXPECT_STREQ(section["bool3"].c_str(), "f");
}

int main(int argc, char** argv) {
	constexpr const char* testfile = "default section value = test value ; section less\n"
	                                 "\n"
	                                 "[Section 1]\n"
	                                 "; comment\n"
	                                 "# comment2\n"
	                                 "Option 1 = value 1                     ; option 'Option 1' has value 'value 1'\n"
	                                 "Option 2 =  value 2                     # option 'Option 2' has value 'value 2'\n"
	                                 "oPtion 1    =  value 2\\ \\ \\          ; option 'oPtion 1' has value ' value 2   ', 'oPtion 1' and 'Option 1' are different\n"
	                                 "\n"
	                                 "[Numbers]\n"
	                                 "num = -1285\n"
	                                 "num_bin = 0b01101001\n"
	                                 "num_hex = 0x12ae,0xAc2B\n"
	                                 "num_oct = 01754\n"
	                                 "\n"
	                                 "float1 = -124.45667356\n"
	                                 "float2 = +4.1234565E+45\n"
	                                 "float3 = 412.34565e45\n"
	                                 "float4 = -1.1245864E-6\n"
	                                 "\n"
	                                 "[Other]\n"
	                                 "bool1 = 1\n"
	                                 "bool2 = on\n"
	                                 "bool3=f";

	std::ofstream writer("test.ini");
	writer << testfile;
	writer.close();

	TestCtx test_ctx;
	test_ctx.ini_file.Parse("test.ini");

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}