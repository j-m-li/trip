#ifndef _FILE_H
#define _FILE_H

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

#include "folder.h"

var file__delete(var path);
var file__rename(var src, var dest);
var file__size(var path);
var file__load(var path, var offset, var size);
var file__save(var path, var offset, var buf, var size);
var folder__create(var path);
var folder__delete(var path);
var folder__list(var path);
var file__get_home();

#endif /* _FILE_H */
