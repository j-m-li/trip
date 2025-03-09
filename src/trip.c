/*
                 TRIP programing language


          MMXXV January 13 PUBLIC DOMAIN by JML

     The authors and contributors disclaim copyright, patents
           and all related rights to this software.

 Anyone is free to copy, modify, publish, use, compile, sell, or
 distribute this software, either in source code form or as a
 compiled binary, for any purpose, commercial or non-commercial,
 and by any means.

 The authors waive all rights to patents, both currently owned
 by the authors or acquired in the future, that are necessarily
 infringed by this software, relating to make, have made, repair,
 use, sell, import, transfer, distribute or configure hardware
 or software in finished or intermediate form, whether by run,
 manufacture, assembly, testing, compiling, processing, loading
 or applying this software or otherwise.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT OF ANY PATENT, COPYRIGHT, TRADE SECRET OR OTHER
 PROPRIETARY RIGHT.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
 ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define var long

#include "../lib/std.h"

#define MAX_MEMBER 127
#define MODE_C 0
#define MODE_INTERP 1
#define MODE_PRE_INTER 2
#define MODE_JS 3
#define MODE_PHP 4
#define MODE_SHELL 5

struct cell {
	char *name;
	char *value;
	var data;
};

struct class_cell {
	char *name;
	char *value;
	var data;
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
var get_data(struct trip *st, char *is_self, char *name, int return_addr);
void skip_body(struct trip *st);
void skip_param(struct trip *st);

int error(char *txt, struct trip *st)
{
	int l = 1;
	int i = 0;
	char *file;

	while (i < st->pos) {
		if (st->buf[i] == '\n') {
			l++;
		}
		i++;
	}
	for (i = 0; st->files[i].buf != st->buf; i++) {
	}
	file = st->files[i].file_name;
	printf("\n#error \"%s @ line %d in %s\"\n", txt, l, file);
	fflush(stdout);
	__builtin_trap();
	exit(-1);
}

int is_eol(struct trip *st)
{
	int pos = st->pos;
	while (pos < st->end) {
		switch (st->buf[pos]) {
		case '\n':
			return 1;
		case ' ':
		case '\t':
		case '\v':
		case '\r':
			pos++;
			break;
		case '#':
			return 1;
		default:
			return 0;
		}
	}
	return 1;
}

void spaces(struct trip *st)
{
	char *p;
	while (st->pos < st->end) {
		p = st->buf + st->pos;
		if (*p != ' ' && *p != '\t') {
		       break;
	      	}
		st->pos++;
 	}		
}

int whitespaces(struct trip *st)
{
	int end;
	while (st->pos < st->end) {
		switch (st->buf[st->pos]) {
		case '\n':
			st->line++;
		case ' ':
		case '\t':
		case '\v':
		case '\r':
		case '\0':
			st->pos++;
			break;
		case '#':
			st->pos++;
			end = 0;
			while (!end && st->pos < st->end) {
				switch (st->buf[st->pos]) {
				case '\n':
					end = 1;
					break;
				default:
					st->pos++;
				}
			}
			break;
		default:
			return 0;
		}
	}
	return 1;
}

void comment(struct trip *st)
{
	while (st->pos < st->end && st->buf[st->pos] == '#') {
		st->pos++;
		while (st->pos < st->end && st->buf[st->pos] != '\n') {
			st->pos++;
		}
		whitespaces(st);
	}
}

void semicolon(struct trip *st)
{
	whitespaces(st);
	if (st->pos < st->end && st->buf[st->pos] == ';') {
		st->pos++;
	} else {
		error("missing ;", st);
	}
}

void indent(struct trip *st)
{
	int i;
	if (st->mode == MODE_INTERP) {
		return;
	} else {
	}
	if (st->class_name || st->func_name) {
		for (i = 1; i < st->indent; i++) {
			printf("\t");
		}
		return;
	}
}


void push(struct trip *st, var b)
{
	st->stack[st->sp] = b;
	if (st->sp < MAX_MEMBER) {
		st->sp++;
	} else {
		exit(-2);
	}
}

var pop(struct trip *st)
{
	if (st->sp > 0) {
		st->sp--;
	} else {
		__builtin_trap();
		exit(-3);
	}
	return st->stack[st->sp];
}


char *get_class(struct trip *st, char *name) { int i;
	for (i = 0; i < st->nb_refs; i++) {
		if (!id_cmp(st->refs[i].name, name)) {
			return st->refs[i].value;
		}
	}
	return NULL;
}

int is_var(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_vars; i++) {
		if (!id_cmp(st->vars[i].name, name)) {
			return 1;
		}
	}
	return 0;
}

int is_def(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_defs; i++) {
		if (!id_cmp(st->defs[i].name, name)) {
			return 1;
		}
	}
	return 0;
}

int is_global(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_global; i++) {
		if (!id_cmp(st->global[i].name, name)) {
			return 1;
		}
	}
	return 0;
}

int is(struct trip *st, char *p, const char *k)
{
	char *end = st->buf + st->end;
	const char *b = k;

	while (p < end && *k && *k == *p) {
		p++;
		k++;
	}
	if (*k == 0 && !((*p >= 'a' && *p <= 'z') 
			|| (*p >= 'A' && *p <= 'Z')
			|| (*p >= '0' && *p <= '9')
			|| *p == '_'))
	{
		return k - b;
	}
	return 0;
}

int end_of_expr(struct trip *st)
{
	int ok;
	ok = 0;
	while (st->pos < st->end && !ok) {
		switch (st->buf[st->pos]) {
		case '\n':
			ok = 1;
			whitespaces(st);
			break;
		case ' ':
		case '\t':
		case '\v':
		case '\r':
		case '\0':
			st->pos++;
			break;
		case '#':
			ok = 1;
			comment(st);
			break;
		case '}':
			return 1;
		case ';':
			st->pos++;
			ok = 1;
			whitespaces(st);
			break;
		default:
			return 0;
		}
	}
	switch (st->buf[st->pos]) {
	case '#':
		comment(st);
	}
	return 1;
}



void skip(struct trip *st)
{
	while (st->pos < st->end && st->buf[st->pos] != '\n') {
		st->pos++;
	}
	st->pos++;
	st->line++;
}

int string_len(char *b)
{
	int e = b[0];
	int i;
	i = 1;
	while (b[i] != e) {
		if (i > 1020) {
			printf("#error string too long\n");
		}
		i++;
	}
	i++;
	return i;
}

int id_cmp(char *a, char *b)
{
	int i;
	i = 0;
	if (!a || !b) {
		return 1;
	}
	while ((b[i] >= 'a' && b[i] <= 'z') ||
		(b[i] >= 'A' && b[i] <= 'Z') ||
		(b[i] == '_') ||
		(i > 0 && b[i] >= '0' && b[i] <= '9')) 
	{
		if (a[i] != b[i]) {
			return -1;
		}
		i++;
	}
	if ((a[i] >= 'a' && a[i] <= 'z') ||
		(a[i] >= 'A' && a[i] <= 'Z') ||
		(a[i] == '_') ||
		(i > 0 && a[i] >= '0' && a[i] <= '9')) 
	{
		return 1;
	}
	return 0;
}

char *id_tmp(struct trip *st, char *b)
{
	int i;
	i = 0;
	if (!b) {
		return NULL;
	}
	while ((b[i] >= 'a' && b[i] <= 'z') ||
		(b[i] >= 'A' && b[i] <= 'Z') ||
		(b[i] == '_') ||
		(i > 0 && b[i] >= '0' && b[i] <= '9')) 
	{
		if (i >= sizeof(st->tmp) - 1) {
			st->tmp[0] = 0;
			return st->tmp;
		}
		st->tmp[i] = b[i];
		i++;
	}
	st->tmp[i] = 0;
	return st->tmp;
}


int id_len(char *b)
{
	int i;
	i = 0;
	while ((b[i] >= 'a' && b[i] <= 'z') ||
		(b[i] >= 'A' && b[i] <= 'Z') ||
		(b[i] == '_') ||
		(i > 0 && b[i] >= '0' && b[i] <= '9')) 
	{
		if (i > 1020) {
			printf("#error identifier too long\n");
		}
		i++;
	}
	return i;
}

char *id_dup(char *b)
{
	int l;
	char *v;
	l = id_len(b);
	v = malloc(l + 1);
	v[l] = 0;
	while (l > 0) {
		l--;
		v[l] = b[l];
	}
	return v;
}


int printsub(char *name, int len)
{
	int o;
	o = name[len];
	name[len] = '\0';
	printf("%s", name);
	name[len] = o;
	return 0;
}

void declare_class(struct trip *st, char *cid)
{
	char *id;
	char *p;
	int l;
	if (st->nb_classes >= MAX_MEMBER) {
		error("too many classes", st);
	}
	st->classes[st->nb_classes].name = cid;
	st->classes[st->nb_classes].nb_methods = 0;
	st->classes[st->nb_classes].data = (var)(st->buf + st->pos);
	st->nb_classes++;

	if (st->mode == MODE_INTERP) {
	} else {
		printf("struct %s {\n", id_tmp(st,cid));
	}
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field")) {
			st->pos += 5;
			whitespaces(st);
			id = identifier(st, &l);
			if (st->mode == MODE_INTERP) {
			} else {
				printf("\tvar %s;\n", id_tmp(st,id));
			}
			end_of_expr(st);
		} else if (is(st, p, "method")) {
			break;
		} else if (*p == '}') {
			break;
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	if (st->mode == MODE_INTERP) {
	} else {
		printf("};\n");
	}
}

void declare_func(struct trip *st, char *id)
{
	if (st->nb_funcs >= MAX_MEMBER) {
		error("too many functions", st);
	}
	st->funcs[st->nb_funcs].name = id;
	st->funcs[st->nb_funcs].data = (var)(st->buf + st->pos);
	st->nb_funcs++;
	if (st->mode == MODE_INTERP) {
	} else {
		printf("var %s();\n", id_tmp(st,id));
	}
}

void declare_method(struct trip *st, char *cid, char *id)
{
	struct class_cell *cl = st->classes + st->nb_classes - 1;
	if (!cl || cl->nb_methods >= MAX_MEMBER) {
		error("too many functions", st);
	}
	cl->methods[cl->nb_methods].name = id;
	cl->methods[cl->nb_methods].data = (var)(st->buf + st->pos);
	cl->nb_methods++;

	if (st->mode == MODE_INTERP) {
	} else {
		printf("var %s__", id_tmp(st, cid));
		printf("%s();\n", id_tmp(st,id));
	}
}


void declare(struct trip *st)
{
	char *p;
	char *id;
	int l;
	char *cid;
	int cl;

	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "class")) {
			st->pos += 5;
			whitespaces(st);
			cid = identifier(st, &cl);
			declare_class(st, cid);
		} else if (is(st, p, "func")) {
			st->pos += 4;
			whitespaces(st);
			id = identifier(st, &l);
			declare_func(st, id);
			skip_param(st);
			skip_body(st);
		} else if (is(st, p, "define")) {
			k_define(st);

		} else if (is(st, p, "include")) {
			k_include(st);
		} else if (is(st, p, "array")) {
			k_array(st);
		} else if (is(st, p, "bytes")) {
			k_bytes(st);
		} else if (is(st, p, "method")) {
			st->pos += 6;
			whitespaces(st);
			id = identifier(st, &l);
			declare_method(st, cid, id);
			skip_param(st);
			skip_body(st);
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = 0;

}

struct cell *save_cells(int nb_cell, struct cell *src)
{	
	struct cell *vars;
	int i;

	vars = malloc((nb_cell+1) * sizeof(*vars));
	for (i = 0; i < nb_cell; i++) {
		if (src[i].name) {
			vars[i].name = src[i].name;
		} else {
			vars[i].name = NULL;
		}
		if (src[i].value) {
			vars[i].value = src[i].value;
		} else {
			vars[i].value = NULL;
		}
		vars[i].data = src[i].data;
	}
	return vars;
}

var push_context(struct trip *st)
{
	struct context *ctx;

	ctx = malloc(sizeof(*ctx));
	ctx->parent = st->ctx;
	ctx->child = NULL;
	if (ctx->parent) {
		ctx->parent->child = ctx;
	}
	st->ctx = ctx;
	ctx->__self = st->__self;
	if (st->func_name) {
		ctx->func_name = st->func_name;
	} else {
		ctx->func_name = NULL;
	}
	if (st->class_name) {
		ctx->class_name = st->class_name;
	} else {
		ctx->class_name = NULL;
	}
	ctx->nb_vars = st->nb_vars;
	ctx->vars = save_cells(st->nb_vars, st->vars);
	st->nb_vars = 0;
	ctx->nb_refs = st->nb_refs;
	ctx->refs = save_cells(st->nb_refs, st->refs);
	st->nb_refs = 0;
	return 0;
}

void free_cells_val(int nb_cell, struct cell *src)
{
	int i;
	for (i = 0; i < nb_cell; i++) {
		if (src[i].name) {
			free(src[i].name);
		}
		if (src[i].value) {
			free(src[i].value);
		}
	}
}

void restore_cells(int nb_cell, struct cell *src, struct cell *dest)
{	
	int i;

	for (i = 0; i < nb_cell; i++) {
		dest[i].name = src[i].name;
		dest[i].value = src[i].value;
		dest[i].data = src[i].data;
	}
}


var pop_context(struct trip *st)
{
	struct context *ctx = st->ctx;

	st->ctx = ctx->parent;
	if (ctx->child) {
		/*free_cells_val(st->nb_vars, st->vars);
		free_cells_val(st->nb_refs, st->refs);

		if (st->func_name) {
			free(st->func_name);
		}
		if (st->class_name) {
			free(st->class_name);
		}*/
	}
	st->__self = ctx->__self;
	st->func_name = ctx->func_name;
	st->class_name = ctx->class_name;
	restore_cells(ctx->nb_vars, ctx->vars, st->vars);
	st->nb_vars = ctx->nb_vars;
	restore_cells(ctx->nb_refs, ctx->refs, st->refs);
	st->nb_refs = ctx->nb_refs;
	free(ctx->vars);
	free(ctx->refs);
	free(ctx);
	return 0;
}

