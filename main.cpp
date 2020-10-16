
#include "read_line.h"

int main(){
	while(1){
		string line = read_line();
		printf("line = [%s]\n",line.c_str());
		if(line=="exit") break;
	}
}

