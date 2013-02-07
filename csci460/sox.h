#ifndef _USING_CRBSOX_H_
#define _USING_CRBSOX_H_

#include <string>

using namespace std;

int  getListeningSocket(sockaddr_in &server);
int socketSend(int sock, string message);
string socketRead(int sock);

#endif

