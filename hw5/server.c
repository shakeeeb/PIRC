// Devon & Shakeeb
// aka: YoureFailingTryHarder

#include "server.h"

int main (int argc, char ** argv) {
  /* Check number of arguments */
  if (argc < 3) { // check if there are enough arguments
  	fprintf(stderr, "usage: %s <port>\n", argv[0]); // if not print to stderr
  	exit(EXIT_FAILURE); // exit failure
  }

  /* Get the arguments from cmd line */
  int opt; // check for flags
  while((opt = getopt(argc, argv, "eh")) != -1) {
    switch(opt) {
      case 'e': // echo flag
        eflag++;
        break;
      case 'h': // help flag
        hflag++;
        printf("./server [-he] PORT_NUMBER MOTD\n
        	-e\t\tEcho messages received on the server's stdout.\n
        	-h\t\tDisplay the help menu & returns EXIT_SUCCESS\n
        	PORT_NUMBER\t\tPort number to listen on.\n
        	MOTD\t\tMessage to display on the client when they connect.\n");
        exit(EXIT_SUCCESS); // exit success
        break;
      default: // default do nothing
        break;
    }
  }

  motd = malloc(MAXMSG); // allocate space for message of the day
  if(**argv[1] != 'e') { // check to see if the second arg contains an e flag
  	port = argv[1];
  	motd = argv[2];
  } else { // if it does contain an e then move to next two
  	port = argv[2];
  	motd = argv[3];
  }

  /* Print port number */
  printf("Currently listening on port %d.", port);

  /* Begin listening for clients */
  int listen_fd, connfd;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  listen_fd = open_listenfd(port);
  while (1) {
  	clientlen = sizeof(clientaddr); // TODO: Wrapper functions
  	connfd = accept(listen_fd, (SA *)&clientaddr, &clientlen);
  	// use epoll instead of select
  }

}