#define STRINGIFY(x) #x					// makes the name of x a string
#define VALUE_STRINGIFY(s) STRINGIFY(s)			// makes the value of x a string
#define flag_update(x)	if (abs(x)==2) (x) >>= 1
