#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
