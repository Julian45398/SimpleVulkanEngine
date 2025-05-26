#pragma once

#ifndef BIT
#define BIT(X) (1<<X)
#else
#error required macro BIT already defined!
#endif
#ifndef CHECK_BIT
#define CHECK_BIT(X, BIT_POSITION) (X & BIT(BIT_POSITION))
#else
#error required macro CHECK_BIT already defined!
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
#else
#error required macro ARRAY_SIZE already defined!
#endif // !ARRAY_SIZE(X)
