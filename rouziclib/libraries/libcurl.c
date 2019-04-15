#ifdef RL_LIBCURL

CURLcode curl_global_init_once(long flags)
{
	static int init=0;

	if (init)
		return 0;

	init = 1;
	return curl_global_init(flags);
}

size_t curl_rec_data(void *ptr, size_t size, size_t nmemb, buffer_t *buf)
{
	bufwrite(buf, ptr, size*nmemb);

	return size*nmemb;
}

int curl_https_get(char *url, int timeout, int retry, uint8_t **data, int *data_alloc)
{
	CURL *handle;
	CURLcode res;
	buffer_t buf={0};

	buf.buf = *data;
	buf.as = *data_alloc;

	curl_global_init_once(CURL_GLOBAL_DEFAULT);

	handle = curl_easy_init();
	if (handle)
	{
		curl_easy_setopt(handle, CURLOPT_URL, url);

		// Data handling callback
		curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curl_rec_data);
		curl_easy_setopt(handle, CURLOPT_WRITEDATA, &buf);

		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);	// skip certification verif
		curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);	// skip host name verif

		res = curl_easy_perform(handle);	// perform the request
		if (res != CURLE_OK)
			fprintf_rl(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

		curl_easy_cleanup(handle);
	}

	//curl_global_cleanup();

	*data = buf.buf;
	*data_alloc = buf.as;
	return buf.len;
}

#endif
