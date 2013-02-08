#ifndef _USING_CRBSOX_H_
#define _USING_CRBSOX_H_

#include <string>
#include <queue>

using namespace std;

int  getListeningSocket(sockaddr_in &server, int type);
int socketSend(int sock, string message);
queue<string> socketRead(int sock);

#endif

