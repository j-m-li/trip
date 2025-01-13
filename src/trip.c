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

#define MAX_MEMBER 127

struct pair {
	char *name;
	char *value;
};

struct trip {
	struct trip *parent;
	struct trip *next;
	struct trip *last;
	char *buf;
	int pos;
	int end;
	int retok;
	int state;
	int depth;
	int line;
	int indent;
	int replaced;
	char *file;
	char *vars[MAX_MEMBER+1];
	int nb_vars;
	char *global[MAX_MEMBER+1];
	int nb_global;
	struct pair refs[MAX_MEMBER+1];
	int nb_refs;
	char *defs[MAX_MEMBER+1];
	int nb_defs;
	int main;
	int returnn;

	char *func_name;
	char *class_name;
	char tmp[128];
	char tmp_var[512];
};

struct trip *load(char *file);
void compound(struct trip *st);
void spaces(struct trip *st);
int expression(struct trip *st);
char *identifier(struct trip *st, int *l);

int file_size(char *path)
{
	FILE *fp;
	int si;
	fp = fopen(path, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	si = ftell(fp);
	fclose(fp);
	return si;
}

char *file_load(char *path, int size)
{
	char *buf;
	FILE *fp;
	int ret;

	fp = fopen(path, "rb");
	if (!fp) {
		return 0;
	}
	buf = malloc(size+1);
	if (!buf) {
		return 0;
	}
	ret = fread(buf, 1, size, fp);
	if (ret != size) {
		free(buf);
		buf = 0;
	}
	buf[size] = '\0';
	fclose(fp);
	return buf;
}

int error(char *txt, struct trip *st)
{
	printf("\n#error \"%s @ line %d in %s\"\n", txt, st->line, st->file);
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
	if (st->class_name || st->func_name) {
		for (i = 1; i < st->indent; i++) {
			printf("\t");
		}
		return;
	}
	st->indent = 0;
	st->func_name = "main";
	printf("int main(int argc, char *argv[]) {\n");
	st->main = 1;
}


char *get_class(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_refs; i++) {
		if (!strcmp(st->refs[i].name, name)) {
			return st->refs[i].value;
		}
	}
	return NULL;
}

int is_var(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_vars; i++) {
		if (!strcmp(st->vars[i], name)) {
			return 1;
		}
	}
	return 0;
}

int is_def(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_defs; i++) {
		if (!strcmp(st->defs[i], name)) {
			return 1;
		}
	}
	return 0;
}

int is_global(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_global; i++) {
		if (!strcmp(st->global[i], name)) {
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
			printf(" %c \n",
		st->buf[st->pos]);
			error("new line or ';' expected", st);
			return 0;
		}
	}
	switch (st->buf[st->pos]) {
	case '#':
		comment(st);
	}
	return 1;
}


