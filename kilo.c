#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>

//defines
#define CTRL_KEY(k) ((k) & 0x1f)

/*data*/
struct editorConfig{
//original raw termios
struct termios orig_termios;

};

struct editorConfig E;



//function to show error when something goes wrong
void die(const char*s){
//clear screen
write(STDOUT_FILENO,"\x1b[2J",4);
//cursor positon to 1,1 row,col
write(STDOUT_FILENO,"\x1b[H",3);
	perror(s);
	exit(1);
}

void disableRawMode(){
  if (tcsetattr(STDIN_FILENO,TCSAFLUSH,&E.orig_termios)==-1)
	die("tcsetattr");
}

void enableRawMode(){


 if (tcgetattr(STDIN_FILENO,&E.orig_termios)==-1) die("tcgetattr");
atexit(disableRawMode);
 struct termios raw = E.orig_termios;
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


//inputs
char editorReadKey(){
char c;
int nread;
  while((nread=read(STDIN_FILENO,&c,1))!=1){
  printf("%d (%c)\r\n",c,c);
  if(nread==-1 && errno!=EAGAIN) die("read");
  }

return c;
}

//outputs
void editorDrawRows(){
 int y;
	for(y=0;y<24;y++){
	write(STDOUT_FILENO,"~\r\n",3);
	}
}


void editorRefreshScreen(){

write(STDOUT_FILENO,"\x1b[2J",4);
//cursor positon
write(STDOUT_FILENO,"\x1b[H",3);
editorDrawRows();
write(STDOUT_FILENO,"\x1b[H",3);
}


void editorProcessKeypress(){
	char c = editorReadKey();
	switch(c){
	case CTRL_KEY('q'):
	//clear screen
	write(STDOUT_FILENO,"\x1b[2J",4);
	//cursor positon to 1,1 row,col
	write(STDOUT_FILENO,"\x1b[H",3);

	exit(0);
	break;
	}
}


int main(){
  enableRawMode();

  while(1){
 	editorRefreshScreen();
	editorProcessKeypress();
  }
  return 0;
}

