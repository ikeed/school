#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <sstream>

using namespace std;

const int MaxUser = 50;

struct Interface {
   string name;
   int sock;
};

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

int main()
{
   int sock;
   Interface users[MaxUser];
   int userSize = 0;
   struct sockaddr_in server;
   fd_set read;
   struct timeval wait;
   int nb;
   char buf[500];
   string name, msg;

   sock = socket(AF_INET, SOCK_STREAM, 0);
   if (sock < 0) {
      cout << "Can't open stream socket\n";
      return 0;
   }

   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = 0;

   if (bind(sock, (const sockaddr *)&server, sizeof(server)) < 0) {
      cout << "Can't binding stream socket\n";
      return 0;
   }

   int length = sizeof(server);
   if (getsockname(sock, (sockaddr *)&server, (socklen_t *)&length) < 0) {
      cout << "Can't get socket name\n";
      return 0;
   }

   char host[80];
   gethostname(host, 80);

   cout << "host name: " << host << endl;
   cout << "port number: " << ntohs(server.sin_port) << endl;

   listen(sock, MaxUser);

   bool shutdown = false;
   string cmd;
   cout << "server> ";
   cout.flush();

   while (!shutdown) {
      wait.tv_sec = 1;
      wait.tv_usec = 0;

      FD_ZERO(&read);
      FD_SET(0, &read);
      FD_SET(sock, &read);
      for(int i = 0; i < userSize; i++)
         FD_SET(users[i].sock, &read);

      nb = select(FD_SETSIZE, &read, (fd_set *)0, (fd_set *)0, &wait);

      if (nb > 0) {
         if (FD_ISSET(0, &read)) {
            getline(cin, cmd);
            if (cmd == "shutdown") {
               shutdown = true;
            } else {
               cout << "Unknown command\n";
               cout << "server> ";
               cout.flush();
            }
         }

         for(int i = 0; i < userSize; i++) {
            if (FD_ISSET(users[i].sock, &read)) {
               buf[0] = '\0';
               recv(users[i].sock, buf, 500, 0);
               if (buf[0] == '\0') {
                  close(users[i].sock);
                  userSize --;
                  users[i].name = users[userSize].name;
                  users[i].sock = users[userSize].sock;
               } else {
		  cout << "Received: " << buf << endl;
                  parse(buf, name, msg);
                  msg = users[i].name + msg;
                  for(int j = 0; j < userSize; j++) {
                     if (users[j].name == name) {
                        write(users[j].sock, msg.c_str(), msg.length());
                     }
                  }
               }
            }
         }

         if (FD_ISSET(sock, &read)) {
            users[userSize].sock
                 = accept(sock, (sockaddr *)&server, (socklen_t *)&length);
            buf[0] = '\0';
            recv(users[userSize].sock, buf, 500, 0);
            users[userSize].name = buf;
            bool found = false;
            for(int j = 0; !found && j < userSize; j++) {
               if (users[j].name == users[userSize].name)
                  found = true;
            }
            if (found) {
               close(users[userSize].sock);
            } else {
               write(users[userSize].sock, "OK", 2);
               userSize++;
            }
         }
      }
   }

   close(sock);
   for(int i = 0; i < userSize; i++)
      close(users[i].sock);

   cout << "Good bye\n";
   return 0;
}

