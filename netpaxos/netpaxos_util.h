#ifndef _NETPAXOS_UTIL_H
#define _NETPAXOS_UTIL_H

void bindSocket(int sock, int port);
void bindToDevice(int sock, char *iname);
void addMembership(int sock, char *group);
void setRecBuf(int sock);
int getRecBuf(int sock);

#endif