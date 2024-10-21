#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
