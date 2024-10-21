#ifndef TEST_CTX_HPP
#define TEST_CTX_HPP

#include "../include/inireader/inireader.hpp"

struct TestCtx {
    ini::Parser ini_file;
};

inline TestCtx* g_testctx = nullptr;

#endif // TEST_CTX_HPP
