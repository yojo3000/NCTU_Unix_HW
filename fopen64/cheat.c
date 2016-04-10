#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#define KRED  "\x1B[31m"
#define RESET "\033[0m"

FILE *fopen64(const char *path, const char *mode) {
	
	FILE *f = fopen("info_output", "w");
	fprintf(f, "==========\n");
	fprintf(f, "I am in fopen64 function\n");
	fprintf(f, "FILE *fopen64(const char *path, const char *mode)\n\n");
	fprintf(f, "path: %s\n", path);		
	fprintf(f, "mode: %s\n", mode);
	fprintf(f, "==========\n");
	
	FILE *(*original_fopen)(const char*, const char*);
	// declare a function pointer

	original_fopen = dlsym(RTLD_NEXT, "fopen64");
	// save the origin fopen64
	fclose(f);
	return (*original_fopen)(path, mode);
}
