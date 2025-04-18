

#include "../src/std.h"

var (*wrapper(struct trip *st, char *clas, char *id, int *argc))()
{
	return (void*)0;
}


int shell()
{
	struct buffer buf = {0};
	struct trip *st;
	int c;

	st = trip__new();
	st->ctx = 0;
	st->mode = MODE_INTERP;
	st->pos = 0;
	st->func_name = "command line";
	st->file_name = NULL;
	st->class_name = NULL;
	st->__self = 0;

	c = 0;
	while (1) {
		while (1) {
			c = fgetc(stdin);
			if (c == EOF) {
				break;
			}
			buffer__append((var)&buf, (var)&c, 1);
			if (c == '\n') {
				break;
			}
		}
		if (c == EOF) {
			break;
		}
		st->buf = (void*)buf.data;
		st->end = buf.length;
		compound(st);
		st->break_ = 0;
		st->pos = st->end;
	}

	trip__delete(st);
	return 0;
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
		load_include(st, argv[2]);
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
				printf("var (*wrapper(struct trip *st, char *clas, char *id, int *argc))() {\n");
				wrap(st, st->nb_funcs, st->funcs, "func");
				for (i = 0; i < st->nb_classes; i++) {
					wrap_class(st, 
						st->classes + i);
				}
				printf("\treturn (void*)0;\n}\n");
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
		return 0;
	}
	return shell();
}

