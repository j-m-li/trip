
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

#include "file.h"
#include "term.h"
#include "socket.h"

var quit(void);

var print10(var n)
{
	printf("%ld", n);
	return 0;

}

var flush()
{
	fflush(stdout);
	return 0;
}

var print(var txt)
{
	if (!txt) {
		printf("(nullptr)");
		return -1;
	}
	printf("%s", (char*)txt);
	return 0;
}

var printb(var buf, var len)
{
#ifdef _WIN32
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), 
			(char*)buf, len, NULL, NULL);
#else
	fwrite((void*)buf, len, 1, stdout);
#endif
	return 0;
}

var buffer__append(var b, var data, var len) 
{
	var *buf = (var*)b;
	var end;
	if (buf[0] == 0) {
		buf[0] = (var)malloc(4096);
		buf[1] = 0;
		buf[2] = 4096;
	}
	end = buf[1] + len;
	if ((end+2) >= buf[2]) {
		buf[2] = end + 4096;
		buf[0] = (var)realloc((void*)buf[0], buf[2]);
	}
	memcpy((char*)(buf[0] + buf[1]), (char*)data, len);
	buf[1] = end;
	return 0;
}

var str_cmp(var a, var b)
{
	return strcmp((void*)a, (void*)b);
}

var str_dup(var a)
{
	return (var)strdup((void*)a);
}

#ifdef _MSC_VER
void __builtin_trap()
{
    __debugbreak();
}
#endif

var run(var a)
{
	return system((char*)a);
}


var quit(void)
{
	term__deinit();
	exit(0);
	return -1;
}

#include "file.c"
#include "term.c"
#include "socket.c"
