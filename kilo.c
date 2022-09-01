#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

// defines
#define CTRL_KEY(k) ((k)&0x1f)

/*data*/
struct editorConfig
{
	// original raw termios
	struct termios orig_termios;
	int screenrows;
	int screencols;
};

struct editorConfig E;

// function to show error when something goes wrong
void die(const char *s)
{
	// clear screen
	write(STDOUT_FILENO, "\x1b[2J", 4);
	// cursor positon to 1,1 row,col
	write(STDOUT_FILENO, "\x1b[H", 3);
	perror(s);
	exit(1);
}

void disableRawMode()
{
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1)
		die("tcsetattr");
}

void enableRawMode()
{

	if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
		die("tcgetattr");
	atexit(disableRawMode);
	struct termios raw = E.orig_termios;
	tcgetattr(STDIN_FILENO, &raw);
	// input flags to turn off to prevent weird behaviour by terminal
	raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
	// opost to disable the conversion of \n to \r\n
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	// disable various ctrl+key options
	raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
	// timer for read input
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// inputs
char editorReadKey()
{
	char c;
	int nread;
	while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
	{
		printf("%d (%c)\r\n", c, c);
		if (nread == -1 && errno != EAGAIN)
			die("read");
	}

	return c;
}

// terminal

int getCursorPosition(int *rows, int *cols) {
  char buf[32];
  unsigned int i = 0;
  if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
    if (buf[i] == 'R') break;
    i++;
  }
  buf[i] = '\0';
//   printf("\r\n&buf[1]: '%s'\r\n", &buf[1]);
//   editorReadKey();

  if (buf[0] != '\x1b' || buf[1] != '[') return -1;
  if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
  return 0;
  
}


int getWindowSize(int *rows, int *cols)
{
	struct winsize ws;

	if (1||ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
	{
		if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
			return -1;
		return getCursorPosition(rows, cols);
		
	}
	else
	{
		*cols = ws.ws_col;
		*rows = ws.ws_row;
		return 0;
	}
}

// outputs
void editorDrawRows()
{
	int y;
	for (y = 0; y < E.screenrows; y++)
	{
		write(STDOUT_FILENO, "~\r\n", 3);
	}
}

void editorRefreshScreen()
{

	write(STDOUT_FILENO, "\x1b[2J", 4);
	// cursor positon
	write(STDOUT_FILENO, "\x1b[H", 3);
	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}

void editorProcessKeypress()
{
	char c = editorReadKey();
	switch (c)
	{
	case CTRL_KEY('t'):
		// clear screen
		write(STDOUT_FILENO, "\x1b[2J", 4);
		// cursor positon to 1,1 row,col
		write(STDOUT_FILENO, "\x1b[H", 3);

		exit(0);
		break;
	}
}

void initEditor()
{
	if (getWindowSize(&E.screenrows, &E.screencols) == -1)
		die("getWindowSize");
	
	
}

int main()
{
	enableRawMode();
	initEditor();

	while (1)
	{
		editorRefreshScreen();
		editorProcessKeypress();
	}
	return 0;
}
