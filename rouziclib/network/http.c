#ifdef RL_INCL_NETWORK

size_t http_request(char *domain, char *port, char *request, int timeout, int retry, uint8_t **data, size_t *data_alloc, int mode)
{
	int error, ret=0;
	uint8_t *tcpdata, *pe, *p;
	int datasize, defli, payload_size=0;

	uint8_t reqcmd[16];

	tcpdata = tcp_request(domain, port, request, timeout, retry, &datasize);

	if (datasize <= 0)
		goto jump_end;

	sscanf(request, "%15s", reqcmd);
	if (strcmp(reqcmd, "GET")==0)
	{
		if (mode==MULTI_GZ_CAT)	// we decompress all the gzip packets and concatenate them together
		{
			p = tcpdata;
			pe = NULL;
			defli = 0;
			while (pe != &tcpdata[datasize])
			{
				p = bstrstr(p, &tcpdata[datasize]-p, "\r\n\r\n", 4);
				p = &p[4];									// point to the start of the current gzip packet
				pe = bstrstr(&p[1], &tcpdata[datasize]-&p[1], "HTTP/1.", strlen("HTTP/1."));	// point to the end of that packet
				if (pe==NULL)									// when we do the last packet
					pe = &tcpdata[datasize];

				size_t defli_as = *data_alloc-defli;
				error = gz_decompress(p, pe-p, &data[defli], &defli_as);
				if (error > 0)
					defli += error;

				ret = defli;
			}
		}
		else
		{
			p = strstr(tcpdata, "\r\n\r\n");
			p[2] = 0;		// null-terminate that part of the buffer to terminate the header
			p = &p[4];		// p points to the payload without the header

			payload_size = datasize-(p-tcpdata);

			if (strstr(tcpdata, "Content-Encoding: gzip"))
			{
				#ifdef RL_ZLIB
				ret = gz_decompress(p, payload_size, data, data_alloc);
				if (ret < 0)
					fprintf_rl(stderr, "gzip code %d\n", ret);
				#else
				fprintf_rl(stderr, "Cannot decode \"Content-Encoding: gzip\" in http_request(). RL_ZLIB needs to be defined.\n");
				goto jump_end;
				#endif
			}
			else
			{
				alloc_enough(data, payload_size+1, data_alloc, sizeof(uint8_t), 1.);
				memcpy(*data, p, payload_size);		// copy the raw payload
				(*data)[payload_size] = 0;
				ret = payload_size;

				if (payload_size==0)
					fprintf_rl(stderr, "No payload in HTTP message, header says:\n\n\"%s\"\n\nfor request:\n\n\"%s\"\n", tcpdata, request);
			}
		}
	}

jump_end:
	free (tcpdata);
	return ret;
}

size_t http_get(char *url, int timeout, int retry, uint8_t **data, size_t *data_alloc)	// works both with or without http://
{
	char *domain=NULL, *request=NULL, *p;
	int ret, start_domain=0, start_path=0;

	// look for the starting indexes in url
	p = strstr(url, "://");
	if (p)
		start_domain = 3 + p - url;

	p = strstr(&url[start_domain], "/");
	if (p)
		start_path = p - url;
	else
	{
		fprintf_rl(stderr, "Server file path not found in url \"%s\" in http_get()\n", url);
		return -1;
	}

	domain = calloc(start_path-start_domain + 1, sizeof(char));
	snprintf(domain, start_path-start_domain, "%s", &url[start_domain]);
	sprintf_realloc(&request, NULL, 0, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", &url[start_path], domain);

	ret = http_request(domain, "http", request, timeout, retry, data, data_alloc, DEFAULT);

	free(domain);
	free(request);

	return ret;
}

size_t http_get_buf(char *url, int timeout, int retry, buffer_t *buf)
{
	buf->len = http_get(url, timeout, retry, &buf->buf, &buf->as);

	return buf->len;
}

#endif
