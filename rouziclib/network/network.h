#ifdef RL_INCL_NETWORK

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib,"ws2_32.lib")

	#ifndef errno
	#define errno WSAGetLastError()
	#endif
#else
	#include <arpa/inet.h>
	#include <sys/socket.h>
	#include <netdb.h>

	#define SOCKET int
	#define closesocket close
#endif

extern void net_init();
extern void net_deinit();

extern uint8_t *tcp_request(char *domain, char *port, char *request, int timeout, int retry, int *datasize);

#endif

enum { NO_RETRY, ONE_RETRY, INF_RETRY };

#define NET_DEF_TIMEOUT 12000
