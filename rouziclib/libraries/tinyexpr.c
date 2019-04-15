#define TE_POW_FROM_RIGHT
#define TE_NAT_LOG
#undef pi

#pragma function (ceil)		// needed to avoid C2099 errors
#pragma function (floor)

#include "orig/tinyexpr.c"
#define pi RL_PI
