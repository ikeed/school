#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <stdlib.h>

#include "sox.h"

using namespace std;

#define BUFLEN 500



int sendMessage(int sock, string msg);

string promptLine(string prompt) {
	string s;

	cout << prompt << "  ";
	getline(cin, s);
	return s;	
}


string promptWSList(string prompt) {
	string s, tmp;
	istringstream iss(promptLine(prompt));

	while (iss.rdbuf()->in_avail()) {
		iss >> tmp;
		if (!tmp.length()) {
			continue;
		}
		if (s.length()) {
			s += " ";
		}
		s += tmp;
	}
	return s;
}		

void showHelp() {
	cout << "\n\thelp: to display the help information of the system.\n";
	cout << "\n\tlist: to list the names (only) of all the client programs currently\n\t\t"
		<< " registered with the server.\n";
	cout << "\n\tpostcard: to send a public message to one or multiple clients. This"
		<< "\n\t\tcommand will ask user to enter all the receivers' names in a "
		<< "\n\t\tline separated by spaces and then a one-line message that could "
		<< "\n\t\thave any ASCII characters in it.  The postcard message would be "
		<< "\n\t\tsent to the server and let the server pass the message to all the "
		<< "\n\t\treceivers. \n";
	cout << "\n\tprivatemessage: to send a private message to one client program. This "
		<< "\n\t\tcommand will ask for one receiver's name and a one-line message "
		<< "\n\t\tthat could have any ASCII characters in it. Then instead of letting"
		<< "\n\t\tthe server to relay the message, the sender contacts the server to "
		<< "\n\t\tget the registered address and port number of the receiver and then "
		<< "\n\t\testablish a direct connection with the receiver to send the message.\n";
	cout << "\n\tquit: to de-register from the server and terminate this client program only"
		<< "\n\t\t. Note that the server should remain running.\n" << endl;
}

bool parse(string line, string & type, string & content)
{
   string tmp;
   if (line == "") {
	type = "";
	content = "";
	return false;
   } else {
	istringstream iss(line);
	iss >> type;
	content = iss.str();
   }
   return true;
}

int sendPostCard(int sock, string recips, string msg) {
	return socketSend(sock, "PostCard " + recips + "\n" + msg);
}

void processCommand(bool &quit, int serverSock) {
    string line, comm, msg;
    string recips;

    getline(cin, line);
    parse(line, comm, msg);
    if (comm == "quit") {
       quit = true;
    }else if (comm == "help") {
	showHelp();
    }else if (comm == "send") {
       sendMessage(serverSock, msg);
    }else if (comm == "list") {
	sendMessage(serverSock, comm);
    }else if (comm == "postcard") {
	recips = promptWSList("Please Enter Receivers' names separated by whitespace");
	if (recips > "") {
		line = promptLine("Please Enter the message (one line only)");
		if (line > "") {
			sendPostCard(serverSock, recips, line);
		}
	}
    }else if (comm == "privatemessage") {
	// TODO
    } else {
       cout << "Unknown command\n";
    }
}

void processPostCard(string msg) {
	cout << msg << endl;  // just leave it how the server formatted it.
}

void processIPResponse(string msg) {
	
}

void processListResponse(string msg) {
	string usr;
	cout << "\nUsers online right now:\n";
	while (parse(msg, usr, msg)) {
		cout << usr << endl;
	}
}


void FD_Reset(struct timeval &wait, fd_set &readfield, int serverSock, int peerSock) {
      wait.tv_sec = 1;
      wait.tv_usec = 0;

      FD_ZERO(&readfield);
      FD_SET(serverSock, &readfield);
      FD_SET(peerSock, &readfield);
      FD_SET(0, &readfield);
}

int getNextEvent(fd_set &readfield, struct timeval &wait) {
      return (select(FD_SETSIZE, &readfield, (fd_set *)0, (fd_set *)0, &wait) > 0);
}




