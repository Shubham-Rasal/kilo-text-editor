#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

//defines

#define CTRL_KEY(k) ((k) & 0x1f)

struct termios orig_termios;

void die(const char*s){
	perror(s);
	exit(1);
}

void disableRawMode(){
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&orig_termios)==-1)
	die("tcsetattr");
}

void enableRawMode(){

//original raw termios;

 if (tcgetattr(STDIN_FILENO,&orig_termios)==-1) die("tcgetattr");
atexit(disableRawMode);
 struct termios raw = orig_termios;
 tcgetattr(STDIN_FILENO, &raw);
//input flags to turn off to prevent weird behaviour by terminal
 raw.c_iflag &= ~(IXON|ICRNL|BRKINT|INPCK|ISTRIP);
//opost to disable the conversion of \n to \r\n
 raw.c_oflag &= ~(OPOST);
 raw.c_cflag |= (CS8);
//disable various ctrl+key options
 raw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
 //timer for read input
 raw.c_cc[VMIN] =0;
 raw.c_cc[VTIME] =1;
 tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);
}



char editorReadKey(){
char c;
int nread;
  while((nread=read(STDIN_FILENO,&c,1))!=1){
  printf("%d (%c)\r\n",c,c);
  if(nread==-1 && errno!=EAGAIN) die("read");
  }

return c;
}
//inputs
void editorProcessKeypress(){
	char c = editorReadKey();
	switch(c){
	case CTRL_KEY('q'):
	exit(0);
	break;
	}
}


int main(){
  enableRawMode();

  while(1){
	editorProcessKeypress();
}
  return 0;
}

