/*
 * Socket manipulation functions
 */
#ifndef SOCKET_H
#define SOCKET_H

/* Get IP address based on protocol version (IPv4 or IPv6) */
void *get_in_addr(struct sockaddr *sa);

#endif /* end of include guard: SOCKET_H */
