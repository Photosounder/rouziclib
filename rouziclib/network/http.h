#ifdef RL_INCL_NETWORK

enum { DEFAULT, RAW, MULTI_GZ_CAT };

extern int http_request(char *domain, char *port, char *request, int timeout, int retry, uint8_t **data, int *data_alloc, int mode);
extern int http_get(char *url, int timeout, int retry, uint8_t **data, int *data_alloc);

#endif
