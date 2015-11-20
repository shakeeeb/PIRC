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
	// flags and variables for arguments created at compile time
	int port = 0, eflag = 0, hflag = 0;
	// message of the day, open client > server, open server > client, open accept
	char *motd, *opencs = "ALOHA! \r\n", *opensc = "!AHOLA \r\n", *opena = "IAM", *sendall = "MSG", *bye = "BYE \r\n", *echoall = "ECHO";
	char *stderror = "ERR 100 INTERNAL SERVER ERROR \r\n";
	char *clienthead = NULL;

	typedef struct client { // structure to hold client information
		char username[MAXNAME];
		char password[MAXNAME];
		int fd;
		int *prev, *next;
	};

#endif
