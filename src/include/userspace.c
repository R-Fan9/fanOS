#include "C/stdbool.h"
#include "C/stdint.h"
#include "C/stdio.h"
#include "C/stdlib.h"
#include "C/string.h"
#include "debug/display.h"
#include "filesystem/file.h"
#include "interrupts/keyboard.h"
#include "interrupts/pit.h"

void cmd();
void input_cmd(char *buf, int len);
bool run_cmd(char *buf);
KEYCODE get_key();
void run();
void cmd_read();

char *logo = "\
    __  _______  _____\n\
   /  |/  / __ \\/ ___/\n\
  / /|_/ / / / /\\__ \\ x86 Microcomputer Operating System \n\
 / /  / / /_/ /___/ / -------------------------------\n\
/_/  /_/\\____//____/  \n\n";

const int MAX_CMD_LEN = 99;

void userspace(void) {
  printf(logo);
  printf("\nType \"exit\" to quit, \"help\" for a list of commands\n");

  run();

  printf("\nExit command received; program halted");
  exit(0);
}

void run() {
  char *cmd_buf = malloc(MAX_CMD_LEN);

  do {
    memset(cmd_buf, 0, MAX_CMD_LEN);

    // display command prompt
    cmd();

    // input command
    input_cmd(cmd_buf, MAX_CMD_LEN);

  } while (run_cmd(cmd_buf) == false);

  free(cmd_buf);
}

void cmd() { printf("\ncmd />"); }

void input_cmd(char *buf, int len) {
  KEYCODE key = KEY_UNKNOWN;

  int i = 0;
  while (i < len) {
    key = get_key();

    if (key == KEY_RETURN) {
      break;

    } else if (key == KEY_BACKSPACE) {
      buf[i--] = 0;

    } else {
      char c = keyboard_key_to_ascii(key);

      if (c) {
        buf[i++] = c;
      }
    }

    sleep(10);
  }

  buf[i] = '\0';
}

bool run_cmd(char *buf) {
  if (strcmp((uint8_t *)buf, (uint8_t *)"help") == 0) {
    printf("\nSupported commands:\n");
    printf(" - exit: quits and halts the system\n");
    printf(" - clear: clears the screen\n");
    printf(" - help: displays this message\n");
    printf(" - read: reads a file\n");

  } else if (strcmp((uint8_t *)buf, (uint8_t *)"exit") == 0) {
    return true;

  } else if (strcmp((uint8_t *)buf, (uint8_t *)"clear") == 0) {
    clear_screen();

  } else if (strcmp((uint8_t *)buf, (uint8_t *)"read") == 0) {
    cmd_read();

  } else {
    printf("\nUnknown command");
  }
  return false;
}

void cmd_read() {
  FILE file = vol_open_file((uint8_t *)"KRNL    SYS");
  uint8_t *buf = malloc(128);

  printf("\n\n\r---------[%s]---------\n\r", file.name);

  while (file.end != 1) {
    vol_read_file(&file, buf);

    for (uint32_t i = 0; i < 128; i++) {
      printf("%x ", buf[i]);
    }

    if (file.end != 1) {
      printf("\n\r---------[Press \"q\" to quit]---------");
      if (get_key() == KEY_Q) {
        break;
      }

      printf("\r");
    }
    memset(buf, 0, 128);
  }
}

KEYCODE get_key() {
  KEYCODE key = KEY_UNKNOWN;

  while (key == KEY_UNKNOWN) {
    key = keyboard_get_last_key();
  }

  keyboard_discard_last_key();
  return key;
}
