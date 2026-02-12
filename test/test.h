#ifndef TEST_H
#define TEST_H

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "epio_priv.h"

extern const struct CMUnitTest init_tests[];
extern const size_t num_init_tests;

#endif