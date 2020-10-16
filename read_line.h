/*
A better smaller alternative to libreadline.

https://github.com/afl5c/read_line

License: Public Domain
*/

#ifndef _WIN32
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h> //for isatty()
#include <termios.h> //for tcgetattr()/tcsetattr()
#endif

#include <vector>
#include <string>
using namespace std;

//read line the lazy way. on windows, history is automatically supported so this is all we need
string read_line_lazy(){
	string buf;
	while(!feof(stdin)){
		char c = fgetc(stdin);
		if(c=='\r') continue; //skip \r
		if(c==EOF || c=='\n') break;
		buf += c;
	}
	return buf;
}

//read a line from stdin, while supporting history on posix
string read_line(const string& prompt="> "){
	//init history
	static vector<string> history;
	if(history.empty()){
		history.push_back("");
	}
	
	//show prompt
	printf("%s",prompt.c_str());
	fflush(stdout);

	//special case for windows
#ifdef _WIN32
	return read_line_lazy();
#else
	
	//if not a tty (like a file pipe)
	if(!isatty(STDIN_FILENO)){
		return read_line_lazy();
	}
	
	//get original mode
	termios org;
	tcgetattr(STDIN_FILENO,&org);
	
	//set raw mode
	//https://man7.org/linux/man-pages/man3/termios.3.html
	termios raw;
	memset(&raw,0,sizeof(termios));
	raw.c_cflag = CS8; //use 8-bit characters
	raw.c_cc[VMIN] = 1; //min characters to trigger read
	tcsetattr(STDIN_FILENO,TCSANOW,&raw);
	
	//process line
	string line;
	int pos = 0; //where to write or delete next character
	int off = 0; //offset in history
	while(1){
		char c;
		read(STDIN_FILENO,&c,1);
		// fprintf(stderr,"%d (%c)\r\n",c,c); //for debug
		if(c==1) pos = 0; //ctrl-a (go to line start)
		else if(c==3) raise(SIGINT); //ctrl-c
		else if(c==5) pos = line.size(); //ctrl-e (go to line end)
		else if(c==13) break; //enter
		else if(c==27){ //escape
			//differentiate between escape sequence and escape to clear
			read(STDIN_FILENO,&c,1);
			if(c==27){ //double escape
				line = "";
			}
			else if(c=='['){ //escape sequence
				read(STDIN_FILENO,&c,1);
				if(c=='A'){ //up
					off = (off-1+history.size())%history.size();
					line = history[off];
					pos = line.size();
				}
				else if(c=='B'){ //down
					off = (off+1)%history.size();
					line = history[off];
					pos = line.size();
				}
				else if(c=='C') pos++; //right
				else if(c=='D') pos--; //left
			}
		}
		else if(c==127){ //backspace
			pos--;
			if(pos>=0) line.erase(pos,1);
		}
		else{ //any other character
			line.insert(pos,1,c);
			pos++;
			// fprintf(stderr,"line = [%s], pos = %d\n",line.c_str(),pos);
		}
		
		//clamp position
		if(pos<0) pos = 0;
		if(pos>line.size()) pos = line.size();
		
		//display line
		printf("\033[2K"); //clear line
		printf("\r%s%s",prompt.c_str(),line.c_str()); //write line
		printf("\r\033[%dC",(int)(prompt.size()+pos)); //move cursor
		fflush(stdout);
	}
	
	//put back original mode
	tcsetattr(STDIN_FILENO,TCSANOW,&org);
	printf("\n");
	fflush(stdout);
	
	//save history
	if(!line.empty()){
		history.push_back(line);
	}
	return line;
#endif
}
