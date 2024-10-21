#include <gtest/gtest.h>
#include "test_ctx.hpp"

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
    g_testctx = &test_ctx;
    test_ctx.ini_file.Parse(testfile, false);

    ::testing::InitGoogleTest(&argc, argv);
    auto ret = RUN_ALL_TESTS();
    std::filesystem::remove("test.ini");
    return ret;
}
