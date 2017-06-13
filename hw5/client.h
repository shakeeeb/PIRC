#ifndef __CLIENT_C
#define __CLIENT_C
  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/time.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <netdb.h>
  #include <string.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include "shared.h"

  #define MAXLEN 1500 // just a reasonable number for the amount of data being sent over
  #define MAXMSG 1001 

  #ifdef DEBUG // use this statement for print debugging statements
		#define debug(fmt, ...) printf("\nDEBUG: %s\n\tFunction: %s\n\tLine #: %d\n\n", fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
	#else
		#define debug(fmt, ...)
	#endif

  void unix_error(char *msg); // prints unix error
  char* reversestr(const char* src); // reverses the strings, mallocs space and places in dest
  int Close(int fd); // wrapper over close
  int open_clientfd(char* hostname, char* port); // uses addrinfo, traverses it, and connects to the server
  int Open_clientfd(char* hostname, char* port); // wrapper around Open_clientfd
  int sendall(int fd, char* buf, int* len); //makes sure a certain number of bytes are sent out
  int sendAloha(int fd); // just sends out the initial aloha
  //int handshake(int fd); //initial aloha as well as identification of the user
  // TODO in handshake-- recieve the correct kind of errors
  int unblock(int fd); // sets a file descriptor to non blocking
  char* combineStrings(char* prefix, char* suffix); // this combines two strings by mallocing space and returns result
  void print_help(); // prints the help menu, for how the client is called
  int recv_all(int fd, char* buf); // receive all receives everything

   char* commands[] = {
    "/tell\0", // TELL 0
    "/createp\0", //CREATEP 1
    "/creater\0", //CREATER 2
    "/kick\0", //KICK 3
    "/quit\0", // BYE 4
    "/leave\0", //LEAVE 5
    "/join\0", //JOIN 6
    "/listrooms\0", //LISTR 7
    "/listusers\0", // LISTU 8
    "/joinp\0", // JOINP 9
    "/help\0", // ~ no verb 10
    NULL //double null terminated
  };
  // less commands than verbs
  // this is a two part array, honestly, i couldve just made an array of two tuples but fuck it
  // two arrays where one integer maps that maps to a command maps to its cooresponding verb

#endif
