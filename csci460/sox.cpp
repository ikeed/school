#include <iostream>
#include <arpa/inet.h>
#include <sstream>
#include "sox.h"
#include <stdio.h>
#include <errno.h>

using namespace std;
#define MAX_BACKLOG 50
#define BUFLEN 500

#define ETX 0x03

int getListeningSocket(sockaddr_in &server, int type) {
   int sock = socket(AF_INET, type, 0);
   int length;
 
   if (sock < 0) {
      cerr << "Can't open socket\n";
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
	message = message + ( (char) ETX);
	return write(sock, message.c_str(), message.length());
}

queue<string> socketRead(int sock) {
	char buf[BUFLEN+1];
	int bytes = read(sock, buf, BUFLEN);
	queue<string> q;
	string s;
	char delim = ETX;

	if (bytes <= 0) {
		perror("socketRead");
		return q;
	}

	buf[bytes] = '\0';
	istringstream ss(buf);
	while (ss.rdbuf()->in_avail()) {
		getline(ss, s, delim);
		q.push(s);	
	}
	
	return q;
}

