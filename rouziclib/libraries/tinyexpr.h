#include "orig/tinyexpr.h"

#define RL_EXPR_FUNC				\
	{"gaussian", gaussian, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"erf", erf, TE_FUNCTION1 | TE_FLAG_PURE},		\
	{"erfr", erfr, TE_FUNCTION1 | TE_FLAG_PURE},		\
	{"lgamma", lgamma, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"gamma", tgamma, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"gamma_dist", gamma_dist, TE_FUNCTION3 | TE_FLAG_PURE}, \
	{"sinc", sinc, TE_FUNCTION2 | TE_FLAG_PURE},		\
	{"blackman", blackman, TE_FUNCTION2 | TE_FLAG_PURE},	\
	{"db_to_vol", db_to_vol, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"vol_to_db", vol_to_db, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"nearbyint", nearbyint, TE_FUNCTION1 | TE_FLAG_PURE},	\
	{"sq", sq, TE_FUNCTION1 | TE_FLAG_PURE}