var run_it(struct trip *st, char *class_, char *name) 
{
	int i;
	char *func=  NULL;
	for (i = 0; i < st->nb_funcs;i++) {
		if (!id_cmp(name, st->funcs[i].name)) {
			func = (char*)st->funcs[i].data;
			break;
		}
	}
	if (!func) {
		st->pos = 0;
		compound(st);
		return 0;
	}
	call_exec(st, func, name, 0, class_, NULL, 0);
	return 0;
}


struct trip *trip__new() 
{
	struct trip *st;
	st = malloc(sizeof(*st));
	st->line = 1;
	st->end = 0;
	st->buf = NULL;
	st->state = 0;
	st->pos = 0;
	st->indent = 0;
	st->incall = 0;
	st->parent = 0;
	st->next = 0;
	st->last = 0;
	st->nb_vars = 0;
	st->nb_defs = 0;
	st->nb_classes = 0;
	st->nb_funcs = 0;
	st->nb_global = 0;
	st->main = 0;
	st->returnn = 0;
	st->return_ = 0;
	st->break_ = 0;
	st->else_ = 0;
	st->continue_ = 0;
	st->ctx = NULL;
	st->__self = 0;
	st->sp = 0;
	st->stack[0] = 0;
	return st;
}

int num_lit(struct trip *st)
{
	char *end = st->buf + st->end;
	char *b;
	char *p;
	int r;
	int o;

	b = st->buf + st->pos;
	p = b;
	if (p[0] == '\'') {
		p++;
		r = *p;
		p++;
		if (*p != '\'') {
			error("in char constant", st);
		}
		p++;
		st->pos += p - b;
		return r;
	} else {
		while (p < end && (*p >= '0' && *p <= '9')) {
			p++;
		}
	}
	st->pos += p - b;
	o = *p;
	*p = 0;
	r = atol(b);
	*p = o;
	return r;
}


char *identifier(struct trip *st, int *l)
{
	char *end = st->buf + st->end;
	char *b;
	char *p;

	b = st->buf + st->pos;
	p = b;
	if (p >= end || !((*p >= 'a' && *p <= 'z') 
			|| (*p >= 'A' && *p <= 'Z')
			|| *p == '_'))
	{
		return NULL;
	}
	while (p < end && ((*p >= 'a' && *p <= 'z') 
			|| (*p >= 'A' && *p <= 'Z')
			|| (*p >= '0' && *p <= '9')
			|| *p == '_'))
	{
		p++;
	}
	*l = p - b;
	st->pos += *l;
	return b;
}

char *get_class_def_ptr(struct trip *st, char *name)
{
	int i;
	char *pos = NULL;
	for (i = 0; i < st->nb_classes; i++) {
		if (!id_cmp(st->classes[i].name, name)) {
			pos = (char*)st->classes[i].data;
			break;
		}
	}
	return pos;
}

void fix_pos_buf(struct trip *st, char *ptr)
{
	int i;
	char *p;
	if (!ptr) {
		return;
	}
	for (i = 0; i < st->nb_files; i++) {
		p = st->files[i].buf;
		if (ptr >= p) {
			if (ptr < (p + st->files[i].end)) {
				st->buf = p;
				st->pos = ptr - p;
				st->end = st->files[i].end;
				return;
			}
		}
	}
	error("mess in pointers", st);
}

