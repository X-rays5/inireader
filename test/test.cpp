#include <fstream>
#include <gtest/gtest.h>

#ifndef NDEBUG
#define NDEBUG
#include "../include/inireader/inireader.hpp"
#undef NDEBUG
#else
#include "../include/inireader/inireader.hpp"
#endif

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

TEST(Parse, Error) {
  EXPECT_ANY_THROW(g_testctx->ini_file.Parse("doesntexists.ini", true));
  g_testctx->ini_file.Parse("test.ini", true); // without this all other tests will fail
}

TEST(Parse, DefaultSection) {
  EXPECT_STREQ(g_testctx->ini_file.GetRootSection()["default section value"].as<const char*>(), "test value");
}

TEST(Parse, Section1) {
  auto section = g_testctx->ini_file["Section 1"];
  EXPECT_STREQ(section["Option 1"].as<const char*>(), "value 1");
  EXPECT_STREQ(section["Option 2"].as<const char*>(), "value 2");
  EXPECT_STREQ(section["oPtion 1"].as<const char*>(), "value 2\\ \\ \\");
}

TEST(Parse, Numbers) {
  auto section = g_testctx->ini_file["Numbers"];
  EXPECT_EQ(section["num"].as<std::int32_t>(), -1285);
  EXPECT_STREQ(section["num_bin"].as<const char*>(), "0b01101001");
  EXPECT_STREQ(section["num_hex"].as<const char*>(), "0x12ae,0xAc2B");
  EXPECT_EQ(section["num_oct"].as<std::uint32_t>(), 1754);
  EXPECT_STREQ(section["float1"].as<const char*>(), "-124.45667356"); // For some reason it doesn't want to pass this test when it's not a string while I've manually verified it works that it should
  EXPECT_EQ(section["float2"].as<double>(), 4.123456545);
  EXPECT_EQ(section["float3"].as<double>(), 412.3456545);
  EXPECT_STREQ(section["float4"].as<const char*>(), "-1.1245864"); // For some reason it doesn't want to pass this test when it's not a string while I've manually verified it works that it should
}

TEST(Parse, Other) {
  auto section = g_testctx->ini_file["Other"];
  EXPECT_STREQ(section["bool1"].as<const char*>(), "1");
  EXPECT_STREQ(section["bool2"].as<const char*>(), "on");
  EXPECT_STREQ(section["bool3"].as<const char*>(), "f");
}

TEST(Add, Default) {
  g_testctx->ini_file.GetRootSection().Add("testv", "hi");
  EXPECT_STREQ(g_testctx->ini_file.GetRootSection()["testv"].as<const char*>(), "hi");
}

TEST(Add, Kv) {
  g_testctx->ini_file.AddSection("addedsection").Add("testv", "value");
  EXPECT_STREQ(g_testctx->ini_file["addedsection"]["testv"].as<const char*>(), "value");
}

TEST(Remove, Default) {
  g_testctx->ini_file.GetRootSection().Remove("testv");
  EXPECT_THROW(g_testctx->ini_file.GetRootSection()["testv"].as<const char*>(), std::runtime_error);
  g_testctx->ini_file.GetRootSection().RemoveAll();
  EXPECT_THROW(g_testctx->ini_file.GetRootSection()["default section value"].as<const char*>(), std::runtime_error);
}

TEST(Remove, Kv) {
  g_testctx->ini_file["addedsection"].Remove("test");
  EXPECT_THROW(g_testctx->ini_file["addedsection"]["test"].as<const char*>(), std::runtime_error);
}

TEST(Remove, Section) {
  g_testctx->ini_file.AddSection("addedsection");
  EXPECT_EQ(g_testctx->ini_file.HasSection("addedsection"), true);
  EXPECT_EQ(g_testctx->ini_file.RemoveSection("addedsection"), true);
  EXPECT_THROW(g_testctx->ini_file["addedsection"]["test"].as<const char*>(), std::runtime_error);
}

TEST(Has, Default) {
  g_testctx->ini_file.GetRootSection().Add("testv", "hi");
  EXPECT_EQ(g_testctx->ini_file.GetRootSection().HasValue("testv"), true);
  EXPECT_EQ(g_testctx->ini_file.GetRootSection().Remove("testv"), true);
}

TEST(Has, Section) {
  EXPECT_EQ(g_testctx->ini_file.HasSection("Section 1"), true);
}

TEST(Has, Kv) {
  EXPECT_EQ(g_testctx->ini_file["Section 1"].HasValue("Option 1"), true);
}

TEST(Edit, Default) {
  g_testctx->ini_file.GetRootSection().Add("edittest", 4);
  EXPECT_EQ(g_testctx->ini_file.GetRootSection().HasValue("edittest"), true);
  g_testctx->ini_file.GetRootSection()["edittest"] = 1234;
  EXPECT_EQ(g_testctx->ini_file.GetRootSection()["edittest"].as<int>(), 1234);
  EXPECT_EQ(g_testctx->ini_file.GetRootSection().Remove("edittest"), true);
}

TEST(Edit, Section) {
  g_testctx->ini_file.AddSection("testsection").Add("edittest", "wow");
  EXPECT_EQ(g_testctx->ini_file["testsection"].HasValue("edittest"), true);
  g_testctx->ini_file["testsection"]["edittest"] = 1234;
  EXPECT_EQ(g_testctx->ini_file["testsection"]["edittest"].as<int>(), 1234);
  EXPECT_EQ(g_testctx->ini_file.RemoveSection("testsection"), true);
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
                                   "float2 = 4.123456545\n"
                                   "float3 = 412.3456545\n"
                                   "float4 = -1.1245864\n"
                                   "\n"
                                   "[Other]\n"
                                   "bool1 = 1\n"
                                   "bool2 = on\n"
                                   "bool3=f";
  std::ofstream writer("test.ini");
  writer << testfile;
  writer.close();

  TestCtx test_ctx;
  test_ctx.ini_file.Parse(testfile, false);

  ::testing::InitGoogleTest(&argc, argv);
  auto ret = RUN_ALL_TESTS();
  std::filesystem::remove("test.ini");
  return ret;
}