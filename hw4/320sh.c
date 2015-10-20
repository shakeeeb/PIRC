#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int main (int argc, char ** argv, char **envp) {

  int finished = 0;
  char *prompt = "320sh> ";
  char cmd[MAX_INPUT]; // an array which holds lines


  while (!finished) {
    char *cursor;
    char last_char; 
    int rv; // check writes
    int count;


    // Print the prompt
    cursor = readline(prompt); //readline grabs the line, and places it into cursor
    // it also prints stuff
    //readline mallocs stuff
    // so it's important that we free stuff at a certain point
    // unless we use the history option, in which case it wouldnt be necessary to free stuff
    //also, if we cant get readline workign on your computer devon, i think it would just be wise to, like, just parse through character by character
    // because thats what jwong straight up told us
    // also make sure cursor is not null, that could potentially fuck shit up

    if (!rv) { // checks to make sure the read was valid
      finished = 1;
      break;
    }
    // we can replace all this with gnu readline 
    // the main reason for readline is to have the history, already done for us. which is glorious


    // Execute the command, handling built-in commands separately 
    // Just echo the command line for now
    write(strlen(cursor), cursor, strnlen(cmd, MAX_INPUT)); // tells us that it recieved our input correctly
    // parse through cursor to make see if it's 'exit ' for the sake of exiting
    // use strncmp for ease of use
    if(strncmp(cursor, "exit ", 5) == 0){ // if it's stright up exit
        Exit(); // itll just exit
    }

  }

  return 0;
}

/**
*Capital E Exit just exits shit
*
*/
public static void Exit(void){
  exit(3);
}
/**
*capital F Fork
*its just a wrapper that checks for any problems in fork
*
*/
public static pid_t Fork(void){
  pid_t pid;

  if((pid = fork()) < 0){
    unix_error("Fork Error");
  }
  return pid;
}
/**
* unix style error from the textbook
*/
void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(0);
}


