#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>

#include "sox.h"
#include "mystring.h"

#define BUFLEN 500
#define HOSTLEN 80
#define MAX_USER 50

using namespace std;

struct Interface {
   string name;
   int sock;
   string hostname;
   int portNo;
};

int  findInterfaceByName(Interface *users, int max, string name);
void prompt();
int  getNextEvent(int sock, Interface *users, int userSize, fd_set &read);
int  addConnection(int sock, Interface *users, int &max, sockaddr_in server);
bool processCommand(bool &shutdown);
void processMessage(Interface *users, int cur, int &max);
void processIPRequest(string name, Interface *users, int cur, int max);
void processPostCard(string contents, Interface *users, int cur, int max);
void processListRequest(Interface *users, int cur, int max);
void processQuitRequest(Interface *users, int cur, int &max);
void showRunning(sockaddr_in server);
void tearDown(int sock, Interface *users, int userSize);


int main()
{
	int sock;
	Interface users[MAX_USER];
	int userSize = 0;
	struct sockaddr_in server;
	fd_set read;
	bool shutdown = false;

	if (!(sock = getListeningSocket(server, SOCK_STREAM))) {
		return 1;
	}
	showRunning(server);
	prompt();

	while (!shutdown) {
		if (!getNextEvent(sock, users, userSize, read)) {
			continue;
		}
		if (FD_ISSET(0, &read) && processCommand(shutdown)) {
			prompt();
		}

		for(int i = 0; i < userSize; i++) {
			if (FD_ISSET(users[i].sock, &read)) {
				processMessage(users, i, userSize);
			}
		}

		if (FD_ISSET(sock, &read)) {
			addConnection(sock, users, userSize, server);
		}
	}

	tearDown(sock, users, userSize);

	cout << "Good bye\n";
	return 0;
}


void prompt() {
	cout << "server> ";
	cout.flush();
}

bool processCommand(bool &shutdown) {
    string cmd;

    getline(cin, cmd);
    if (cmd == "shutdown") {
      	shutdown = true;
    } else {
      	cout << "Unknown command\n";
    }
    return !shutdown;
}

void processMessage(Interface *users, int cur, int &max) {
	queue<string> msgs = socketRead(users[cur].sock);
	string msg, type, contents;

	if (msgs.size() == 0) {
		processQuitRequest(users, cur, max);
	} else {
		while (msgs.size()) {
			msg = msgs.front();
			msgs.pop();
    			parse(msg, type, contents);
			if (type == "PostCard") {
				processPostCard(contents, users, cur, max);
			}else if (type == "IPRequest") {
				processIPRequest(contents, users, cur, max);
			}else if (type == "List") {
				processListRequest(users, cur, max);
			}else if (type == "Quit") {
				processQuitRequest(users, cur, max);	
			}
		}
	}
}

void processIPRequest(string name, Interface *users, int cur, int max) {
	string addr;
	ostringstream oss;
	int j;

	if ((j = findInterfaceByName(users, max, name)) >= 0) {
		oss << users[j].portNo;
		addr = users[j].hostname + " " + oss.str();
		addr = "IPRequest " + addr;
		socketSend(users[cur].sock, addr);
		//write(users[cur].sock, addr.c_str(), addr.length());
	}else {
		socketSend(users[cur].sock, "IPRequest No such user");
	}
}

void processListRequest(Interface *users, int cur, int max) {
	string names;
	Interface *p = users;

	for (int i = 0; i < max; i++, p++) {
		if (names > "") {
			names += " ";
		}
		names += p->name;
	}
	names = "List " + names;
	socketSend(users[cur].sock, names);
	//write(users[cur].sock, names.c_str(), names.length());
}	

void processPostCard(string contents, Interface *users, int cur, int max) {
	istringstream iss(contents);
	string msg, line, recip;
	int j;

	getline(iss, line);
	getline(iss, msg);

	istringstream recipients(line);

	msg = "PostCard " + users[cur].name + ":\t" + msg;

	while (recipients.rdbuf()->in_avail()) {
		recipients >> recip;
		if ((j = findInterfaceByName(users, max, recip)) >=0) {
			socketSend(users[j].sock, msg);
			//write(users[j].sock, msg.c_str(), msg.length());
		}
	} 
}



void processQuitRequest(Interface *users, int cur, int &max) {
	close(users[cur].sock);
	max--;
	users[cur].name = users[max].name;
	users[cur].sock = users[max].sock;
}

int findInterfaceByName(Interface *users, int max, string name) {
   for(int j = 0; j < max; j++) {
       if (users[j].name == name) {
		return j;
	}
    }
    return -1;
}
	
void rejectConnection(int sock, string err) {
	cout << "Rejected connection: " << err << endl;
	socketSend(sock, err);
	//write(sock, err.c_str(), err.length());
	close(sock);
}

int initInterface(Interface *p, int sock, string msg, sockaddr_in server) {
	istringstream iss(msg);

	p->sock = sock;
	if (!iss.rdbuf()->in_avail()) {
		return 0;
	}
	iss >> p->name;
	if (!iss.rdbuf()->in_avail()) {
		return 0;
	}

	iss >> p->hostname;
	iss >> p->portNo;

	return 1;
}


int addConnection(int sock, Interface *users, int &max, sockaddr_in server) {
	int newSock;
	queue<string> v;
	string msg;
	int length = sizeof(server);

	if (0 > (newSock = accept(sock, (sockaddr *)&server, (socklen_t *)&length))) {
		cerr << "Unable to accept connection" << endl;
		return 0;
	}else if (max >= MAX_USER) {
		rejectConnection(newSock, "ERROR: Too many users.  Come back tomorrow.");
		return 0;
	}
	v = socketRead(newSock);
	msg = v.front();
	
	if (!initInterface(&users[max], newSock, msg, server)) {
		rejectConnection(newSock, "ERROR: Malformed registration request");
		return 0;
	}

        if (findInterfaceByName(users, max, users[max].name) >= 0) { 
		rejectConnection(newSock, "ERROR: Username taken.  Be more creative!");
		return 0;
        }
	socketSend(users[max++].sock, "OK"); 
	//write(users[max++].sock, "OK", 2);
	return 1;
}


void showRunning(sockaddr_in server) {
	char host[HOSTLEN+1];

	gethostname(host, HOSTLEN);
	cout << "host name: " << host << endl;
	cout << "port number: " << ntohs(server.sin_port) << endl;
}

int getNextEvent(int sock, Interface *users, int userSize, fd_set &read) {
	struct timeval wait;

	FD_ZERO(&read);
	FD_SET(0, &read);
	FD_SET(sock, &read);

	for(int i = 0; i < userSize; i++)
		FD_SET(users[i].sock, &read);

	wait.tv_sec = 1;
	wait.tv_usec = 0;

	return (select(FD_SETSIZE, &read, (fd_set *)0, (fd_set *)0, &wait) > 0);
}

void tearDown(int sock, Interface *users, int userSize) {
	close(sock);
	for(int i = 0; i < userSize; i++)
 		close(users[i].sock);
}