char *get_member_ptr(struct trip *st, char *clas, char *type, char *name)
{
	int l;
	char *p;
	char *id;
	char *n;
	int opos;
	char *obuf;
	opos = st->pos;
	obuf = st->buf;
	fix_pos_buf(st, get_class_def_ptr(st, clas));
	if (st->pos < 0) {
		st->pos = opos; 
		st->buf = obuf; 
		return 0;
	}
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field")) {
			st->pos += 5;
			whitespaces(st);
			id = identifier(st, &l);
			end_of_expr(st);
			if (!strcmp(type, "field")) {
				if (!id_cmp(id, name)) {
					n = st->buf + st->pos;
					st->pos = opos; 
					st->buf = obuf; 
					return n;
				}
			}
		} else if (is(st, p, "method")) {
			st->pos += 6;
			whitespaces(st);
			id = identifier(st, &l);
			whitespaces(st);
			if (!strcmp(type, "method")) {
				if (!id_cmp(id, name)) {
					n = st->buf + st->pos;
					st->pos = opos; 
					st->buf = obuf; 
					return n;
				}
			}
			skip(st);
		} else if (is(st, p, "class")) {
			break;
		} else if (is(st, p, "func")) {
			break;
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = opos; 
	st->buf = obuf; 
	return NULL;
}

int get_member_pos(struct trip *st, char *clas, char *type, char *name)
{
	int l;
	char *p;
	char *id;
	int n;
	int opos;
	char *obuf;
	opos = st->pos;
	obuf = st->buf;
	fix_pos_buf(st, get_class_def_ptr(st, clas));
	if (st->pos < 0) {
		st->pos = opos; 
		st->buf = obuf; 
		return -1;
	}
	n = 0;
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field")) {
			st->pos += 5;
			whitespaces(st);
			id = identifier(st, &l);
			end_of_expr(st);
			if (!strcmp(type, "field")) {
				if (!id_cmp(id, name)) {
					st->pos = opos; 
					st->buf = obuf; 
					return n;
				}
			}
			n++;
		} else if (is(st, p, "method")) {
			break;
		} else if (is(st, p, "class")) {
			break;
		} else if (is(st, p, "func")) {
			break;
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = opos; 
	st->buf = obuf; 
	return -1;
}




int size_of(struct trip *st, char *name)
{
	int l;
	char *p;
	int n;
	int opos;
	char *obuf;
	char *df;
	
	opos = st->pos;
	obuf = st->buf;
	n = 0;
	df = get_class_def_ptr(st, name);
	if (!df) {
		st->pos = opos; 
		st->buf = obuf; 
		return 0;
	}
	fix_pos_buf(st, df);
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field")) {
			st->pos += 5;
			whitespaces(st);
			identifier(st, &l);
			end_of_expr(st);
			n++;
		} else if (is(st, p, "method")) {
			break;
		} else if (*p == '}') {
			break;
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = opos; 
	st->buf = obuf; 
	return n * sizeof(var);
}



var get_data(struct trip *st, char *is_self, char *name, int return_addr)
{
	int i;
	int idx;
	var *arr;
	if (is_self && is_self[0]) {
		idx = get_member_pos(st, st->class_name, "field", name);
		if (idx < 0) {
			error("cannot find field", st);
		}
		arr = (var*)st->__self;
		if (return_addr) {
			return	(var)(arr + idx);
		}
		return arr[idx];
	} else {
		for (i = 0; i < st->nb_vars; i++) {
			if (!id_cmp(st->vars[i].name, name)) {
				if (return_addr) {
					return (var)&st->vars[i].data;
				}
				return st->vars[i].data;
			}
		}
		for (i = 0; i < st->nb_refs; i++) {
			if (!id_cmp(st->refs[i].name, name)) {
				if (return_addr) {
					return (var)&st->refs[i].data;
				}
				return st->refs[i].data;
			}
		}
	
		for (i = 0; i < st->nb_defs; i++) {
			if (!id_cmp(st->defs[i].name, name)) {
				return st->defs[i].data;
			}
		}
		for (i = 0; i < st->nb_funcs; i++) {
			if (!id_cmp(st->funcs[i].name, name)) {
				return st->funcs[i].data;
			}
		}
		for (i = 0; i < st->nb_global; i++) {
			if (!id_cmp(st->global[i].name, name)) {
				return st->global[i].data;
			}
		}
	
	}
	error("Cannot find variable", st);
	return 0;
}

char *variable(struct trip *st, int return_addr)
{
	char *xid;
	char *id;
	int l;
	int x;
	var *arr;
	char *sel;
	char *cls;
	sel = "";
	id = identifier(st, &l);
	if (!id) {
		return NULL;
	}
	if (st->buf[st->pos] == '[') {
		if (!is_var(st, id) && !get_class(st, id) 
				&& !is_def(st, id) && !is_global(st, id)) 
		{
			if (id_cmp(st->func_name, "main")) {
				sel = "__self->";
			}
		}
		if (st->mode == MODE_INTERP) {
			arr = (var*)get_data(st, sel, id, 0);
		} else {
			printf("((var*)(%s%s))[", sel, id_tmp(st,id));
		}
		st->pos++;
		expression(st);
		if (st->buf[st->pos] != ']') {
			error("] !", st);
		}
		if (st->mode == MODE_INTERP) {
			if (return_addr) {
				push(st, (var)(arr + pop(st)));
			} else {
				push(st, arr[pop(st)]);
			}
		} else {
			printf("]");
		}
		st->pos++;
	} else {
		if (!id_cmp("null", id)) {
			if (st->mode == MODE_INTERP) {
				push(st, 0);
			} else {
				printf("((var)0)");
			}
		} else if (!id_cmp("false", id)) {
			if (st->mode == MODE_INTERP) {
				push(st, 0);
			} else {
				printf("((var)0)");
			}
		} else if (!id_cmp("true", id)) {
			if (st->mode == MODE_INTERP) {
				push(st, 1);
			} else {
				printf("((var)1)");
			}
		} else if (!id_cmp("this", id)) {
			if (st->mode == MODE_INTERP) {
				push(st, (var)st->__self);
			} else {
				printf("self");
			}
		} else if (!id_cmp("continue", id)) {
			if (st->mode == MODE_INTERP) {
				st->continue_ = 1;
			} else {
				indent(st);
				printf("\tcontinue;\n");
			}
		} else if (!id_cmp("break", id)) {
			if (st->mode == MODE_INTERP) {
				st->break_ = 1;
			} else {
				indent(st);
				printf("\t__while = 0; continue;\n");
			}
		} else {
			cls = get_class(st, id);
			if (is_def(st, id) || is_global(st, id)) {
				if (st->mode == MODE_INTERP) {
					push(st, get_data(st, NULL, 
						id, return_addr));
				} else {
					printf("%s", id_tmp(st,id));
				}
			} else if (!is_var(st, id) && !cls) {
				sel = "__self->";
				if (st->mode == MODE_INTERP) {
					push(st, get_data(st, sel, 
							id, return_addr));
				} else {
					printf("%s%s", sel, id_tmp(st,id));
				}
			} else if (id[l] == '.' && cls) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf("(var)%s__", id_tmp(st,cls));
				}
				st->pos++;
				xid = identifier(st, &x);
				if (xid) {
					if (st->mode == MODE_INTERP) {
						push(st, 
							(var)get_member_ptr(st,	
							cls, "method", xid));
					} else {
						printf("%s", id_tmp(st,xid));
					}
				} else {
					error("error0", st);
				}
			} else if (cls) {
				if (st->mode == MODE_INTERP) {
					push(st, get_data(st, NULL, 
						id, return_addr));
				} else {
					printf("(*((var*)&%s))", id_tmp(st,id));
				}
			} else if (!id_cmp("this", id)) {
				if (st->mode == MODE_INTERP) {
					push(st, st->__self);
				} else {
					printf("self");
				}
			} else {
				if (st->mode == MODE_INTERP) {
					push(st, get_data(st, NULL, 
						id, return_addr));
				} else {
					printf("%s", id_tmp(st,id));
				}
			}
		}
	}
	return id;
}


int operator(struct trip *st)
{
	char *end = st->buf + st->end;
	char *b;
	char *p;
	int buf;
	char *tmp = (char*)&buf;

	buf = 0;
	b = st->buf + st->pos;
	p = b;
	if (p >= end || ((*p >= 'a' && *p <= 'z') 
			|| (*p >= 'A' && *p <= 'Z')
			|| (*p >= '0' && *p <= '9')
			|| *p == '_' || *p == '"'))
	{
		return 0;
	}
	tmp[0] = *p;
	switch (*p) {
	case '<':
		if (p[1] == '>' || p[1] == '~') {
			p++;
		}
		break;
	case '>':
		if (p[1] == '~') {
			p++;
		}
		break;
	}
	tmp[p - b] = *p;
	p++;
	st->pos += p - b;
	return buf;
}


