
#include "client.h"

int main (int argc, char** argv){
  // what will a client do?
  // a client takes three arguments
  // a name, ip address and a port number

  // the first argument could also be -h, which is help. if it's -h, print out the help menu
  // elsewise, don't print out the help menu. just do shit.
  char* host;
  char* port;
  char buffer[MAXMSG];
  char* intermediary;
  int sock_fd = 0;
  int bytes_sent = 0;
  int execution_done = 0;
  int bytes_recv = 0;
  char* username;
  bytes_recv = bytes_recv;
  //int j = 0; // location is argv changes based on (lack) of a help menu
  int running = 0;
  fd_set readfds;
  // never tokenize static strings

  if(strcmp(argv[1], "-h") == 0){ // -h flag
    print_help();
    exit(0);
  }
  if(argc < 3){
    unix_error("not enough arguments");
    exit(1);
  }
  // 0 name
  // 1 username
  // 2 IP in dotted decimal notation
  // 3 Port number-- no its okay right now
  username = argv[1]; // username
  host = argv[2]; // server ip
  port = argv[3]; // server port
  username = username;

  sock_fd = Open_clientfd(host, port); // now i have an open socket file descriptor
  if(sock_fd == -1){
    unix_error("the socket's returning negative one. this is a port cooldown issue");
    exit(1);
  }
  // this should also return the addrinfo somehow, so i have a handle to it here.
  // initiate login by sending aloha
  // teh question is, should i i/o multiplex before sending aloha?
  // or should i send aloha, and then i/o multiplex after establishing a connection
  // try the handshake, and this willr eturn 0 if successful
  //unblock(sock_fd);
  if(sendAloha(sock_fd) != 0){
    // something happened and its up to me to find out who it was and why
    unix_error("send aloha error");
    exit(4);
    // lol jk youre fucked
  }

  //after this, start the io multiplexing
  // gotta io multiplex between the socket and stdin
  // i can select here because this isnt multithreaded
  // int select(int numfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
  // numfds should be highest fd + 1
  FD_ZERO(&readfds); // zeroes it out
  FD_SET(STDIN_FILENO, &readfds); //set stuff
  FD_SET(sock_fd, &readfds); // sets more stuff
  int i;
  char* cmd;
  char* leftover;
  //unblock(sock_fd); //honestly, why would be need to not block? doesnt blocking help,
  //becuase it waits for input to finish
  while(running == 0){
    if(select(sock_fd+1, &readfds, NULL, NULL, NULL) == -1){
      unix_error("select error");
      exit(4);
    }
    execution_done = 0;
    //                          //
    // IF READING FROM THE USER //
    //                          //
    if(FD_ISSET(STDIN_FILENO, &readfds)){ // the user is sending us information
      // fgets(buffer, MAXMSG, stream)
      memset(buffer, 0, MAXMSG); // zero out the buffer
      fgets(buffer, MAXMSG, STDIN_FILENO); //get the command from the user
      //check if it's a slash command, otherwise it's just a regular message
      char c = buffer[0]; //the first character of the buffer needs to be a /
      // IF ITS NOT A SLASH COMMAND
      if(c != '/'){ // its a message
        // append msg and send message msg is 12
        intermediary = malloc(MAXMSG+24);
          execution_done = 1;
          snprintf(intermediary, MAXMSG, "%s %s", verbs[12], buffer); // everything is written into int
          if(sendall(sock_fd, intermediary, &bytes_sent) != 0){ // its supposed to be zero
            unix_error("couldnt send the message");
          }
        // end of if it's not a slash command
      } else { // its a slash command
        // i gotta look for what kind of slash command it is
        // grab it out
        leftover = strdup(buffer);
        // parse buffer
        cmd = strsep(&leftover, " ");//parse on whitespace
        for(i = 0;i<sizeof(commands);i++){
          if(commands[i] == NULL){
            // not legitimate command
            unix_error("not legitimate command");
            break;
          }
          if(strcmp(cmd, commands[i]) == 0){
            switch(i){ // case on the index of the slash command.
              // this is gonna be a big ass switch statement
              // certain commands are done differently
              // miuki did a really smart way, with a helper function w
              // where certain possible things could eb set to null
              case 0: // /tell > TELL <name> <message>
              case 1: // /createp > CREATEP <name> <password>
              case 2: // /creater > CREATER <name>
              case 3: // /kick > KICK <name>
              case 4: // /bye > BYE <noargs>
              case 5: // /leave > LEAVE <noargs>
              case 6: // /join > JOIN <id>
              case 7: // /listrooms > LISTR <noargs>
              case 8: // /listusers > LISTU <noargs>
              case 9: // /joinp > JOINP <id> <password>
                //append the the verb to the leftover
                intermediary = malloc(MAXMSG);
                snprintf(intermediary, MAXMSG, "%s %s%s", verbs[i], leftover, cr);
                if(sendall(sock_fd, buffer, &bytes_sent) != 0){
                  unix_error("unable to send all bytes");
                }
                execution_done = 1;
                break;
              case 10: // /help <doesnt go to server>
                print_help();
                execution_done = 1;
                break;
              default: // how the fuck
                unix_error("hit default on switch statment");
              // should't neeed to break at default
            } // end of switch
          } // end of if statement
          if(execution_done==1){
            // execution is done
            break; // break the for loop
          }
        } // end of for loop
        execution_done = 0;
      } // end of else statement, for else it's a slash command
    } // end of if isset, which is the selection
    // so this is the end of teh case where we recieve input from the user //
    //                       //
    // GET INPUT FROM SERVER //
    //                       //
    if(FD_ISSET(sock_fd, &readfds)){ // the server is sending us information
      //is the server sends us information its either an acknoweldgement
      // a message to be echoed , or some sort of commmand, or some sort of error
      recv_all(sock_fd, buffer); // receive all from the server
      //after recieving all, parse out the command, because it has to be some sort of command
      // these are the variables you have: buffer(the string from serv),
      // int bytes sent int bytes recv int execution done, char* intermediary
      leftover = leftover;
      leftover = strdup(buffer);
      cmd = strsep(&leftover, " ");
      // you can't handshake before iomultiplexing, unfortunately
      // so i've gotta send the initial aloha, and then wait for ahola in the iomultiplexed client code
      //printf("%s", buffer);
      for(i=0;i<sizeof(verbs);i++){ // iterate through the list of verbs
        //okay
        if(verbs[i] == NULL){
          printf("not a supported command");
          break;
        }
        if(strcmp(verbs[i], cmd) == 0){
          switch(i){ // switch on the index
            //WAIT! it could be an acknoweldgement --- ughhh
            // so i guess i have to include all possible acks in the array that contains all verbs
            // only cases i have to support are
            // 11 ahola, 15 hi, and motd, so default, print everything
            case 11: // !AHOLA
              // ask user for username
              // this is where i send the username, which should be a command line argument
              //printf("please enter your username: ");
              snprintf(buffer, MAXMSG, "%s %s%s", verbs[12], username, cr);
              if(sendall(sock_fd, buffer, &bytes_sent)){
                unix_error("couldn't send username");
              }
              execution_done = 1;
              // and then the user will enter user information
              break;
            case 15: //HI <username>
              printf("%s", buffer);
              execution_done = 1;
              break;
            case 16: //ECHO <message>
              printf("%s", leftover);
              execution_done = 1;
              break;
            default: // literally everything else, at least for now
              printf("%s", buffer);
              execution_done = 1;
              break;
          }
        }
        if(execution_done == 1){
          break;
        }
      } // end of for loop
    } // end of if im getting things from the server
  } // end of major while loop
  return 0;
} // end of main

