//please note.  This is just an algorithm.  It will not compile.

#include "MyQueue.h"

queue initialize();
struct program_info ReadProgramInfo(FileObjecT);

typedef enum commands {
	QUIT,
	RANDOM,
	...
};


struct program_info {
	float expected_running_time,
	int user_id,
	string command_line;
	string resource_list;
};

int main() {
	queue q = initialize();
	commands cmd;	
	while(GetCommand(cmd)) {
		switch (cmd) {
			case SUBMIT:
				struct program_info s = ReadProgramInfo(stdin);
				q.insert(s);
			case :
			case :
			default:
		}
	return 0;
}


queue initialize() {
	queue q;
	FileObject <- open the file batch.txt
	n <- read in the number of programs to schedule
	for (i = 0; i < n; i++) {
		struct program_info s = ReadProgramInfo(FileObject);
		q.insert(s);
	}
	return q; 
}

bool GetCommand(commands &cmd) {
	// prompt the user for next command
	//get input from the user.
	// if not a valid command, try again.
	// if command is "quit" return false;
	// else: set cmd = user's command and return true;
}

struct program_info ReadProgramInfo(FileObjecT) {
	struct program_info s;

	s.expected_running_time <-  read another line from the file;
	s.user_id = <- read
	s.command_line <- read
	s.resource_list <- read;
	return s;
}