char *str_lit(struct trip *st)
{
	char *b = st->buf + st->pos;
	char *end = st->buf + st->end;
	char *p = b;
	int l;
	if (*p == '`') {
		/* already expanded */
		while (*p) {
			p++;
			st->pos++;
		}
		while (!*p) {
			p++;
			st->pos++;
		}
		return b + 1;
		return b + 1;
	}
	if (*p != '"') {
		return NULL;
	}
	*p = '`';
	p++;
	l = 1;
	while (p < end && *p && *p != '"') {
		if (*p == '=' && (p[1] == '\n' || p[1] == '\r')) {
			p++;
			if (*p == '\r') {
				p++;
			}
		} else if (*p == '=') {
			p++;
			if (*p >= '0' && *p <= '9') {
				b[l] = (*p - '0') << 4;
			} else {
				b[l] = (*p - 'A' + 10) << 4;
			}
			p++;
			if (*p >= '0' && *p <= '9') {
				b[l] |= *p - '0';
			} else {
				b[l] |= *p - 'A' + 10;
			}
			if (st->mode == MODE_INTERP) {
			} else {
				switch (b[l]) {
				case '\n':
					b[l] = '\\';
					l++;
					b[l] = 'n';
					break;
				case '\r':
					b[l] = '\\';
					l++;
					b[l] = 'r';
					break;
				case '\\':
					b[l] = '\\';
					l++;
					b[l] = '\\';
					break;
				case '"':
					b[l] = '\\';
					l++;
					b[l] = '"';
					break;
				}
			}
			l++;
		} else {
			b[l] = *p;
			switch (b[l]) {
			case '\\':
				error("'\\' in string", st);
				break;
			}
			l++;
		}
		p++;
	}
	b[l] = 0;
	while ((b + l) < p) {
		l++;
		b[l] = 0;
	}
	p++;
	st->pos = p - st->buf;
	return b + 1;
}

int trip__delete(struct trip *st)
{
	if (st->next) {
		trip__delete(st->next);
		st->next = NULL;
	}
	while (st->nb_files > 0) {
		st->nb_files--;
		free(st->files[st->nb_files].buf);
	}
	free(st);
	return 0;
}

int const_expr(struct trip *st)
{
	int ok;
	ok = 0;
	while (st->pos < st->end && !ok) {
		switch (st->buf[st->pos]) {
		case ';':
			ok = 1;
			break;
		case '\n':
			ok = 1;
			break;
		default:
			st->pos++;
			break;
		}
	}
	return ok;
}

int expression1(struct trip *st, int prec, int spc)
{
	int ok;
	int c;
	var a, b;
	char *id;
	var *arr;

	int has_primary = 1;
	if (spc == 0) {
		has_primary = 0;
	}
	ok = 0;
	while (st->pos < st->end && !ok) {
		if (st->mode == MODE_INTERP) {
			if (st->break_ || st->else_ || 
				st->return_ || st->continue_) 
			{
				return 0;
			}
		}
		c = st->buf[st->pos];
		switch (c) {
		case ';':
		case '?':
			ok = 1;
			break;
		case '\n':
			ok = 1;
			break;
		case '#':
		case '}':
		case ']':
		case ')':
			return 1;
		case '(':
			st->pos++;
			if (st->mode == MODE_INTERP) {
				expression(st);
			} else {
				printf("(");
				expression(st);
				printf(")");
			}
			c = st->buf[st->pos];
			if (c == ')') {
				st->pos++;
			} else {
				error(") expected", st);
			}
			has_primary = 1;
			break;
	
		case '[':
			st->pos++;
			if (st->mode == MODE_INTERP) {
				arr = (var*)pop(st);
				expression(st);
				push(st, arr[pop(st)]);
			} else {
				printf("[");
				expression(st);
				printf("]");
			}
			c = st->buf[st->pos];
			if (c == ']') {
				st->pos++;
			} else {
				error("] expected", st);
			}
			has_primary = 1;
			break;
		case '{':
			if (st->mode == MODE_INTERP) {
				if (st->return_) {
					skip_body(st);
				} else {
					st->pos++;
					st->incall++;
					compound(st);
					st->incall--;
				}
			} else {
				st->pos++;
				st->incall++;
				compound(st);
				st->incall--;
			}
			c = st->buf[st->pos];
			if (c == '}') {
				st->pos++;
			} else {
				error("} expected", st);
			}
			has_primary = 1;
			break;
		case '-':
		case '+':
			if (!has_primary) {
				if (prec > 16) {
					return 0;
				}
			} else {
				if (prec > 13) {
					return 0;
				}
			}
			st->pos++;
			if (!has_primary) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf("%c", c); 
				}
				expression1(st, 16 + 1, spc);
				if (c == '-') {
					if (st->mode == MODE_INTERP) {
						push(st, -pop(st));
					}
				}
			} else {
				if (st->mode == MODE_INTERP) {
					expression1(st, 13 + 1, spc);
					b = pop(st);
					a = pop(st);
					if (c == '-') {
						push(st, a - b);
					} else {
						push(st, a + b);
					}
				} else {
					printf("%c", c); 
					expression1(st, 13 + 1, spc);
				}
			}
			break;
		case '*':
		case '/':
		case '%':
			if (prec > 14) {
				return 0;
			}
			st->pos++;
			if (st->mode == MODE_INTERP) {
				expression1(st, 14 + 1, spc);
				b = pop(st);
				a = pop(st);
				switch (c) {
				case '*':
					push(st, a * b);
					break;
				case '/':
					push(st, a / b);
					break;
				case '%':
					push(st, a % b);
					break;
				}
			} else {
				printf("%c", c); 
				expression1(st, 14 + 1, spc);
			}
			break;
		case ' ':
		case '\t':
			if (st->mode == MODE_INTERP) {
			} else {
				printf(" ");
		        }	
			spaces(st);
			break;
		case '"':
		case '`':
			id = str_lit(st);
			if (st->mode == MODE_INTERP) {
				push(st, (var)id);
			} else {
				printf("(var)\"%s\"", id);
			}
			has_primary = 1;
			break;
		case '\'':
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (st->mode == MODE_INTERP) {
				push(st, num_lit(st));
			} else {
				printf("%d", num_lit(st)); 
			}
			has_primary = 1;
			break;
		default:
			id = variable(st, 0);
			if (!id) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf("%c", c); 
				}
				st->pos++;
			} else {
				has_primary = 1;
			}
		}
		if (spc) {
			c = st->buf[st->pos];
			if (c == ' ' || c == '\t') {
				ok = 1;
			}
		}
	}
	return ok;

}

int expression(struct trip *st)
{
	int spc;

	switch (st->buf[st->pos]) {
	case ' ':
	case '\t':
		spaces(st);
		spc = 1;
		break;
	default:
		spc = 0;
	}

	return expression1(st, 0, spc);
}

int skip_expression(struct trip *st)
{
	char *p;
	int ok = 0;
	p = st->buf + st->pos;
	while (st->pos < st->end && !ok) {
		switch (*p) {
		case '\n':
		case ';':
		case '}':
			ok = 1;
			break;
		case '{':
			skip_body(st);
			break;
		case '"':
		case '`':
			str_lit(st);
			break;
		default:
			st->pos++;
		}
		p = st->buf + st->pos;
	}
	return 0;
}

void condbody_interp(struct trip *st, var cond, int is_while)
{

	whitespaces(st);
	
	if (st->break_ || st->else_ || st->continue_ || st->return_) {
		cond = 0;
	}
	if (st->pos < st->end && st->buf[st->pos] == '{') {
		if (cond) {
			st->pos++;
			compound(st);
			if (st->pos < st->end && 
				st->buf[st->pos] == '}') 
			{
				st->pos++;
			} else {
				error("expression body expected", st);
			}
		} else {
			skip_body(st);
		}
	} else {
		if (cond) {
			expression(st);
		} else {
			skip_expression(st);
		}
	}
	whitespaces(st);
	if (cond) {
		if (is_while) {
			st->continue_ = 1;
		} else {
			st->else_ = 1;
		}
	}
}

void condbody(struct trip *st)
{
	printf("{\n");
	whitespaces(st);
	
	if (st->pos < st->end && st->buf[st->pos] == '{') {
		st->pos++;
		compound(st);
		if (st->pos < st->end && st->buf[st->pos] == '}') {
			st->pos++;
			whitespaces(st);
		} else {
			error("expression body expected", st);
		}
	} else {
		expression(st);
		whitespaces(st);
	}
	
	indent(st);
	printf("}");
}

int conditional_interp(struct trip *st, int is_while)
{
	int c;
	var cond;

	while (st->pos < st->end) {
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
			return 0;
		case ',':
			if (is_while) {
				return 1;
			}
			st->pos++;
			break;
		case '<':
			st->pos++;
			c = st->buf[st->pos];
			if (c == '>') {
				cond = st->cond != 0;
				st->pos++;
			} else if (c == '~') {
				cond = st->cond <= 0;
				st->pos++;
			} else {
				cond = st->cond <= 0;
			}
			condbody_interp(st, cond, is_while);
			break;
		case '>':
			st->pos++;
			c = st->buf[st->pos];
			if (c == '~') {
				st->pos++;
				cond = st->cond >= 0;
			} else {
				cond = st->cond > 0;
			}
			condbody_interp(st, cond, is_while);
			break;
		case '~':
			st->pos++;
			cond = st->cond == 0;
			condbody_interp(st, cond, is_while);
			break;
		case '-':
			st->pos++;
			cond = st->cond == -num_lit(st);
			condbody_interp(st, cond, is_while);
			break;
		case '+':
			st->pos++;
		default:
			c = st->buf[st->pos];
			if (c == '\'' || (c >= '0' && c <= '9')) {
				cond = st->cond == num_lit(st);
			} else {
				error("cond expected", st);
			}
			condbody_interp(st, cond, is_while);
		}
		whitespaces(st);
	}
	if (is_while) {
		st->break_ = 1;
	} else {
		st->else_ = 1;
	}
	return 0;
}
	
