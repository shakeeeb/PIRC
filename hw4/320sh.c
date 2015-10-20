#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
// fuck it. i'm not gonna use readline.

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

// function declarations
static void Exit(void);
//static pid_t Fork(void);
char** parse_args(char* command, char* delimiter); // splits a command into a token array
char* find_path(char** path, char* command); // finds a legitimate path
void unix_error(char *msg); // textbook unix error
void mass_print(char** tokens); // will eventually print a character array

int main (int argc, char ** argv, char **envp) {
  // envp contains the environment variables? the PATH varaible
  //PATH will be used to check wher binaries are stored
  int finished = 0;
  char *prompt = "320sh> ";
  char *whitespace = " \n\r\t"; // for delimiting
  char cmd[MAX_INPUT]; // the buffer, for the string
  // this is all path stuff
  // this just gets the PATH variable and all the possible paths
  char** path; // this holds all the paths
  char* pathholder;
  char* pathdelimiter = ":";
  pathholder = malloc(strlen(getenv("PATH")) * sizeof(char));
  pathholder = getenv("PATH");
  path = parse_args(pathholder, pathdelimiter);
  path = path;


  while (!finished) {
    char *cursor;
    char last_char;
    int rv; // check writes
    int count;
    char **args; // holds arguments parsed from command
    args = args;

    rv = write(1, prompt, strlen(prompt)); // write the prompt
    if (!rv) { // check to make sure it wrote correctly
      finished = 1;
      break;
    }

    // read the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; // all of the variables
	   rv   && (++count < (MAX_INPUT-1))  && (last_char != '\n'); cursor++) { // all of the conditions
      rv = read(0, cursor, 1); // reads one byte into cursor
     last_char = *cursor; // it holds the last character, makes sure it aint \n
   }
   *cursor = '\0'; //null terminates the cursor, so our string is now null terminated

    write(1, cmd, strnlen(cmd, MAX_INPUT)); // to verify that we read input successfully

    // see if it's 'exit ' for the sake of exiting
    if(strncmp(cmd, "exit ", 5) == 0){ // if it's stright up exit, note the extra space
        Exit(); // itll just exit
    }
    if(strncmp(cmd, "print", 3) == 0){
      mass_print(path);
    }
    args = parse_args((char*)cmd, whitespace); //parse the commands from the line
    // ive got to search for certain commands for builtins and shit
    //cd, cd ., cd .. cd ../../ pwd (builtins)-- ls la mkdir (not builtins but try to incorporate them at a certain point)

  }

  return 0;
}
/**
* find_path
* specifically used to test out several paths, and find the one that works
* takes a command as well as the string array of possible paths
* and then it concatenates the command with a '/' and appends it to the end of each
* possible path, and then it creates a stat object and uses fstat to check if the
* file exists in the specified path.
*
*/
char* find_path(char** path, char* command){ // the command is something like ls or mkdir
  char* result; // the trial string
  int index = 0; // the index
  char* currentPath = path[index]; // current path
  while(currentPath != NULL){
    result = malloc(strlen(path[index])+strlen(command)+2);// extra 2 for the / and the /0 remember to free this
    result = strcat(path[index], "/"); // add the slash
    result = strcat(result, command); // add the command itself, automatically adds in a /0
    struct stat *tester = malloc(sizeof(struct stat)); // the stat object
    if(stat(result, tester)== 0){ // if it exists, it'll be zero
      //in this case, we break and return the result
      free(tester); // dont forget to free this
      break;
    }
    // if it doesnt exist, free the string and the stat object
    index++;// and then we increment the index
    currentPath = path[index]; // go to the next index
    free(result); // dont forget
    free(tester); // to free malloced shit
  }
  if(result == NULL){
    // we didnt find anything in the paths
    // print some message shit
    return NULL;
  } else {
    // we found something!
    return result;
  }

}
/**
*parse args
*takes a string, and delimiters as arguments
*parses through the string and generates an array of tokens
*it then returns this array of tokens
*/
char** parse_args(char *command, char* delimiter){
  int size = sizeof(command)*2; // if every sharacter is a token
  char** result = malloc(size * sizeof(char*)); // malloc space for a big ass buffer
  int spot = 0; // we start at the first spot
  char* token; // each particular string

  if(result == NULL){
    // malloc error
  }
  token = strtok(command, delimiter); // grab the first token
  while (token != NULL){
    printf("result: %s spot: %d\n", token, spot);
    token = strcat(token, "\0"); // just append a zero to token
    result[spot] = token; //
    spot++; // increment the spot
    // if we exceed the size of the buffer, we could've dynamically reallocated but, here we dont need to
    if(spot > size){
      // dynamic reallocation
      size += MAX_INPUT; // size increases
      result = realloc(result, size * sizeof(char*));
      if(result == NULL){
        // realloc error
        exit(EXIT_FAILURE);
      }
    }
    // becasue we've got a finite line size
    // however a path may need to be dynamically reallocated
    // so WOOO
    token = strtok(NULL, delimiter);// go grab the next token by passing in null
  }
  result[spot] = NULL;
  return result;
}

/**
*Capital E Exit just exits shit
*
*/
static void Exit(void){
  exit(3);
}
/**
*capital F Fork
*its just a wrapper that checks for any problems in fork
*its commented out becuse i havent used it yet
*/
/*
static pid_t Fork(void){
  pid_t pid;

  if((pid = fork()) < 0){
    unix_error("Fork Error");
  }
  return pid;
}
*/
/**
* unix style error from the textbook
*/
void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
  exit(0);
}
/**
* mass print takes an array of strings and
* just prints them all out to the shell
*/
void mass_print(char** tokens){ // i made this because for whatever reason, printf wasn't working for me.
// by changing all possible prints to writes, we dont need to worry about streaming output to other files though
// assuming tokens is a null terminated array
  int index = 0;
  int last = sizeof(tokens); // this sint calulating right
  //printf("last: %d\n",last);
  while(index < last){ //well, this doesnt work completely
    write(1, tokens[index], strlen(tokens[index]));
    write(1, "\n", 1);
    index++;
  }
}
/*
learn to debug forks from jason
in gdb:
set detach-on-fork off

and then in the debugger info inferiors
*/