int connectToServer(const char *host, int port)
{
   int sock;
   struct sockaddr_in server;
   struct hostent *hp;

   hp = gethostbyname(host);
   bzero((char *)&server, sizeof(server));
   bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
   server.sin_family = hp->h_addrtype;
   server.sin_port = htons(port);

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      cerr << "Opening stream socket error\n";
      return -1;
   }

   if (connect(sock, (const struct sockaddr *)&server, sizeof(server)) < 0) {
      return -1;
   } else {
      return sock;
   }
}

int parseArgs(int argc, char **argv, string &server, int &port, string &name) {
	if (argc != 4) {
		return 0;
	}
	server = argv[1];
	port = atoi(argv[2]);
	name = argv[3];
	return  (server.length() >= 1 && name.length() >= 1 && port >= 1);
}

int processRegResponse(string msg) {
	istringstream iss(msg);
	string token;
	iss >> token;
	if (token == "OK") {
		cout << "Registered successfully" << endl;
		return 1;
	}else if (token == "ERROR") {
		getline(iss, token);
		cerr << "Error registering with server: " << token << endl;
		return 0;
	}
	cerr << "Server response from registration attempt: " << msg << endl;
	return 0;
}

void showUsage(string me) {
	cerr << "Usage: " << me << " host port name" << endl;
}	

string int2String(int a) {
	ostringstream oss;
	oss << a;
	return oss.str();
}

int init(int argc, char **argv, string &name, int &serverSock, int &peerSock) {
   int port;
   sockaddr_in sa;
   string msg, server;

   if (!parseArgs(argc, argv, server, port, name)) {
	showUsage(argv[0]);
	return 0;
   }

   if ((serverSock = connectToServer(server.c_str(), port)) < 0) {
      cerr << "Can't connect to server\n";
      return 0;
   }

   if ((peerSock = getListeningSocket(sa)) < 0) {
	cerr << "Can't open a listening socket for peer-to-peer messaging" << endl;
	close(serverSock);
	return 0;
   }

   msg = name + " " + int2String(peerSock);
   socketSend(serverSock, msg);
   if (!processRegResponse(socketRead(serverSock))) {
	close(serverSock);
	return 0;
   }
   return 1;
}

void prompt(string name) {
   cout << name << "> ";
   cout.flush();
}

void tearDown(int serverSock, int peerSock) {
	socketSend(serverSock, "Quit");
	// I don't really care about the server's response.
	// I'm going away no matter what.
	close(serverSock);
	close(peerSock);
	cout << "Good Bye" << endl;
}


void processServerMessage(bool &quit, int serverSock) {
    string msg, type, name;
    
    msg = socketRead(serverSock);

    if (msg.length() == 0) {
       cout << "\nShutting down by the server\n";
       quit = true;
    }else if (parse(msg, type, msg)) {
	if (type == "PostCard") {
		processPostCard(msg);
	}else if (type == "IPRequest") {
		processIPResponse(msg);
	}else if (type == "List") {
		processListResponse(msg);
	}
    } else {
       cerr << "\nUnknown message format: " << msg << endl;
    }
}

void processPeerMessage(int peerSock) {
	string type, name;
	string msg = socketRead(peerSock);
}	

int main(int argc, char *argv[])
{
   fd_set readfield;
   struct timeval wait;
   string server, name, msg;
   int serverSock, peerSock;
   bool quit = false;

   if (!init(argc, argv, name, serverSock, peerSock)) {
	return 1;
   }
   prompt(name);
  
   while (!quit) {
      FD_Reset(wait, readfield, serverSock, peerSock);
      if (getNextEvent(readfield, wait)) {
         if (FD_ISSET(0, &readfield)) {  // Input from stdin
	    processCommand(quit, serverSock);
            prompt(name); 
	}

        if (FD_ISSET(serverSock, &readfield)) {  // Input from server socket
		processServerMessage(quit, serverSock);
		prompt(name);
        }else if (FD_ISSET(peerSock, &readfield)) { // Input from peer socket
		processPeerMessage(peerSock);
		prompt(name);
	}
      }
   }

   tearDown(serverSock, peerSock);
   return 0;
}

int sendMessage(int sock, string msg) {
	msg = "MSG:" + msg;
	return (write(sock, msg.c_str(), msg.length()+1) >= 0);
}

