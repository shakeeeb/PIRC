// Devon & Shakeeb
// aka: YoureFailingTryHarder
#ifndef __SERVER_H
	#define __SERVER_H
	#include <stdio.h>
	#include <string.h>
	#include <unistd.h>
	#include <stdlib.h>
	#include <sys/socket.h>
	#include <sys/epoll.h>
	#include <pthread.h>
	#include <sys/types.h>
	#include <netdb.h>
	#include <errno.h>

	int open_listenfd(int port);
	void *login_thread(void *pfd);
	void* echo_thread(void *fd);

	#define MAXMSG 1001
	#define MAXNAME 1028
	int port = 0, eflag = 0, hflag = 0;
	char *motd;

	/*typedef struct {
		int maxfd;
		fd_set read_set;
		fd_set ready_set;
		int nready;
		int maxi;
		int clientfd[2048];
	} pool;

	typedef struct client { // structure to hold client information
		char username[MAXNAME];
		char password[MAXNAME];
		int fd;
	};*/

#endif
