#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios original;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &original);
  atexit(disableRawMode); 

  struct termios raw = original;
  raw.c_lflag &= ~(ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main()
{
  enableRawMode();
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
  return 0;
}