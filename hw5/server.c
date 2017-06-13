// Devon & Shakeeb
// aka: YoureFailingTryHarder

#include "server.h"

int main (int argc, char ** argv) {
  /* Check number of arguments */
  if (argc < 3) { // check if there are enough arguments
  	fprintf(stderr, "Error: Too few args\n"); // if not print to stderr
    help_menu();
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
        help_menu();
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

  debug("debug is working?!?");

  /* Print port number */
  fprintf(stdout, "Currently listening on port %d.\n", port);
  fflush(stdout);

  /* Begin listening for clients */
  struct epoll_event ev, events[10]; // create inital epollevent
  int listen_fd, *connfd, epollfd, numfd, i; // several file descriptors
  socklen_t clientlen; // length of teh cleitn in socklen
  struct sockaddr_storage clientaddr; // sockadder
  pthread_t lid = 0; // pthread id
  lid = lid;
  // listen for clients
  listen_fd = open_listenfd(port);

  // create epoll event to initailize the epoll event struct
  epollfd = epoll_create(10);
  ev.events = EPOLLIN; // set events so you can read & write
  ev.data.fd = listen_fd; //
  if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &ev) == -1) {
  	perror("epoll_ctl: listen_sock");
  	exit(EXIT_FAILURE);
  }
  // wait for clients and accept them one by one, or as a group to login
  while (1) {
  	numfd = epoll_wait(epollfd, events, 10, -1);
    // keep accepting new clients in a loop
  	for (i = 0; i < numfd; i++) {
      if(events[i].events & EPOLLERR || events[i].events & EPOLLHUP){ // if there is some sort of error
        fprintf(stderr, "epoll error\n");
        close(events[i].data.fd);
        continue;
      } else if (events[i].data.fd == listen_fd) { // if the listen fd has found a connection
        // the second check is for the listening socket.
        // the last check is for epollin
  			// create copy of connection file descriptor and accept fd
  			clientlen = sizeof(clientaddr); // TODO: Wrapper functions
  			connfd = malloc(sizeof(int));
  			*connfd = accept(listen_fd, (struct sockaddr *)&clientaddr, &clientlen);

        non_blocking_fd(*connfd);

        ev.events = EPOLLIN | EPOLLET; // set ability to read/write to fd
        ev.data.fd = *connfd; // adds teh file descripotor to the epollset
        epoll_ctl(epollfd, EPOLL_CTL_ADD, *connfd, &ev);

        // create thread to login this fd
        pthread_create(&lid, NULL, login_thread, connfd);
  		} else { // if its not listenfd that has found a connection, what's going on?
      // there is data on the current, anonymous file descriptor waiting to be read
      // check if it's an aloha
      // WAIT RETRY WITH NONBLOCKINGS
      char* readbuf = malloc(MAXMSG);
      int cbytes = 0;
      if((cbytes = recv_all(events[i].data.fd, readbuf) == 0)){
        printf("i didnt get anything");
      }
      printf("%s", readbuf);
      // if it's aloha, begin login protocol
      // lock the filedescriptor for the time being,
      // epoll will intercept the use of the file descriptor. use a mutex


      }
  	}
  }
}

