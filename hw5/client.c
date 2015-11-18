
#include "client.h"

int main (int argc, char** argv){
  // what will a client do?
  // a client takes three arguments
  // a name, ip address and a port number

  // the first argument could also be -h, which is help. if it's -h, print out the help menu
  // elsewise, don't print out the help menu. just do shit.
  char* host, char* port;
  char buffer[MAXLEN];
  int sock_fd = 0;
  int n = 0;
  int j = 0; // location is argv changes based on (lack) of a help menu

  if(argc != 4){
    unix_error("not enough arguments");
    exit(1);
  }
  if(strcmp(argv[1], "-h")) // if its -h

  host = argv[2]; // server ip
  port = argv[3]; // server port

  clientfd = Open_clientfd(host, port); // now i have an open socket file descriptor


  while(){

  }


}

/**
*Close is a wrapper around close
*
*/
int Close(int fd){
  int result = 0;
  if((result = close(fd)) < 0){
    unix_error("unable to close file");
  }
  return result;
}

/**
* unix style error from the textbook
*/
void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}
/**
*
*/
int open_clientfd(char* hostname, char* port){ // this function opens a client file descriptor
  int clientfd;
  struct addrinfo hints, *listp, *p;

  // get a list of potential server addresses

  memset(&hints, 0, sizeof(struct, addrinfo)); // zero it out
  hints.ai_socktype = SOCK_STREAM; // open a connection
  hints.ai_flags = AI_NUMERICSERV; // use a numeric port arg
  hints.ai_flags |= AI_ADDRCONFIG; // recommended for connections
  getaddrinfo(hostname, port, &hints, &listp);

  //traverse the list to get one that we can successfully connect to
  for(p = listp; p; p = p->ai_next){
    // create a socket descriptor
    if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
      continue; // socket failed, try again
    }
    // else it's not, connect to the server
    if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1){
      break; // success
    }
    Close(clientfd); // connect failed
  }

  //clean up
  freeaddrinfo(listp);
  if(!p){
    return -1;
  } else {
    return clientfd;
  }
}
/**
*
*/
int Open_clientfd(char* hostname, char* port){
  int result = 0;
  if((result = open_clientfd(hostname, port)) < 0){
    unix_error("unable establish connection");
  }
  return result;
}

void print_help(){
  printf("./client [-h] NAME SERVER_IP SERVER_PORT\n");
  printf("-h \t\t\t Displays help menu & returns EXIT_SUCCESS.\n");
  printf("NAME \t\t Username to display when chatting.\n");
  printf("SERVER_IP \t IP address of the server to connect to.\n");
  printf("SERVER_PORT \t Port to connect to.\n");
}
