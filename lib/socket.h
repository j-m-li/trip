#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef _WIN32
#include <winsock2.h>
#include <stdio.h>
#pragma comment(lib, "Ws2_32.lib")
#else
       #include <netdb.h>
       #include <stdio.h>
       #include <stdlib.h>
       #include <string.h>
       #include <sys/socket.h>
       #include <sys/types.h>
       #include <unistd.h>
#endif

struct socket {
	var fd;
};

var socket__new(var host, var port);
var socket__read(var self);
var socket__write(var self);
var socket__dispose(var self);

#endif /*_SOCKET_H */

