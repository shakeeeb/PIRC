// Devon & Shakeeb
// aka: YoureFailingTryHarder
#include "320sh.h"

int main (int argc, char ** argv, char **envp) {
  shell = getpid(); // the pid of the shell itself
  //fgroup = setpgid(shell, 0); // get the foreground process group starting with shell

  int opt; // check for debug & time flags
  while((opt = getopt(argc, argv, "dt")) != -1) {
    switch(opt) {
      case 'd': // debug flag
        dflag++;
        break;
      case 't': // time flag
        tflag++;
        break;
      default: // default do nothing
        break;
    }
  }

  // envp contains the environment variables? the PATH variable
  //PATH will be used to check wher binaries are stored

  int finished = 0; // keeps track of running while loop
  char *prompt = "320sh> "; // prompt
  char *whitespace = " \n\r\t"; // for delimiting
  char cmd[MAX_INPUT_2] = ""; // the buffer, for the string

  // this is all path stuff
  // this just gets the PATH variable and all the possible paths
  char** path; // this holds all the paths
  char* pathholder; // particular path in path array
  char* pathdelimiter = ":"; // delimeter for args
  pathholder = malloc(strlen(getenv("PATH")) * sizeof(char) + 1);
  strcpy(pathholder, getenv("PATH")); // GETENV IS NOT REENTRANT (same copy in memory can be used by mult users)
  path = parse_args(pathholder, pathdelimiter); // pass particular path and delimeter

  setenv("?", "0", 1); // create "?" environment variable & set default to zero

  //sigset_t mask, cmask, pmask;
  //sigfillset(&mask);
  //sigemptyset(&cmask);
  //sigaddset(&cmask, SIGINT);
  //sigprocmask(SIG_BLOCK, &mask, &pmask);
  signal(SIGINT, handle_c);
  //sigprocmask(SIG_SETMASK, &pmask, NULL);

  // CHECK IF FILE WAS PASSED IN AT COMPILE TIME
  int fd;
  int n = 1;
  while(argv[n] != NULL ) {
    if ((strcmp(argv[n], "-d") == 0) || (strcmp(argv[n], "-t") == 0)) { // if flags, continue to next arg
      n++;
      continue;
    } else if ((fd = open(argv[n], O_RDONLY)) > 0) { // if a non flag is found try to open and run the file
      n++;
      parse_file(argv[n], fd);
    }
  }

  // CODE TO ADD CURRENT DIRECTORY AT COMPILE TIME TO PATH
  /*char *ipath = malloc(MAX_INPUT_2); // allocates space for the initial path of the directory
  getcwd(ipath, MAX_INPUT_2); // gets the initial path of the directory

  char *envpath = malloc(MAX_INPUT_2);
  strcpy(envpath, getenv("PATH"));
  strcat(envpath, ":");
  strcat(envpath, ipath); // add the path where 320sh.c is located
  setenv("PATH", envpath, 1); // change the environment variable PATH to the new set of paths

  free(ipath);
  free(envpath);*/

  // GETTING INPUT TO SHELL
  while (!finished) { // while finish == 0
    char *cpath = malloc(MAX_INPUT_2); // allocates space for the current path of the directory
    getcwd(cpath, MAX_INPUT_2); // gets the current path of the directory

    char *cursor; // current input char from user
    char *fprompt = malloc(MAX_INPUT_2); // allocate space for full prompt
    char last_char; // last input char from user
    int rv, rw; // check writes
    int count; // make sure input is < MAX_INPUT_2
    char **args; // holds arguments parsed from commands

    // gets current path and places it into the string to print to stdout (with prompt)
    strcpy(fprompt, "[");
    strcat(fprompt, cpath);
    strcat(fprompt, "] ");
    strcat(fprompt, prompt);

    // Print the prompt and check if it is successful
    rv = write(1, fprompt, strlen(fprompt));
    if (!rv) { // check to make sure it wrote correctly
      finished = 1;
      break;
    }
    // read the input
    int newlineFlag = 0; // check if user just inputted a '\n'
    for(rv = 1, rw = 1, count = 0, cursor = cmd, last_char = 1; // all of the variables
	   rv && rw && (++count < (MAX_INPUT_2-1))  && (last_char != '\n'); cursor++) { // all of the conditions
      rv = read(0, cursor, 1); // reads one byte into cursor
      rw = write(1, cursor, 1); // writes byte that user typed (for use with launcher)
      last_char = *cursor; // it holds the last character, makes sure it aint \n
      if (count == 1 && last_char == '\n') { // set '\n' flag if first char == '\n'
        newlineFlag = 1;
        break;
      }
    }
    *cursor = '\0'; //null terminates the cursor, so our string is now null terminated
    start = time(0); // start timer for function

    if (newlineFlag == 1) { // for if they just press enter
      continue;
    }

    args = parse_args((char*)cmd, whitespace); // parse the command from the line

    // this is just a tester command
    if(strncmp(cmd, "print", 3) == 0){
      mass_print(path);
    }

    begin_execute(args); // send in args to be executed

    // remember to free EVERYTHING
    free(cpath);
    free(fprompt);
    free(args);

    // if time flag is set print times
    if(tflag == 1) {
      print_times(start);
    }
  }

  return 0;
}

