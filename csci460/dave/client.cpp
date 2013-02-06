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

void parse(string line, string & comm, string & para)
{
   if (line == "") {
      comm = "";
      para = "";
   } else {
      istringstream iss(line);
      iss >> comm;
      getline(iss, para);
   }
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

int main(int argc, char *argv[])
{
   fd_set readfield;
   struct timeval wait;
   string comm;
   string msg;
   string line;
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
            } else if (comm == "send") {
               write(sock, msg.c_str(), msg.length()+1);
               cout << name << "> ";
               cout.flush();
            } else if (comm == "") {
               cout << name << "> ";
               cout.flush();
            } else {
               cout << "Unknown command\n";
               cout << name << "> ";
               cout.flush();
            }
         }

         if (FD_ISSET(sock, &readfield)) {  // Input from socket
            buf[0] = '\0';
            int bytes = read(sock, buf, 500);
            if (buf[0] == '\0') {
               cout << "\nShutting down by the server\n";
               quit = true;
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

