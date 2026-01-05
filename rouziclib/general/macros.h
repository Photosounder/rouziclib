#define STRINGIFY(x) #x					// makes the name of x a string
#define VALUE_STRINGIFY(s) STRINGIFY(s)			// makes the value of x a string
#define flag_update(x)	if (abs(x)>=2) (x) >>= 1

#if (defined(__GNUC__) && !defined(__clang__) && !defined(__INTEL_COMPILER))
  #define _gcc_
#endif

#if defined(_MSC_VER) || defined(__clang__)
  #define PRAGMA_COMMENT
#endif

#define RL_PI 3.1415926535897931
#define pi RL_PI

#define sign(x)		(((x) > 0) - ((x) < 0))
#define MAXN(x, y)	(((x) > (y)) ? (x) : (y))
#define MINN(x, y)	(((x) < (y)) ? (x) : (y))