int conditional(struct trip *st, int is_while)
{
	int ok;
	int c;
	char *f;
	char *w;
	int n = 0;
	ok = 0;
	if (st->mode == MODE_INTERP) {
		return conditional_interp(st, is_while);
	} else {
	}
	if (is_while) {
		f = "while";
		w = "__while && ";
	} else {
		f = "if";
		w = "";
	}
	st->indent++;
	while (st->pos < st->end && !ok) {
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
			st->indent--;
			if (is_while) {
				printf(" else { break; }");
			}
			return 0;
		case ',':
			if (is_while) {
				st->indent--;
				return 1;
			}
			st->pos++;
			break;
		case '<':
			if (n) {
				printf(" else ");
			} else {
				indent(st);
			}
			st->pos++;
			c = st->buf[st->pos];
			if (c == '>') {
				printf("if (%s__cond%s != 0) ", w, f);
				st->pos++;
			} else if (c == '~') {
				printf("if (%s__cond%s <= 0) ", w, f);
				st->pos++;
			} else {
				printf("if (%s__cond%s < 0) ", w, f);
			}
			condbody(st);
			n = 1;
			break;
		case '>':
			if (n) {
				printf(" else ");
			} else {
				indent(st);
			}
			st->pos++;
			c = st->buf[st->pos];
			if (c == '~') {
				st->pos++;
				printf("if (%s__cond%s >= 0) ", w, f);
			} else {
				printf("if (%s__cond%s > 0) ", w, f);
			}
			condbody(st);
			n = 1;
			break;
		case '~':
			if (n) {
				printf(" else ");
			} else {
				indent(st);
			}
			st->pos++;
			printf("if (%s__cond%s == 0) ", w, f);
			condbody(st);
			n = 1;
			break;
		case '-':
			if (n) {
				printf(" else ");
			} else {
				indent(st);
			}
			st->pos++;
			printf("if (%s__cond%s == %d) ",w,f, -num_lit(st));
			condbody(st);
			n = 1;
			break;
		case '+':
			st->pos++;
		default:
			c = st->buf[st->pos];
			if (c == '\'' || (c >= '0' && c <= '9')) {
				if (n) {
					printf(" else ");
				} else {
					indent(st);
				}
				printf("if (%s__cond%s == %d) ", w, f, num_lit(st));
			} else {
				error("cond expected", st);
			}
			condbody(st);
			n = 1;
		}
		whitespaces(st);
	}
	st->indent--;
	return 0;
}
	
void k_define(struct trip *st)
{
	char *s;
	char *id;
	char *val;
	int n;
	int l;
	int i;

	st->pos += 6;
	whitespaces(st);
	id = identifier(st, &l);
	if (id) {
		whitespaces(st);
		for (i = 0; i < st->nb_defs; i++) {
			if (!id_cmp(st->defs[i].name, id)) {
				skip(st);
				return;	
			}
		}
		if (st->mode == MODE_INTERP) {
		} else {
			printf("#define %s ", id_tmp(st,id));
		}
		s = str_lit(st);
		if (st->nb_defs >= MAX_MEMBER) {
			error("too many define", st);
		}
		st->defs[st->nb_defs].name = id_dup(id);
		if (s) {
			if (st->mode == MODE_INTERP) {
				st->defs[st->nb_defs].data = (var)id_dup(s);
			} else {
				printf("(var)\"%s\"\n", s);
			}
		} else {
			val = identifier(st, &l);
			if (val) {
				if (st->mode == MODE_INTERP) {
					st->defs[st->nb_defs].data = 
						(var)id_dup(val);
				} else {
					printf("%s\n", val);
				}
			} else {
				n = num_lit(st);
				if (st->mode == MODE_INTERP) {
					st->defs[st->nb_defs].data = n;
				} else {
					printf("%d\n", n);
				}
			}
		}
		st->nb_defs++;
	}
	end_of_expr(st);
}


void k_class(struct trip *st)
{
	char *s;
	int op;
	int l;
	st->pos += 5;
	whitespaces(st);
	s = identifier(st, &l);
	whitespaces(st);
	op = operator(st);
	if (op != '{') {
		error("{", st);
	}
	s[l] = 0;
	st->class_name = s;
	st->func_name = NULL;
	if (st->mode == MODE_INTERP) {
	} else {
		printf("struct %s {\n", id_tmp(st, st->class_name));
	}
	whitespaces(st);
}

void parameters(struct trip *st, int n)
{
	char *s;
	int op;
	int l;

	whitespaces(st);
	op = operator(st);
	while (op != ')') {
		whitespaces(st);
		s = identifier(st, &l);
		if (n > 0) {
			printf(", ");
		}
		n++;
		printf("var %s", id_tmp(st,s));
		if (st->nb_vars >= MAX_MEMBER) {
			error("too many parmeters", st);
		}
		st->vars[st->nb_vars].name = s;
		st->nb_vars++;
		whitespaces(st);
		op = operator(st);
	}
}

void k_func(struct trip *st)
{
	char *s;
	int op;
	int l;
	
	st->nb_refs = 0;
	st->nb_vars = 0;
	st->pos += 4;
	whitespaces(st);
	s = identifier(st, &l);
	whitespaces(st);
	op = operator(st);
	if (op == '(') {
		st->func_name = s;
		printf("var %s(", id_tmp(st, st->func_name));
		parameters(st, 0);
		whitespaces(st);
		op = operator(st);
		if (op == '{') {
			printf(")\n{\n");
			compound(st);
		}
		if (!st->returnn) {
			printf("\treturn 0;\n");
		}
		st->returnn = 0;
		printf("}\n");
		if (st->buf[st->pos] == '}') {
			st->pos++;
			whitespaces(st);
			st->func_name = NULL;
			st->return_ = 1;
			return;
		}
	}
	error("in function decl", st);
	
}

void k_method(struct trip *st)
{
	char *s;
	int op;
	int l;
	st->nb_refs = 0;
	st->nb_vars = 0;
	st->pos += 6;
	if (st->func_name == NULL) {
		printf("};\n");
	}
	whitespaces(st);
	s = identifier(st, &l);
	whitespaces(st);
	op = operator(st);
	if (op == '(') {
		s[l] = 0;
		st->func_name = s;
		printf("var %s__",id_tmp(st,st->class_name));
		printf("%s(var self", id_tmp(st,st->func_name));
		whitespaces(st);
		parameters(st, 1);
		whitespaces(st);
		op = operator(st);
		if (op == '{') {
			printf(")\n{\n");
			printf("\tstruct %s *__self = (void*)self;(void)__self;\n", 
					id_tmp(st,st->class_name));
			compound(st);
		}
		if (!st->returnn) {
			printf("\treturn 0;\n");
		}
		st->returnn = 0;
		printf("}\n");
		if (st->buf[st->pos] == '}') {
			st->pos++;
			whitespaces(st);
			if (st->buf[st->pos] == '}') {
				st->func_name = NULL;
				st->class_name = NULL;
				st->pos++;
				whitespaces(st);
			}
			st->return_ = 1;
			return;
		}
	}
	error("in method decl", st);
	
}

void k_field(struct trip *st)
{
	char *id;
	int l;
	st->pos += 5;
	whitespaces(st);
	id = identifier(st, &l);
	end_of_expr(st);
	id[l] = 0;
	printf("\tvar %s;\n", id);
}

void k_set(struct trip *st)
{
	int i;
	var *arr;
	var v;
	st->pos += 3;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
	} else {
		indent(st);
	}
	variable(st, 1);
	if (st->mode == MODE_INTERP) {
	} else {
		printf(" = ");
		i = st->indent;
		st->indent = -1;
	}
	whitespaces(st);
	expression(st);
	if (st->mode == MODE_INTERP) {
		v = pop(st);
		arr = (var*)pop(st);
		arr[0] = v;
	} else {
		st->indent = i;
		printf(";\n");
	}
}

void k_delete(struct trip *st)
{
	char *id;
	int l;
	st->pos += 6;
	whitespaces(st);
	id = identifier(st, &l);
	end_of_expr(st);
	id[l] = 0;
	indent(st);
	if (!id_cmp(id, "this")) {
		if (st->mode == MODE_INTERP) {
			free((void*)st->__self);
		} else {
			printf("free((void*)self);\n");
		}
	} else {
		if (st->mode == MODE_INTERP) {
			free((void*)get_data(st, NULL, id, 0));
		} else {
			printf("free((void*)%s);\n", id);
		}
	}
}

