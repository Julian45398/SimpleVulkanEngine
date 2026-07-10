#pragma once

#ifndef SGF_BIT
#define SGF_BIT(X) (1LL<<X)
#else
#error required macro BIT already defined!
#endif
#ifndef SGF_HAS_BIT
#define SGF_HAS_BIT(X, BIT_POSITION) (X & SGF_BIT(BIT_POSITION))
#else
#error required macro SGF_HAS_BIT already defined!
#endif

#ifndef SGF_SET_BIT
#define SGF_SET_BIT(X, BIT_POSITION) do { (X |= SGF_BIT(BIT_POSITION)); } while(0)
#else
#error required macro SGF_SET_BIT already defined!
#endif

#ifndef SGF_UNSET_BIT
#define SGF_UNSET_BIT(X, BIT_POSITION) do { (X &= ~SGF_BIT(BIT_POSITION)); } while(0)
#else
#error required macro UNSET_BIT already defined!
#endif

#ifndef SGF_HAS_FLAG
#define SGF_HAS_FLAG(X, FLAG) (X & FLAG)
#else
#error required macro HAS_FLAG already defined!
#endif

#ifndef SGF_SET_FLAG
#define SGF_SET_FLAG(X, FLAG) do { (X |= FLAG); } while(0)
#else
#error required macro SGF_SET_FLAG already defined!
#endif

#ifndef SGF_UNSET_FLAG
#define SGF_UNSET_FLAG(X, FLAG) do { (X &= ~FLAG); } while(0)
#else
#error required macro SGF_UNSET_FLAG already defined!
#endif

#ifndef SGF_ARRAY_SIZE
#define SGF_ARRAY_SIZE(X) (sizeof(X)/sizeof(X[0]))
#else
#error required macro SGF_ARRAY_SIZE already defined!
#endif // !SGF_ARRAY_SIZE(X)

#if defined(_MSC_VER)
#define SGF_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define SGF_DEBUG_BREAK() __builtin_trap()
#else
#include <cstdlib>
#define SGF_DEBUG_BREAK() std::abort()
#endif

#ifndef SGF_ASSERT
#ifdef SGF_ENABLE_ASSERTS 
#define SGF_ASSERT(X) do { if(!(X)) { \
	std::cerr << "Assertion failed: " << #X << ", in file " << __FILE__ << ", line " << __LINE__ << std::endl; \
	SGF_DEBUG_BREAK(); \
} } while(0)
#else
#define SGF_ASSERT(X)
#endif
#endif // !SGF_ASSERT

