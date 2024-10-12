#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#ifndef NO_X
#include <X11/Xlib.h>
#endif

#include "blocks.h"

#define SIGNAL_PLUS   SIGRTMIN
#define SIGNAL_MINUS  SIGRTMIN

#define ARRAY_LENGTH(_arr)  (sizeof(_arr) / sizeof(_arr[0]))
#define COMMAND_LENGTH      50
#define MIN(_x, _y)         ((_x < _y) ? _x : _y)
#define STATUS_LENGTH       (ARRAY_LENGTH(blocks) * COMMAND_LENGTH + 1)

void handle_signal(int signal_number);
void retrieve_commands(int current_time);
void retrieve_signal_commands(unsigned int signal);
void setup_signal_handlers();
int combine_status(char *combined_status, char *last_status);
void status_update_loop();
void terminate_handler();
void output_to_stdout();

#ifndef NO_X
void set_root_window();
static void (*update_status) () = set_root_window;
static int initialize_x();
static Display *display;
static int screen_number;
static Window root_window;
#else
static void (*update_status) () = output_to_stdout;
#endif

static char status_bar[ARRAY_LENGTH(blocks)][COMMAND_LENGTH] = {0};
static char status_strings[2][STATUS_LENGTH];
static char status_delimiter[] = " / ";
static int continue_status_updates = 1;

void retrieve_command(const StatusBlock *block, char *output) {
  char temp_status[COMMAND_LENGTH] = {0};
  strcpy(temp_status, block->icon);
  FILE *command_file = popen(block->command, "r");
  if (!command_file)
    return;

  int i = strlen(block->icon);
  fgets(temp_status + i, COMMAND_LENGTH - i - strlen(status_delimiter), command_file);
  i = strlen(temp_status);

  if (i != 0) {
    i = temp_status[i - 1] == '\n' ? i - 1 : i;
    if (status_delimiter[0] != '\0') {
      strncpy(temp_status + i, status_delimiter, strlen(status_delimiter));
    } else {
      temp_status[i++] = '\0';
    }
  }
  strcpy(output, temp_status);
  pclose(command_file);
}

void retrieve_commands(int current_time) {
  const StatusBlock* current_block;
  for (unsigned int i = 0; i < ARRAY_LENGTH(blocks); i++) {
    current_block = blocks + i;
    if ((current_block->interval != 0 && current_time % current_block->interval == 0) || current_time == -1)
      retrieve_command(current_block, status_bar[i]);
  }
}

void retrieve_signal_commands(unsigned int signal) {
  const StatusBlock *current_block;
  for (unsigned int i = 0; i < ARRAY_LENGTH(blocks); i++) {
    current_block = blocks + i;
    if (current_block->signal == signal)
      retrieve_command(current_block, status_bar[i]);
  }
}

void setup_signal_handlers() {
  for (unsigned int i = 0; i < ARRAY_LENGTH(blocks); i++) {
    if (blocks[i].signal > 0)
      signal(SIGNAL_MINUS + blocks[i].signal, handle_signal);
  }
}

int combine_status(char *combined_status, char *last_status) {
  strcpy(last_status, combined_status);
  combined_status[0] = '\0';
  for (unsigned int i = 0; i < ARRAY_LENGTH(blocks); i++)
    strcat(combined_status, status_bar[i]);

  combined_status[strlen(combined_status) - strlen(status_delimiter)] = '\0';
  return strcmp(combined_status, last_status);
}

#ifndef NO_X
void set_root_window() {
  if (!combine_status(status_strings[0], status_strings[1]))
    return;
  XStoreName(display, root_window, status_strings[0]);
  XFlush(display);
}

int initialize_x() {
  display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "dwmblocks: Failed to open display\n");
    return 0;
  }
  screen_number = DefaultScreen(display);
  root_window = RootWindow(display, screen_number);
  return 1;
}
#endif

void output_to_stdout() {
  if (!combine_status(status_strings[0], status_strings[1]))
    return;
  printf("%s\n", status_strings[0]);
  fflush(stdout);
}

void status_update_loop() {
  setup_signal_handlers();
  int current_time = 0;
  retrieve_commands(-1);
  while (1) {
    retrieve_commands(current_time++);
    update_status();
    if (!continue_status_updates)
      break;
    sleep(1.0);
  }
}

void handle_signal(int signal_number) {
  retrieve_signal_commands(signal_number - SIGNAL_PLUS);
  update_status();
}

void terminate_handler() {
  continue_status_updates = 0;
}

int main(int argc, char** argv) {
  for (int i = 0; i < argc; i++) {
    if (!strcmp("-d", argv[i]))
      strncpy(status_delimiter, argv[++i], sizeof(status_delimiter) - 1);
    else if (!strcmp("-p", argv[i]))
      update_status = output_to_stdout;
  }

  status_delimiter[strlen(status_delimiter)] = '\0';

  signal(SIGTERM, terminate_handler);
  signal(SIGINT, terminate_handler);
#ifndef NO_X
  if (!initialize_x())
    return 1;
#endif
  status_update_loop();
#ifndef NO_X
  XCloseDisplay(display);
#endif
  return 0;
}

