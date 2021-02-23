#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "basic_helpers.h"
char *read_file(char *file_name, bool null_terminated) {
	FILE *f = fopen(file_name, "rb");
	fseek(f, 0, SEEK_END);
	long f_size = ftell(f);
	fseek(f, 0, SEEK_SET);
   char *string = malloc(null_terminated ? f_size + 1: f_size);
	fread(string, 1, f_size, f);
   fclose(f);
	if (null_terminated)
		string[f_size] = '\0';
	return string;
}

long get_length(char *file_name){
	FILE *f = fopen(file_name, "rb");
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fclose(f);
	return length;
}