int string_len(char *b)
{
	int e = b[0];
	int i;
	i = 1;
	while (b[i] != e) {
		if (b[i] == '\\') {
			i++;
		}
		if (i > 1020) {
			printf("#error string too long\n");
		}
		i++;
	}
	i++;
	return i;
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

int printsub(char *name, int len)
{
	int o;
	o = name[len];
	name[len] = '\0';
	printf("%s", name);
	name[len] = o;
	return 0;
}

void declare(struct trip *st)
{
	char *p;
	char *id;
	int l;
	int o;
	char *cid;
	int cl;
	int co;

	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "class")) {
			st->pos += 5;
			whitespaces(st);
			cid = identifier(st, &cl);
			co = cid[cl];
			cid[cl] = 0;
			printf("struct %s;\n", cid);
			cid[cl] = co;
		} else if (is(st, p, "func")) {
			st->pos += 4;
			whitespaces(st);
			id = identifier(st, &l);
			o = id[l];
			id[l] = 0;
			printf("var %s();\n", id);
			id[l] = o;
		} else if (is(st, p, "method")) {
			st->pos += 6;
			whitespaces(st);
			id = identifier(st, &l);
			o = id[l];
			id[l] = 0;
			cid[cl] = 0;
			printf("var %s__%s();\n", cid, id);
			id[l] = o;
			cid[cl] = co;
	
		} else {
			while (*p != '\n') {
				if (*p == '\\' && p[1] == '\n') {
					st->pos++;
				}
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = 0;

}
struct trip *load(char *file) 
{
	struct trip *st;
	st = malloc(sizeof(*st));
	st->line = 1;
	st->end = file_size(file);
	st->buf = file_load(file, st->end);
	st->state = 0;
	st->pos = 0;
	st->indent = 0;
	st->file = strdup(file);
	st->parent = 0;
	st->next = 0;
	st->last = 0;
	st->nb_vars = 0;
	st->nb_defs = 0;
	st->nb_global = 0;
	st->main = 0;
	st->returnn = 0;
	if (!st->buf) {
		free(st);
		st = NULL;
	}
	declare(st);
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
		if (p[0] == '\\') {
			p++;
			switch(*p) {
			case 'n': r = '\n'; break;
			case 'r': r = '\r'; break;
			case 't': r = '\t'; break;
			case 'v': r = '\v'; break;
			case 'b': r = '\b'; break;
			case 'f': r = '\f'; break;
			case 'a': r = '\a'; break;
			default:
				r = *p;
			}
		} else {
			r = *p;
		}
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

char *variable(struct trip *st)
{
	char *xid;
	char *id;
	int l;
	int o;
	int x;
	int ox;
	char *sel;
	char *cls;
	sel = "";
	id = identifier(st, &l);
	if (!id) {
		return NULL;
	}
	o = id[l];
	if (st->buf[st->pos] == '[') {
		id[l] = 0;
		if (!is_var(st, id) && !get_class(st, id) 
				&& !is_def(st, id) && !is_global(st, id)) 
		{
			sel = "__self->";
		}
		printf("((var*)(%s%s))[", sel, id);
		id[l] = o;
		st->pos++;
		expression(st);
		if (st->buf[st->pos] != ']') {
			error("] !", st);
		}
		printf("]");
		st->pos++;
	} else {
		id[l] = 0;
		if (!strcmp("null", id)) {
			printf("((var)0)");
		} else if (!strcmp("false", id)) {
			printf("((var)0)");
		} else if (!strcmp("true", id)) {
			printf("((var)1)");
		} else if (!strcmp("this", id)) {
			printf("self");
		} else if (!strcmp("continue", id)) {
			indent(st);
			printf("\tcontinue;\n");
		} else if (!strcmp("break", id)) {
			indent(st);
			printf("\tbreak;\n");
		} else {
			cls = get_class(st, id);
			if (is_def(st, id) || is_global(st, id)) {
				printf("%s", id);
			} else if (!is_var(st, id) && !cls) {
				sel = "__self->";
				printf("%s%s", sel, id);
			} else if (o == '.' && cls) {
				printf("(var)%s__", cls);
				st->pos++;
				xid = identifier(st, &x);
				if (xid) {
					ox = xid[x];
					xid[x] = 0;
					printf("%s", xid);
					xid[x] = ox;
				}
			} else if (cls) {
				printf("(*((var*)&%s))", id);
			} else if (!strcmp("this", id)) {
				printf("self");
			} else {
				printf("%s", id);
			}
		}
		id[l] = o;
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
		if (p[1] == '>' || p[1] == '=') {
			p++;
		}
		break;
	case '>':
		if (p[1] == '=') {
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
	if (*p != '"') {
		return NULL;
	}
	p++;
	l = 1;
	while (p < end && *p && *p != '"') {
		if (*p == '\\') {
			switch (p[1]) {
			default:
				b[l] = *p;
			}
			l++;
		} else {
			b[l] = *p;
			l++;
		}
		p++;
	}
	b[l] = 0;
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
	free(st->buf);
	free(st->file);
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

int expression(struct trip *st)
{
	int ok;
	int c;
	char *id;
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
	ok = 0;
	while (st->pos < st->end && !ok) {
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
		case '?':
			ok = 1;
			break;
		case '\n':
			ok = 1;
			break;
		case '\\':
			c = st->buf[st->pos + 1];
			if (c == '\r') {
				st->pos++;
				c = st->buf[st->pos + 1];
			}
			if (c == '\n') {
				st->line++;
				st->pos++;
				st->pos++;
				c = st->buf[st->pos];
				while (c == ' ' || c == '\t') {
					st->pos++;
					c = st->buf[st->pos];
				}
			} else {
				printf("\\"); 
				st->pos++;
			}
			break;
		case '#':
		case '}':
		case ']':
		case ')':
			return 1;
		case '(':
			st->pos++;
			printf("(");
			expression(st);
			printf(")");
			c = st->buf[st->pos];
			if (c == ')') {
				st->pos++;
			} else {
				error(") expected", st);
			}
			break;
	
		case '[':
			st->pos++;
			printf("[");
			expression(st);
			printf("]");
			c = st->buf[st->pos];
			if (c == ']') {
				st->pos++;
			} else {
				error("] expected", st);
			}
			break;
		case '{':
			st->pos++;
			compound(st);
			c = st->buf[st->pos];
			if (c == '}') {
				st->pos++;
			} else {
				error("} expected", st);
			}
			break;
		case '+':
		case '-':
		case '*':
		case '/':
		case '%':
			printf("%c", c); 
			st->pos++;
			break;
		case ' ':
		case '\t':
			printf(" "); 
			spaces(st);
			break;
		case '"':
			id = str_lit(st);
			printf("(var)\"%s\"", id);
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
			printf("%d", num_lit(st)); 
			break;
		default:
			id = variable(st);
			if (!id) {
				printf("%c", c); 
				st->pos++;
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
	}
	
	indent(st);
	printf("}");
}

int conditional(struct trip *st, int is_while)
{
	int ok;
	int c;
	char *f;
	int n = 0;
	ok = 0;
	if (is_while) {
		f = "while";
	} else {
		f = "if";
	}
	st->indent++;
	while (st->pos < st->end && !ok) {
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
			st->indent--;
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
				printf("%s (__cond != 0) ", f);
				st->pos++;
			} else if (c == '=') {
				printf("%s (__cond <= 0) ", f);
				st->pos++;
			} else {
				printf("%s (__cond < 0) ", f);
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
			if (c == '=') {
				st->pos++;
				printf("%s (__cond >= 0) ", f);
			} else {
				printf("%s (__cond > 0) ", f);
			}
			condbody(st);
			n = 1;
			break;
		case '=':
			if (n) {
				printf(" else ");
			} else {
				indent(st);
			}
			st->pos++;
			printf("%s (__cond == 0) ", f);
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
			printf("%s (__cond == %d) ",f, -num_lit(st));
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
				printf("%s (__cond == %d) ", f, num_lit(st));
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

	st->pos += 6;
	whitespaces(st);
	id = identifier(st, &l);
	if (id) {
		whitespaces(st);
		id[l] = 0;
		s = str_lit(st);
		printf("#define %s ", id);
		if (st->nb_defs >= MAX_MEMBER) {
			error("too many define", st);
		}
		st->defs[st->nb_defs] = id;
		st->nb_defs++;
		if (s) {
			printf("(var)\"%s\"\n", s);
		} else {
			val = identifier(st, &l);
			if (val) {
				printf("%s\n", val);
			} else {
				n = num_lit(st);
				printf("%d\n", n);
			}
		}
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
	printf("struct %s {\n", st->class_name);
	whitespaces(st);
}

void parameters(struct trip *st, int n)
{
	char *s;
	int op;
	int l;
	int o;

	whitespaces(st);
	op = operator(st);
	while (op != ')') {
		whitespaces(st);
		s = identifier(st, &l);
		o = s[l];
		s[l] = 0;
		if (n > 0) {
			printf(", ");
		}
		n++;
		printf("var %s", s);
		if (st->nb_vars >= MAX_MEMBER) {
			error("too many parmeters", st);
		}
		st->vars[st->nb_vars] = s;
		st->nb_vars++;
		s[l] = o;
		whitespaces(st);
		op = operator(st);
		s[l] = 0;
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
		s[l] = 0;
		st->func_name = s;
		printf("var %s(", st->func_name);
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
		printf("var %s__%s(var self",st->class_name, st->func_name);
		whitespaces(st);
		parameters(st, 1);
		whitespaces(st);
		op = operator(st);
		if (op == '{') {
			printf(")\n{\n");
			printf("\tstruct %s *__self = (void*)self;(void)__self;\n", 
					st->class_name);
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
			return;
		}
	}
	error("in method decl", st);
	
}

void k_print(struct trip *st)
{
	char *s;
	st->pos += 5;
	whitespaces(st);
	s = str_lit(st);
	indent(st);
	if (s) {
		end_of_expr(st);
		printf("printf(\"%%s\",\"%s\");\n", s);
	} else {
		printf("printf(\"%%s\",(char*)(");
		expression(st);
		printf("));\n");
		end_of_expr(st);
	}
}

void k_print10(struct trip *st)
{
	st->pos += 7;
	whitespaces(st);
	indent(st);
	printf("printf(\"%%ld\",(long)(");
	expression(st);
	printf("));\n");
	end_of_expr(st);
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
	st->pos += 3;
	whitespaces(st);
	indent(st);
	variable(st);
	printf(" = ");
	i = st->indent;
	st->indent = -1;
	whitespaces(st);
	expression(st);
	st->indent = i;
	printf(";\n");
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
	if (!strcmp(id, "this")) {
		printf("free((void*)self);\n");
	} else {
		printf("free((void*)%s);\n", id);
	}
}

void k_var(struct trip *st)
{
	char *id;
	int l;
	int n = 0;
	st->pos += 3;
	whitespaces(st);
	printf("\tvar ");

	id = identifier(st, &l);
	whitespaces(st);
	while (st->pos < st->end && st->buf[st->pos] == ',') {
		id[l] = 0;
		if (n) {
			printf(", %s", id);
		} else {
			printf(" %s", id);
		}
		n++;
		if (st->nb_vars >= MAX_MEMBER) {
			error("too many var", st);
		}
		st->vars[st->nb_vars] = id;
		st->nb_vars++;
		st->pos++;
		whitespaces(st);
		id = identifier(st, &l);
	}
	end_of_expr(st);
	id[l] = 0;
	if (st->nb_vars >= MAX_MEMBER) {
		error("too many var", st);
	}
	st->vars[st->nb_vars] = id;
	st->nb_vars++;
	if (n) {
		printf(", %s;\n", id);
	} else {
		printf(" %s;\n", id);
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
	id[l] = 0;
	printf("\tstruct %s *", id);
	st->refs[st->nb_refs].value = id;
	id = identifier(st, &l);
	end_of_expr(st);
	id[l] = 0;
	printf("%s;\n", id);
	st->refs[st->nb_refs].name = id;
	st->nb_refs++;
}

void k_poke(struct trip *st)
{
	char *id;
	int l;
	int o;
	st->pos += 4;
	whitespaces(st);
	id = identifier(st, &l);
	o = id[l];
	id[l] = 0;
	indent(st);
	printf("((unsigned char*)%s)[", id);
	id[l] = o;
	expression(st);
	printf("] = ");
	expression(st);
	printf(";\n");
	end_of_expr(st);
}

void k_peek(struct trip *st)
{
	char *id;
	int l;
	int o;
	st->pos += 4;
	whitespaces(st);
	id = identifier(st, &l);
	o = id[l];
	id[l] = 0;
	printf("((unsigned char*)%s)[", id);
	id[l] = o;
	expression(st);
	printf("]");
	end_of_expr(st);
}

void k_bytes(struct trip *st)
{
	char *id;
	int l;
	int op;
	st->pos += 5;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	if (operator(st) != '{') {
		error("{ expected", st);
	}
	id[l] = 0;
	printf("unsigned char %s[] = {\n", id);
	if (st->nb_global >= MAX_MEMBER) {
		error("too many global", st);
	}
	st->global[st->nb_global] = id;
	st->nb_global++;
	
	whitespaces(st);
	while (st->pos < st->end && st->buf[st->pos] != '}') {
		printf("%d", num_lit(st));
		whitespaces(st);
		op = operator(st);
		whitespaces(st);
		if (op == ',') {
			printf(",");
		} else if (op == '}') {
			break;
		} else {
			error(", expected", st);

		}
	}
	printf("};\n");
}


void k_array(struct trip *st)
{
	char *id;
	int l;
	int op;
	st->pos += 5;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	if (operator(st) != '{') {
		error("{ expected", st);
	}
	id[l] = 0;
	printf("var %s[] = {\n", id);
	if (st->nb_global >= MAX_MEMBER) {
		error("too many global", st);
	}
	st->global[st->nb_global] = id;
	st->nb_global++;
	
	whitespaces(st);
	while (st->pos < st->end && st->buf[st->pos] != '}') {
		printf("%d", num_lit(st));
		whitespaces(st);
		op = operator(st);
		whitespaces(st);
		if (op == ',') {
			printf(",");
		} else if (op == '}') {
			break;
		} else {
			error(", expected", st);

		}
	}
	printf("};\n");
}

void k_new(struct trip *st)
{
	char *id;
	int l;
	int o;
	st->pos += 3;
	whitespaces(st);
	id = identifier(st, &l);
	whitespaces(st);
	o = id[l];
	id[l] = 0;
	if (!strcmp(id, "array")) {
		id[l] = o;
		printf("(var)malloc(sizeof(var) * %d)", num_lit(st));
	} else if (!strcmp(id, "bytes")) {
		id[l] = o;
		printf("(var)malloc(%d)", num_lit(st));
	} else {
		printf("(var)malloc(sizeof(struct %s))", id);
	}
	id[l] = o;
	end_of_expr(st);
}


void k_if(struct trip *st)
{
	st->pos += 2;
	whitespaces(st);
	indent(st);
	printf("{\n");
	indent(st);
	printf("\tvar __cond = ");
	expression(st);
	printf(";\n");
	st->pos++;
	whitespaces(st);
	conditional(st,0);
	printf("\n");
	indent(st);
	printf("}\n");
	end_of_expr(st);
}

void k_while(struct trip *st)
{
	st->pos += 5;
	whitespaces(st);
	indent(st);
	printf("{");
	indent(st);
	printf("var __cond = ");
	expression(st);
	printf(";\n");
	st->pos++;
	whitespaces(st);
	conditional(st,1);
	printf("\n");
	indent(st);
	printf("}\n");
	end_of_expr(st);
}

void k_return(struct trip *st)
{
	st->pos += 6;
	whitespaces(st);
	indent(st);
	printf("return ");
	expression(st);
	printf(";\n");
	whitespaces(st);
	st->returnn = 1;
}

void k_break(struct trip *st)
{
	st->pos += 5;
	whitespaces(st);
	end_of_expr(st);
	indent(st);
	printf("break;\n");
}

void k_continue(struct trip *st)
{
	st->pos += 8;
	whitespaces(st);
	end_of_expr(st);
	indent(st);
	printf("continue;\n");
}

void k_include(struct trip *st)
{
	char *s;
	struct trip *stn;
	int len;
	char *buf;
	int i;
	st->pos += 7;
	whitespaces(st);
	s = str_lit(st);
	if (strstr(s, "std.3p")) {
		s[strlen(s)-2] = 0;
		printf("#include \"%sh\"\n", s);
		return;
	}
	len = strlen(st->file) + strlen(s) + 2;
	buf = malloc(len);
	buf[0] = 0;
	strcat(buf, st->file);
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
	stn = load(buf);
	free(buf);
	if (stn) {
		stn->parent = st;
		while (st->parent) {
			st = st->parent;
		}
		st->last->next = stn;
		st->last = stn;
		stn->func_name = NULL;
		stn->class_name = NULL;
		compound(stn);
	} else {
		error("cannot include file", st);
	}
	for (i = 0; i < stn->nb_global; i++) {
		if (st->nb_global >= MAX_MEMBER) {
			error("Too many globals", stn);
		}
		st->global[st->nb_global] = stn->global[i];
		st->nb_global++;
	}
	for (i = 0; i < stn->nb_defs; i++) {
		if (st->nb_defs >= MAX_MEMBER) {
			error("Too many defs", stn);
		}
		st->defs[st->nb_defs] = stn->defs[i];
		st->nb_defs++;
	}
}

void call(struct trip *st)
{
	char *p;
	char *id;
	char *meth;
	char *clas;
	int l;
	int o;
	int c;
	int end;
	int n = 0;
	whitespaces(st);
	id = identifier(st, &l);
	c = st->buf[st->pos];
	if (!id) {
		error("identifier expected..", st);
		return;
	}
	o = id[l];
	id[l] = 0;
	clas = NULL;
	if (st->indent == 2) {
		printf("\t");
	}
	if (c == '.') {
		st->pos++;
		whitespaces(st);
		meth = identifier(st, &l);
		c = st->buf[st->pos];
		o = meth[l];
		meth[l] = 0;
		clas = get_class(st, id);
		if (clas == NULL) {
			printf("((var(*)())%s__%s)(self", 
					st->class_name, meth);
		} else {
			printf("((var(*)())%s__%s)((var)%s", clas, meth, id);
		}
		meth[l] = o;
		n = 1;
	} else {
		printf("((var(*)())%s)(", id);
		id[l] = o;
	}
	end = 0;
	while (st->pos < st->end && !end) {
		p = st->buf + st->pos;
		switch (*p) {
		case '}':
		case '\r':
		case '\n':
		case ';':
		case '#':
			end = 1;
			break;
		case '{':
			if (n > 0) {
				printf(", ");
			}
			st->pos++;
			compound(st);
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
				printf(", ");
			}
			n++;
		default:
			expression(st);
		}
	}
	p = st->buf + st->pos;
	//printf(" PPP %c\n", *p);
	if (*p != '}') {
		printf(");\n");
	} else {
		printf(")");
	}
	end_of_expr(st);
}


void compound(struct trip *st)
{
	char *p;
	int l;
	whitespaces(st);
	st->indent++;
	while (st->pos < st->end) {
		p = st->buf + st->pos;
		l = st->pos;
		switch (*p) {
		case '}':
			st->indent--;
			return;
		case 'a':
			if (is(st, p, "array")) {
				k_array(st);
			}
			break;
		case 'b':
			if (is(st, p, "bytes")) {
				k_bytes(st);
			} else if (is(st, p, "break")) {
				k_break(st);
			}
			break;
		case 'c':
			if (is(st, p, "class")) {
				k_class(st);
			} else if (is(st, p, "continue")) {
				k_continue(st);
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
				k_func(st);
			} else if (is(st, p, "field")) {
				k_field(st);
			}
			break;
		case 'i':
			if (is(st, p, "include")) {
				k_include(st);
			} else if (is(st, p, "if")) {
				k_if(st);
			}
			break;
		case 'm':
			if (is(st, p, "method")) {
				k_method(st);
			}
			break;
		case 'n':
			if (is(st, p, "new")) {
				k_new(st);
			}
			break;
		case 'p':
			if (is(st, p, "print")) {
				k_print(st);
			} else if (is(st, p, "print10")) {
				k_print10(st);
			} else if (is(st, p, "peek")) {
				k_peek(st);
			} else if (is(st, p, "poke")) {
				k_poke(st);
			}
			break;
		case 'r':
			if (is(st, p, "return")) {
				k_return(st);
			} else if (is(st, p, "ref")) {
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
			call(st);
			if (l == st->pos) {
				error("call expected", st);
			}
		} else {
			whitespaces(st);
		}
	}
	st->indent--;
}

int main(int argc, char *argv[]) 
{
	struct trip *st;
	if (argc > 1) {
		printf("typedef long var;\n");
		st = load(argv[1]);
		st->last = st;
		st->func_name = NULL;
		st->class_name = NULL;
		if (st) {
			compound(st);
			if (st->func_name) {
				printf("\treturn 0;\n}\n");
			}
			if (!st->main) {
				printf("int main(int argc, char *argv[]) {\n");
				printf("\treturn startup((var)argc,(var)argv);\n");
				printf("}\n");
			}
			trip__delete(st);
		}
	}
	return 0;
}

