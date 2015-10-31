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
  char *cmd = malloc(MAX_INPUT_2); // the buffer, for the string

  // this is all path stuff
  // this just gets the PATH variable and all the possible paths
  char** path; // this holds all the paths
  char* pathholder; // particular path in path array
  char* pathdelimiter = ":"; // delimeter for args
  pathholder = malloc(strlen(getenv("PATH")) * sizeof(char) + 1);
  strcpy(pathholder, getenv("PATH")); // GETENV IS NOT REENTRANT (same copy in memory can be used by mult users)
  path = parse_args(pathholder, pathdelimiter); // pass particular path and delimeter

  setenv("?", "0", 1); // create "?" environment variable & set default to zero

  //readout the history from the history file
  //HISTORY

  FILE* fpointer = fopen(".history.txt", "r+");
  if(fpointer == NULL){
    fpointer = fopen(".history.txt", "w+");
  }
  char* intermediary = malloc(1025);
  int col = 0;
  // and then set the filepath for history
  getcwd(history_filepath, 1025);
  strcat(history_filepath, "/.history.txt"); // it'll always be in the same working directory
  // now i've gotta read out all of the history from the history file into the array
  while(fgets(intermediary, 1024, fpointer) != NULL){
    // now write each character by character into the thingie
    for(col = 0; col < strlen(intermediary); col++){
      if(intermediary[col] == '\n'){
        history[history_index][col] = '\n';
        history[history_index][col+1] = '\0';
        break;
      }
      history[history_index][col] = intermediary[col];
    }
    history_index++;
  }
  fclose(fpointer);

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
    int i = 0;
    cursor = cmd;
    // read the input
    int newlineFlag = 0; // check if user just inputted a '\n'
    for(rv = 1, rw = 1, count = 0, /*cursor = cmd, */last_char = 1; // all of the variables
	   rv && rw && (++count < (MAX_INPUT_2-1))  && (last_char != '\n'); cursor++) { // all of the conditions
      rv = read(0, cursor, 1); // reads one byte into cursor
      i++;

      if ((*cursor == 127 || *cursor == 8) && count != 0) {
        count--;
        if (count > 0) {
          //printf("\b\003[K");
          write(1, "\b\003[K", 1);
          write(1, " ", 1);
          write(1, "\b\003[K", 1);
          cursor--;
          cursor--;
          //cmd[i-1] = '\0';
          count--;
          continue;
        }
      } else {
        rw = write(1, cursor, 1); // writes byte that user typed (for use with launcher)
      }

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

    cursor = cmd;
    while(*cursor != '\0') {
      if (*cursor == '&') {
        *cursor = ' ';
        cursor++;
        *cursor = '&';
        cursor++;
        *cursor = '\0';
      }
      cursor++;
    }

    // before i parse the arguments out, i should normalize input
    char* butt = normalize((char*)cmd);
    // here i can take a line, and place it into the array of characters
    // HISTORY
    if(history_index  == 50){
      history_index = 0;
    }
    int e = 0;
    for(e = 0; e < 1025 ;e++){
      history[history_index][e] = cmd[e];
    }
    strcat(history[history_index], "\0"); // place a newline for the sake of whatever
    history_index++;

    debug("normalized command: %s\n", butt);
    args = parse_args((char*)butt, whitespace); // parse the command from the line

    // this is just a tester command
    if(strncmp(cmd, "print", 3) == 0){
      mass_print(path);
    }
    // i need to check if it's got any sort of redirection
    // i've got to think about how to abstract things out
    char j = find_redirect_or_pipe(args); //
    if(j != '0'){
      // then there is either at least one redirect or pipe
      //actually, call another function
      // that function will take care of where to send the commands
      //via slice, instead of going straight to begin redirect
      //begin_redirect(args, j, 0);
      process_pipes(args); //this will include begin redirect
    } else {
      begin_execute(args);
    }

     // send in args to be executed

    // remember to free EVERYTHING
    // free(butt); // this was malloced
    // free(cpath);
    // free(fprompt);
    // free(args); loop through args before freeing it
    // if time flag is set print times
    if(tflag == 1) {
      print_times(start);
    }
  }
  free(cmd);
  return 0;
}
/*
*just frees a whole array of strings
*/
/*
void free_string_array(char** array){
  // assume is double null terminated
  int i = 0;
  while(array[i] != NULL){
    free(array[i]);
    i++;
  }
  free array;
}
*/
int find_number_of_pipes(char** args){
  //assume args is 2x null terminated
  int i = 0;
  int count = 0;
  for(i = 0; args[i] != NULL; i++){
    if(!strcmp(args[i], "|")){
      count++;
    }
  } return count;
}

