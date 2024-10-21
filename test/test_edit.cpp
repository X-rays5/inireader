#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
