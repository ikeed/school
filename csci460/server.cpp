#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>

#include <stdio.h>

#define BUFLEN 500
#define HOSTLEN 80
#define MAX_USER 50

using namespace std;

struct Interface {
   string name;
   int sock;
};

int  getConnected(sockaddr_in &server);
void prompt();
int  getNextEvent(int sock, Interface *users, int userSize, fd_set &read);
int  acceptConnection(int sock, Interface *users, int &max, sockaddr_in server);
bool processCommand(bool &shutdown);
void processMessage(Interface *users, int cur, int &max);
void parse(char buf[], string & name, string & msg);
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

	if (!(sock = getConnected(server))) {
		return 1;
	}

	prompt();

	while (!shutdown) {
		if (getNextEvent(sock, users, userSize, read)) {
			if (FD_ISSET(0, &read) && processCommand(shutdown)) {
				prompt();
			}

			for(int i = 0; i < userSize; i++) {
				if (FD_ISSET(users[i].sock, &read)) {
					processMessage(users, i, userSize);
				}
			}

			if (FD_ISSET(sock, &read)) {
				acceptConnection(sock, users, userSize, server);
			}
		}
	}

	tearDown(sock, users, userSize);

	cout << "Good bye\n";
	return 0;
}

int getConnected(sockaddr_in &server) {
   int sock = socket(AF_INET, SOCK_STREAM, 0);
   int length;
 
   if (sock < 0) {
      cerr << "Can't open stream socket\n";
      return 0;
   }

   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = 0;

   if (bind(sock, (const sockaddr *)&server, sizeof(server)) < 0) {
      cerr << "Can't binding stream socket\n";
      return 0;
   }

   length = sizeof(server);
   if (getsockname(sock, (sockaddr *)&server, (socklen_t *)&length) < 0) {
      cerr << "Can't get socket name\n";
      return 0;
   }
   listen(sock, MAX_USER);
   showRunning(server);

   return sock;

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
	char buf[BUFLEN+1];
	buf[0] = '\0';
	string msg, name;

	recv(users[cur].sock, buf, BUFLEN, 0);

	if (buf[0] == '\0') {
		close(users[cur].sock);
		max--;
		users[cur].name = users[max].name;
		users[cur].sock = users[max].sock;
	} else {
		parse(buf, name, msg);
		msg = users[cur].name + msg;
		for(int j = 0; j < max; j++) {
			if (users[j].name == name) {
				write(users[j].sock, msg.c_str(), msg.length());
		     	}
		}
	}
}

int findInterfaceByName(Interface *users, int max, string name) {
    for(int j = 0; j < max; j++) {
       if (users[j].name == name) {
		return j;
	}
    }
    return -1;
}
	

int acceptConnection(int sock, Interface *users, int &max, sockaddr_in server) {
	char buf[BUFLEN] = "";
	int length = sizeof(server);

        users[max].sock = accept(sock, (sockaddr *)&server, (socklen_t *)&length);
        recv(users[max].sock, buf, BUFLEN, 0);
        users[max].name = buf;
        if (findInterfaceByName(users, max, buf) >= 0) {       
		close(users[max].sock);
		return 0;
        } else {
		write(users[max].sock, "OK", 2);
               	max++;
	}
	return 1;
}

void parse(char buf[], string & name, string & msg)
{
	string line = buf;
	if (line == "") {
		name = "";
		msg = "";
	} else {
		istringstream iss(line);
		iss >> name;
		getline(iss, msg);
	}
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

