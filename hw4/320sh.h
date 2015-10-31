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
  #include <sys/types.h>
  #include <fcntl.h>
  #include <time.h>
  #include <dirent.h>
  #include <signal.h>
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
  void print_times(time_t start); // prints the times if the -t flag is set at run time
  void handle_c(int sig); // handles control c (hopefully)
  void handle_child(int sig); // handles removing a child
  void print_jobs(); // prints all the jobs currently working (the job data structure)
  void add_job(int fg_bg, pid_t pid, pid_t gpid, char *name); // add a child to the process list
  void remove_job(pid_t pid); // deletes a child from the process list
  void foreground(char **args); // moves process into the foreground & restarts the process
  void background(char **args); // moves a process into the background
  void stopprocess(char **args); // stopps a process
  int find_fg_bg(char **args); // checks for '&' to see if the process is a background process


  #define MAX_INPUT_2 1025 // the full line + a null termination
  extern char* environ;
  char history[50][1025];
  int history_index = 0;
  pid_t shell; // the pid of the shell itself
  pid_t fgroup; // get the foreground process group starting with shell
  pid_t bgroup; // get the background process group
  int dflag = 0; // initializes the debug flag
  int tflag = 0; // initializes the time flag
  suseconds_t user = 0, sys = 0; // time variables for time flags
  time_t real = 0, start = 0, end = 0, ustart = 0, uend = 0, uuser = 0; // more variables for time flags
  struct rusage *rusage; // malloc space for the rusage
  struct child *job_list[MAX_INPUT_2]; // create a list for jobs

	volatile sig_atomic_t listening;

	const char* builtins[] = { // these are all of our builtins
	  "cd\0",
	  "pwd\0",
	  "echo\0",
	  "set\0",
	  "ls\0",
	  "jobs\0",
	  "fg\0",
	  "bg\0",
	  "exit\0"
	};

  struct child {
	  int running; // 1 if stopped
	  int jobid; // job id assigned by 320sh
	  pid_t pid; // pid of process
	  pid_t gpid; // group id of process
	  char name[MAX_INPUT_2]; // name of cmd
  };

#endif