void handle_c(int sig) {
  write(1, "^C", 3);
  sigset_t mask, pmask;
  sigfillset(&mask);
  int i = 0;
  while(job_list[i] != NULL) {
    //if (job_list[i]->gpid == fgroup) {
      sigprocmask(SIG_BLOCK, &mask, &pmask);
      kill(job_list[i]->pid, SIGINT);
      sigprocmask(SIG_SETMASK, &pmask, NULL);
    //}
  }
  //kill(0, SIGINT);
  //raise(SIGINT);
  //exit(0);

  //sigprocmask(SIG_SETMASK, &pmask, NULL);
  // async-singal safe functions --> don't use printf, sprintf, malloc, etc
  // preserve error number on entry and exit
  // protect shared data structures by blocking signals
  // declare global variables as "volatile"
  // global flags volatile sig_atomic_t
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
char* find_filepath(char** paths, char* command){ // the command is something like ls or mkdir
  // this returns the path of the command
  char* result; // the trial string
  int index = 0; // the index
  char* currentPath = paths[index]; // current path
  while(currentPath != NULL){
    result = malloc(strlen(paths[index])+strlen(command)+2);// extra 2 for the / and the /0 remember to free this 
    strcpy(result , paths[index]); // copies the current path into result
    result = strcat(result, "/"); // add the slash
    result = strcat(result, command); // add the command itself, automatically adds in a /0
    struct stat *tester = malloc(sizeof(struct stat)); // the stat object
    if(stat(result, tester)== 0){ // if it exists, it'll be zero
      //in this case, we break and return the result
      debug("found filepath! filepath is: %s\n", result);
      free(tester); // dont forget to free this
      break;
    }
    // if it doesnt exist, free the string and the stat object
    index++;// and then we increment the index
    currentPath = paths[index]; // go to the next index
    free(result); // dont forget
    free(tester); // to free malloced
  }
  if(result == NULL){
    // we didnt find anything in the paths
    // print some message
    debug("did not find the filepath \n");
    return NULL;
  } else {
    // we found something!
    debug("returning filepath \n");
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
    debug("result: %s spot: %d\n", token, spot);
    token = strcat(token, "\0"); // just append a zero to token
    result[spot] = token; //
    spot++; // increment the spot
    // if we exceed the size of the buffer, we could've dynamically reallocated but, here we dont need to
    if(spot > size){
      // dynamic reallocation
      size += MAX_INPUT_2; // size increases
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
    setenv("?", "1", 1);
    unix_error("Fork Error");
  }
  return pid;
}

/**
* unix style error from the textbook
*/
void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
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
  //debug("last: %d\n",last);
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
        //debug("searching for builtins...\n");
        switch(i){ // i switch on the index of the builtin because it's easier
          case 0: // cd
            debug("found a builtin: %s\n", builtins[i]);
            if (dflag == 1)
              printf("RUNNING: cd\n");
            ustart = time(0);
            cd(args);
            uend = time(0);
            if (dflag == 1)
              printf("ENDED: cd (ret=%s)\n", getenv("?"));
            execution_done = 1; // finished executing a command
            break;
          case 1: // pwd
            debug("found a builtin: %s\n", builtins[i]);
            if (dflag == 1)
              printf("RUNNING: pwd\n");
            ustart = time(0);
            pwd();
            uend = time(0);
            if (dflag == 1)
              printf("ENDED: pwd (ret=%s)\n", getenv("?"));
            execution_done = 1; // finished executing a command
            break;
          case 2: // echo
            debug("found a builtin: %s\n", builtins[i]);
            if (dflag == 1)
              printf("RUNNING: echo\n");
            ustart = time(0);
            echo(args);
            uend = time(0);
            if (dflag == 1)
              printf("ENDED: echo (ret=%s)\n", getenv("?"));
            execution_done = 1; // finished executing a command
            break;
          case 3: // set
            debug("found a builtin: %s\n", builtins[i]);
            if (dflag == 1)
              printf("RUNNING: set\n");
            ustart = time(0);
            set(args);
            uend = time(0);
            if (dflag == 1)
              printf("ENDED: set (ret=%s)\n", getenv("?"));
            execution_done = 1;
            break;
          case 4: // globbing
            debug("found a builtin: %s\n", builtins[i]);
            ustart = time(0);
            globbing(args);
            uend = time(0);
            execution_done = 1;
            break;
          case 5: // jobs
            ustart = time(0);
            print_jobs();
            uend = time(0);
            execution_done = 1;
            break;
          case 6: // fg
            ustart = time(0);
            foreground(args);
            uend = time(0);
            execution_done = 1;
            break;
          case 7: // bg
            ustart = time(0);
            background(args);
            uend = time(0);
            execution_done = 1;
            break;
          case 8: //exit
            debug("found a builtin: %s\n", builtins[i]);
            if (dflag == 1)
              printf("RUNNING: exit\n");
            ustart = time(0);
            Exit();
            uend = time(0);
            if (dflag == 1)
              printf("ENDED: exit (ret=%s)\n", getenv("?"));
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
      debug("i have recognized a slash character!\n");
      // i can send it straight to execvp with no worries
      Execute(args);
    } else if ((a == '.') && (b == '/')) {
      // if its ./ something
      debug("i have recognized a dot slash character!\n");
      Execute(args);
    } else {
      // it doesnt have the slash or dot slash
      // get the path
      char** paths;
      char* unparsedpath = malloc(strlen(getenv("PATH")) * sizeof(char)+ 1);
      strcpy(unparsedpath, getenv("PATH"));
       // getenv is not reentrant
      paths = parse_args(unparsedpath, ":");
      free(unparsedpath); // i free this, cuz i don't need it anymore
      // and this will get the path
      char* filepath = find_filepath(paths ,cmd_holder); // i dont malloc stuff here, i malloced in find filepath
      //either this filepath is NULL, meaning the path wasn't able to be found, OR
      // its not NULL, and i found the path
      if(filepath == NULL){
        setenv("?", "127", 1);
        printf("%s: command not found\n", args[0]);
      } else {
        // yay
        Execute(args);
      }
      free(paths);
    }
    return 1;
}

void Execute(char **args) {
  pid_t cpid, ppid; // saves the pid of the child
  sigset_t allmask, cmask, pmask;
  //int waiting, waitplz; // saves the integer to see if the parents should continue waiting
  //int status = 0;
  //int bg; // holds background job status for child
  //char *prints = malloc(MAX_INPUT_2);

  //bg = find_fg_bg(args); // see if child is foreground process or not
  // start new process by forking, copies parent space for child
  if((ppid = Fork()) == 0) {
    // HANDLE SINGALS
    sigfillset(&allmask); // set all mask
    sigemptyset(&cmask); // set child mask to zero
    signal(SIGCHLD, handle_child);
    sigprocmask(SIG_BLOCK, &cmask, &pmask); // block the child process
    if ((cpid = Fork()) == 0) {
      sigprocmask(SIG_SETMASK, &pmask, NULL); // unblock the child within the child
      if (dflag == 1)
        printf("RUNNING: %s\n", args[0]); // print the running portion TODO: add flag
      if(execvp(*args, args) < 0) { // call execvp and check to see if it was successful
        printf("%s: command not found\n", args[0]); // if not successful print error message
        exit(127); // exit with error
      }
    } else {
      /*while(wait(&waitplz) != cpid);
      add_job(bg, ppid, getpgid(cpid), args[0]);
      exit(WEXITSTATUS(waitplz));*/
    }
  }
  /*else { // for parents process ONLY
    sigprocmask(SIG_BLOCK, &allmask, NULL);
    //add_job(bg, cpid, getpgid(cpid), args[0]);
    sigprocmask(SIG_SETMASK, &pmask, NULL);
    rusage = malloc(MAX_INPUT_2);
    if (!bg) {
      while(wait3(&waiting, 0, rusage) != ppid); // wait for the child to finish & reap before continuing
      status += WEXITSTATUS(waiting);
      snprintf(prints, MAX_INPUT_2, "%d", status);
    } else {
      add_job(0, ppid, getpgid(ppid), args[0]);
    }
    if (dflag == 1)
      printf("ENDED: %s (ret=%d)\n", args[0], status); // print ending portion TODO: add flag
    setenv("?", prints, 1);
  }
  free(prints);*/
  return;
}

void pwd(void){ // print working directory
  char *result = malloc(MAX_INPUT_2);
  getcwd(result, MAX_INPUT_2); // getcwd gets the current working directory
  printf("%s\n", result); // and prints the output to the screen
  // do i free that shit? --> yes shakeeb, freeing is good and it won't segfault :P
  free(result);
  // also idk what to do in the case of an error here,
  // or even how i would get an error here
  setenv("?", "0", 1);
  return;
}

void globbing(char **args) {
  char *ptr = args[1];
  char *cwdptr = malloc(MAX_INPUT_2); // holds current working directory
  getcwd(cwdptr, MAX_INPUT_2);
  if (ptr != NULL && *ptr == '*') { // if globbing
    if (dflag == 1)
      printf("RUNNING: ls\n");
    int found = 0;
    char *ftype; // holds the file type
    ptr++; // move to file type
    DIR *directory; // current directory
    struct dirent *entry; // current entry in directory
    if ((directory = opendir(cwdptr)) != NULL) { // open the current directory
      while((entry = readdir(directory)) != NULL) { // move to next entry in directory
        if ((ftype = strrchr(entry->d_name, '.')) != NULL) { // if entry contains a '.' (extension)
          if (strcmp(ftype, ptr) == 0) { // if the typed extension matches this file extension
            printf("%s ", entry->d_name); // print the file w/ extension
            found++; // say we found something
          }
        }
      }
      printf("\n");
      fflush(stdout);
      closedir(directory); // close the current directory when done
    }
    if (found == 0) { // if we found nothing print error
      printf("ls: cannot access %s: No such file or directory\n", args[1]);
      fflush(stdout);
      setenv("?", "2", 1);
    } else {
      setenv("?", "0", 1);
    }
    if (dflag == 1)
      printf("ENDED: ls (ret=%s)\n", getenv("?"));
  } else { // if regular ls
    Execute(args);
  }
  free(cwdptr);
}

void cd(char** args){ // change directory --> TODO: STILL NEED TO IMPLEMENT THE "cd -" COMMAND!!!!!!
  //gotta work in the whole ./../../.. thing
  // to cd down, you just add the directory to the current working directory
  // check if its got a slash in front of it, to know whether to cd down or up
  // check if it's . or ..
  // for a repeated series of ./../../../..
  //loop over the string, finding and locating ..'s
  // and then, remove a directory from the cwd for each .. you find.
  // if its a . dont worry about it
  // if its just cd with no arguments, then just go to home
  char * result = malloc(MAX_INPUT_2); // dont forget to free this shit at some point
  char * directory = args[1]; // a cd usually only has 1 argument, a directory location
  char* dotslash = "./";
  char* dotdotslash = "../";
  if(directory == NULL){ // if it's just cd, go home
    strcpy(result, getenv("HOME")); // instead of setting shit equal, better to use strcpy to get deep copies
    if(chdir(result) != 0){
      setenv("?", "1", 1);
      unix_error(args[1]); // if there is an error then it should print the file path & then the unix error
    } else {
      setenv("?", "0", 1);
    }
    free(result);
    return; // now i return with no worries
  } else if(strcmp(directory, "..") == 0){
    getcwd(result, MAX_INPUT_2); // copy over the current workign directory
    strcat(result, "/"); // i need to put a slash
    strcat(result, ".."); // strcat autoappends a null, but there needs to be space for it doe
    if(chdir(result) != 0){
      setenv("?", "1", 1);
      unix_error(args[1]);
    } else {
      setenv("?", "0", 1);
    }
    free(result);
    return; // return with no worries
  } else if(strcmp(directory, ".") == 0){
    // you literally dont need to do shit, but whatever
    getcwd(result, MAX_INPUT_2);
    strcat(result, "/");
    strcat(result, ".");
    if(chdir(result) != 0){
      setenv("?", "1", 1);
      unix_error(args[1]);
    } else {
      setenv("?", "0", 1);
    }
    free(result);
    return; // return with no worries
  } else if(strcmp(directory, "-") == 0){ // dash
    // use the getenv oldpwd
    strcpy(result, getenv("OLDPWD"));
    // now i have to set OLDPWD to be getCWD
    char *currentDirHolder = malloc(MAX_INPUT_2);
    // overwrite needs to be nonzero to overwrite existing enviernment variables
    setenv("OLDPWD", getcwd(currentDirHolder, MAX_INPUT_2), 1);
    // setenv makes copies so i need to free currentDirHolder
    free(currentDirHolder);
    if(chdir(result) != 0){
      setenv("?", "1", 1);
      unix_error(args[1]);
    } else {
      setenv("?", "0", 1);
    }
    return;
  } else if((strncmp(directory, dotslash, 2) == 0) || (strncmp(directory, dotdotslash, 3) == 0)){
    // count the number of ../
    char *cursor = directory;
    int count = 0;
    while(*cursor != '\0'){ //
      if(strncmp(cursor, dotslash, 2) == 0){
        cursor = cursor + 2; // move it forwards 3, so it points at the thing after the ./
         //only increase the count if its a ..
      } else if (strncmp(cursor, dotdotslash, 3) == 0){
        cursor = cursor + 3; // move it forwards 4. so it points at the thing after the ../
        count++;
      } else {
        // else, its invalid, thats what else
        setenv("?", "1", 1);
        unix_error(args[1]);
        return;
      }
    }
    // now weve got count dotdots
    for(;count != 0; count--){
      getcwd(result, MAX_INPUT_2);
      strcat(result, "/.."); // slash dot dot to go up a level
      if(chdir(result) != 0){
        setenv("?", "1", 1);
        unix_error(args[1]);
      } else {
        setenv("?", "0", 1);
      }
    }
    return; // return without worries
  } else { // its just a subdirectory
    getcwd(result, MAX_INPUT_2);
    strcat(result, "/");
    strcat(result, directory);
    if(chdir(result) != 0){
      setenv("?", "1", 1);
      unix_error(args[1]);
    } else {
      setenv("?", "0", 1);
    }
    return;
  }
  //if it reaches here, something went wrong
  unix_error("cd error");
  free(result);
  return;
}

void set(char **args) {
  char *variable, *setTo; // variables to hold the environment variable and what it should be set to
  variable = strtok(args[1], "="); // find the environment variable
  setTo = strtok(NULL, "="); // find what it should be set to
  setenv(variable, setTo, 1); // set the environment variable
  setenv("?", "0", 1);
}

void echo(char **args) { // echo should print the environment variable if preceded with a "$"
  char *envariable;
  char *estatus = malloc(MAX_INPUT_2);
  if (*args[1] == 0x24) { // if the first char does equal "$" print the value of the environment variable
    envariable = args[1];
    envariable += 1;
    if (getenv(envariable) != NULL) {
      strcpy(estatus, getenv(envariable));
      printf("%s\n", estatus);
      setenv("?", "0", 1);
    } else {
      Execute(args);
    }
  } else { // if the first char of the second argument does not equal a "$"
    Execute(args);
  }
  free(estatus);
}

void parse_file(char *filename, int fd) { // ASSUMES FILE HAS ALREADY BEEN OPENED!!!!
  // read the first line of the file and see if it contains "#!"
  char *file_line = malloc(MAX_INPUT_2); // malloc space for each line
  char *ptr, **args;
  char *whitespace = " \n\r\t";
  int chars;
  while((chars = read_line(file_line, fd)) > 0) { // while there are lines in the file
    debug("Read Line: %s", file_line);
    ptr = file_line; // point to beginning of file line
    if (*ptr == '#') { // if beginning of file line == '#' then it's a comment
      continue; // ignore line
    } else { // esle run the command
      args = parse_args(ptr, whitespace);
      begin_execute(args);
    }
  }
}

int read_line(const char *file_line, int fd) { // reads a line from a file
  char *ptr = (char*) file_line;
  int n, count = 0;
  while((read(fd, ptr, 1) > 0) && *ptr != '\n') { // read each character until you meet a '\n'
    ptr++;
    count++;
  }
  n = count;
  while(n < MAX_INPUT_2) { // null terminate string to erase previous line in pointer
    *ptr = '\0';
    ptr++;
    n++;
  }
  return count;
}

void print_times(time_t start) { // print times for the -t flag
  end = time(0);
  real = end - start;
  puts("I hope I make it...");
  if(rusage != NULL) { // if the command was a binary or used execute
    user = rusage->ru_utime.tv_usec;
    sys = rusage->ru_stime.tv_usec;
    printf("TIMES: real=%.2fseconds user=%.2fmicroseconds sys=%.2fmircroseconds", (float)real, (float)user, (float)sys);
    puts("");
  } else { // if the command was a builtin
    uuser = uend - ustart;
    printf("TIMES: real=%.2fseconds user=%.2fseconds sys=%.2fmircroseconds", (float)real, (float)uuser, (float)sys);
    puts("");
  }
}

void handle_child(int sig) {
  sigset_t allmask, pmask;
  pid_t cpid;
  sigfillset(&allmask);
  while((cpid = (waitpid(-1, NULL, 0))) > 0);
  sigprocmask(SIG_BLOCK, &allmask, &pmask);
  remove_job(cpid);
  sigprocmask(SIG_SETMASK, &pmask, NULL);
}

void print_jobs() {
  struct child *ptr;
  int i = 0;
  while(job_list[i] != NULL) {
    ptr = job_list[i];
    if (ptr->running == 1) {
      printf("[%d] (%d) {%d} Stopped\t%s\n", ptr->jobid, ptr->pid, ptr->gpid, ptr->name);
    } else if (ptr->running == 0) {
      printf("[%d] (%d) {%d} Running\t%s\n", ptr->jobid, ptr->pid, ptr->gpid, ptr->name);
    } else {
      // do nothing --> running is negative??
    }
    i++;
  }
}

void add_job(int running, pid_t pid, pid_t gpid, char *name) { // add a child to the process list
  struct child *current;
  current = calloc(1, sizeof(struct child));
  //cptr = &current;
  current->running = running;
  current->pid = pid;
  current->gpid = gpid;
  char *ptr = name;
  int i;
  while(*ptr != '\0') {
    current->name[i] = *ptr;
    i++;
    ptr++;
  }
  current->name[i] = '\0';
  i = 0;
  while(job_list[i] != NULL) {
    i++;
  }
  job_list[i] = current;
  current->jobid = (i + 1);
  if (running == 0) {
    printf("[%d] (%d) Running\t%s\n", current->jobid, current->pid, current->name);
  } else {
    printf("[%d] (%d) Stopped\t%s\n", current->jobid, current->pid, current->name);
  }
}

void remove_job(pid_t pid) { // deletes a child from the process list
  // child removes themselves from the job list
  int i = 0, found = 0;
  while(job_list[i] != NULL && found != 1) {
    if (job_list[i]->pid == pid) {
      free(job_list[i]);
      job_list[i] = NULL;
      found++;
    }
    i++;
  }
}

void foreground(char **args) {
  char *ptr = args[1];
  pid_t pid;
  int i = 0, found = 0, temp;
  if(*ptr == '%') {
    // check all the jobids for the correct job
    ptr++;
    temp = atoi(ptr);
    while (job_list[i] != NULL && found == 0) {
      if ((job_list[i]->jobid) == temp) {
        pid = job_list[i]->pid;
        found++;
      }
    }
  } else {
    // check all the pids for the correct job
    temp = atoi(ptr);
    while (job_list[i] != NULL && found == 0) {
      if ((job_list[i]->pid) == temp) {
        pid = job_list[i]->pid;
        job_list[i]->gpid = fgroup;
        found++;
      }
    }
  }

  // move the process to the foreground and give it a SIGCONT signal
  kill(pid, SIGCONT);
}

void background(char **args) {
  // handle background process
}

void stopprocess(char **args) {
  // stop a process?
  char *ptr = args[1];
  pid_t pid;
  int i = 0, found = 0, temp;
  if(*ptr == '%') {
    // check all the jobids for the correct job
    ptr++;
    temp = atoi(ptr);
    while (job_list[i] != NULL && found == 0) {
      if ((job_list[i]->jobid) == temp) {
        pid = job_list[i]->pid;
        found++;
      }
    }
  } else {
    // check all the pids for the correct job
    temp = atoi(ptr);
    while (job_list[i] != NULL && found == 0) {
      if ((job_list[i]->pid) == temp) {
        pid = job_list[i]->pid;
        found++;
      }
    }
  }
  // move the process to the foreground and give it a SIGCONT signal
  kill(pid, SIGTSTP);
}

int find_fg_bg(char **args) { // sees if process is a background process or a foreground process based on '&'
  int bg = 0; // returns 0 if FOREGROUND process, 1 if BACKGROUND
  char *ptr = args[0];
  while(*ptr != '\0') { // THIS IS NOT GOING TO WORK USE SHAKEEB'S NORMALIZER
    if (*ptr == '&') {
      bg = 1;
    }
    ptr++;
  }
  if (args[1] != NULL && *args[1] == '&') { // check to see if process is a background process
    bg = 1; // send command to background
  }
  return bg; // return if background or not
}
