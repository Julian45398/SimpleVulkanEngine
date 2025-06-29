#pragma once

#ifndef BIT
#define BIT(X) (1LL<<X)
#else
#error required macro BIT already defined!
#endif
#ifndef HAS_BIT
#define HAS_BIT(X, BIT_POSITION) (X & BIT(BIT_POSITION))
#else
#error required macro HAS_BIT already defined!
#endif

#ifndef SET_BIT
#define SET_BIT(X, BIT_POSITION) do { (X |= BIT(BIT_POSITION)); } while(0)
#else
#error required macro SET_BIT already defined!
#endif

#ifndef UNSET_BIT
#define UNSET_BIT(X, BIT_POSITION) do { (X &= ~BIT(BIT_POSITION)); } while(0)
#else
#error required macro UNSET_BIT already defined!
#endif

#ifndef HAS_FLAG
#define HAS_FLAG(X, FLAG) (X & FLAG)
#else
#error required macro HAS_FLAG already defined!
#endif

#ifndef SET_FLAG
#define SET_FLAG(X, FLAG) do { (X |= FLAG); } while(0)
#else
#error required macro SET_FLAG already defined!
#endif

#ifndef UNSET_FLAG
#define UNSET_FLAG(X, FLAG) do { (X &= ~FLAG); } while(0)
#else
#error required macro UNSET_FLAG already defined!
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
#else
#error required macro ARRAY_SIZE already defined!
#endif // !ARRAY_SIZE(X)
