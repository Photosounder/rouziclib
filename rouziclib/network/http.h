#ifdef RL_INCL_NETWORK

enum { DEFAULT, RAW, MULTI_GZ_CAT };

extern size_t http_request(char *domain, char *port, char *request, int timeout, int retry, uint8_t **data, size_t *data_alloc, int mode);
extern size_t http_get(char *url, int timeout, int retry, uint8_t **data, size_t *data_alloc);
extern size_t http_get_buf(char *url, int timeout, int retry, buffer_t *buf);

#endif
