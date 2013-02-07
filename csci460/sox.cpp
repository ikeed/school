#include <iostream>
#include <arpa/inet.h>

#include "sox.h"

using namespace std;
#define MAX_BACKLOG 50
#define BUFLEN 500

int getListeningSocket(sockaddr_in &server) {
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
      cerr << "Can't bind socket\n";
      return 0;
   }

   length = sizeof(server);
   if (getsockname(sock, (sockaddr *)&server, (socklen_t *)&length) < 0) {
      cerr << "Can't get socket name\n";
      return 0;
   }
   listen(sock, MAX_BACKLOG);

   return sock;

}


int socketSend(int sock, string message) {
	cout << "Sent: " << message << endl;
	return write(sock, message.c_str(), message.length());
}

string socketRead(int sock) {
	char buf[BUFLEN+1];
	int bytes = read(sock, buf, BUFLEN);

	buf[bytes] = '\0';
	cout << "Recv: " << buf << endl;
	return (string) buf;
}
		
