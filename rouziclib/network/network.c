#ifdef RL_INCL_NETWORK

static int init_net=0;

void net_init()
{
	#ifdef _WIN32
	WSADATA wsaData;

	if (init_net==0)
	{
		WORD version_wanted = MAKEWORD(1,1);

		if (WSAStartup(MAKEWORD(2, 2), &wsaData))
		{
			fprintf_rl(stderr, "Couldn't initialize Winsock 2.2\n");
			return ;
		}

		init_net = 1;
	}
	#endif
}

void net_deinit()
{
	#ifdef _WIN32
	if (init_net)
	{
		WSACleanup();

		init_net = 0;
	}
	#endif
}

uint8_t *tcp_request(char *domain, char *port, char *request, int timeout, int retry, int *datasize)
{
	struct addrinfo *ai=NULL, hints={0};
	SOCKET client;
	struct sockaddr_in sadin;

	const int reczise = 1024;
	uint8_t *received;
	int i, ret=0, rec=1;

	int dataindex, data_alloc = 65536;
	uint8_t *data, *p;

	net_init();

	if (timeout < 0)
		timeout = NET_DEF_TIMEOUT;

	// Get the IP for the domain

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	ret=1;
	i=0;
	while (ret && (i==0 || retry==INF_RETRY || (retry==ONE_RETRY && i <= 1)))
	{
		ret = getaddrinfo(domain, port, &hints, &ai);
		if (ret)
		{
			i++;
			fprintf_rl(stderr, "getaddrinfo(%s, %s, ... ) failed\n", domain, port);
			freeaddrinfo(ai);

			if (retry==INF_RETRY || (retry==ONE_RETRY && i <= 1))
				sleep_ms(1000);
			else
			{
				fprintf_rl(stderr, "tcp_request(%s, %s, ... ) abandons\n", domain, port);
				*datasize = 0;
				return NULL;
			}
		}
	}

	received = calloc (reczise, sizeof(uint8_t));
	data = calloc (data_alloc, sizeof(uint8_t));

loopstart:
	client = socket(ai->ai_family, SOCK_STREAM, IPPROTO_TCP);		// Create the socket
	connect (client, ai->ai_addr, (int) ai->ai_addrlen);		// Connect to the server

	if (setsockopt (client, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout)))
		fprintf_rl(stderr, "Setting the recv() timeout failed\n");

	if (setsockopt (client, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, sizeof(timeout)))
		fprintf_rl(stderr, "Setting the send() timeout failed\n");

	// Successfully connected, HTTP request goes here
	send (client, request, strlen(request), 0);

	i=0;
	rec=1;
	dataindex = 0;
	while (rec > 0)
	{
		rec = recv (client, received, reczise, 0);
		if (rec < 0)
		{
			fprintf_rl(stderr, "Connection to %s lost... Error %d\n", domain, errno);
			closesocket(client);

			i++;

			if (retry==INF_RETRY || (retry==ONE_RETRY && i <= 1))
				goto loopstart;
			else
			{
				fprintf_rl(stderr, "Connection lost, tcp_request(%s, %s, %s, ... ) abandons\n", domain, port, request);
				goto jump_end;
			}
		}
		else
		{
			alloc_enough(&data, dataindex + rec, &data_alloc, sizeof(uint8_t), 4.);

			memcpy(&data[dataindex], received, rec * sizeof(uint8_t));
			dataindex += rec;
		}
	}

	closesocket(client);

jump_end:
	freeaddrinfo(ai);
	free (received);

	*datasize = dataindex;

	if (dataindex==0)
	{
		free(data);
		data = NULL;
	}
	else
		data = realloc(data, *datasize);	// shrink the data buffer to the right size

	return data;					// data isn't null-terminated
}

#endif
