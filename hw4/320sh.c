#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <readline.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int main (int argc, char **argv, char **envp) {

  int finished = 0; // as long as zero the while loop will run
  char *prompt = "320sh> "; // prompt for the shell input
  char cmd[MAX_INPUT]; // array to keep the command entered into the shell


  while (!finished) { // while finish == 0
    char *cursor;
    char last_char;
    int rv;
    int count;


    // Print the prompt and check if it is successful
    rv = write(1, prompt, strlen(prompt));
    if (!rv) {
      finished = 1;
      break;
    }
    
    // read and parse the input
    for(rv = 1, count = 0, cursor = cmd, last_char = 1;
      rv && (++count < (MAX_INPUT-1)) && (last_char != '\n'); cursor++) {

      rv = read(0, cursor, 1);
      last_char = *cursor;
    } 
    *cursor = '\0';

    if (!rv) { 
      finished = 1;
      break;
    }

    // Execute the command, handling built-in commands separately
    //while ()

    // Just echo the command line for now
    write(1, cmd, strnlen(cmd, MAX_INPUT));

  }

  return 0;
}
