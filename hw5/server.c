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
        printf("./server [-he] PORT_NUMBER MOTD\n"
        	"-e\t\tEcho messages received on the server's stdout.\n"
        	"-h\t\tDisplay the help menu & returns EXIT_SUCCESS\n"
        	"PORT_NUMBER\t\tPort number to listen on.\n"
        	"MOTD\t\tMessage to display on the client when they connect.\n");
        exit(EXIT_SUCCESS); // exit success
        break;
      default: // default do nothing
        break;
    }
  }

  motd = malloc(MAXMSG); // allocate space for message of the day
  if(*argv[1] != 'e') { // check to see if the second arg contains an e flag
  	port = atoi(argv[1]);
  	motd = argv[2];
  } else { // if it does contain an e then move to next two
  	port = atoi(argv[2]);
  	motd = argv[3];
  }

  /* Print port number */
  printf("Currently listening on port %d.", port);

  /* Begin listening for clients */
  struct epoll_event ev, events[10];
  int listen_fd, *connfd, epollfd, numfd, i;
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;
  //static pool pool;

  // listen for clients
  listen_fd = open_listenfd(port);

  // create epoll event to initailize the epoll event struct
  epollfd = epoll_create(10);
  ev.events = EPOLLIN; // set events so you can read & write
  ev.data.fd = listen_fd;
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
  	perror("epoll_ctl: listen_sock");
  	exit(EXIT_FAILURE);
  }

  while (1) {
  	numfd = epoll_wait(epollfd, events, 10, -1);

  	for (i = 0; i < numfd; i++) {
  		if (events[i].data.fd == listen_fd) {
  			// create copy of connection file descriptor and accept fd
  			clientlen = sizeof(clientaddr); // TODO: Wrapper functions
  			connfd = malloc(sizeof(int));
  			*connfd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen);

            ev.events = EPOLLIN | EPOLLOUT; // set ability to read/write to fd
            ev.data.fd = *connfd;
            epoll_ctl(epollfd, EPOLL_CTL_ADD, *connfd, &ev);

            // create thread to login this fd
            pthread_create(&tid, NULL, login_thread, &connfd);
  		}
  	}
  }

}

int open_listenfd(int port) {
	// opens socket connection
	int listen_fd;
	struct sockaddr_in serveraddr;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serveraddr, sizeof(serveraddr));

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	serveraddr.sin_port = htons(port);

	bind(listen_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

	listen(listen_fd, 10);

	return listen_fd;
}

void* login_thread(void *fd) {
	int pfd = *((int *)fd);
	pfd = pfd;
	// code to login a client to the server
	// first check to see if the client's name is already in use
	// if yes then reject connection
	// if no then connect with the client
	// double check this pseudocode with the hw pdf
	if (eflag == 1) {
		// start echo thread
		eflag++;
	} else if (eflag == 2) {
		// do nothing?
	}

	return NULL;
}

void* echo_thread(void *fd) {
	// echo thread
}