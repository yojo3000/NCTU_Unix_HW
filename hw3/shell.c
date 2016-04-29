#include <stdio.h>
#include <stdlib.h>
#include "shell.h"

/*
  implement a simple shell named ysh (yojo3000 shell)
*/

int main()
{
  ysh_loop();
  return EXIT_SUCCESS;
}

char *ysh_read_command()
{
  /*
    read user input command in standard input
  */
  char *command_input_line = NULL;
  ssize_t buf_size = 0;
  getline(&command_input_line, &buf_size, stdin);
  return command_input_line;
}

void ysh_loop()
{
  /*
    shell will keep reading user input by using loop
  */
  int status = 0;
  char *input_command;

  do {
    printf("User > ");
    input_command = ysh_read_command();
  } while(status == 0); // end condition

  //free(input_command);
}

void ysh_launch()
{
  /*
    shell will fork itself to execute a command
  */
  pid_t pid;

  pid = fork();
  if(pid < 0)
  {

  }else if (pid == 0)
  {

  }else
  {

  }
}
