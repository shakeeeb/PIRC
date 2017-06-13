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
	#include <fcntl.h>
	#include "shared.h"

	#ifdef DEBUG // use this statement for print debugging statements
		#define debug(fmt, ...) printf("\nDEBUG: %s\n\tFunction: %s\n\tLine #: %d\n\n", fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#else
		#define debug(fmt, ...)
	#endif

	int open_listenfd(int port);
	int non_blocking_fd(int fd);
	void *login_thread(void *pfd);
	int check_username(char *token);
	void* echo_thread(void *fd);
	void clear_buf(char buffer[], int size);
	int sendall(int fd, char* buf, int* len);
	int recv_all(int fd, char* buf);
	void help_menu();

	#define MAXMSG 1001
	#define MAXNAME 1028
	// flags and variables for arguments created at compile time
	int port = 0, eflag = 0, hflag = 0;
	// message of the day, open client > server, open server > client, open accept
	char *motd, *opencs = "ALOHA! \r\n", *opensc = "!AHOLA \r\n", *opena = "IAM", *msg = "MSG", *bye = "BYE \r\n", *echoall = "ECHO";
	char* initecho = "ECHO server";
	char* sorryerror = "ERR 00 SORRY";
	char *stderror = "ERR 100 INTERNAL SERVER ERROR \r\n";
	struct client *clienthead = NULL;

	struct client { // structure to hold client information
		char username[MAXNAME];
		char password[MAXNAME];
		int fd;
		struct client *prev, *next;
	};

#endif
