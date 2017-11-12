#ifndef RL_EXCL_NETWORK

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib,"ws2_32.lib")

	#define errno WSAGetLastError()
#else
	#include <arpa/inet.h>
	#include <sys/socket.h>

	#define SOCKET int
#endif

enum { NO_RETRY, ONE_RETRY, INF_RETRY };

#define NET_DEF_TIMEOUT 12000

extern void net_init();
extern void net_deinit();

extern uint8_t *tcp_request(char *domain, char *port, char *request, int timeout, int retry, int *datasize);

#endif
