#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <signal.h>

static int output_fd;
static struct termios initial;
static uint16_t terminal_width;
static uint16_t terminal_height;

void hui_print_sz(char* string, size_t size) {
 write(output_fd, string, size); 
}

void hui_print(char* string) {
  hui_print_sz(string, strlen(string));
}

static void hui_resize(int i) {
  (void)i;
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	terminal_width = ws.ws_col;
	terminal_height = ws.ws_row;
}

static void hui_restore() {
	tcsetattr(1, TCSANOW, &initial);
  char* clean_buffer = "\x1b]2J";
  hui_print(clean_buffer);

  char* switch_back_to_terminal = "\x1b[?1049l";

  hui_print(switch_back_to_terminal);

  char* show_cursor = "\x1b[?25h";

  hui_print(show_cursor);
}

static void hui_die(int i) {
(void)i;
	exit(1);
}

typedef enum {
  RED,
  BLUE,
  GREEN,
} Color;

typedef struct Window {
  size_t width;
  size_t height;
  size_t x;
  size_t y;
  Color bg_color;
  Color fg_color;
  struct Window* child;
} Window;

Window hui_init() {
	struct termios term;
	tcgetattr(1, &term);
	initial = term;
	atexit(hui_restore);
  signal(SIGWINCH, hui_resize);
	signal(SIGTERM, hui_die);
	signal(SIGINT, hui_die);
	term.c_lflag &= (~ECHO & ~ICANON);
	tcsetattr(1, TCSANOW, &term);
  output_fd = 1;
  hui_resize(0);
  
  char* enter_alternate_buffer = "\x1b[?1049h";
  
  hui_print(enter_alternate_buffer);

  char* clean_buffer = "\x1b]2J";
  hui_print(clean_buffer);

  char* hide_cursor = "\x1b[?25l";
  hui_print(hide_cursor);

  Window window = {
    .width = terminal_width,
    .height = terminal_height,
    .x = 0,
    .y = 0,
    .bg_color = RED,
    .fg_color = GREEN,
  };

  return window;
}

void hui_move_cursor_to(int y, int x) {
  char buffer[50];
  hui_print("\x1b[");   
  sprintf(buffer, "\x1b[%d;%dH", y, x);
  hui_print(buffer); 
}

void hui_put_character_at(char c, int y, int x) {
  hui_move_cursor_to(y,x);
  write(1, &c, 1);
}

void hui_draw_window(Window window) {
  char* clean_buffer = "\x1b]2J";
  hui_print(clean_buffer);
  size_t window_width = window.x + window.width;
  size_t window_height = window.y + window.height;

  for (size_t y = window.y; y < window_height; y++) {
    for (size_t x = window.x; x < window_width; x++) {
      hui_put_character_at('x', y, x);           
    }
  }
  dx++;
  dy++;
}

int main() {
	Window window = hui_init();
  char buffer = ' ';
  while (buffer != '\n') {
    hui_draw_window(window);
    read(1, &buffer, 1);
	}
	return 0;
}

