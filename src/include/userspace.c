#include "C/stdbool.h"
#include "C/stdint.h"
#include "C/stdio.h"
#include "C/stdlib.h"

void cmd();
void input_cmd(char *buf);
bool run_cmd(char *buf);

int userspace(void) {
  char *cmd_buf;
  bool exit = false;

  while (exit == false) {

    // // display command prompt
    // cmd();
    //
    // // input command
    // input_cmd(cmd_buf);
    //
    // // run command
    exit = run_cmd(cmd_buf);
  }

  return 0;
}

void cmd(){
  printf("\ncommand>");
}

void input_cmd(char *buf){
}

bool run_cmd(char *buf){
  return false;
}


