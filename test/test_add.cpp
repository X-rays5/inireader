#include <gtest/gtest.h>
#include "test_ctx.hpp"

TEST(Add, Default) {
  g_testctx->ini_file.GetRootSection().Add("testv", "hi");
  EXPECT_STREQ(g_testctx->ini_file.GetRootSection()["testv"].as<const char*>(), "hi");
}

TEST(Add, Kv) {
  g_testctx->ini_file.AddSection("addedsection").Add("testv", "value");
  EXPECT_STREQ(g_testctx->ini_file["addedsection"]["testv"].as<const char*>(), "value");
}
