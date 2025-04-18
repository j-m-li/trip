
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
#include <string.h>

struct term {
	var handler;
	var width;
	var height;
	var evt_type;
	var evt_length;
	var evt_data;
};

var term__new(var self);
var term__width(var self);
var term__height(var self);
var term__wait(var self, var timeout);
var term__dispose(var self);
var term__deinit();
var clipboard__set(var txt, var len);
var clipboard__get();

#endif

