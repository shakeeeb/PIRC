#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

// Assume no input line will be longer than 1024 bytes
#define MAX_INPUT 1024

int main (int argc, char **argv, char **envp) {

  int finished = 0; // as long as zero the while loop will run
  char *prompt = "320sh> "; // prompt for the shell input
  char cmd[MAX_INPUT]; // array to keep the command entered into the shell


  while (!finished) { // while finish == 0
    char *cursor;
    char last_char; 
    int rv; // check writes
    int count;


    // Print the prompt and check if it is successful
    rv = write(1, prompt, strlen(prompt));

    if (!rv) { // check to make sure it wrote correctly
      finished = 1;
      break;
    }
    cursor = readline(prompt); //readline grabs the line, and places it into cursor

    // we can replace all this with gnu readline 
    // the main reason for readline is to have the history, already done for us. which is glorious

    // Execute the command, handling built-in commands separately

    // Just echo the command line for now
    write(1, cmd, strnlen(cmd, MAX_INPUT)); // tells us that it recieved our input correctly
    // parse through cursor to make see if it's 'exit ' for the sake of exiting

  }

  return 0;
}
/**
*Exit just exits shit
*
*/
public static void Exit(void){
  exit(0);
}
