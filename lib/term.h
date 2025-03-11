
#ifndef _TERM_H
#define _TERM_H

#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct term {
	var handler;
	var width;
	var height;
	var evt_type;
	var evt_length;
	var evt_data;
};

var term__new();
var term__size(var a);
var term__init(var a);
var term__wait(var term_, var timeout);
var term__deinit();
var clipboard__set(var txt, var len);
var clipboard__get();

#endif

