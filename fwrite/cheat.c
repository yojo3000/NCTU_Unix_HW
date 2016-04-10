#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
void begin (void) __attribute__((constructor));
void end (void) __attribute__((destructor));

//static FILE *(*original_fopen)(const char*, const char*);
static size_t *(*original_fwrite)(const void *ptr, size_t size, size_t nmenb, FILE *stream);
static int (*original_fprintf)(FILE*, const char*, ...);
static FILE *f;

void begin (void)
{
		
	original_fprintf = dlsym(RTLD_NEXT, "fprintf");
//	original_fopen = dlsym(RTLD_NEXT, "fopen");
	original_fwrite = dlsym(RTLD_NEXT, "fwrite");
		
}
void end (void)
{
	fclose(f);
}

size_t fwrite(const void *ptr, size_t size, size_t nmenb, FILE *stream) {
		
	f = fopen("info_output", "w");
	original_fprintf(f,"==========\n");
	original_fprintf(f, "I am in fwrite function\n");
	original_fprintf(f, "size_t fwrite(const void *ptr, size_t size, size_t nmenb, FILE *stream)\n");
	original_fprintf(f, "ptr:	%p\n", ptr);
	original_fprintf(f, "size:	%d\n", size);
	original_fprintf(f, "nmenb:	%d\n", nmenb);
	original_fprintf(f,"==========\n");
	
	// declare a function pointer
	
	return original_fwrite(ptr, size, nmenb, stream);
}
