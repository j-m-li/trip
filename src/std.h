
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


#define var long

#define MAX_MEMBER 1270
#define MODE_C 0
#define MODE_INTERP 1


struct cell {
	char *name;
	char *value;
	var data;
};

struct class_cell {
	char *name;
	char *value;
	var data;
	struct cell fields[MAX_MEMBER+1];
	int nb_fields;
	struct cell methods[MAX_MEMBER+1];
	int nb_methods;
};

struct file {
	int start;
	int end;
	char *buf;
	char *file_name;
};

struct context {
	struct context *parent;
	struct context *child;
	char *func_name;
	char *class_name;
	struct cell *vars;
	int nb_vars;
	struct cell *refs;
	int nb_refs;
	var __self;
	char *buf;
	var pos;
};

struct trip {
	struct trip *parent;
	struct trip *next;
	struct trip *last;
	struct file files[MAX_MEMBER+1];
	int nb_files;
	char *file_name;
	char *buf;
	int pos;
	int end;
	int retok;
	int state;
	int depth;
	int line;
	int indent;
	int incall;
	int replaced;
	struct cell vars[MAX_MEMBER+1];
	int nb_vars;
	struct cell refs[MAX_MEMBER+1];
	int nb_refs;

	struct cell global[MAX_MEMBER+1];
	int nb_global;
	struct cell defs[MAX_MEMBER+1];
	int nb_defs;
	struct class_cell classes[MAX_MEMBER+1];
	int nb_classes;
	struct cell funcs[MAX_MEMBER+1];
	int nb_funcs;

	int main;
	int returnn;
	int mode;
	int break_;
	int else_;
	int continue_;
	int return_;
	var cond;
	var return_val;

	struct context *ctx;
	var __self;
	var stack[MAX_MEMBER+1];
	int sp;

	char *func_name;
	char *class_name;
	char tmp[128];
	char tmp_var[512];
};

struct trip *load(char *file);
void compound(struct trip *st);
void spaces(struct trip *st);
int expression(struct trip *st);
var run_it(struct trip *st, char *class_, char *name);
void k_func(struct trip *st);
void k_array(struct trip *st);
void k_bytes(struct trip *st);
void k_method(struct trip *st);
void k_define(struct trip *st);
void k_include(struct trip *st);
char *id_tmp(struct trip *sti, char *id);
char *identifier(struct trip *st, int *l);
int id_cmp(char *a, char *b);
char *id_dup(char *a);
void call_exec(struct trip *st, char *func, char *id, var self, char *clas, char *meth, int is_v);
var get_data(struct trip *st, var sel, char *cls, char *name, int return_addr);
void skip_body(struct trip *st);
void skip_param(struct trip *st);
struct trip *trip__new();
int trip__delete(struct trip *st);
int error(char *txt, struct trip *st);
var (*wrapper(struct trip *st, char *clas, char *id, int *argc))();


#include "file.h"
#include "term.h"
#include "socket.h"

var quit(void);

var print10(var n)
{
	printf("%ld", n);
	return 0;

}

var print16(var n)
{
	printf("0x%lX", n);
	return 0;

}

#define pow trip__pow

var pow(var x, var y)
{
	var r = 1;
	if (x == 0) {
		return 0;
	}
	while (y > 0) {
		r = r * x;
		y--;
	}
	while (y < 0) {
		r = r / x;
		y++;
	}
	return r;
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

struct buffer {
	var data;
	var length;
	var alloced;
};

var singleton()
{
	static struct buffer b = {0,0,0};
	return (var)&b;
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
	((char*)buf[0])[end] = 0;
	return 0;
}

var buffer__append10(var b, var n) 
{
	char tmp[64];
	int i = 0;
	int sub = 0;
	var max = 10000;
	var t;

	if (n == 0) {
		tmp[0] = '0';
		return buffer__append(b, (var)tmp, 1);
	}
	t = max * 10;
	while (t > max) {
		max = t;
		t = t * 10;
	}

	if (n == -n) { /* -32768 or -2147483648 or -9223372036854775808 */
		sub = 1; 
		n = n + 1;
		tmp[0] = '-';
		i = 1;
	} else if (n < 0) {
		n = -n;
		tmp[0] = '-';
		i = 1;
	}
	while (max > 1) {
		t = max / 10;
		if (n >= t) {
			break;
		}
		max = t;
	}	
	while (max > 1) {
		tmp[i] = '0' + (n / max);
		i++;
		n = (n % max) - sub;
		sub = 0;
		max = max / 10;
	}	
	return buffer__append(b, (var)tmp, i);
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

var command(struct trip *st, var cmd_, var len)
{
	var r = 0;
	char *cmd = (void*)cmd_;
	int mode = -1;
	char *obuf;
	int opos;
	int oend;
	char *ofunc_name;
	char *ofile_name;
	char *oclass_name;
	var ti;

	if (!len || !cmd_) {
		error("expecting command", st);
	}
	if (st) {
		ti = st->__self;
		mode = st->mode;
		opos = st->pos;
		obuf = st->buf;
		oend = st->end;
		ofunc_name =  st->func_name;
		ofile_name =  st->file_name;
		oclass_name = st->class_name;
	} else {
		st = trip__new();
		st->ctx = 0;
	}
	st->mode = MODE_INTERP;
	st->pos = 0;
	st->buf = cmd;
	st->end = len;
	st->func_name = "command line";
	st->file_name = "command line";
	st->class_name = NULL;
	compound(st);
	
	if (mode == -1) {
		trip__delete(st);
	} else {
		st->__self = ti;
		st->file_name =  ofile_name;
		st->func_name =  ofunc_name;
		st->class_name = oclass_name;
		st->pos = opos;
		st->buf = obuf;
		st->end = oend;
	}
	return r;
}
#include "trip.c"
#include "file.c"
#include "term.c"
#include "socket.c"
