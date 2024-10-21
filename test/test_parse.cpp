#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
