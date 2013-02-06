#include <iostream>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sstream>
#include <stdlib.h>

using namespace std;

int sendMessage(int sock, string msg);

int sendPostCard(int sock, string recips, string msg) {
	return 1;
}

bool parse(string line, string & comm, string & para)
{
   if (line == "") {
      comm = "";
      para = "";
   } else {
      istringstream iss(line);
      iss >> comm;
      getline(iss, para);
   }
   return true;
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


int getConnected(char *host, int port)
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
      cout << "Opening stream socket error\n";
      return -1;
   }

   if (connect(sock, (const struct sockaddr *)&server, sizeof(server)) < 0) {
      return -1;
   } else {
      return sock;
   }
}

string promptLine(string prompt) {
	return "";
}


string promptWSList(string prompt) {
	return "";
}


int main(int argc, char *argv[])
{
   fd_set readfield;
   struct timeval wait;
   string comm;
   string msg;
   string line;
   string recips;
   int nb;
   char buf[500];

   if (argc != 4) {
      cout << "Usage: " << argv[0] << " host port name\n";
      return 0;
   }

   int sock = getConnected(argv[1], atoi(argv[2]));

   if (sock < 0) {
      cout << "Can't connect to server\n";
      close(sock);
      return 0;
   }

   string name = argv[3];

   write(sock, name.c_str(), name.length()+1);
   buf[0] = '\0';
   read(sock, buf, 500);
   if (buf[0] == '\0') {
      cout << "Duplicate names.\n";
      close(sock);
      return 0;
   }

   cout << name << "> ";
   cout.flush();

   bool quit = false; 
   while (!quit) {
      wait.tv_sec = 1;
      wait.tv_usec = 0;

      FD_ZERO(&readfield);

      FD_SET(sock, &readfield);
      FD_SET(0, &readfield);

      nb = select(FD_SETSIZE, &readfield, (fd_set *)0, (fd_set *)0, &wait);

      if (nb > 0) {
         if (FD_ISSET(0, &readfield)) {  // Input from stdin
            getline(cin, line);
            parse(line, comm, msg);
            if (comm == "quit") {
               quit = true;
            }else if (comm == "help") {
		showHelp();
            }else if (comm == "send") {
	       sendMessage(sock, msg);
	    }else if (comm == "list") {
		sendMessage(sock, comm);
	    }else if (comm == "postcard") {
		recips = promptWSList("Please Enter Receivers' names separated by whitespace");
		if (recips > "") {
			line = promptLine("Please Enter the message (one line only)");
			if (line > "") {
				sendPostCard(sock, recips, line);
			}
		}
	    }else if (comm == "privatemessage") {
		// TODO
            } else {
               cout << "Unknown command\n";
            }
            cout << name << "> ";
            cout.flush();
         }

         if (FD_ISSET(sock, &readfield)) {  // Input from socket
            buf[0] = '\0';
            int bytes = read(sock, buf, 500);
            if (buf[0] == '\0') {
               cout << "\nShutting down by the server\n";
               quit = true;
	    }else if (parse(line, comm, msg)) {
		
            } else {
               buf[bytes] = '\0';
               cout << "\nFrom " << buf << endl;
               cout << name << "> ";
               cout.flush();
            }
         }
      }
   }

   close(sock);
   cout << "Good bye\n";

   return 0;
}

int sendMessage(int sock, string msg) {
	msg = "MSG:" + msg;
	write(sock, msg.c_str(), msg.length()+1);
}

