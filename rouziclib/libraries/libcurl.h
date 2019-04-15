#ifdef RL_LIBCURL

#include <curl/curl.h>

#ifdef _MSC_VER
#pragma comment (lib, "libcurl.dll.a")
#endif

extern CURLcode curl_global_init_once(long flags);
extern int curl_https_get(char *url, int timeout, int retry, uint8_t **data, int *data_alloc);

#endif