void k_var(struct trip *st)
{
	char *id;
	int l;
	int n = 0;
	st->pos += 3;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
	} else {
		printf("\tvar ");
	}
	id = identifier(st, &l);
	whitespaces(st);
	while (st->pos < st->end && st->buf[st->pos] == ',') {
		if (st->mode == MODE_INTERP) {
		} else {
			if (n) {
				printf(", %s", id_tmp(st,id));
			} else {
				printf(" %s", id_tmp(st,id));
			}
		}
		n++;
		if (st->nb_vars >= MAX_MEMBER) {
			error("too many var", st);
		}
		st->vars[st->nb_vars].name = id;
		st->nb_vars++;
		st->pos++;
		whitespaces(st);
		id = identifier(st, &l);
	}
	end_of_expr(st);
	if (st->nb_vars >= MAX_MEMBER) {
		error("too many var", st);
	}
	st->vars[st->nb_vars].name = id;
	st->nb_vars++;
	if (st->mode == MODE_INTERP) {
	} else {
		if (n) {
			printf(", %s;\n", id_tmp(st,id));
		} else {
			printf(" %s;\n", id_tmp(st,id));
		}
	}
}

void k_ref(struct trip *st)
{
	char *id;
	int l;
	if (st->nb_refs >= MAX_MEMBER) {
		error("too many ref", st);
	}

	st->pos += 3;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
	} else {
		printf("\tstruct %s *", id_tmp(st,id));
	}
	st->refs[st->nb_refs].value = id;
	id = identifier(st, &l);
	end_of_expr(st);
	if (st->mode == MODE_INTERP) {
	} else {
		printf("%s;\n", id_tmp(st,id));
	}
	st->refs[st->nb_refs].name = id;
	st->nb_refs++;
}

void k_poke(struct trip *st)
{
	char *id;
	int l;
	char *arr;

	st->pos += 4;
	whitespaces(st);
	id = identifier(st, &l);
	indent(st);
	if (st->mode == MODE_INTERP) {
		arr = (char*)get_data(st, NULL, id, 0);
	} else {
		printf("((unsigned char*)%s)[", id_tmp(st,id));
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		arr = arr + pop(st);
	} else {
		printf("] = ");
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		arr[0] = pop(st);
	} else {
		printf(";\n");
	}
	end_of_expr(st);
}

void k_peek(struct trip *st)
{
	char *id;
	int l;
	char *arr;

	st->pos += 4;
	whitespaces(st);
	id = identifier(st, &l);
	if (st->mode == MODE_INTERP) {
		arr = (char*)get_data(st, NULL, id, 0);
	} else {
		printf("((unsigned char*)%s)[", id_tmp(st,id));
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		push(st, ((var)arr[pop(st)]) & 0xFF);
	} else {
		printf("]");
	}
	end_of_expr(st);
}

void k_bytes(struct trip *st)
{
	char *id;
	int l;
	int op;
	int n;
	char *arr;
	int pos;

	st->pos += 5;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	if (operator(st) != '{') {
		error("{ expected", st);
	}
	if (st->nb_global >= MAX_MEMBER) {
		error("too many global", st);
	}
	if (st->mode == MODE_INTERP) {
	} else {
		printf("unsigned char %s[] = {\n", id_tmp(st,id));
	}
	st->global[st->nb_global].name = id;

	whitespaces(st);
	n = 0;
	pos = st->pos;
	if (st->mode == MODE_INTERP) {
		n = 1;
		while (st->pos < st->end && st->buf[st->pos] != '}') {
			if (st->buf[st->pos] == ',') {
				n++;
			}
			st->pos++;	
		}
		arr = malloc(n+1);
		st->global[st->nb_global].data = (var)arr;
	}
	st->nb_global++;
	st->pos = pos;
	n = 0;
	while (st->pos < st->end && st->buf[st->pos] != '}') {
		if (st->mode == MODE_INTERP) {
			arr[n] = num_lit(st) & 0xFF;
			n++;
		} else {
			printf("%d", num_lit(st));
		}
		whitespaces(st);
		op = operator(st);
		whitespaces(st);
		if (op == ',') {
			if (st->mode == MODE_INTERP) {
			} else {
				printf(",");
			}
		} else if (op == '}') {
			break;
		} else {
			error(", expected", st);

		}
	}
	if (st->mode == MODE_INTERP) {
		arr[n] = 0;
	} else {
		printf("};\n");
	}
}


void k_array(struct trip *st)
{
	char *id;
	int l;
	int op;
	int old;
	int n;
	var *arr;

	st->pos += 5;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	if (operator(st) != '{') {
		error("{ expected", st);
	}
	if (st->nb_global >= MAX_MEMBER) {
		error("too many global", st);
	}
	st->global[st->nb_global].name = id;
	
	whitespaces(st);
	old = st->pos;
	if (st->mode == MODE_INTERP) {
		n = 1;
		while (st->pos < st->end && st->buf[st->pos] != '}') {
			if (st->buf[st->pos] == ',') {
				n++;
			}	
			st->pos++;
		}
		arr = malloc(sizeof(var) * n);
		st->global[st->nb_global].data = (var)arr;
	} else {
		printf("var %s[] = {\n", id_tmp(st, id));
	}
	n = 0;
	st->pos = old;
	while (st->pos < st->end && st->buf[st->pos] != '}') {
		if (st->mode == MODE_INTERP) {
			arr[n] = num_lit(st);
		} else {
			printf("%d", num_lit(st));
		}
		n++;
		whitespaces(st);
		op = operator(st);
		whitespaces(st);
		if (op == ',') {
			if (st->mode == MODE_INTERP) {
			} else {
				printf(",");
			}
		} else if (op == '}') {
			break;
		} else {
			error(", expected", st);

		}
	}
	if (st->mode == MODE_INTERP) {
	} else {
		printf("};\n");
	}
	st->nb_global++;
}

void k_new(struct trip *st)
{
	char *id;
	int l;
	st->pos += 3;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
	}
	id = identifier(st, &l);
	whitespaces(st);
	if (!id_cmp(id, "array")) {
		if (st->mode == MODE_INTERP) {
			push(st, (var)malloc(sizeof(var) * num_lit(st)));
		} else {
			printf("(var)malloc(sizeof(var) * %d)", num_lit(st));
		}
	} else if (!id_cmp(id, "bytes")) {
		if (st->mode == MODE_INTERP) {
			push(st, (var)malloc(num_lit(st)));
		} else {
			printf("(var)malloc(%d)", num_lit(st));
		}
	} else {
		if (st->mode == MODE_INTERP) {
			push(st, (var)malloc(size_of(st, id)));
		} else {
			printf("(var)malloc(sizeof(struct %s))", id_tmp(st,id));
		}
	}
	end_of_expr(st);
}


void k_if(struct trip *st)
{
	int sp;
	var cond;
	var else_;

	st->pos += 2;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
		sp = st->sp;
	} else {
		indent(st);
		printf("{\n");
		indent(st);
		printf("\tvar __condif = ");
	}
	expression(st);
	st->pos++;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
		if ((sp + 1)!= st->sp) {
			error("missing condition", st);
		}
		cond = st->cond;
		else_ = st->else_;
		st->cond = pop(st);
		conditional(st,0);
		st->cond = cond;
		st->else_ = else_;
	} else {
		printf(";\n");
		conditional(st,0);
		printf("\n");
		indent(st);
		printf("}\n");
	}
	end_of_expr(st);
}

void k_while(struct trip *st)
{
	int sp;
	int pos;
	var cond;

	st->pos += 5;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
		sp = st->sp;
	} else {
		indent(st);
		printf("while (1) {");
		indent(st);
		printf("\tvar __while = 1; var __condwhile = ");
	}

	if (st->mode == MODE_INTERP) {
		pos = st->pos;
		while (!st->break_ && !st->else_ &&
			!st->return_ && !st->continue_) 
		{
			expression(st);
			st->pos++;
			whitespaces(st);
			if ((sp + 1) != st->sp) {
				error("missing condition.", st);
			}
			cond = st->cond;
			st->cond = pop(st);
			conditional(st,1);
			st->cond = cond;
			if (st->break_ || 
				st->buf[st->pos] == ';' ||
				st->buf[st->pos] == ',') 
			{
				if (!st->continue_) {
					break;	
				}
			}
			if (st->sp > sp) {
				st->sp = sp;
			}
			st->continue_ = 0;
			st->pos = pos;
		}
		if (st->continue_ || st->break_ || st->else_ ||
				st->return_) 
		{
		printf("BBBB %d\n", st->pos);
			st->pos = pos;
			skip_expression(st);
			whitespaces(st);
			if (st->buf[st->pos] == '{') {
				skip_body(st);
			}
			st->continue_ = 0;
			st->break_ = 0;
		printf("BBBBii %d %c\n", st->pos, st->buf[st->pos]);
		}
	} else {
		expression(st);
		st->pos++;
		whitespaces(st);
		printf(";\n");
		conditional(st,1);
		printf("\n");
		indent(st);
		printf("}\n");
	}
	end_of_expr(st);
}

