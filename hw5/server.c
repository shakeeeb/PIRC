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
        // print the help menu
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
  	port = atoi(argv[1]); // change the port to an integer
  	motd = argv[2]; // set the message of the day global variable
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
  pthread_t lid;

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
  // wait for clients and accept them one by one, or as a group to login
  while (1) {
  	numfd = epoll_wait(epollfd, events, 10, -1);
    // keep accepting new clients in a loop
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
        pthread_create(&lid, NULL, login_thread, &connfd);
  		}
  	}
  }
}

/* Open the listening file descriptor */
int open_listenfd(int port) {
	// opens socket connection
	int listen_fd;
	struct sockaddr_in serveraddr;

  // create the socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&serveraddr, sizeof(serveraddr));

  // assign everything
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htons(INADDR_ANY);
	serveraddr.sin_port = htons(port);

  // bind the socket
	bind(listen_fd, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

  // listen on that file descriptor
	listen(listen_fd, 10);

	return listen_fd;
}

/* Thread for logging in clients */
void* login_thread(void *fd) {
	int pfd = *((int *)fd);
  char *whitespace = " \n\r\t", buffer[MAXMSG];
  struct client = malloc(sizeof(client));
  char *bptr = &buffer[];
  int bytes, count = 0;
  char *token;
	pthread_t eid;

  clear_buf(buffer[], MAXMSG);
  // FIRST connect to the server, so there should be an initial "ALOHA!", "!AHOLA", and "IAM <username>"
  if ((bytes = recv(pfd, buffer, MAXMSG, 0)) != 0) {
    if (strcmp(buffer, opencs) == 0) {
      send(fd, opensc, 9, 0);
    } else {
      send(fd, "ERR 00 SORRY \r\n")
      send(fd, bye, strlen(bye), 0);
      return NULL;
    }
  }
  // SECOND take the username and check to see if it's already in use
  clear_buf(buffer[], MAXMSG);
  // get the information from the client and start tokenizing on space
  if ((bytes = recv(pfd, buffer, MAXMSG, 0)) != 0) {
    token = strtok(bptr, " ");
  }
  // while the token isn't null, find username. IAM on count zero and username on count one
  while(token != NULL) {
    // add a null character to the end
    token = strcat(token, "\0");
    // check for the "IAM" message, if found continue
    if ((strcmp(token, opena) == 0) && count == 0) {
      continue;
    // if not found, standard error
    } else {
      // something went wrong, send the standard error
      send(pfd, stderror, strlen(stderror), 0);
      break;
    }
    // if count is one check for the username
    if (count == 1) {
      if (check_username(token) == 0) {
        // create struct for the new client
        client newclient = malloc(client);
        strcpy(newclient.username, token);
        newclient.fd = fd;

        // add the new client to the linked list
        if (clienthead == NULL) {
          clienthead = &newclient;
          newclient.prev = NULL;
          newclient.next = NULL;
        } else {
          newclient.next = *clienthead;
          newclient.prev = NULL;
          clienthead->prev = &newclient;
          clienthead = &newclient;
        }

        // send greeting to the client
        char *greeting = "HI " + token;
        send(pfd, greeting, strlen(greeting), 0);
        char *sendmotd = "ECHO server " + motd;
        send(pfd, sendmotd, strlen(sendmotd), 0);
      } else {
        // the username is taken, reject connection with the client
        char *reject = "ERR 00 SORRY " + token + " \r\n";
        send(pfd, reject, strlen(reject), 0);
        send(pfd, bye, strlen(bye), 0);
      }
    } else {
      // something went wrong, send the standard error
      send(pfd, stderror, strlen(stderror), 0);
      break;
    }
    // increment count
    count++;
    // get the next token
    token = strtok(NULL, " ");
  }
  //  IF the username is NOT in use then create a new port to communicate on & create client link
  //    Also check to make sure username is within the visible ascii spectrum --> NO WHITESPACE
  //  ELSE the username is in use then reject the connection with "ERR 00 SORRY <username>"

  // After user is logged in and connected then check to see if echo thread has been started using flag
	if (eflag == 1) { // echo thread not started
		// start echo thread
    pthread_create(&eid, NULL, echo_thread, NULL);
		eflag++;
	} else if (eflag == 2) { // if already running do nothing
		// do nothing?
	}
  // uhhh... isn't this self explanitory?
	return NULL;
}

/* Check to make sure the username isn't already used */
int check_username(token) {
  // if username is found return a 1
  // if username is not found return 0
  char *ptr = clienthead;
  // if the head is null indicate the username is not used
  if (ptr == NULL) {
    return 0;
  } else { // check to see if the username is present in the linked list
    while(ptr->next != NULL) {
      // if it's present then return 1
      if ((strcmp(token, ptr->username)) == 0) {
        return 1;
      } else { // else advance the pointer
        ptr = ptr->next;
      }
    }
    // finally compare the last username in the list
    if ((strcmp(token, ptr->username)) == 0) {
      return 1;
    }
    // if nothing matches return 0
    return 0;
  }
}

/* Function for starting the echo thread */
void* echo_thread(void *n) {
	// echo thread
}

/* This function clears any buffer */
void clear_buf(char buffer[], int size) {
  int count = 0;
  while (count < size) {
    buffer[count] = '\0';
  }
}