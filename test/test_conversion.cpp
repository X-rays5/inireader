#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