char* reversestr(const char* src){ //dest is unmalloced
  char* dest;
  int i = 0;
  int lastpos = strlen(src);
  int currentpos = lastpos -1;
  dest = malloc(lastpos);
  char currentchar;

  // i ahve to change this so i start from the beginning of dest and the end of src

  for(i = 0;i<lastpos; i++){
    currentchar = src[currentpos];
    dest[i] = currentchar;
    currentpos--;
  }
  dest[lastpos] = '\0';
  // now dest has been mutated
  return dest;

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
  // pass in listp and p, because im gonna need to reference them in the main
  struct addrinfo hints, *listp, *p;

  // get a list of potential server addresses
  memset(&hints, 0, sizeof(struct addrinfo)); // zero it out
  hints.ai_socktype = SOCK_STREAM; // open a connection
  hints.ai_flags = AI_NUMERICSERV; // use a numeric port arg
  hints.ai_flags |= AI_ADDRCONFIG; // recommended for connections
  // use htons 
  // use inetaddr to get the ipddr
  // then just connect
  getaddrinfo(hostname, port, &hints, &listp);

  //traverse the list to get one that we can successfully connect to
  for(p = listp; p; p = p->ai_next){
    // create a socket descriptor
    if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
      continue; // socket failed, try again
    }
    // i ahve to use htons at a certain point on both ip and port
    // else it's not, connect to the server
    if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1){
      break; // success
    }
    Close(clientfd); // connect failed
  }

  //clean up
  freeaddrinfo(listp);
  if(!p){ // not p? does p still exist at this point?
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

int recv_all(int fd, char* buf){
  // the buffer should already be malloced or just be a static array
  // assume buf is currently empty, or ust make it empty here
  memset(buf, 0, MAXMSG); // cleans all of the buffer
  int result = 0; // result is the number of bytes read out
  int n = 0;
  char intermediary[MAXMSG];
  while((n = recv(fd, intermediary, MAXMSG, 0)) >= 0){ // while it hasnt recieved zero OR i could change zero to a -1
    // append the recieved stuff into buf
    result += n;
    buf = strcat(buf, intermediary); //so everything slowly adds on into the buffer
    memset(intermediary, 0, MAXMSG); //
  }
  // this mutates buffer in place
  return result;
}

int sendall(int fd, char* buf, int* len){
  // send all returns 0 if successful
  // returns -1 if not
  // dont actuallt send en bytes, send strlen bytes and use len as a return
  int total = 0;
  int tosend = strlen(buf);
  int bytesleft = strlen(buf);
  int n;

  while(total < tosend){
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
/* this will handle aloha
*
*/
int sendAloha(int fd){
  char buffer[MAXMSG];
  memset(buffer, 0, MAXMSG);
  int bytes_sent;

  snprintf(buffer, MAXMSG, "%s%s", verbs[10], cr);

  if(sendall(fd, buffer,  &bytes_sent) != 0){
    unix_error("unable to send ALOHA");
    return -1;
  }
  return 0;
}
/*
int handshake(int fd){
  // will return 0 if successful
  //elseewise -1
  char* ack = 0;
  char recvbuf[MAXMSG];
  char buffer[MAXMSG];
  char sendbuf[MAXMSG];
  char* temp;
  // in the client the thing is non blocking, so set it to non blocking
  int bytes_sent;
  bytes_sent = strlen(verbs[10]);
  int bytes_recv;
  // now i have to remember numbers instead of commands themselves
  // aloha is verb 10, iam is verb 11, hi is verb 14
  ack = reversestr(verbs[10]);
  ack = combineStrings(ack, cr); // add on carraige returns
  // bytes sent gets changed by sendall
  snprintf(sendbuf, MAXMSG, "%s %s", verbs[10], cr);
  //temp = combineStrings(verbs[10], " \0");
  //temp = combineStrings(temp, cr);
  //temp = "ALOHA! \r\n\0";
  bytes_sent = strlen(temp);
  printf("%s", temp);
  if(sendall(fd, temp, &bytes_sent)!= 0){
    // failure to send
    unix_error("failure to send");
    return -1;
  } // else it gets here
  // now we have to wait for an acknowledgement
  // int recv(int sock_fd, void *buf, int len, int flags)
  bytes_recv = recv_all(fd, recvbuf); // aloha backwards is the same length as aloha
  if(bytes_recv <= 0){
    // failure to recieve
    printf("%s", recvbuf);
    unix_error("failure to recieve");
    return -1;
    //otherwise, we must have recieved some nonzero number of bytes
  }
  // check to see if the recieved bytes are an accurate ack
  if(strcmp(recvbuf, ack) != 0){
    unix_error("not correct ack");
    return -1;
  }
  memset(recvbuf, 0, sizeof(recvbuf)); // clear the recieve buffer
  // then get the Username from the user and send it over to the server
  printf("please enter your username:\n");
  //char *fgets(char *restrict s, int n, FILE *restrict stream);
  fgets(buffer, MAXMSG, STDIN_FILENO); // read the shit from stdin
  // buffer now has the username, send message IAM <username>
  temp = malloc(MAXMSG);// extra 3 just in case
  //snprintf(writestring, size, formatstring, varargs)
  snprintf(temp, MAXMSG, "%s %s%s", verbs[11], buffer, cr); // snprintf concats using printf
  if(sendall(fd, buffer, &bytes_sent) != 0){
    unix_error("couldn't send");
    return -1;
  }
  free(temp);
  // now i gotta wait for the recieve
  bytes_recv = recv_all(fd, recvbuf);
  if(bytes_recv <= 0){
    unix_error("failed to receive");
    return -1;
  }
  // print the return message out for the user
  printf("%s\n", recvbuf); // this should be HI <username>
  // then, i should wait for the MOTD
  recv_all(fd, recvbuf);
  temp = strtok(recvbuf, " "); // the first message is just ECHO motd
  printf("%s", temp);
  return 0;
}
*/
int unblock(int fd){ // makes socket non blocking
  int flags;
  int n;
  flags = fcntl(fd, F_GETFL, 0); // get the flags
  if(flags == -1){
    unix_error("fcntl error");
    return -1;
  }
  flags |= O_NONBLOCK;
  n = fcntl(fd, F_SETFL, flags); // set the flags
  if(n == -1){
    unix_error("fcntl error");
    return -1;
  }
  return 0;
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

void print_help(){
  printf("./client [-h] NAME SERVER_IP SERVER_PORT\n");
  printf("-h \t\t\t Displays help menu & returns EXIT_SUCCESS.\n");
  printf("NAME \t\t Username to display when chatting.\n");
  printf("SERVER_IP \t IP address of the server to connect to.\n");
  printf("SERVER_PORT \t Port to connect to.\n");
  exit(0); // returns exit success
}

void list_commands(){
  // prints out a list of commands for the client
  printf("/command\t\tVERB <argument1> <argument2> -- explanation of what the command does\n");
  printf("/tell\t\tTELL <name> <message> -- tells another user a private message\n");
  printf("/createp\t\tCREATEP <username> <password> -- creates a new private user\n");
  printf("/creater\t\tCREATER <name> -- creates and names a new chatroom\n");
  printf("/kick\t\tKICK <username> -- kicks a user out of a chatroom\n");
  printf("/quit\t\tBYE <noargs> -- logs a user out of the chat server\n");
  printf("/leave\t\tLEAVE <noargs> -- removes the user from the chatroom\n");
  printf("/join\t\tJOIN <id> -- places the user into a chatroom\n");
  printf("/listrooms\t\tLISTR <noargs>-- lists the chatrooms currently available\n");
  printf("/listusers\t\tLISTU <noargs>-- lists the users currently in the chatroom\n");
  printf("/joinp\t\tJOINP <id> <password>-- joins a user to a private chatroom\n");
  printf("/help\t\t <noverb> <noargs>-- prints this help menu\n");
}

void clean(char* buffer){ // memset's a buffer in place
  memset(buffer, 0, strlen(buffer));
}
