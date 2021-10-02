#include <ctype.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <termios.h> 
#include <unistd.h> 
#include <errno.h> 
#include <sys/ioctl.h>



#define CTRL_KEY(k) ((k) & 0x1f)

struct editorConfig {
  int screenrows;
  int screencolumns;
  struct termios original;
};

struct editorConfig E;

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.original) == -1) {
    die("tcsetattr");
  }
}

void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &E.original) == -1) {
    die("tcgetattr");
  }
  atexit(disableRawMode); 

  struct termios raw = E.original;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
    die("tcsetattr");
  }
}

char editorReadKey() {
  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      die("read");
    }
    return c;
  }
}

int getCursorPosition(int *rows, int *columns) {
  char buffer[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4)!=4) {
    return -1;
  }
  while (i < sizeof(buffer)-1) {
    if (read(STDIN_FILENO, &buffer[i], 1)!=1) {
      break;
    }
    if (buffer[i]=='R') {
      break;
    }
    i++;
  }
  buffer[i] = '\0';
  if (buffer[0] != '\x1b' || buffer[1] != '[') {
    return -1;
  }
  if (sscanf(&buffer[2], "%d;%d", rows, columns) != 2) {
    return -1;
  }
  return 0;
}

int getWindowSize(int *rows, int *columns) {
  struct winsize ws; 
  if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws)==-1 || ws.ws_col==0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12)!=12) {
      return -1;
    }
    return getCursorPosition(rows, columns);
  }
  else {
    *columns = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}

void editorProcessKeypress() {
  char c = editorReadKey();
  switch (c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
    break;
  }
}

void initialEditor() {
  if (getWindowSize(&E.screenrows, &E.screencolumns)==-1) {
    die("getWindowSize");
  }
}

int main()
{
  enableRawMode();
  initialEditor();
  char c;
  while (1) {
    editorProcessKeypress();
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
      die("read");
    }
    if (iscntrl(c)) 
    {
      printf("%d\r\n", c); 
    }
    else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == CTRL_KEY('q'))
    {
      break;
    }
  }
  return 0;
}