int process_pipes(char** args){
  // this function will take args
  // look through args, and execute each command
  //whether it be a pipe or a redirect
  // if it's a redirect, it will call begin redirect with the slice that is the redirect
  // it'll specifically look for the first 'clause' or the last 'clause', which can be redirects
  // and it'll do the pipes in between
  // so first, find the redirect, if there is one
  char* cursor = args[0]; //cursor will look through the array of strings
  int size_of_clause = 0;
  char direction;
  int n = find_number_of_pipes(args);
  int d = size_of_string_array(args);
  int in_exists = 0;
  int out_exists = 0;
  int hiddendottxt = 0;
  //grabbing each line
  // i've gotta find the redirect
  in_exists = find_position_of_in_array(args, "<");
  out_exists = find_position_of_in_array(args, ">");
  if((in_exists > out_exists) && (out_exists > 0)){ // if the > is before the <
    unix_error("invalid redirection");
    return 1;
  }
  while((cursor != NULL) && (strcmp(cursor, "|"))){
    // it'll stop once it hits a pipe, or if the command is done
    // this, however, sets the whole cluase to  x<y >z

    if(strcmp(cursor, "<") == 0){
      direction = '<';
    }
    if(strcmp(cursor, ">") == 0){
      direction = '>';
    }
    size_of_clause++;
    cursor = args[size_of_clause];
    if(cursor == NULL){ // apprently, because of string
      break;
    }
  }
  //specifically if >, then you can't have any pipes. so, end after that.
  char** call_redirect = slice(args, args[0], size_of_clause); // not sure if slice will include the pipe symbol itself
  if(direction == '>' && (in_exists == -1)){
    // call begin redirect, and end
    begin_redirect(call_redirect, direction, 0);
    //free(call_redirect); // this isnt reight either, use free_string_array
    return 0;
  }
  // if the cursor is null, then your'e done.
  // otherwise, just call begin redirect, and it will return here.
  // thte case ls -l < in.txt > out.txt can be taken care of here
  int doubleredirect = 0;
  doubleredirect = find_position_of_in_array(args, ">"); // find the position of the output.
  if(n > 0 || doubleredirect != -1){ // if there are actually pipes, things have to redirect into .hidden.txt
    //i've got to edit the begin redirect so that it knows to redirect into hidden .txt when i tell it to
    //create/open .hiddendottxt
    // these are two cases
    if (doubleredirect != -1){ // ls -l < in.txt > out.txt
      //now redirect outwards, from hiddendottxt
      // the last argument, before the null terminator should be the filename
      char* output_filename = args[d-1];
      char* input_filename = args[d-3];
      int last_output = 0;
      int first_input = 1;
      if(output_filename == NULL){
        unix_error("file not specified");
      } else {
        if(access(output_filename, F_OK) != -1){ // file already exists
          last_output = open(output_filename, O_RDWR, S_IRUSR | S_IWUSR);
        } else { // file doesnt already exist
          last_output = open(output_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        }
      } if (last_output == -1){
        unix_error("cannot open file");
      }
      // open the input file
      if(input_filename == NULL){
        unix_error("file not specified");
      } else {
        if(access(input_filename, F_OK) != -1){ // file already exists
          first_input = open(input_filename, O_RDWR, S_IRUSR | S_IWUSR);
        } else { // file doesnt already exist
          first_input = open(input_filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
        }
      } if (first_input == -1){
        unix_error("cannot open file");
      }
      // now stream the output
      // ive gotta slice the args now
      char** smallargs = slice(args, args[0], d-4); // take a big slice out of it
      Redirect(first_input ,last_output, STDERR_FILENO, smallargs);
      // then free smallargs
      // and close the fds
      // and then return
      Close(first_input);
      Close(last_output);
      return 0;
      //all this code is literally for the case of somefunction < in.txt > out.txt
    } else { //its just part of a regular pipe. stream to hiddendottxt
      hiddendottxt = 0;
      hiddendottxt = open(".hidden.txt", O_RDWR | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR);
      //begin_redirect(call_redirect, '>', hiddendottxt); // remember to close hiddendottxt, also! not direction, its >
      // why not just call redirect? and redirect into hiddendottext? im no opening any files.
      Redirect(STDIN_FILENO, hiddendottxt, STDERR_FILENO, call_redirect);
    }
  } else { // its not greater than zero, then just redirect once and return
    begin_redirect(call_redirect, direction, 0);
    //free(call_redirect); // gotta use free string array
    return 0;
  }
  //we're gonna have more homework on monday?
  //but why? im just so tired...
  if(cursor == NULL){
    //then, just return here. no need to do anyhting else
    return 0;
  }
  // free(call_redirect); // gotta use free string array
  // after this initial redirect, cursor will be at the top of the new pipe
  int i = 0; // control variable for pipes
  int j = 0; // control variable for searching through clauses
  int in = 0; // in file desriptor
  int index_in_cmd_stream = size_of_clause; // this keeps track of the index in the array
  char* secondary_cursor = args[0]; // a secondary_cursor
  char** arraycursor = &args[size_of_clause]; // one past the first pipe
  int read_from_hidden = 1; // the first one reads from hidden.txt, the next ones don't, they just pipe
  size_of_clause = 0;
  int fd[2];
  //since the first redirect has to be an input redirect, we don't have to worry about ... wait a second.
  for(i = 0; i < n-1; i++){
    // i have to slice each individual command
    // cursor is currently pointing at the first '|' symbol, move forwards one
    // move through from that point forwards,
    cursor = args[index_in_cmd_stream+1]; // move teh cursor forwards, past the '|'
    for(j = index_in_cmd_stream+1;((strcmp(secondary_cursor, "|")));j++){ // NULL case should never happen
      secondary_cursor = args[j];
      arraycursor = &args[j]; // this is a pointer, to a subarray of teh major array
      size_of_clause++; // this increments the size of the slice, for slice
      if(secondary_cursor == NULL){
        break; // breka out of the for loop
      }
    }
    call_redirect = slice(args, cursor, size_of_clause - 1); // this might jsut be size of clause - 1
    cursor = secondary_cursor; // resetting the cursor to the position of the secondary cursor
    if(read_from_hidden == 1){
      // in this case, it reads from  hidden.txt
      pipe(fd); // so i call pipe here?
      Close(hiddendottxt);
      hiddendottxt = open(".hidden.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      Redirect(hiddendottxt, fd[1], STDERR_FILENO ,call_redirect);// call redirect straight, ive got shit already
      Close(fd[1]); // close the write end, child messes with taht shit
      // keep the read end of the pipe
      in = fd[0];
    } else { //every other case, where it's not reading from hidden.txt
      // after slicing each command, send it over
      pipe(fd);
      Redirect(in, fd[1], STDERR_FILENO, call_redirect);
      Close(fd[1]); // close the write end, child messes with that shit
      in = fd[0]; // keep the read end of the pipe
    }

    // remember to stop reading from hidden.txt after the first loop
    // free(call_redirect);
    index_in_cmd_stream += size_of_clause; // add the size of the clause to the index in cmd stream
    read_from_hidden = 0;
  }
  // ok so now here, and the last clause can be a redirection into a file
  // but it's supposed to do every clause but the last clause, so the loop may be for n-2 rather than n-1
  // so check the command to see if there's a redirect
  char c = find_redirect_or_pipe(arraycursor); // i send the lower part of the array into this function
  if(c == '>'){ // only legal case, except for a pipe of 1
    // ughh, sinc begin redirect doesnt take open file descriptors, i have to manually open the last file here.. great
    int last_redirect_fd;
    char* fileptr = args[d-1]; //the final argumetn should be the filename.. hopefully this is right
    if(fileptr == NULL){
      // it doesn't exist... which can't happen
      unix_error("file doesn't exist");
    } else {
      if(access(fileptr, F_OK) != -1){
        last_redirect_fd = open(fileptr, O_RDWR, S_IRUSR | S_IWUSR); // it exists, just open it
      } else {
        last_redirect_fd = open(fileptr, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR); // gotta create it
      }
      if(last_redirect_fd == -1){
        unix_error("cannot open file");
      }
    }
    // i've opened the file, now ive gotta redirect into it
    Redirect(STDIN_FILENO, last_redirect_fd, STDERR_FILENO, arraycursor); // straight up redirect

    // close up all of my file descriptors
    // which means, last redirect fd and dothiddendottext
    // and that's it. return.
    Close(last_redirect_fd);
  } else if (c == '|'){ // it could possibly be a single pipe
    // in which case, i have to read into that command from hiddendottxt
    // i might need to close it, and then open it again
    if(read_from_hidden == 1){
      Close(hiddendottxt);
      hiddendottxt = open(".hidden.txt", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
      int w = size_of_string_array(arraycursor); // arraycursor includes the pipe itself
      call_redirect = slice(args, arraycursor[1], w - 1);
      Redirect(hiddendottxt, STDOUT_FILENO, STDERR_FILENO, call_redirect);
    } else {
      Redirect(fd[0], STDOUT_FILENO, STDERR_FILENO, call_redirect);
    }
  }
  // this is the true end of the function
  // so all cleanup can take place here
  Close(hiddendottxt);
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
*it finds the position of a string in an array of strings
*/
int find_position_of_in_array(char** haystack, char* needle){
  char* cursor;
  int i = 0;
  cursor = haystack[0];
  if(needle == NULL){
    return -1;
  }
  while(cursor != NULL){
    if(strcmp(cursor, needle) == 0){
      return i;
    }
    i++;
    cursor = haystack[i];
    if(cursor == NULL){
      break;
    }
  }
  return -1;
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
    unix_error("malloc error");
    // malloc error
  }
  token = strtok(command, delimiter); // grab the first token
  while (token != NULL){
    debug("result: %s spot: %d\n", token, spot);
    token = strcat(token, "\0"); // just append a zero to token
    result[spot] = strdup(token); // actually have the pointer pointe to a duplicate string
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
  // before exit, write the history array to the history file
  // HISTORY
  FILE* fpointer = fopen(history_filepath, "r+");
  int i = 0;
  int j = 0;
  for(i = 0; i < 50;i++){
    for(j = 0; j < sizeof(history[i]) ;j++){
      if(history[i][j] == '\0'){
        break;
      }
      fputc(history[i][j], fpointer);
    }
  }
  fclose(fpointer);
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
/*
int check_validity_of_pipes(char** args){ // 0 if valid, 1 if invalid
  // see i don't even know if this will work lol
  int i;
  int found_redirect_in = 0; // <
  int found_redirect_out = 0; // >
  int found_pipe = 0; // |
  for(i = 0; args[i] != NULL; i++){
    if(found_redirect_out > 0){ // if an out redirection was first
      if(strcmp(args[i], "<") == 0){ // you find a redirect inwards
        return 1; // invalid
      }
      if(strcmp(args[i], ">") == 0){ // you found another redirect outwards
        return 1; // invalid
      }
      if(strcmp(args[i], "|") == 0){ // or you found a pipe
        return 1; // invalid
      }
      //basically an out redirection must be last
    }

    if(found_redirect_in == 1){// an inward redirection was first, it needs to be first. can't be anything else
      if(strcmp(args[i], "<") == 0){ // you find another redirect inwards
        return 1;
      }
      // everything else is okay, but you've gotta find it
      if(strcmp(args[i], ">") == 0){ // an outward redirection
        found_redirect_out = 2;
      }
      if(strcmp(args[i], "|") == 0){ // a pipe
        found_pipe = 2;
      }
    }

    if(found_pipe > 0){ // if i found a pipe
      if(strcmp(args[i], "<") == 0){ // found a inward redirect
        return 1; // invalid
      }
      if(strcmp(args[i], ">") == 0){ // found an outward redirect
        found_redirect_out = 2;
      }
    }

    if(strcmp(args[i], "<") == 0){ // found a inward redirection first
      found_redirect_in = 1;
    }
    if(strcmp(args[i], ">") == 0){ /d/ found an outward redirection first
      found_redirect_out = 1;
    }
    if(strcmp(args[i], "|") == 0){ // found a pipe first
      found_pipe = 1;
    }
  } return 0;

} */

/**
* unix style error from the textbook
*/
void unix_error(char *msg){
  fprintf(stderr, "%s: %s\n", msg, strerror(errno));
}
/**slice takes a slice out of an array
*
*/
char** slice(char** args, char* start, int size) {
  int i = 0;
  // need to find start first
  int start_index = 0;
  for(start_index = 0; args[start_index]; start_index++){
    if(strcmp(args[start_index], start) == 0){
      break;
    }
  }
  if(args[start_index] == NULL){
    return NULL; // start doesnt exist in the array
  }
  char** result;
  //char* end = NULL;
  result = malloc((size+2) * sizeof(*result));
  if(result == NULL){
    return NULL;
  }
  for(i = 0; i < size; i++){
    if(args[start_index+i] == NULL){
      // place a null terminator
      result[i] = NULL;
      break;
    }
    result[i] = strdup(args[start_index+i]);
  }
  //result[i+1] = malloc(sizeof(char*)); // doub
  result[i] = NULL;
  return result;
}
/**
* mass print takes an array of strings and
* just prints them all out to the shell
*/
void mass_print(char** tokens) { // i made this because for whatever reason, printf wasn't working for me.
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

int begin_redirect(char** args, char direction, int hiddendottxt){
  // look through
  // lets say that redirect can only be an executable
  // hidden dot text is either a file descriptor for a file
  // called hidden dot text, which
  // when beginning a pipe, is written to
  // otherwise, it will be zero
  // assume that hidden dot text has already been opened
  char *cmd_holder = args[0];
  int i;
  char a, b;
  a = *cmd_holder; // the first character
  b = *(cmd_holder+1); // the second character
  char* filename;
  char* direction_as_string = NULL;
  char** newargs;

  //i have to know the number of 'clauses'
  // the number of pipes,
  // whether or not a redirection was made


  // locate the file so i can set a pointer to it
  int k = size_of_string_array(args);
  for(i = 0; i<k;i++){
    if(strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0){ // if i've found the redirect
      // the next token is the filename
      //int y = sizeof(args[i]);
      direction_as_string = args[i];
      filename = args[i+1];
      break;
    }
  }
  int n = find_position_of_in_array(args, direction_as_string);
  newargs = slice(args, args[0], n);
  // open the file. but first, check if its null.
  int fd = 0;
  if(filename == NULL){
    unix_error("nowhere to redirect");
  } else { // open the file
    if(access(filename, F_OK) != -1){
      // file exists
      fd = open(filename, O_RDWR, S_IRUSR|S_IWUSR); // mode is user can read and write to file
    } else {
      // file doesnt exist
      fd = open(filename, O_RDWR | O_CREAT, S_IRUSR|S_IWUSR); // again, mode includes readwrite permissions
    }
    if(fd == -1){
      unix_error("couldnt open file");
      exit(127);
    }
  }
 // so now i have a file descriptor in fd
  if(a == '/'){
    // if a is slash
    debug("i have recognized a slash character!\n");
    // i can send it straight to execvp with no worries
    // i have to add the code for recognzing which direction here
    if(hiddendottxt == 0){
      switch(direction){
        case '<':  //redirect inwards
          Redirect(fd, STDOUT_FILENO, STDERR_FILENO, newargs);
          break;
        case '>': //redirect outwards
          Redirect(STDIN_FILENO, fd, STDERR_FILENO, newargs);
          break;
        case '2': // redirect error
          Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
          break;
        default: //otherwise, break
          unix_error("redirecting in unknown direction");
          break;
      }
    } else {
      // now i need to redirect into hiddendottxt
      switch(direction){
        case '<':  //redirect inwards
          Redirect(fd, hiddendottxt, STDERR_FILENO, newargs);
          break;
        case '>': //redirect outwards
          Redirect(hiddendottxt, fd, STDERR_FILENO, newargs);
          break;
        case '2': // redirect error
          Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
          break;
        default: //otherwise, break
          unix_error("redirecting in unknown direction");
          break;
      }
    }

  } else if ((a == '.') && (b == '/')) {
    // if its ./ something
    debug("i have recognized a dot slash character!\n");
    if(hiddendottxt == 0){ // not part of a pipe
      switch(direction){
        case '<':  //redirect inwards
          Redirect(fd, STDOUT_FILENO, STDERR_FILENO, newargs);
          break;
        case '>': //redirect outwards
          Redirect(STDIN_FILENO, fd, STDERR_FILENO, newargs);
          break;
        case '2': // redirect error
          Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
          break;
        default: //otherwise, break
          unix_error("redirecting in unknown direction");
          break;
      }
    } else { // redirect into hidden dot text
      switch(direction){
        case '<':  //redirect inwards
          Redirect(fd, hiddendottxt, STDERR_FILENO, newargs);
          break;
        case '>': //redirect outwards
          Redirect(hiddendottxt, fd, STDERR_FILENO, newargs);
          break;
        case '2': // redirect error
          Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
          break;
        default: //otherwise, break
          unix_error("redirecting in unknown direction");
          break;
      }
    }
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
    char* filepath = find_filepath(paths, cmd_holder); // i dont malloc stuff here, i malloced in find filepath
    //either this filepath is NULL, meaning the path wasn't able to be found, OR
    // its not NULL, and i found the path
    if(filepath == NULL){
      setenv("$?", "127", 1);
      printf("%s: command not found\n", args[0]);
    } else {
      // yay
      if(hiddendottxt == 0){ // normal redirect
        switch(direction){
          case '<':  //redirect inwards
            Redirect(fd, STDOUT_FILENO, STDERR_FILENO, newargs);
            break;
          case '>': //redirect outwards
            Redirect(STDIN_FILENO, fd, STDERR_FILENO, newargs);
            break;
          case '2': // redirect error
            Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
            break;
          default: //otherwise, break
            unix_error("redirecting in unknown direction");
            break;
        }
      } else { // redirect into hiddendottxt
        switch(direction){
          case '<':  //redirect inwards
            Redirect(fd, hiddendottxt, STDERR_FILENO, newargs);
            break;
          case '>': //redirect outwards
            Redirect(hiddendottxt, fd, STDERR_FILENO, newargs);
            break;
          case '2': // redirect error
            Redirect(STDIN_FILENO, STDOUT_FILENO, fd, newargs);
            break;
          default: //otherwise, break
            unix_error("redirecting in unknown direction");
            break;
        }
      }

    }
    free(paths);
  }
  //free(newargs);
  // close the file descriptor fd
  close(fd);
  return 1; // why 1? i dont remember...
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
    // else its just a normal thing
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
          case 4: // ls -- globbing
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
          case 9: // history
            debug("found a builtin: %s\n", builtins[i]);
            if(dflag == 1)
              printf("RUNNING: history\n");
            ustart = time(0);
            History();
            uend = time(0);
          case 10:
          debug("found a builtin: %s\n", builtins[i]);
          if(dflag == 1)
            printf("RUNNING: clear-history\n");
          ustart = time(0);
          clear_History();
          uend = time(0);
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
  int waiting, waitplz; // saves the integer to see if the parents should continue waiting
  int status = 0;
  int bg; // holds background job status for child
  char *prints = malloc(MAX_INPUT_2);

  bg = find_fg_bg(args); // see if child is foreground process or not
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
      while(wait(&waitplz) != cpid);
      //add_job(bg, ppid, getpgid(cpid), args[0]);
      exit(WEXITSTATUS(waitplz));
    }
  }
  else { // for parents process ONLY
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
  free(prints);
  return;
}
/**
* a wrapper around close
*/
int Close(int fd){
  int result = 0;
  if((result = close(fd)) < 0){
    setenv("?", "1", 1);
    unix_error("unable to close file");
  }
  return result;
}
/**
* this is an execution with a redirection
* file descriptor- the file descriptor of the file that im
* if direction is '<', redirecting stdin
* if direction is '>', redirecting stdout
* if direction is '2', redirects stderr
*/
void Redirect(int in, int out, int err, char **args){

  pid_t cpid; // the pid of the child
  int waiting; // saves the integer to see if parents continue waiting
  int status = 0;
  char *prints = malloc(MAX_INPUT_2);

  cpid = Fork();//

  if(cpid == 0){
    // this is child, dup based on direction. the child execs
    if(in != STDIN_FILENO){ // if the input isn't regular STDIN_FILENO, dup2 will change it
      dup2(in, STDIN_FILENO);
      Close(in);
    }
    if(out != STDOUT_FILENO){
      dup2(out, STDOUT_FILENO);
      Close(out);
    }
    if(err != STDERR_FILENO){
      dup2(err, STDERR_FILENO);
      Close(err);
    }
    if(execvp(*args, args) < 0) { // call execvp and check to see if it was successful
      printf("%s: command not found\n", args[0]); // if not successful print error message
      exit(127); // exit with error
    }
  } else { // its the parent, so it waits
    while(wait(&waiting) != cpid); // wait for the child to finish & reap before continuing
    status += WEXITSTATUS(waiting);
    snprintf(prints, MAX_INPUT_2, "%d", status);
    //itoa(status, prints, 10);
    setenv("$?", prints, 1);
    //close(file_descriptor); // oooh so it does close things
  }
  free(prints);
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

char find_redirect_or_pipe(char** args){ // this looks for a > or a < or a |
  // this checks arg1 in particualr,
  // as well as the whole array in general
  // returns either: 0, 1, 2, 3
  // 0 if nothing is there
  // 1 if its a <
  // 2 if its a >
  // 3 if its a | and there can be multiple of these
  //first check arg1

  char result = '0';
  // now check the array for this stuff
  int k = size_of_string_array(args); //
  int i = 0;
  for(i = 0;i < k;i++){ //
    if(args[i] == NULL){
      break;
    }
    if(strcmp(args[i], "<") == 0){
      result = '<';
      return result;
    }else if(strcmp(args[i], ">") == 0){
      result = '>';
      if(strcmp(args[i - 1], "2") == 0){
        result = '2';
      }
      return result;
    }else if(strcmp(args[i], "|") == 0){
      result = '|';
      return result;
    }
    // in this case it's just gonna be zero
  } return '0';
}

int contains(char* haystack, char* needle){ // this some shit in the string
  if(strstr(haystack, needle) != NULL){
    return 1; // return 1 found
  } else {
    return 0; // return 0 if not found
  }
}

void globbing(char *args) {
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
  char* value = malloc(MAX_INPUT_2);
  int limit = size_of_string_array(args);
  if(limit > 2){ // i've got to piece the arguments back together
    int i; // because zero is the command
    for(i = 1; i < limit; i++){
      // take that argumetn and append it onto the string`1
      if(i == 1){
        strcpy(value, args[i]); // copy it in if it's the first one
      } else {
        strcat(value, args[i]); // else concatenate it
      }
      // and then pass in this new argument string to the
    }
  } else {
    strcpy(value, args[1]); //else it's all just in arg1, like it's supposed to be
  }
  variable = strtok(value, "="); // find the environment variable
  setTo = strtok(NULL, "="); // find what it should be set to
  setenv(variable, setTo, 1); // set the environment variable
  free(value);
  setenv("?", "0", 1);
  return;
}
/**
*get size of string array
*
*/
int size_of_string_array(char **args){ // assume the array is double null terminated
  // because parser double null terminates things
  int i = 1;
  char* cursor = args[0];
  while(cursor != NULL){
    i++;
    cursor = args[i];
  }
  return i;
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

char* normalize(char* command){ // normalize the input so we dont have to deal with bullshit cases
  char * buffer = malloc(MAX_INPUT_2);
  // look for =, <, >, |, ... 2> is wierd, so fuck that shit
  int k = strlen(command);
  int i = 0;
  char* cursor;
  char* buffercursor;
  cursor = command; //
  buffercursor = buffer; //

  for(i = 0; i < k; i++){
    switch(*cursor){
      case '<': //
        strncpy(buffercursor, " < ", 3);
        buffercursor = buffercursor + 2;
        break;
      case '>':
        strncpy(buffercursor, " > ", 3);
        buffercursor = buffercursor + 2;
        break;
      case '=':
        strncpy(buffercursor, " = ", 3);
        buffercursor = buffercursor + 2;
        break;
      case '|':
        strncpy(buffercursor, " | ", 3);
        buffercursor = buffercursor + 2;
        break;
      default:
        // its just a regular character, it doesnt matter
        // just write it
        strncpy(buffercursor, cursor, 1); //
        break;
    }
    buffercursor = buffercursor+1;
    cursor = cursor+1;
  }
  return buffer;
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
  while((cpid = (waitpid(-1, NULL, 0))) > 0) {
    sigprocmask(SIG_BLOCK, &allmask, &pmask);
    remove_job(cpid);
    sigprocmask(SIG_SETMASK, &pmask, NULL);
  }
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

void History(){
  // this just prints out the file
  FILE * fpointer = fopen(history_filepath, "r");
  int count = 0;
  char* intermediary = malloc(1025);
  while(fgets(intermediary, 1025, fpointer) != NULL){
    printf("%d %s", count, intermediary);
    count++;
  }
  fclose(fpointer);
}

void clear_History(){
  FILE* fpointer = fopen(history_filepath, "w+");
  fclose(fpointer);
}
