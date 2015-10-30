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
  #include <dirent.h>

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
  char* normalize (char* command); // takes any input string and makes it uniform on > < = for the parser
  int contains(char* haystack, char* needle); // boolean, searches for a substring
  char find_redirect_or_pipe(char** args); // looks for a redirect or a pipe in a char**
  void Redirect(int in, int out, int err, char** args);
  int begin_redirect(char** args, char direction, int hiddendottxt);
  int find_position_of_in_array(char** haystack, char* needle);
  char** slice(char** args, char* start, int size); // slice takes a slice out of an array
  void globbing(char **args); // handles globbing with ls
  void parse_file(char *filename, int fd); // parses through a file for commands
  int read_line(const char *file_line, int fd); // reads a line from the file, returns # of chars read
  int process_pipes(char** args);
  int Close(int fd);
  int find_number_of_pipes(char** args);
  // void free_string_array(char** array);


  #define MAX_INPUT_2 1025 // the full line + a null termination
  int dflag = 0; // initializes the debug flag
  extern char* environ;
  //char history[50][1025];
  //int history_index = 0;

  const char* builtins[] = { // these are all of our builtins
    "cd\0",
    "pwd\0",
    "echo\0",
    "set\0",
    "ls\0",
    "exit\0"
  };

#endif