/* Open the listening file descriptor */
int open_listenfd(int port) {
  debug("opening listenfd");
	// opens socket connection
	int listen_fd;
	struct sockaddr_in serveraddr;

  // create the socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(listen_fd, F_SETFL, O_NONBLOCK);
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

int non_blocking_fd(int fd) {
  int flag, e;

  flag = fcntl(fd, F_GETFL, 0);
  if (flag == -1) {
    return -1;
  }

  flag |= O_NONBLOCK;
  e = fcntl (fd, F_SETFL, flag);
  if (e == -1) {
    return -1;
  }

  return 0;
}

/* Thread for logging in clients */
void* login_thread(void *fd) {
  pthread_detach(pthread_self());
  debug("starting login thread");

	int pfd = *((int *)fd);
  char /**whitespace = " \n\r\t",*/ *buffer = malloc(MAXMSG);
  struct client *newclient;
  int bytes, count = 0, n;
  char *token = NULL;

	pthread_t eid;
  //clear_buf(buffer, MAXMSG);
  // FIRST connect to the server, so there should be an initial "ALOHA!", "!AHOLA", and "IAM <username>"
  if ((bytes = recv_all(pfd, buffer)) != 0) {
    debug(buffer);
    //char *bptr = buffer;
    if (strcmp(buffer, opencs) == 0) {
      n = strlen(opensc);
      sendall(pfd, opensc, &n);
      debug(opensc);
    } else {
      n = 15;
      sendall(pfd, "ERR 00 SORRY \r\n", &n);
      debug("ERR 00 SORRY \r\n");
      n = strlen(bye);
      sendall(pfd, bye, &n);
      return NULL;
    }
  } else {
    debug("We didn't get anything...");
  }
  // SECOND take the username and check to see if it's already in use
  //clear_buf(buffer, MAXMSG);
  // get the information from the client and start tokenizing on space
  if ((bytes = recv_all(pfd, buffer)) != 0) {
    token = strsep(&buffer, " ");
  }
  // while the token isn't null, find username. IAM on count zero and username on count one

  while(token != NULL) {
    // add a null character to the end
    token = strcat(token, "\0");
    // check for the "IAM" message, if found continue
    if(count == 0){
      if(strcmp(token, opena) == 0){
        token = strsep(&buffer, " ");
        count++;
        continue;
        // if not found, standard error
      } else {
        // something went wrong, send the standard error
        n = strlen(stderror);
        sendall(pfd, stderror, &n);
        debug(stderror);
        break;
      }
    } // end of if count == 0
    // don't set things to not blocking
    // if count is one check for the username
    if (count == 1) {
      if (check_username(token) == 0) {
        // create struct for the new client
        newclient = malloc(sizeof(struct client));
        strcpy(newclient->username, token);
        newclient->fd = pfd; // sets the file descriptor for the

        // add the new client to the linked list
        if (clienthead == NULL) {
          clienthead = newclient;
          newclient->prev = NULL;
          newclient->next = NULL;
        } else {
          newclient->next = clienthead;
          newclient->prev = NULL;
          clienthead->prev = newclient;
          clienthead = newclient;
        }

        // send greeting to the client
        char *greeting = malloc(MAXMSG);; // ok so this is segfaulting, so lets make it so it doesn't segfault
        snprintf(greeting, MAXMSG, "%s %s%s", verbs[15], token, cr); //place shit int greeting
        n = strlen(greeting);
        sendall(pfd, greeting, &n); // i need to know when to catch this...
        debug(greeting);
        char* sendmotd = malloc(MAXMSG);
        snprintf(sendmotd, MAXMSG, "%s %s%s", initecho, motd, cr);
        n = strlen(sendmotd);
        sendall(pfd, sendmotd, &n); // i also need to know how to catch this...
        debug(sendmotd);
        //free(greeting);
        //free(motd);
      } else {
        // the username is taken, reject connection with the client
        char* reject = malloc(MAXMSG);
        snprintf(reject, MAXMSG,"%s %s%s", sorryerror, token, cr);
        n = strlen(reject);
        sendall(pfd, reject, &n);
        debug(reject);
        n = strlen(bye);
        memset(reject, 0, MAXMSG); // clean reject
        snprintf(reject, MAXMSG, "%s %s", bye, cr);
        sendall(pfd, bye, &n);
        debug(bye);
      }
    } else { // count isn't equal to 1
      // something went wrong, send the standard error
      n = strlen(stderror);
      sendall(pfd, stderror, &n);
      debug(stderror);
      break;
    }
    // increment count
    count++;
    // get the next token
    token = strsep(&buffer, " ");
  } // endof while token = null
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
  // kill this thread
  free(buffer);
  pthread_exit(NULL); // exit the login thread as it is done logging in the client
	return NULL;
}

/* Check to make sure the username isn't already used */
int check_username(char *token) {
  // if username is found return a 1
  // if username is not found return 0
  struct client *ptr = clienthead;
  // if the head is null indicate the username is not used
  if ((strcmp(token, "server")) == 0) {
    return 1;
  }
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
  return NULL;
}

/* This function clears any buffer */
void clear_buf(char buffer[], int size) {
  int count = 0;
  while (count < size) {
    buffer[count] = '\0';
    count++;
  }
}

int sendall(int fd, char* buf, int* len) {
  // send all returns 0 if successful
  // returns -1 if not
  int total = 0;
  int tosend = strlen(buf);
  int bytesleft = *len;
  int n;

  while(total < tosend) {
      n = send(fd, buf+total, bytesleft, 0);
      if(n == -1)
        break;
      total += n;
      bytesleft -= n;
  }
  *len = total; // the bytes actually sent will be recorded here

  if(n == -1){
    return -1;
  } else {
    return 0;
  }
}

int recv_all(int fd, char* buf) {
  // the buffer should already be malloced or just be a static array
  // assume buf is currently empty, or ust make it empty here
  memset(buf, 0, MAXMSG); // cleans all of the buffer
  int result = 0, n; // result is the number of bytes read out
  char intermediary[MAXMSG];
  while((n = recv(fd, intermediary, MAXMSG, 0)) >= 0){ // while it hasnt recieved zero OR i could change zero to a -1
    // append the recieved stuff into buf
    result += n;
    buf = strcat(buf, intermediary); //so everything slowly adds on into the buffer
    memset(intermediary, 0, MAXMSG);
  }
  // this mutates buffer in place
  return result;
}

void help_menu() { // prints the help menu, go figure :P
  printf("./server [-he] PORT_NUMBER MOTD\n"
          "-e\t\tEcho messages received on the server's stdout.\n"
          "-h\t\tDisplay the help menu & returns EXIT_SUCCESS\n"
          "PORT_NUMBER\tPort number to listen on.\n"
          "MOTD\t\tMessage to display on the client when they connect.\n");
}

char* getUsername(char* buffer){ // just grabs the username, if it's sequenced properly
  char* username;
  char* endl;
  char* iam = strsep(&buffer, " ");
  if(strcmp(verbs[12], iam) == 0){
    username = strsep(&buffer, " ");
    endl = strsep(&buffer, " ");
    if(strcmp(cr_nospace, endl) == 0){
      return username;
    }
  } else { return NULL; }
  return NULL;
}

char* combineStrings(char* prefix, char* suffix){
  // this mallocs sace as well as putting two strings together
  //strcat adds a null, so remeber that
  char* result;
  result = malloc(strlen(prefix) + strlen(suffix) + 1);
  strcpy(result, prefix); // copy prefix over to result
  strcat(result, suffix); // append on the suffix
  return result;
}
