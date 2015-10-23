#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
// fuck it. i'm not gonna use readline.

// function declarations
static void Exit(void);
static pid_t Fork(void);
char** parse_args(char* command, char* delimiter); // splits a command into a token array
char* find_filepath(char** path, char* command); // finds a legitimate path
void unix_error(char *msg); // textbook unix error
void mass_print(char** tokens); // will eventually print a character array
int begin_execute(char** args); // begins execution
int Execute(char **argv); // executes the binary -- or tries to
void cd(char** args);// change directory. uhhh, sorta half done
void pwd(void); // print working directory

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1025 // the full line + a null termination

const char* builtins[] = {
  "cd\0",
  "pwd\0",
  "echo\0",
  "set\0",
  "exit\0"
}; // these are all of our builtins

int main (int argc, char ** argv, char **envp) {
  // envp contains the environment variables? the PATH variable
  //PATH will be used to check wher binaries are stored
  int finished = 0;
  char *prompt = "320sh> ";
  char *whitespace = " \n\r\t"; // for delimiting
  char cmd[MAX_INPUT] = ""; // the buffer, for the string
  // this is all path stuff
  // this just gets the PATH variable and all the possible paths
  char** path; // this holds all the paths
  char* pathholder; // particular path in path array
  char* pathdelimiter = ":";
  pathholder = malloc(strlen(getenv("PATH")) * sizeof(char) + 1);
  strcpy(pathholder, getenv("PATH")); // GETENV IS NOT REENTRANT (same copy in memory can be used by mult users)
  path = parse_args(pathholder, pathdelimiter); // pass particular ath and delimeter
  path = path; // ?? redudant shakeeb?


  while (!finished) { // while finish == 0
    char *cursor;
    char last_char;
    int rv; // check writes
    int count;
    char **args; // holds arguments parsed from command
    args = args; // ... why?????? ... oh wait this is cuz that unused variable crap isn't it? okay. Got it.

    // Print the prompt and check if it is successful
    rv = write(1, prompt, strlen(prompt));
    if (!rv) { // check to make sure it wrote correctly
      finished = 1;
      break;
    }
    // we can replace all this with gnu readline
    // the main reason for readline is to have the history, already done for us. which is glorious

    // Execute the command, handling built-in commands separately

    // Just echo the command line for now
    //write(1, cmd, strnlen(cmd, MAX_INPUT)); // tells us that it recieved our input correctly
    // parse through cursor to make see if it's 'exit ' for the sake of exiting
    // read the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1; // all of the variables
	   rv   && (++count < (MAX_INPUT-1))  && (last_char != '\n'); cursor++) { // all of the conditions
      rv = read(0, cursor, 1); // reads one byte into cursor
     last_char = *cursor; // it holds the last character, makes sure it aint \n
   }
   *cursor = '\0'; //null terminates the cursor, so our string is now null terminated

    write(1, cmd, strnlen(cmd, MAX_INPUT)); // to verify that we read input successfully
    args = parse_args((char*)cmd, whitespace); // parse the command from the line

    // see if it's 'exit ' for the sake of exiting
    if(strncmp(cmd, "exit", 4) == 0){ // TODO: need to fix so it will accept anything AFTER "exit " as well
        Exit(); // itll just exit
    }
    // actually this is just there for now. after we get builtins fully up and running we can completely remove this
    // this is just a tester command
    if(strncmp(cmd, "print", 3) == 0){
      mass_print(path);
    }
    // ive got to search for certain commands for builtins and shit
    //cd, cd ., cd .. cd ../../ pwd (builtins)-- ls la mkdir (not builtins but try to incorporate them at a certain point)
    Execute(args);
    begin_execute(args); // send in args to be executed
  }

  return 0;
}
/**
* find_filepath
* specifically used to test out several paths, and find the one that works
* takes a command as well as the string array of possible paths
* and then it concatenates the command with a '/' and appends it to the end of each
* possible path, and then it creates a stat object and uses fstat to check if the
* file exists in the specified path.
*
*/
char* find_filepath(char** path, char* command){ // the command is something like ls or mkdir
  // this returns the path of the command
  char* result; // the trial string
  int index = 0; // the index
  char* currentPath = path[index]; // current path
  while(currentPath != NULL){
    result = malloc(strlen(path[index])+strlen(command)+2);// extra 2 for the / and the /0 remember to free this --> TODO: use slides to keep track of when child finishes,
                                                                // after child finishes then free all memory and continue in the loop.
    strcpy(result , path[index]); // copies the current path into result
    result = strcat(result, "/"); // add the slash
    result = strcat(result, command); // add the command itself, automatically adds in a /0
    struct stat *tester = malloc(sizeof(struct stat)); // the stat object
    if(stat(result, tester)== 0){ // if it exists, it'll be zero
      //in this case, we break and return the result
      printf("found filepath! filepath is: %s\n", result);
      free(tester); // dont forget to free this
      break;
    }
    // if it doesnt exist, free the string and the stat object
    index++;// and then we increment the index
    currentPath = path[index]; // go to the next index
    free(result); // dont forget
    free(tester); // to free malloced
  }
  if(result == NULL){
    // we didnt find anything in the paths
    // print some message
    printf("did not find the filepath \n");
    return NULL;
  } else {
    // we found something!
    printf("returning filepath \n");
    return result;
  }

}
/**
*parse args
*takes a string, and delimiters as arguments
*parses through the string and generates an array of tokens
*it then returns this array of tokens
*/
char** parse_args(char *command, char* delimiter){ // okay so as I understand it this is just taking all the arguements off the shell and putting it into an array?
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
static pid_t Fork(void){
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
/**
* mass print takes an array of strings and
* just prints them all out to the shell
*/
void mass_print(char** tokens){ // i made this because for whatever reason, printf wasn't working for me.
// by changing all possible prints to writes, we dont need to worry about streaming output to other files though
// assuming tokens is a null terminated array
  int index = 0;
  int last = sizeof(tokens);
  //printf("last: %d\n",last);
  while(index < last){
    write(1, tokens[index], strlen(tokens[index]));
    write(1, "\n", 1);
    index++;
  }
}

int begin_execute(char** args){ // args just contains the list of arguments
  // also, so path isnt a constant thing nooo, we've gotta getenv(PATH) each time, because the user can change the path
  // variable
  // process for execution:
  // check if it's a builtin
  // if its a builtin, run it
  // if it's not a builtin, then its an executable
  // get the PATH and parse it, just like you would parse arguments
  // after parsing launch find_filepath
  // using the return from find_filepath, fork and exec.
    char *cmd_holder = args[0]; // also check size of builtins
    int i;
    int execution_done = 0; // this checks if iv'e finished executing stuff. at certain points, i may need to return
    // based on what i've executed
    for(i = 0; i < sizeof(builtins)/sizeof(char*); i++){
      if (strcmp(cmd_holder, builtins[i]) == 0){
        // we found a builtin
        //printf("searching for builtins...\n");
        switch(i){ // i switch on the index of the builtin because it's easier
          case 0: // cd
            printf("found a builtin: %s\n", builtins[i]);
            cd(args);
            execution_done = 1; // finished executing a command
            break;
          case 1: // pwd
            printf("found a builtin: %s\n", builtins[i]);
            pwd();
            execution_done = 1; // finished executing a command
            break;
          case 2: // echo
            break;
          case 3: // set
            break;
          case 4: //exit
            printf("found a builtin: %s\n", builtins[i]);
            Exit();
            break;
          default: // how the fuck did you get here then?
            break;

        }

      }// end of if
      if(execution_done == 1){
        return 0; // if it reaches this point, it has executed a builtin function cleanly
      }
    }
    // if it reaches this point without finding a builtin, then its an executable
    // check if it has a slash (/), or a dot slash (./), cuz then we dont need to pass it through the filepath checking
    char a, b;
    a = *cmd_holder; // the first character
    b = *(cmd_holder+1); // the second character
    if(a == '/'){
      // if a is slash
      printf("i have recognized a slash character!\n");
      // i can send it straight to execvp with no worries
    } else if ((a == '.') && (b == '/')) {
      // if its ./ something
      printf("i have recognized a dot slash character!\n");
      // i dont know
    } else {
      // it doesnt have the slash or dot slash
      // get the path
      char** path;
      char* unparsedpath = malloc(strlen(getenv("PATH")) * sizeof(char)+ 1);
      strcpy(unparsedpath, getenv("PATH"));
       // getenv is not reentrant
      path = parse_args(unparsedpath, ":");
      //free(unparsedpath); // i free this, cuz i don't need it anymore
      // and this will get the path
      char* filepath = find_filepath(path ,cmd_holder); // i dont malloc stuff here, i malloced in find filepath
      //either this filepath is NULL, meaning the path wasn't able to be found, OR
      // its not NULL, and i found the path
      if(filepath == NULL){
        //uh oh
        printf("uh oh\n");
        unix_error("filepath not found");
      } else {
        // yay
        printf("filepath: %s\n", filepath);
      }

    }
    return 1;

    //Execute(char **args);
    //OOOOOOHHH WE GOTTA THINK ABOUT /<exec> AND ./<exec>

}

int Execute(char **argv) {
  pid_t cpid; // saves the pid of the child
  int waiting; // saves the integer to see if the parents should continue waiting

  cpid = Fork(); // start new process by forking, copies parent space for child
  if(cpid == 0) {
    if(execvp(*argv, argv) < 0) { // call execvp and check to see if it was successful
      unix_error("ERROR: exec failed\n"); // if not successful print error message
      exit(1); // exit with error
    }
  }
  else { // for parents process ONLY
    while(wait(&waiting) != cpid); // wait for the child to finish & reap before continuing
  }
  return 0;
}

void pwd(void){ // print working directory
  char * result = malloc(MAX_INPUT);
  getcwd(result, MAX_INPUT); // getcwd gets the current working directory
  printf("%s\n", result); // and prints the output to the screen
  // do i free that shit?
  //free(result);
  // also idk what to do in the case of an error here,
  // or even how i would get an error here
  return;
}

void cd(char** args){ // change directory
  //gotta work in the whole ./../../.. thing
  // to cd down, you just add the directory to the current working directory
  // check if its got a slash in front of it, to know whether to cd down or up
  // check if it's . or ..
  // for a repeated series of ./../../../..
  //loop over the string, finding and locating ..'s
  // and then, remove a directory from the cwd for each .. you find.
  // if its a . dont worry about it
  // if its just cd with no arguments, then just go to home
  char * result = malloc(MAX_INPUT); // dont forget to free this shitat some point
  char * directory = args[1]; // a cd usually only has 1 argument, a directory location
  if(directory == NULL){ // if it's just cd, go home
    strcpy(result, getenv("HOME")); // instead of setting shit equal, better to use strcpy to get deep copies
    if(chdir(result) != 0){
      unix_error("change directory error");
    }
    //free(result);
    return; // now i return with no worries
  }else if(strcmp(directory, "..") == 0){
    getcwd(result, MAX_INPUT); // copy over the current workign directory
    strcat(result, "/"); // i need to put a slash
    strcat(result, ".."); // strcat autoappends a null, but there needs to be space for it doe
    if(chdir(result) != 0){
      unix_error("change directory error");
    }
    //free(result);
    return; // return with no worries
  }else if(strcmp(directory, ".") == 0){
    // you literally dont need to do shit, but whatever
    getcwd(result, MAX_INPUT);
    strcat(result, "/");
    strcat(result, ".");
    if(chdir(result) != 0){
      unix_error("change directory error");
    }
    //free(result);
    return; // return with no worries
  } else { // its just a subdirectory
    getcwd(result, MAX_INPUT);
    strcat(result, "/");
    strcat(result, directory);
    if(chdir(result) != 0){
      unix_error("change directory error");
    }
    return;
  }
  // gotta take care of dash too
  // doesnt take care of the case with repeated ../../../../
  // i know how to do that but
  // im not sure if they would give us something wierd like  ../blehdeirectory/../../bluhh or something
  // and i'm not sure how to handle the errors given that case
  //if it reaches here, something went wrong
  unix_error("cd error");
  //free(result);
  return;

}
/*
learn to debug forks from jason
in gdb:
set detach-on-fork off

and then in the debugger info inferiors
*/
/* slide 50*** important for forking */



