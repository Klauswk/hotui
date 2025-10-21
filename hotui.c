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
static uint16_t width;
static uint16_t height;

void hui_print(char* string, size_t size) {
 write(output_fd, string, size); 
}

static void hui_resize(int i) {
	struct winsize ws;
	ioctl(1, TIOCGWINSZ, &ws);
	width = ws.ws_col;
	height = ws.ws_row;
}

static void hui_restore() {
	tcsetattr(1, TCSANOW, &initial);
}

static void hui_die(int i) {
	exit(1);
}

void hui_init() {
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
}

int main() {
	hui_init();
  char buffer = ' ';
  while (buffer != '\n') {
		read(1, &buffer, 1);
    char* string = "\r ";
		hui_print(string, strlen(string));
		write(1, &buffer, 1);
	}
	return 0;
}

