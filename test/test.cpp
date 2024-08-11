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

TEST(Parse, CommentVal) {
  auto section = g_testctx->ini_file["comment_val"];

  EXPECT_STREQ(section["val1"].as<const char*>(), "##hello");
  EXPECT_STREQ(section["val2##"].as<const char*>(), "world");
  EXPECT_STREQ(section["val3"].as<const char*>(), "he##llo");
}

TEST(Parse, Section1) {
  auto section = g_testctx->ini_file["Section 1"];
  EXPECT_STREQ(section["test_line_break"].as<const char*>(), "test1");
  EXPECT_STREQ(section["Option 1"].as<const char*>(), "value 1");
  EXPECT_STREQ(section["Option 2"].as<const char*>(), "value 2");
  EXPECT_STREQ(section["oPtion 1"].as<const char*>(), "value 2\\ \\ \\");
  EXPECT_STREQ(section["Option 3"].as<const char*>(), "value 3 = not value 2");
  EXPECT_STREQ(section["option 3"].as<const char*>(), "value 3 = not value 2 = not value 1\\");
}

TEST(Parse, Numbers) {
  auto section = g_testctx->ini_file["Numbers"];
  EXPECT_EQ(section["num"].as<std::int32_t>(), -1285);
  EXPECT_STREQ(section["num_bin"].as<const char*>(), "0b01101001");
  EXPECT_EQ(section["num_hex"].as<std::int32_t>(), 4782);
  EXPECT_EQ(section["num_oct"].as<std::uint32_t>(), 1754);
  EXPECT_EQ(section["num_uint64"].as<std::uint64_t>(), 1122334400000000ull);
  EXPECT_EQ(section["float1"].as<double>(), -124.45667356);
  EXPECT_EQ(section["float2"].as<double>(), 4.123456545);
  EXPECT_EQ(section["float3"].as<double>(), 412.3456545);
  EXPECT_EQ(section["float4"].as<double>(), -1.1245864);
}

TEST(Parse, Other) {
  auto section = g_testctx->ini_file["Other"];
  EXPECT_EQ(section["bool1"].as<bool>(), true);
  EXPECT_EQ(section["bool2"].as<bool>(), true);
  EXPECT_EQ(section["bool3"].as<bool>(), false);
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

TEST(Edit, Reference) {
  ini::Parser::IniSection& section = g_testctx->ini_file.AddSection("edit_ref");
  section.Add("test_num", 1234);
  section.Add("test_str", "hello");

  EXPECT_TRUE(g_testctx->ini_file["edit_ref"]["test_num"].is<std::int32_t>());
  EXPECT_TRUE(g_testctx->ini_file["edit_ref"]["test_str"].is<std::string>());
  EXPECT_EQ(g_testctx->ini_file["edit_ref"]["test_num"].as<std::int32_t>(), 1234);
  EXPECT_EQ(g_testctx->ini_file["edit_ref"]["test_str"].as<std::string>(), "hello");
  EXPECT_EQ(g_testctx->ini_file["edit_ref"]["test_str"].as<std::string_view>(), "hello");

  section.Remove("test_num");
  EXPECT_FALSE(g_testctx->ini_file["edit_ref"].HasValue("test_num"));
}

TEST(Conversion, UTF8) {
  std::string hello_world{ u8"hello, 世界" };

  std::u16string hello_world_u16{ini::conversion::utility::DecodeUTF(hello_world)};
  EXPECT_EQ(hello_world, ini::conversion::utility::EncodeUTF(hello_world_u16));
  #ifdef _WIN32
  EXPECT_EQ(hello_world_u16.length(), 13); // 13 bytes, 9 characters
  #else
  EXPECT_EQ(hello_world_u16.length(), 9); // 9 bytes, 9 characters
  #endif

  std::u32string hello_world_u32{ini::conversion::utility::DecodeUTF<char32_t>(hello_world)};
  EXPECT_EQ(hello_world, ini::conversion::utility::EncodeUTF<char32_t>(hello_world_u32));
  #ifdef _WIN32
  EXPECT_EQ(hello_world_u32.length(), 13); // 13 bytes, 9 characters
  #else
  EXPECT_EQ(hello_world_u32.length(), 9); // 9 bytes, 9 characters
  #endif
}

int main(int argc, char** argv) {
  constexpr const char* testfile = "default section value = test value ; section less\n"
                                   "\n"
                                   "[comment_val]\n"
                                   "val1 = \"##hello\"\n"
                                   "val2## = world\n"
                                   "val3 = he##llo\n"
                                   "[Section 1]\n"
                                   "; comment\n"
                                   "# comment2\n"
                                   "test_line_break = test1\r"
                                   "Option 1 = value 1                     ; option 'Option 1' has value 'value 1'\n"
                                   "Option 2 =  value 2                     # option 'Option 2' has value 'value 2'\n"
                                   "oPtion 1    =  value 2\\ \\ \\          ; option 'oPtion 1' has value ' value 2   ', 'oPtion 1' and 'Option 1' are different\n"
                                   "Option 3= value 3 = not value 2\n"
                                   "option 3  =value 3 = not value 2 = not value 1\\\n"
                                   "\n"
                                   "[Numbers]\n"
                                   "num = -1285\n"
                                   "num_bin = 0b01101001\n"
                                   "num_hex = 0x12ae\n"
                                   "num_oct = 01754\n"
                                   "num_uint64 = 1122334400000000\n"
                                   "\n"
                                   "float1 = -124.45667356\n"
                                   "float2 = 4.123456545\n"
                                   "float3 = 412.3456545\n"
                                   "float4 = -1.1245864\n"
                                   "\n"
                                   "[Other]\n"
                                   "bool1 = 1\n"
                                   "bool2 = on\n"
                                   "bool3=off";
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