void k_return(struct trip *st)
{
	int op;
	st->pos += 6;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
	} else {
		indent(st);
		printf("return ");
	}
	op = st->pos;
	expression(st);
	if (st->mode == MODE_INTERP) {
		if (op != st->pos) {
			st->return_val = pop(st);
		} else {
			st->return_val = 0;
		}
		st->return_ = 1;
	} else {
		printf(";\n");
	}
	whitespaces(st);
	st->returnn = 1;
}

void k_load(struct trip *st, char *s)
{
	int len;
	char *buf;
	char *obuf;
	int opos;
	int oend;
	char *ofunc_name;
	char *ofile_name;
	char *oclass_name;

	len = strlen(s) + 2;
	if (st->nb_files > 0) {
		len += strlen(st->file_name);
	}
	buf = malloc(len);
	buf[0] = 0;
	if (st->nb_files > 0) {
		strcat(buf, st->file_name);
	}
	len = strlen(buf);
	while (len > 0) {
		len--;
		if (buf[len] == '/') {
			break;
		}
	}
	if (buf[len] == '/') {
		buf[len+1] = 0;
	}
	strcat(buf, s);
	opos = st->pos;
	obuf = st->buf;
	oend = st->end;
	st->pos = 0;
	st->end = file_size((var)buf);
	if (st->end < 1 || st->nb_files >= MAX_MEMBER) {
		error("caonot open file", st);
	}
	st->buf = (char*)file_load((var)buf, 0, st->end);
	if (st->nb_files < 1) {
		st->files[st->nb_files].start = 0; 
	} else {
		st->files[st->nb_files].start =  
			st->files[st->nb_files-1].start +   
			st->files[st->nb_files-1].end;   
	}
	st->files[st->nb_files].end = st->end; 
	st->files[st->nb_files].buf = st->buf; 
	st->files[st->nb_files].file_name = buf;
	st->nb_files++;

	ofunc_name =  st->func_name;
	ofile_name =  st->file_name;
	oclass_name = st->class_name;
	st->func_name = NULL;
	st->file_name = buf;
	st->class_name = NULL;
	declare(st);
	if (obuf) {
		st->file_name =  ofile_name;
		st->func_name =  ofunc_name;
		st->class_name = oclass_name;
		st->pos = opos;
		st->buf = obuf;
		st->end = oend;
	}
}

void k_include(struct trip *st)
{
	char *s;
	char *p;
	char o;
	st->pos += 7;
	whitespaces(st);
	s = str_lit(st);
	if (strstr(s, "std.3p")) {
		p = s +strlen(s)-2;
		o = *p;
		*p = 0;
		if (st->mode == MODE_INTERP) {
		} else {
			printf("#include \"%sh\"\n", s);
		}
		*p = o;
		return;
	}
	k_load(st, s);
}

var (*builtin(struct trip *st, char *id, int *argc))()
{
	switch(id[0]) {
	case 'f':
		if (!id_cmp(id, "file_size")) {
			*argc = 1;
			return (var(*)())file_size;
		} else if (!id_cmp(id, "file_load")) {
			*argc = 3;
			return (var(*)())file_load;
		} else if (!id_cmp(id, "file_save")) {
			*argc = 4;
			return (var(*)())file_save;
		}
		break;
	case 'p':
		if (!id_cmp(id, "print")) {
			*argc = 1;
			return (var(*)())print;
		} else if (!id_cmp(id, "print10")) {
			*argc = 1;
			return (var(*)())print10;
		}
		break;
	case 'q':
		if (!id_cmp(id, "quit")) {
			*argc = 0;
			return (var(*)())quit;
		}
	case 's':
		if (!id_cmp(id, "str_cmp")) {
			*argc = 2;
			return (var(*)())str_cmp;
		} else if (!id_cmp(id, "str_dup")) {
			*argc = 1;
			return (var(*)())str_dup;
		}
		break;
	case 't':
		if (!id_cmp(id, "term_wait")) {
			*argc = 2;
			return (var(*)())term_wait;
		} else if (!id_cmp(id, "term_init")) {
			*argc = 1;
			return (var(*)())term_init;
		} else if (!id_cmp(id, "term_deinit")) {
			*argc = 0;
			return (var(*)())term_deinit;
		} else if (!id_cmp(id, "term_size")) {
			*argc = 1;
			return (var(*)())term_size;
		}
		break;
	}
	return (var(*)())NULL;
}


void call_builtin(struct trip *st, var(*built)())
{
	switch (st->nb_vars) {
	case 1:
		st->return_val = built(st->vars[0].data);
		break;
	case 2:
		st->return_val = built(st->vars[0].data, st->vars[1].data);
		break;
	case 3:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data);
		break;
	case 4:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data);
		break;		
	case 5:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data,
					st->vars[4].data);
		break;
	case 6:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data,
					st->vars[4].data,
					st->vars[5].data);
		break;
	case 7:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data,
					st->vars[4].data,
					st->vars[5].data,
					st->vars[6].data);
		break;
	case 8:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data,
					st->vars[4].data,
					st->vars[5].data,
					st->vars[6].data,
					st->vars[7].data);
		break;
	case 9:
		st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data,
					st->vars[3].data,
					st->vars[4].data,
					st->vars[5].data,
					st->vars[6].data,
					st->vars[7].data,
					st->vars[9].data);
		break;
	default:
		st->return_val = built();
	}
}

void call_exec(struct trip *st, char *func, char *id, var self, char *clas, char *meth, int is_v)
{
	int l;
	int end;
	int opos;
	int oend;
	char *obuf;
	int vr;
	char *p;
	vr = 0;
	opos = st->pos;
	obuf = st->buf;
	oend = st->end;

	fix_pos_buf(st,func);
	whitespaces(st);
	p = st->buf + st->pos;
	if (*p != '(') {
		error("invalid function call", st);
	}
	st->pos++;
	end = 0;
	if (!self && func && !clas && !meth && is_v) {
		self = st->vars[0].data;
		st->vars[0].name = "self";
		vr++;
	}
	while (st->pos < st->end && !end) {
		whitespaces(st);
		p = st->buf + st->pos;
		switch (*p) {
		case ',':
			st->pos++;
			break;
		case ')':
			st->pos++;
			end = 1;
			break;
		default:
			id = identifier(st, &l);
			if (!id) {
				error("param expected", st);
			} else if (vr >= st->nb_vars) {
				error("not enough arguments", st);
			}
			st->vars[vr].name = id;
			vr++;
		}
	}
	whitespaces(st);
	if (st->buf[st->pos] != '{') {
		error("invalid function body call", st);
	} else if (vr < st->nb_vars) {
		error("too many arguments", st);
	}
	st->pos++;
	st->__self = self;
	st->class_name = clas;
	if (meth) {
		st->func_name = meth;
	} else {
		st->func_name = id;
	}
	compound(st);
	st->pos = opos;
	st->buf = obuf;
	st->end = oend;
}

void call(struct trip *st)
{
	char *p;
	char *id;
	char *meth = NULL;
	char *clas;
	int l;
	int c;
	int end;
	int n = 0;
	char *func = NULL;
	var self = 0;
	int sp;
	var (*built)() = NULL;
	int nb_args = 0;
	int i;
	int is_v = 0;

	whitespaces(st);
	id = identifier(st, &l);
	c = st->buf[st->pos];
	if (!id) {
		error("identifier expected..", st);
		return;
	}
	sp = st->sp;
	clas = NULL;
	if (st->mode == MODE_INTERP) {
	} else {
		if (st->indent == 2) {
			printf("\t");
		}
	}
	if (c == '.') {
		st->pos++;
		whitespaces(st);
		meth = identifier(st, &l);
		c = st->buf[st->pos];
		clas = get_class(st, id);
		
		if (st->mode == MODE_INTERP) {
			if (clas == NULL) {
				self = st->__self;
				func = get_member_ptr(st,	
					st->class_name, "method", meth);
			} else {
				self = get_data(st, NULL, id, 0);
				func = get_member_ptr(st, clas, "method", meth);
			}
		} else {
			if (clas == NULL) {
				printf("((var(*)())%s", 
						id_tmp(st,st->class_name));
				printf("__%s)(self", id_tmp(st,meth));
			} else {
				printf("((var(*)())%s", id_tmp(st,clas));
				printf("__%s", id_tmp(st,meth));
				printf(")((var)%s", id_tmp(st,id));
			}
		}
		n = 1;
	} else {
		if (st->mode == MODE_INTERP) {
			built = builtin(st, id, &nb_args);
			if (!built) {
				func = (char*)get_data(st, NULL, id, 0);
			}
		} else {
			printf("((var(*)())%s)(", id_tmp(st,id));
		}
		is_v = is_var(st, id);
	}
	end = 0;
	while (st->pos < st->end && !end) {
		p = st->buf + st->pos;
		switch (*p) {
		case '\r':
		case '}':
		case '\n':
		case ';':
		case '#':
			end = 1;
			break;
		case '{':
			if (n > 0) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf(", ");
				}
			}
			st->pos++;
			st->indent = 0;
			st->incall++;
			compound(st);
			st->incall--;
			p = st->buf + st->pos;
			if (*p != '}') {
				error("} !", st);
			}
			st->pos++;
			n++;
			break;
		case ' ':
		case '\t':
			if (n > 0 && !is_eol(st)) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf(", ");
				}
			}
			n++;
		default:
			expression(st);
		}
	}

	p = st->buf + st->pos;

	if (st->mode == MODE_INTERP) {
		push_context(st);
		i = st->sp - sp;
		while (sp < st->sp) {
			i--;
			st->vars[i].name = "";
			st->vars[i].data = pop(st);
			st->nb_vars++;
		}
	} else {
	}

	end_of_expr(st);
	if (st->mode == MODE_INTERP) {
		if (built) {
			if (nb_args != st->nb_vars) {
				error("wrong number of args", st);
			}
			call_builtin(st, built);
		} else {
			call_exec(st, func, id, self, clas, meth, is_v);
		}
	} else {
		if (st->incall == 0) {
			printf(");\n");
		} else {
			printf(")");
		}
	}
	st->return_ = 0;
	if (st->mode == MODE_INTERP) {
		st->sp = sp;
		pop_context(st);
	} else {
	}
}

