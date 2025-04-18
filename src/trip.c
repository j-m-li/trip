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

	for (i = 0; st->files[i].buf != st->buf && i < st->nb_files; i++) {
	}
	if (st->nb_files < i) {
		file = st->files[i].file_name;
	} else {
		file = "command line";
	}
	printf("\n#error \"%s @ line %d in %s\"\n", txt, l, file);
	fflush(stdout);
	if (st->nb_files > 0) {
		__builtin_trap();
		exit(-1);
	}
	st->break_ = 1;
	return 0;
}

int is_end_of_command(struct trip *st)
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
		case ')':
		case '}':
		case ';':
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
		__builtin_trap();
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


char *get_class(struct trip *st, char *name) 
{ 
	int i;
	struct class_cell *c = NULL;

	for (i = 0; i < st->nb_refs; i++) {
		if (!id_cmp(st->refs[i].name, name)) {
			return st->refs[i].value;
		}
	}
	if (!st->class_name) {
		return NULL;
	}
	for (i = 0; i < st->nb_classes; i++) {
		if (!id_cmp(st->classes[i].name, st->class_name)) {
			c = &st->classes[i];
			break;
		}
	}
	if (!c) {
		return NULL;
	}
	for (i = 0; i < c->nb_fields; i++) {
		if (!id_cmp(c->fields[i].name, name)) {
			return c->fields[i].value;
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

int is_ref(struct trip *st, char *name)
{
	int i;
	for (i = 0; i < st->nb_refs; i++) {
		if (!id_cmp(st->refs[i].name, name)) {
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
	int opos;
	int oend;
	struct class_cell *clsc;

	if (st->nb_classes >= MAX_MEMBER) {
		error("too many classes", st);
	}
	clsc = &st->classes[st->nb_classes];
	st->nb_classes++;
	clsc->name = cid;
	clsc->nb_methods = 0;
	clsc->nb_fields = 0;
	clsc->data = (var)(st->buf + st->pos);

	if (st->mode == MODE_INTERP) {
	} else {
		printf("struct %s {\n", id_tmp(st,cid));
	}
	opos = st->pos;
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field") || is(st, p, "wire") 
				|| is(st, p, "object")) 
		{
			if (is(st, p, "wire")) {
				st->pos += 4;
			} else if (is(st, p, "object")) {
				st->pos += 6;
			} else {
				st->pos += 5;
			}
			
			if (clsc->nb_fields >= MAX_MEMBER) {
				error("too many fields", st);
			}
			whitespaces(st);
			id = identifier(st, &l);
			spaces(st);
			p = st->buf + st->pos;
			if (*p != '\n' && *p  != '\r' && *p != '#') {
				clsc->fields[clsc->nb_fields].value = id;
				id = identifier(st, &l);
			} else {
				clsc->fields[clsc->nb_fields].value = NULL;
			}
			clsc->fields[clsc->nb_fields].name = id;
			if (st->mode == MODE_INTERP) {
			} else {
				printf("\tvar %s;\n", id_tmp(st,id));
			}
			clsc->nb_fields++;
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
	oend = st->pos;
	st->pos = opos;
	while (st->pos < oend) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field") || is(st, p, "wire") 
				|| is(st, p, "object")) 
		{
			if (is(st, p, "wire")) {
				st->pos += 4;
			} else if (is(st, p, "object")) {
				st->pos += 6;
			} else {
				st->pos += 5;
			}
			whitespaces(st);
			id = identifier(st, &l);
			spaces(st);
			p = st->buf + st->pos;
			if (*p != '\n' && *p  != '\r' && *p != '#') {
				id = identifier(st, &l);
			}
			if (st->mode == MODE_INTERP) {
			} else {
			}
			end_of_expr(st);
		} else {
			while (*p != '\n') {
				st->pos++;
				p = st->buf + st->pos;
			}
		}
	}
	st->pos = oend;
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

var num_lit(struct trip *st)
{
	char *end = st->buf + st->end;
	char *b;
	char *p;
	var r = 0;
	int l;
	char *id;
	id = identifier(st, &l);
	if (id) {
		return get_data(st, st->__self, "", id, 0);
	}
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
			r = r * 10 + (*p - '0');
			p++;
		}
	}
	st->pos += p - b;
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
		st->pos = -1;
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
	int oend;
	char *obuf;
	int ok;
	opos = st->pos;
	oend = st->end;
	obuf = st->buf;
	fix_pos_buf(st, get_class_def_ptr(st, clas));
	if (st->pos < 0) {
		st->pos = opos; 
		st->end = oend; 
		st->buf = obuf; 
		return 0;
	}
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		ok = 0;
		if (is(st, p, "field")) {
			ok = 5;
		} else if (is(st, p, "wire")) {
			ok = 4;
		} else if (is(st, p, "object")) {
			ok = 6;
		}
		if (ok) {
			st->pos += ok;
			whitespaces(st);
			id = identifier(st, &l);
			spaces(st);
			p = st->buf + st->pos;
			if (*p != '\n' && *p  != '\r' && *p != '#') {
				id = identifier(st, &l);
			}
			end_of_expr(st);
			if (!strcmp(type, "field")) {
				if (!id_cmp(id, name)) {
					n = st->buf + st->pos;
					st->pos = opos; 
					st->end = oend; 
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
					st->end = oend; 
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
	st->end = oend; 
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
	int oend;
	char *obuf;
	opos = st->pos;
	oend = st->end;
	obuf = st->buf;
	fix_pos_buf(st, get_class_def_ptr(st, clas));
	if (st->pos < 0) {
		st->pos = opos; 
		st->end = oend; 
		st->buf = obuf; 
		return -1;
	}
	n = 0;
	while (st->pos < st->end) {
		whitespaces(st);
		p = st->buf + st->pos;
		if (is(st, p, "field") || is(st, p, "wire") ||
				is(st, p, "object")) 
		{
			if (is(st, p, "wire")) {
				st->pos += 4;
			} else if (is(st, p, "object")) {
				st->pos += 6;
			} else {
				st->pos += 5;
			}
			whitespaces(st);
			id = identifier(st, &l);
			spaces(st);
			p = st->buf + st->pos;
			if (*p != '\n' && *p  != '\r' && *p != '#') {
				id = identifier(st, &l);
			}
			end_of_expr(st);
			if (!strcmp(type, "field")) {
				if (!id_cmp(id, name)) {
					st->pos = opos; 
					st->end = oend; 
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
	st->end = oend; 
	st->buf = obuf; 
	return -1;
}




int size_of(struct trip *st, char *name)
{
	int l;
	char *p;
	int n;
	int opos;
	int oend;
	char *obuf;
	char *df;
	
	opos = st->pos;
	obuf = st->buf;
	oend = st->end;
	n = 0;
	df = get_class_def_ptr(st, name);
	if (!df) {
		st->pos = opos; 
		st->buf = obuf; 
		st->end = oend;
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
		} else if (is(st, p, "wire")) {
			st->pos += 4;
			whitespaces(st);
			identifier(st, &l);
			end_of_expr(st);
			n++;
		} else if (is(st, p, "object")) {
			st->pos += 6;
			whitespaces(st);
			identifier(st, &l);
			spaces(st);
			p = st->buf + st->pos;
			if (*p != '\n' && *p  != '\r' && *p != '#') {
				identifier(st, &l);
			}
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
	st->end = oend;
	return n * sizeof(var);
}



var get_data(struct trip *st, var sel, char *cls, char *name, int return_addr)
{
	int i;
	int idx;
	var *arr = (var*)sel;
	char *p;
	if (cls && cls[0]) {
		idx = get_member_pos(st, cls, "field", name);
		if (idx < 0) {
			p = get_member_ptr(st, cls, "method", name);
			if (!p) {
				printf("CLLL %s\n", id_tmp(st,cls));
				printf("CLLL %s\n", id_tmp(st,name));
				error("cannot find field", st);
			}
			return (var)p;
		}
		if (!arr) {
			printf("%s.", id_tmp(st,cls));
			printf("%s -- ", id_tmp(st,name));
			error("null this pointer", st);
		}
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
		if (st->nb_files == 0 && st->nb_vars < MAX_MEMBER) {
			i = st->nb_vars;
			st->vars[i].name = name;
			st->vars[i].data = 0;
			st->nb_vars++;
			if (return_addr) {
				return (var)&st->vars[i].data;
			}
			return st->vars[i].data;
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
				sel = st->class_name;
			}
		}
		if (st->mode == MODE_INTERP) {
			arr = (var*)get_data(st, st->__self, sel, id, 0);
		} else {
			if (sel && sel[0]) {
				printf("((var*)(__self->%s))[",id_tmp(st,id));
			} else {
				printf("((var*)(%s))[",id_tmp(st,id));
			}
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
	} else if (st->buf[st->pos] == '.') {
		cls = get_class(st, id);
		st->pos++;
		xid = identifier(st, &l);
		if (st->mode == MODE_INTERP) {
			push(st, get_data(st, st->__self, cls, xid, return_addr));
		} else {
			if (return_addr) {
				printf("((struct %s *)", id_tmp(st,cls));
				if (!is_var(st, id) && !is_ref(st, id)) {
					printf("__self->");
				}
				printf("%s", id_tmp(st,id));
				printf(")->%s", id_tmp(st,xid));
			} else {
				if (get_member_pos(st, cls, "field", xid) >= 0) {
					printf("((struct %s *)", 
							id_tmp(st,cls));
					if (!is_var(st, id) && 
							!is_ref(st, id)) 
					{
						printf("__self->");
					}
					printf("%s", id_tmp(st,id));
					printf(")->%s", id_tmp(st,xid));
				} else {
					printf("(var)%s__", id_tmp(st,cls));
					printf("%s", id_tmp(st,xid));
				}
			}
		}
		return ".";
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
				printf("\tcontinue;\n");
			}
		} else {
			cls = get_class(st, id);
			if (is_def(st, id) || is_global(st, id)) {
				if (st->mode == MODE_INTERP) {
					push(st, get_data(st, st->__self, NULL, 
						id, return_addr));
				} else {
					printf("%s", id_tmp(st,id));
				}
			} else if (!is_var(st, id) && !is_ref(st, id)) {
				sel = st->class_name;
				if (st->mode == MODE_INTERP) {
					if (st->nb_files == 0 && 
						st->class_name == 0) 
					{
						sel = NULL;
					}
					push(st, get_data(st, st->__self, sel, 
							id, return_addr));
				} else {

					printf("__self->%s", id_tmp(st,id));
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
					push(st, get_data(st, st->__self, NULL, 
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
					push(st, get_data(st, st->__self, NULL, 
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
	int has_primary = 0; 
	ok = 0;
	while (st->pos < st->end && !ok) {
		if (st->mode == MODE_INTERP) {
			if (st->break_ || st->else_ || 
				st->return_ || st->continue_) 
			{
				return has_primary;
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
			return has_primary;
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
					return has_primary;
				}
			} else {
				if (prec > 13) {
					return has_primary;
				}
			}
			st->pos++;
			if (!has_primary) {
				if (st->mode == MODE_INTERP) {
				} else {
					if (c == '-') {
						printf("-");
					}	
				}
				expression1(st, 16 + 1, spc);
				if (c == '-') {
					if (st->mode == MODE_INTERP) {
						push(st, -pop(st));
					}
				}
				has_primary = 1;
			} else {
				if (st->mode == MODE_INTERP) {
					has_primary = expression1(
							st, 13 + 1, spc);
					b = pop(st);
					a = pop(st);
					if (c == '-') {
						push(st, a - b);
					} else {
						push(st, a + b);
					}
				} else {
					printf("%c ", c); 
					has_primary = expression1(st, 13 + 1, spc);
				}
			}
			break;
		case '*':
		case '/':
		case '%':
			if (prec > 14) {
				return has_primary;
			}
			st->pos++;
			if (st->mode == MODE_INTERP) {
				has_primary = expression1(st, 14 + 1, spc);
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
				has_primary = expression1(st, 14 + 1, spc);
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
				printf("%ld", num_lit(st)); 
			}
			has_primary = 1;
			break;
		default:
			id = variable(st, 0);
			if (!id) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf("%c", c); 
				} // FIXME
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
	return has_primary;

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

void condbody_interp(struct trip *st, var cond)
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
		st->else_ = 1;
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
//	expression(st);
//		whitespaces(st);
	}
	
	indent(st);
	printf("}");
}

int conditional_interp(struct trip *st)
{
	int c,l;
	char *id;
	var cond = 0;
	var real_cond = 0;
	int or = 0;

	while (st->pos < st->end) {
		if (cond != 0) {
			real_cond = cond;
		}
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
			return 0;
		case ',':
			st->pos++;
			or = 0;
			break;
		case '{':
			if (!or) {
				error("missing condition", st);
			}
			condbody_interp(st, real_cond);
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
				cond = st->cond < 0;
			}
			or++;
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
			or++;
			break;
		case '~':
			st->pos++;
			cond = st->cond == 0;
			or++;
			break;
		case '-':
			st->pos++;
			cond = st->cond == -num_lit(st);
			or++;
			break;
		case '+':
			st->pos++;
		default:
			c = st->buf[st->pos];
			if (c == '\'' || (c >= '0' && c <= '9')) {
				cond = st->cond == num_lit(st);
			} else {

				id = identifier(st, &l);
				if (id) {
					cond = st->cond == get_data(
						st, st->__self, NULL, id, 0);
				} else {
					error("cond expected", st);
				}
			}
			or++;
		}
		whitespaces(st);
	}
	st->else_ = 1;
	return 0;
}
int print_if(struct trip *st, int n, int or)
{
	if (or) {
		printf("||"); 
	} else {
		if (n) {
			printf(" else ");
		} else {
			indent(st);
		}
		printf("if ("); 
	}
	return 0;
}

int conditional(struct trip *st)
{
	int ok;
	int c,l;
	char *id;
	int n = 0;
	int or = 0;
	ok = 0;
	if (st->mode == MODE_INTERP) {
		return conditional_interp(st);
	} else {
	}
	st->indent++;
	while (st->pos < st->end && !ok) {
		c = st->buf[st->pos];
		switch (st->buf[st->pos]) {
		case ';':
			st->indent--;
			return 0;
		case '{':
			printf(") ");
			condbody(st);
			n = 1;
			break;

		case ',':
			if (!or) {
				error("body expected", st);
			}
			st->pos++;
			or = 0;
			break;
		case '<':
			st->pos++;
			c = st->buf[st->pos];
			print_if(st,n,or);
			if (c == '>') {
				printf("(if__cond != 0)");
				st->pos++;
			} else if (c == '~') {
				printf("(if__cond <= 0)");
				st->pos++;
			} else {
				printf("(if__cond < 0)");
			}
			or++;
			break;
		case '>':
			st->pos++;
			c = st->buf[st->pos];
			print_if(st,n,or);
			if (c == '~') {
				st->pos++;
				printf("(if__cond >= 0)");
			} else {
				printf("(if__cond > 0)");
			}
			or++;
			break;
		case '~':
			st->pos++;
			print_if(st,n,or);
			printf("(if__cond == 0)");
			or++;
			break;
		case '-':
			st->pos++;
			print_if(st,n,or);
			printf("(if__cond == %ld)",-num_lit(st));
			or++;
			break;
		case '+':
			st->pos++;
		default:
			c = st->buf[st->pos];
			if (c == '\'' || (c >= '0' && c <= '9')) {
				print_if(st,n,or);
				printf("(if__cond == %ld)", num_lit(st));
				or++;
			} else {
				id = identifier(st, &l);
				if (id) {
					print_if(st,n,or);
					printf("(if__cond == %s)", 
						id_tmp(st, id));
					or++;
				} else {
					error("cond expected", st);
				}
			}
			break;
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
	var n;
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
					printf("%ld\n", n);
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

void k_set(struct trip *st)
{
	int i;
	var *arr;
	var v;
	int sp;

	st->pos += 3;
	whitespaces(st);
	sp = st->sp;
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
		if (sp != (st->sp - 2)) {
			error("Bad set expression",st);
		}
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
	var data;
	st->pos += 6;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
		variable(st,0);
		data = pop(st);
		if (data) {
			free((void*)data);
		}
	} else {
		printf("free((void*)(");
		variable(st,0);
		printf("));\n");
	}
	end_of_expr(st);
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
		st->refs[st->nb_refs].data = 0;
	} else {
		printf("%s = (void*)0;\n", id_tmp(st,id));
	}
	st->refs[st->nb_refs].name = id;
	st->nb_refs++;
}

void k_poke(struct trip *st)
{
	int i;
	var v;
	int sp;
	char *arr;
	int idx;

	st->pos += 4;
	whitespaces(st);

	sp = st->sp;
	if (st->mode == MODE_INTERP) {
	} else {
		indent(st);
		printf("((unsigned char*)");
	}
	variable(st, 0);
	if (st->mode == MODE_INTERP) {
		arr = (char*)pop(st);
	} else {
		printf(")[ ");
		i = st->indent;
		st->indent = -1;
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		idx = pop(st);
	} else {
		printf("] = ");
		i = st->indent;
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		v = pop(st);
		arr[idx] = v;
	} else {
		st->indent = i;
		printf(";\n");
	}
	if (sp != st->sp) {
		error("BAD poke", st);
	}

	end_of_expr(st);
}

void k_peek(struct trip *st)
{
	int sp;
	char *arr;

	st->pos += 4;
	whitespaces(st);

	sp = st->sp;
	if (st->mode == MODE_INTERP) {
	} else {
		indent(st);
		printf("((unsigned char*)");
	}
	variable(st, 0);
	if (st->mode == MODE_INTERP) {
		arr = (char*)pop(st);
	} else {
		printf(")[ ");
	}
	expression(st);
	if (st->mode == MODE_INTERP) {
		if ((sp+1) != st->sp) {
			error("BAD peek", st);
		}
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
			printf("%ld", num_lit(st));
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
			printf("%ld", num_lit(st));
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
			expression(st);
			push(st, (var)malloc(sizeof(var) * pop(st)));
		} else {
			printf("(var)malloc(sizeof(var) * (");
			expression(st);
			printf("))");
		}
	} else if (!id_cmp(id, "bytes")) {
		if (st->mode == MODE_INTERP) {
			expression(st);
			push(st, (var)malloc(pop(st)));
		} else {
			printf("(var)malloc(");
			expression(st);
			printf(")");
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
		printf("\tvar if__cond = ");
	}
	expression(st);
	whitespaces(st);
	if (st->buf[st->pos] != '?') {
		error("missing ?", st);
	}
	st->pos++;
	whitespaces(st);
	if (st->mode == MODE_INTERP) {
		if ((sp + 1)!= st->sp) {
			error("missing condition", st);
		}
		cond = st->cond;
		else_ = st->else_;
		st->cond = pop(st);
		conditional(st);
		st->cond = cond;
		st->else_ = else_;
	} else {
		printf(";\n");
		conditional(st);
		printf("\n");
		indent(st);
		printf("}\n");
	}
	end_of_expr(st);
}

void k_loop(struct trip *st)
{
	int sp;
	int pos;

	st->pos += 4;
	whitespaces(st);
	if (st->buf[st->pos] != '{') {
		error("{ expected", st);
	}
	if (st->mode == MODE_INTERP) {
		sp = st->sp;
	} else {
		indent(st);
		printf("while (1) {\n");
		indent(st);
	}

	if (st->mode == MODE_INTERP) {
		pos = st->pos;
		while (!st->break_ && !st->else_ &&
			!st->return_ && !st->continue_) 
		{
			st->pos++;
			compound(st);
			if (st->break_) {
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
			st->pos = pos;
			whitespaces(st);
			if (st->buf[st->pos] == '{') {
				skip_body(st);
			}
			st->continue_ = 0;
			st->break_ = 0;
		}
	} else {
		st->pos++;
		compound(st);
		if (st->pos < st->end && st->buf[st->pos] == '}') {
			st->pos++;
		} else {
			error("expression body expected", st);
		}

		printf("}\n");
		indent(st);
	}
	whitespaces(st);
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

void load_include(struct trip *st, char *s)
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
	st->end = file__size((var)buf);
	if (st->end < 1 || st->nb_files >= MAX_MEMBER) {
		printf(">> %s\n", buf);
		error("cannot open file", st);
	}
	st->buf = (char*)file__load((var)buf, 0, st->end);
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
	load_include(st, s);
}

var (*builtin(struct trip *st, char *clas, char *id, int *argc))()
{
	
	switch(clas[0]) {
	case 0:
		break;
	case 't':
		if (!id_cmp(clas, "term")) {
			if (!id_cmp(id, "wait")) {
				*argc = 2;
				return (var(*)())term__wait;
			} else if (!id_cmp(id, "new")) {
				*argc = 1; /*dummy this */
				return (var(*)())term__new;
			} else if (!id_cmp(id, "width")) {
				*argc = 1;
				return (var(*)())term__width;
			} else if (!id_cmp(id, "height")) {
				*argc = 1;
				return (var(*)())term__height;
			}
		}
	default:
		return wrapper(st,clas,id,argc);
	}
	switch(id[0]) {
	case 'b':
		if (!id_cmp(id, "buffer__append")) {
			*argc = 3;
			return (var(*)())buffer__append;
		} else if (!id_cmp(id, "buffer__append10")) {
			*argc = 2;
			return (var(*)())buffer__append10;
		}
	       	break;	
	case 'c':
		if (!id_cmp(id, "clipboard__set")) {
			*argc = 2;
			return (var(*)())clipboard__set;
		} else if (!id_cmp(id, "clipboard__get")) {
			*argc = 0;
			return (var(*)())clipboard__get;
		} else if (!id_cmp(id, "command")) {
			*argc = 3;
			return (var(*)())command;
		}
		break;
	case 'f':
		if (!id_cmp(id, "file__size")) {
			*argc = 1;
			return (var(*)())file__size;
		} else if (!id_cmp(id, "flush")) {
			*argc = 0;
			return (var(*)())flush;
		} else if (!id_cmp(id, "file__load")) {
			*argc = 3;
			return (var(*)())file__load;
		} else if (!id_cmp(id, "file__rename")) {
			*argc = 2;
			return (var(*)())file__rename;
		} else if (!id_cmp(id, "file__delete")) {
			*argc = 1;
			return (var(*)())file__delete;
		} else if (!id_cmp(id, "folder__list")) {
			*argc = 1;
			return (var(*)())folder__list;
		} else if (!id_cmp(id, "folder__create")) {
			*argc = 1;
			return (var(*)())folder__create;
		} else if (!id_cmp(id, "folder__delete")) {
			*argc = 1;
			return (var(*)())folder__delete;
		} else if (!id_cmp(id, "file__save")) {
			*argc = 4;
			return (var(*)())file__save;
		}
		break;
	case 'p':
		if (!id_cmp(id, "print")) {
			*argc = 1;
			return (var(*)())print;
		} else if (!id_cmp(id, "print10")) {
			*argc = 1;
			return (var(*)())print10;
		} else if (!id_cmp(id, "printb")) {
			*argc = 2;
			return (var(*)())printb;
		} else if (!id_cmp(id, "print16")) {
			*argc = 1;
			return (var(*)())print16;
		} else if (!id_cmp(id, "pow")) {
			*argc = 2;
			return (var(*)())pow;
		} else if (!id_cmp(id, "panic")) {
			error("PANIC!", st);
		}
		break;
	case 'q':
		if (!id_cmp(id, "quit")) {
			*argc = 0;
			return (var(*)())quit;
		}
		break;
	case 's':
		if (!id_cmp(id, "str_cmp")) {
			*argc = 2;
			return (var(*)())str_cmp;
		} else if (!id_cmp(id, "str_dup")) {
			*argc = 1;
			return (var(*)())str_dup;
		} else if (!id_cmp(id, "singleton")) {
			*argc = 0;
			return (var(*)())singleton;
		}
		break;
	}
	return wrapper(st,clas,id,argc);
}


void call_builtin(struct trip *st, var(*built)())
{
	switch (st->nb_vars) {
	case 1:
		st->return_val = built(st->vars[0].data);
		break;
	case 2:
		st->return_val = built(st->vars[0].data, 
					st->vars[1].data);
		break;
	case 3:
		if (built == command) {
			st->return_val = command(st,
					st->vars[1].data,
					st->vars[2].data);
		} else {
			st->return_val = built(st->vars[0].data,
					st->vars[1].data,
					st->vars[2].data);
		}
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
	var oself;
	int vr;
	char *p;
	vr = 0;
	opos = st->pos;
	obuf = st->buf;
	oend = st->end;
	oself = st->__self;
	fix_pos_buf(st,func);
	whitespaces(st);
	p = st->buf + st->pos;
	if (*p != '(') {
		printf("%s\n_______%s\n", p, func);
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
				printf("--- %d %d\n", vr, st->nb_vars);
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
	if (id_cmp("this", id)) {
		st->class_name = clas;
	}
	if (meth) {
		st->func_name = meth;
	} else {
		st->func_name = id;
	}
	compound(st);
	st->__self = oself;
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
				clas = st->class_name;
				self = st->__self;
				func = get_member_ptr(st,	
					st->class_name, "method", meth);
				if (!func) {
					printf("%s.", id_tmp(st, st->class_name));
					printf("%s ", id_tmp(st, meth));
					error("method not found", st);
				}
			} else {
				if (!id_cmp("this", id) && !st->__self) {
					error("null this", st);

				}
				if (!is_ref(st, id)) {
					self = get_data(st, st->__self, st->class_name, id, 0);
				} else {
					self = get_data(st, st->__self, NULL, id, 0);
				}
				func = get_member_ptr(st, clas, "method", meth);
				if (!func) {
					built = builtin(st, clas, meth, &nb_args);
					if (!built) {
						printf ("%s.", id_tmp(st,clas));
						printf ("%s(", id_tmp(st,meth));
						error("method not found..", st);
					}
				}
			}
		} else {
			if (clas == NULL) {
				printf("((var(*)())%s", 
						id_tmp(st,st->class_name));
				printf("__%s)(self", id_tmp(st,meth));
			} else {
				printf("((var(*)())%s", id_tmp(st,clas));
				printf("__%s)((var)", id_tmp(st,meth));
				if (!is_ref(st, id)) {
					printf("__self->");
				}
				printf("%s", id_tmp(st,id));
			}
		}
		n = 1;
	} else {
		if (st->mode == MODE_INTERP) {
			if (!id_cmp("continue", id)) {
				st->continue_ = 1;
				return;
			} else if (!id_cmp("break", id)) {
				st->break_ = 1;
				return;
			}
	
			built = builtin(st, "", id, &nb_args);
			if (!built) {
				func = (char*)get_data(st, st->__self, NULL, id, 0);
			}
		} else {
			if (!id_cmp("continue", id)) {
				printf("continue;\n");
				end_of_expr(st);
				return;
			} else if (!id_cmp("break", id)) {
				printf("break;\n");
				end_of_expr(st);
				return;
			} else if (!id_cmp("command", id)) {
				printf("command((void*)");
			} else {
				printf("((var(*)())%s)(", id_tmp(st,id));
			}
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
					printf(", (var)");
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
			if (n > 0 && !is_end_of_command(st)) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf(", (var)");
				}
			}
			n++;
		default:
			if (n == 0) {
				if (st->mode == MODE_INTERP) {
				} else {
					printf("(var)");
				}
			}
			expression(st);
		}
	}

	p = st->buf + st->pos;

	if (st->mode == MODE_INTERP) {
		push_context(st);
		i = st->sp - sp;
		if (clas && built) {
			i++;
		}
		while (sp < st->sp) {
			i--;
			st->vars[i].name = "";
			st->vars[i].data = pop(st);
			st->nb_vars++;
		}
		if (clas && built) {
			i--;
			st->vars[i].name = "self";
			st->vars[i].data = self;
			st->nb_vars++;
		}
	} else {
	}

	end_of_expr(st);
	if (st->mode == MODE_INTERP) {
		if (built) {
			if (nb_args != st->nb_vars) {
				printf("%d %d\n", nb_args, st->nb_vars);
				error("wrong number of args", st);
			}
			call_builtin(st, built);
		} else if (func) {
			call_exec(st, func, id, self, clas, meth, is_v);
		} else {
			printf ("%s -- ", id_tmp(st, id));
			error("function not found", st);
		}
	} else {
		if (st->incall == 0) {
			printf(");\n");
		} else {
			printf(")");
		}
	}
	if (st->mode == MODE_INTERP) {
		if (st->sp > sp) {
			//printf ("%s -- %d %d\n", id_tmp(st, id), st->sp, sp);
		}
		st->sp = sp;
		pop_context(st);
	} else {
	}
	st->return_ = 0;
	st->else_ = 0;
	st->continue_ = 0;
	st->break_ = 0;
			
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
			if (st->class_name && st->__self == 0) {
				/*error("null this...", st);*/
			}
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
		case 'g':
			break;
		case 'i':
			if (is(st, p, "include")) {
				skip(st);
			} else if (is(st, p, "if")) {
				k_if(st);
			}
			break;
		case 'l':
			if (is(st, p, "loop")) {
				k_loop(st);
			};
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
			if (is(st, p, "wire")) {
				skip(st);
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

void wrap(struct trip *st, int n, struct cell *c, char *type)
{

	int i;
	int com;
	for (i = 0; i < n; i++) {
		st->func_name = c[i].name;
		fix_pos_buf(st, (void*)c[i].data);
		st->pos -= id_len(st->func_name);
		while (id_cmp(type, st->buf + st->pos)) {
			st->pos--;
		}
		com = 0;
		while(st->buf[st->pos] != '(') {
			st->pos++;
		}
		st->pos++;
		whitespaces(st);
		if (st->buf[st->pos] != ')') {
			com++;
		}
		while(st->buf[st->pos] != ')') {
			if (st->buf[st->pos] == ',') {
				com++;
			}
			st->pos++;
		}
		if (type[0] == 'f') {
			printf("\t\tif (!id_cmp(\"%s\", id)) {\n", id_tmp(st, st->func_name));
			printf("\t\t\t*argc = %d;\n", com);
			printf("\t\t\treturn &%s;\n", id_tmp(st, st->func_name));
			printf("\t\t}\n");
		} else {
			printf("\t\tif (!id_cmp(\"%s\", id)) {\n", id_tmp(st, st->func_name));
			printf("\t\t\t*argc = %d;\n", com + 1);
			printf("\t\t\treturn &%s", id_tmp(st, st->class_name));
			printf("__%s;\n", id_tmp(st, st->func_name));
			printf("\t\t}\n");
		}
	}
}

void wrap_class(struct trip *st, struct class_cell *c)
{
	fix_pos_buf(st, (void*)c->data);
	st->class_name = c->name;
	st->pos -= id_len(st->class_name);
	while (id_cmp("class", st->buf + st->pos)) {
		st->pos--;
	}
	printf("\tif (!id_cmp(\"%s\", clas)) {\n", id_tmp(st, st->class_name));
	wrap(st, c->nb_methods, c->methods, "method");	
	printf("\t}\n");
}

