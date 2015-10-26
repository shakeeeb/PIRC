// Devon & Shakeeb
// aka: YoureFailingTryHarder
#ifndef __320SH_H
#define __320SH_H
  #include <stdio.h>
  #include <string.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <errno.h>
  #include <sys/stat.h>
  #include <sys/wait.h>
  #include <fcntl.h>

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
  void set(char** args); // manipulates environment variables
  void echo(char **args); // provides ehco $? support
  int size_of_string_array(char **args); // grabs the size of string array
  char* normalize (char* command);
  int contains(char* haystack, char* needle);
  char find_redirect_or_pipe(char** args);
  void Redirect(char** args, int file_descriptor, char direction);
  int begin_redirect(char** args, char direction);
  int find_position_of_in_array(char** haystack, char* needle);
  char** slice(char** args, char* start, int size);

  #define MAX_INPUT 1025 // the full line + a null termination

  const char* builtins[] = { // these are all of our builtins
    "cd\0",
    "pwd\0",
    "echo\0",
    "set\0",
    "exit\0"
  };
#endif
