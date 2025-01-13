#include <stdio.h>
#include <stdlib.h>
#include <string.h>

var file_size(var path)
{
	FILE *fp;
	var si;
	fp = fopen((char*)path, "rb");
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	si = ftell(fp);
	fclose(fp);
	return si;
}

var file_load(var path, var offset, var size)
{
	char *buf;
	FILE *fp;
	var ret;
	fp = fopen((char*)path, "rb");
	if (!fp) {
		return 0;
	}
	buf = malloc(size+1);
	if (!buf) {
		return 0;
	}
	fseek(fp, offset, SEEK_SET);
	ret = fread(buf, 1, size, fp);
	if (ret != size) {
		free(buf);
		buf = 0;
	}
	buf[size] = '\0';
	fclose(fp);
	return (var)buf;
}

var file_save(var path, var offset, var size, var buf)
{
	FILE *fp;
	var ret;
	fp = fopen((char*)path, "rb+");
	if (!fp) {
		fp = fopen((char*)path, "wb+");
		if (!fp) {
			return -1;
		}
	}
	fseek(fp, offset, SEEK_SET);
	ret = fwrite((void*)buf, 1, size, fp);
	fclose(fp);
	return ret;
}

var str_cmp(var a, var b)
{
	return strcmp((void*)a, (void*)b);
}

var str_dup(var a)
{
	return (var)strdup((void*)a);
}

