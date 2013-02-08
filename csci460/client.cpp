#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <stdlib.h>
#include <queue>
#include "sox.h"
#include <arpa/inet.h>
#include "mystring.h"

using namespace std;

#define BUFLEN 500


string promptLine(string prompt);
string promptToken(string prompt);
string promptWSList(string prompt);
void showHelp();
int sendPostCard(int sock, string recips, string msg);
void processCommand(bool &quit, int serverSock, queue<string> &pendingPrivateMessages);
void processIPResponse(string myname, string msg, queue<string> &pendingPrivateMessages);
void processListResponse(string msg);
void FD_Reset(struct timeval &wait, fd_set &readfield, int serverSock, int peerSock);
int getNextEvent(fd_set &readfield, struct timeval &wait);
int connectToServer(const char *host, int port, int type);
int parseArgs(int argc, char **argv, string &server, int &port, string &name);
int processRegResponse(string msg);
void showUsage(string me);
string int2String(int a);
int init(int argc, char **argv, string &name, int &serverSock, int &peerSock);
void showPrompt(string name);
void tearDown(int serverSock, int peerSock);
void processServerMessage(bool &quit, string name, int serverSock, queue<string> &pendingPrivateMessages);
void processPeerMessage(int peerSock);


int main(int argc, char *argv[])
{
   fd_set readfield;
   struct timeval wait;
   string server, name, msg;
   int serverSock, peerSock;
   bool quit = false;
   queue<string> pendingPrivateMessages;

   if (!init(argc, argv, name, serverSock, peerSock)) {
	return 1;
   }
   showPrompt(name);
  
   while (!quit) {
      FD_Reset(wait, readfield, serverSock, peerSock);
      if (getNextEvent(readfield, wait)) {
         if (FD_ISSET(0, &readfield)) {  // Input from stdin
	    processCommand(quit, serverSock, pendingPrivateMessages);
	 }

         if (FD_ISSET(serverSock, &readfield)) {  // Input from server socket
		processServerMessage(quit, name, serverSock, pendingPrivateMessages);
         }else if (FD_ISSET(peerSock, &readfield)) { // Input from peer socket
		processPeerMessage(peerSock);
	 }
         showPrompt(name); 
      }
   }

   tearDown(serverSock, peerSock);
   return 0;
}


string promptLine(string prompt) {
	string s;

	cout << prompt << "  ";
	getline(cin, s);
	return s;	
}

string promptToken(string prompt) {
	string s = promptLine(prompt);
	parse(s, s, prompt);
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


int sendPostCard(int sock, string recips, string msg) {
	return socketSend(sock, "PostCard " + recips + "\n" + msg);
}

void processCommand(bool &quit, int serverSock, queue<string> &pendingPrivateMessages) {
    string line, comm, msg;
    string recips;

    getline(cin, line);
    parse(line, comm, msg);
    if (comm == "quit") {
       quit = true;
    }else if (comm == "help") {
	showHelp();
    }else if (comm == "list") {
	socketSend(serverSock, "List");
    }else if (comm == "postcard") {
	recips = promptWSList("Please Enter Receivers' names separated by whitespace");
	if (recips > "") {
		line = promptLine("Please Enter the message (one line only)");
		if (line > "") {
			sendPostCard(serverSock, recips, line);
		}
	}
    }else if (comm == "privatemessage") {
	if ((recips = promptToken("Please enter the recipient's name:")).length() == 0) {
		return;
	}else if ((comm = promptLine("Please enter the message:")).length() == 0) {
		return;
	}
	pendingPrivateMessages.push(comm);
	line = "IPRequest " + recips;
	socketSend(serverSock, line);
    } else {
       cout << "Unknown command\n";
    }
}


void processPostCard(string msg) {
	cout << msg << endl;  // just leave it how the server formatted it.
}

void processIPResponse(string myname, string msg, queue<string> &pendingPrivateMessages) {
	string host, port, ppm;
	int sock, iport;

	if (!pendingPrivateMessages.size()) {
		cerr << "bailing out of IPResponse.  no pending messages" << endl;
		return;
	}
	if (!parse(msg, host, port)) {
		cerr << "Failed to send private message" << endl;
		pendingPrivateMessages.pop();	
		return;
	}
	iport = atoi(port.c_str());
	if ((sock = connectToServer(host.c_str(), iport, SOCK_DGRAM)) < 0) {
		cerr << "Couldn't connect to " << host << ":" << port << " for private message." << endl;
		pendingPrivateMessages.pop();
		return;
	}
	ppm = pendingPrivateMessages.front();
	ppm = "PeerMessage " + myname + " " + ppm;
	pendingPrivateMessages.pop(); 
	socketSend(sock, ppm);
}

void processListResponse(string msg) {
	string usr;
	cout << "\nUsers online right now:\n";
	istringstream iss(msg);
	while (iss.rdbuf()->in_avail()) {
		iss >> usr;
		cout << "\t" << usr << endl;
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


int connectToServer(const char *host, int port, int type)
{
   int sock;
   struct sockaddr_in server;
   struct hostent *hp;

   hp = gethostbyname(host);
   bzero((char *)&server, sizeof(server));
   bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
   server.sin_family = hp->h_addrtype;
   server.sin_port = htons(port);

   sock = socket(AF_INET, type, 0);
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
   queue<string> msgs;
   char buf[BUFLEN+1];

   if (!parseArgs(argc, argv, server, port, name)) {
	showUsage(argv[0]);
	return 0;
   }

   if ((peerSock = getListeningSocket(sa, SOCK_DGRAM)) < 0) {
	cerr << "Can't open a listening socket for peer-to-peer messaging" << endl;
	close(serverSock);
	return 0;
   }
   
   if ((serverSock = connectToServer(server.c_str(), port, SOCK_STREAM)) < 0) {
      cerr << "Can't connect to server\n";
      return 0;
   }

   gethostname(buf, BUFLEN);
   msg = name + " " + buf + " " + int2String(ntohs(sa.sin_port));
   socketSend(serverSock, msg);
   msgs = socketRead(serverSock);
   if (msgs.size() == 0) {
	return 0;
   }
   msg = msgs.front();
   if (!processRegResponse(msg)) {
	close(serverSock);
	return 0;
   }
   return 1;
}

void showPrompt(string name) {
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


void processServerMessage(bool &quit, string name, int serverSock, queue<string> &pendingMessages) {
    string msg, type, content;
    queue<string> msgs = socketRead(serverSock);

    if (msgs.size() == 0) {
       cout << "\nShutting down by the server\n";
       quit = true;
    }else {
	while(msgs.size()) {
		msg = msgs.front();
		msgs.pop();
		
		if (parse(msg, type, content)) {
			if (type == "PostCard") {
				processPostCard(content);
			}else if (type == "IPRequest") {
				processIPResponse(name, content, pendingMessages);
			}else if (type == "List") {
				processListResponse(content);
			}else {
			        cerr << "\nUnknown message format: " << msg << endl;
			}
		}
	}
    }
}

void processPeerMessage(int peerSock) {
	string type, name, msg, content;
	queue<string> msgs = socketRead(peerSock);

	if (msgs.size() == 0) {
		cerr << "Could not read from peer socket!" << endl;
		return;
	}
	while (msgs.size()) {
		msg = msgs.front();
		msgs.pop();
		if (parse(msg, type, content) && type == "PeerMessage") {
			if (parse(content, name, msg)) {
				cout << name << ":(private)\t" << msg << endl;
			}
		}
	}
}	

