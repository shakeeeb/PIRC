// Devon & Shakeeb
// aka: YoureFailingTryHarder
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/resource.h>

#ifdef debug // use this statement for print debugging statements
  #define debug(fmt, ...) printf("DEBUG: %s:%s:%d ", fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
  #define debug(fmt, ...)
#endif

/* FUNCTIONS */
static void Exit(void); // our version of exit
static pid_t Fork(void); // our version of fork
char** parse_args(char* command, char* delimiter); // splits a command into a token array
char* find_filepath(char** path, char* command); // finds a legitimate path
void unix_error(char *msg); // prints unix error
void mass_print(char** tokens); // will eventually print a character array
int begin_execute(char** args); // begins execution of builtins and binaries
void Execute(char **args); // executes the binaries if they have been found
void cd(char** args);// change the current working directory
void pwd(void); // print the current working directory
void globbing(char **args); // handles globbing with ls
void set(char** args); // manipulates environment variables
void echo(char **args); // provides ehco $? support
void parse_file(char *filename, int fd); // parses through a file for commands
int read_line(const char *file_line, int fd); // reads a line from the file, returns # of chars read
void print_times(time_t start); // prints the times if the -t flag is set at run time

#define MAX_INPUT_2 1025 // the full line + a null termination
int dflag = 0; // initializes the debug flag
int tflag = 0; // initializes the time flag
suseconds_t user = 0, sys = 0; // time variables for time flags
time_t real = 0, start = 0, end = 0, ustart = 0, uend = 0, uuser = 0; // more variables for time flags
struct rusage *rusage; // malloc space for the rusage

const char* builtins[] = { // these are all of our builtins
  "cd\0",
  "pwd\0",
  "echo\0",
  "set\0",
  "ls\0",
  "exit\0"
};