void skip_param(struct trip *st)
{
	whitespaces(st);
	if (st->buf[st->pos] != '(') {
		error("( mess", st);
	}
	st->pos++;
	while (st->pos < st->end) {
		switch (st->buf[st->pos]) {
		case ')':
			st->pos++;
			return;
		default:
			st->pos++;
		}
	}
}


void skip_body_end(struct trip *st)
{
	while (st->pos < st->end) {
		switch (st->buf[st->pos]) {
		case '"':
		case '`':
			str_lit(st);
			break;
		case '\'':
			num_lit(st);
			break;
		case '{':
			skip_body(st);
			break;
		case '}':
			st->pos++;		
			return;
		case '#':
			comment(st);
			break;
		default:
			st->pos++;
		}
	}
}

void skip_body(struct trip *st)
{
	whitespaces(st);
	if (st->buf[st->pos] != '{') {
		error("{ mess", st);
	}
	st->pos++;
	skip_body_end(st);
}


void check_main(struct trip *st)
{
	if (!st->main && !st->class_name && !st->func_name) {
		st->indent = 1;
		st->__self = 0;
		st->func_name = "main";
		if (st->mode == MODE_INTERP) {
			
		} else {
			printf("int main(int argc,"
				 " char *argv[]) {\n");
		}
		st->main = 1;
	}
}

void compound(struct trip *st)
{
	char *p;
	int l;
	int k;
	whitespaces(st);
	st->indent++;
	while (st->pos < st->end) {
		if (st->mode == MODE_INTERP) {
			if (st->break_ || st->else_ ||
				st->continue_ || st->return_) 
			{
				skip_body_end(st);
				st->pos--;
				return;
			}
		} else {
		}
		p = st->buf + st->pos;
		l = st->pos;
		switch (*p) {
		case '}':
			st->indent--;
			return;
		case 'a':
			if (is(st, p, "array")) {
				st->pos += 5;
				whitespaces(st);
				identifier(st, &k);	
				skip_body(st);
			}
			break;
		case 'b':
			if (is(st, p, "bytes")) {
				st->pos += 5;	
				whitespaces(st);
				identifier(st, &k);	
				skip_body(st);
			}
			break;
		case 'c':
			if (is(st, p, "class")) {
				st->pos += 5;	
				whitespaces(st);
				identifier(st, &k);	
				skip_body(st);
			}
			break;
		case 'd':
			if (is(st, p, "define")) {
				k_define(st);
			} else if (is(st, p, "delete")) {
				k_delete(st);
			}
			break;
		case 'f':
			if (is(st, p, "func")) {
				st->pos += 4;	
				whitespaces(st);
				identifier(st, &k);
				skip_param(st);	
				skip_body(st);
			} else if (is(st, p, "field")) {
				skip(st);
			}
			break;
		case 'i':
			if (is(st, p, "include")) {
				skip(st);
			} else if (is(st, p, "if")) {
				k_if(st);
			}
			break;
		case 'm':
			if (is(st, p, "method")) {
				st->pos += 6;	
				whitespaces(st);
				identifier(st, &k);	
				skip_param(st);	
				skip_body(st);
			}
			break;
		case 'n':
			if (is(st, p, "new")) {
				k_new(st);
			}
			break;
		case 'p':
			if (is(st, p, "peek")) {
				k_peek(st);
			} else if (is(st, p, "poke")) {
				k_poke(st);
			}
			break;
		case 'r':
			if (is(st, p, "return")) {
				k_return(st);
			} else if (is(st, p, "ref")) {
				check_main(st);
				k_ref(st);
			}
			break;
		case 's':
			if (is(st, p, "set")) {
				k_set(st);
			}
			break;
		case 'v':
			if (is(st, p, "var")) {
				check_main(st);
				k_var(st);
			}
			break;
		case 'w':
			if (is(st, p, "while")) {
				k_while(st);
			}
			break;
		case '#':
			comment(st);
			break;
		case ';':
			st->pos++;
			break;
		}
		if (l == st->pos) {
			check_main(st);
			call(st);
			push(st, st->return_val);
			if (l == st->pos) {
				error("call expected", st);
			}
		} else {
			whitespaces(st);
		}
	}
	st->indent--;
}

void implement(struct trip *st, int n, struct cell *c, char *type)
{

	int i;
	for (i = 0; i < n; i++) {
		st->func_name = c[i].name;
		fix_pos_buf(st, (void*)c[i].data);
		st->pos -= id_len(st->func_name);
		while (id_cmp(type, st->buf + st->pos)) {
			st->pos--;
		}
		if (type[0] == 'f') {
			k_func(st);
		} else {
			k_method(st);
		}
	}
}

void implement_class(struct trip *st, struct class_cell *c)
{
	fix_pos_buf(st, (void*)c->data);
	st->class_name = c->name;
	st->pos -= id_len(st->class_name);
	while (id_cmp("class", st->buf + st->pos)) {
		st->pos--;
	}
	compound(st);
	implement(st, c->nb_methods, c->methods, "method");	
}

int main(int argc, char *argv[]) 
{
	struct trip *st;
	var args[MAX_MEMBER+1];
	var nb_args;
	int i;

	if (argc > 2) {
		st = trip__new();
		if (!st) {
			return -1;
		}
		switch (argv[1][1]) {
		case 'c':
			printf("typedef long var;\n");
			st->mode = MODE_C;
			break;
		default:
			st->mode = MODE_INTERP;
		}
		k_load(st, argv[2]);
		st->last = st;
		st->func_name = NULL;
		st->class_name = NULL;
		st->ctx = NULL;
		if (st) {
			if (st->mode == MODE_C) {
				for (i = 0; i < st->nb_files; i++) {
					st->buf = st->files[i].buf;
					st->end = st->files[i].end;
					st->file_name = st->files[i].file_name;
					st->func_name = NULL;
					st->class_name = NULL;
					st->__self = 0;
					st->pos = 0;
					compound(st);
				}
				if (st->func_name) {
					printf("\treturn 0;\n}\n");
				}
				st->func_name = NULL;
				st->class_name = NULL;
				st->__self = 0;
				st->pos = 0;
				implement(st, st->nb_funcs, st->funcs, "func");
				for (i = 0; i < st->nb_classes; i++) {
					implement_class(st, 
						st->classes + i);
				}
			}
			if (st->func_name) {
				if (st->mode == MODE_C) {
					printf("\treturn 0;\n}\n");
				}
			}
			if (!st->main) {
				if (st->mode == MODE_INTERP) {
				} else if (st->mode == MODE_C) {
					printf("int main(int argc, char *argv[]) {\n");
					printf("\treturn startup((var)argc,(var)argv);\n");
					printf("}\n");
				}
			}
			if (st->mode == MODE_INTERP) {
				nb_args = 0;
				st->pos = 0;
				while (nb_args < (argc - 2)) {
					args[nb_args] = (var)argv[nb_args+2];
					nb_args++;
				}
				st->vars[0].name = "argc";
				st->vars[0].value = NULL;
				st->vars[0].data = nb_args;
				st->vars[1].name = "argv";
				st->vars[1].value = NULL;
				st->vars[1].data = (var)args;
				st->nb_vars = 2;
				st->line = 1;
				return (int)run_it(st, NULL, "startup");
			}
			trip__delete(st);
		}
	}
	return 0;
}

