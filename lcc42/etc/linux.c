/* x86s running Linux */
//20-05-10 default version xr18CX
//21-10-01 changing LCCDIR to LCCEXE for executables, defaulting to /lcc1802/ for includes
//21-10-01 using ash.sh to invoke asl - otherwise it doesn't recognize parameters
//21-10-04 changing as[] to separate parameters
#include <linux/limits.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <assert.h>

static char rcsid[] = "$Id: linux.c,v 1.5 1998/09/16 20:41:09 drh Exp $";

#ifndef LCC_PREFIX
#define LCC_PREFIX "/usr"
#endif
#ifndef LCC_SUFFIX_DIR
#define LCC_SUFFIX_DIR "/share/lcc1802/"
#endif
#ifndef LCC_SUFFIX_EXE
#define LCC_SUFFIX_EXE "/bin/"
#endif

char *suffixes[] = { ".c", ".i", ".s", ".o", ".out", 0 };
char inputs[256] = "";
char *include[] = {"-I" LCC_PREFIX LCC_SUFFIX_DIR "include", 0 };
char *cpp[] = { "cpp", "-D__STRICT_ANSI__", "$1", "$2", "$3", 0 };
char *com[] = { "rcc", "-target=xr18CX", "$1", "$2", "$3", 0 }; //wjr 20-18-10
char *as[] = { "asl", "-cpu", "1805a", "-i", LCC_PREFIX LCC_SUFFIX_DIR "include", "-L", "-quiet", "", "-o", "$3", "$1", "$2", 0 };
//char *as[] = { "/lcc1802/ash.sh", "-cpu 1802", "-i " LCCDIR "include", "-L", "-quiet", "", "-o", "$3", "$1", "$2", 0 };
char *ld[] = { "p2hex", "", "", "", "", "$2", "$3", "","",0 }; //wjr dec 12
char *peep[] = { "copt", LCC_PREFIX LCC_SUFFIX_DIR "include/lcc1806.opt", "-I", "$2", "-O", "$3", 0 }; //#wjr 20-05-10

extern char *concat(char *, char *);

int resolve_prefix(char *exe) {
	char* exe_absolute = exe;
	unsigned char exe_absolute_should_free = 0;
	char* exe_base = basename(exe);

	// if it's just "lcc" we should search PATH
	if (strcmp(exe_base, exe) == 0) {
		exe_absolute_should_free = 1;

		const char* path_env = getenv("PATH");
		const size_t path_len = strlen(path_env);

		const size_t exe_base_len = strlen(exe_base);

		char* exe_buffer = malloc(path_len + exe_base_len + 2);

		const char* path_start = path_env;
		while (*path_start != '\0') {
			const char* path_end = path_start;
			// move end cursor until we find the end or a separator
			while (*path_end != '\0' && *path_end != ':') {
				path_end++;
			}

			size_t path_entry_len = path_end - path_start;
			memcpy(exe_buffer, path_start, path_entry_len);

			// append a '/' if not present at the end
			if (exe_buffer[path_entry_len - 1] != '/') {
				exe_buffer[path_entry_len] = '/';
				memcpy(&exe_buffer[path_entry_len + 1], exe_base, exe_base_len);
				exe_buffer[path_entry_len + 1 + exe_base_len] = 0;
			} else {
				memcpy(&exe_buffer[path_entry_len], exe_base, exe_base_len);
				exe_buffer[path_entry_len + exe_base_len] = 0;
			}

			if (access(exe_buffer, X_OK) == 0) {
				exe_absolute = exe_buffer;
				break;
			}

			// start from the end of the current one
			path_start = path_end;
			if (*path_start == '\0') {
				// we found the end
				break;
			} else {
				// skip a separator
				path_start++;
			}
		}
	}

	char *prefix = exe_absolute;
	// char *prefix = realpath(exe_absolute, NULL);
	// free if we needed to allocate it
	// if (exe_absolute_should_free) free(exe_absolute);

	// slice off 2 components (.../bin/lcc)
	for (int i = 0; i < 2; i++) {
		char *prefix_last_slash = strrchr(prefix, '/');
		assert(prefix_last_slash);
		*prefix_last_slash = '\0';
	}

	include[0] = concat("-I", concat(prefix, LCC_SUFFIX_DIR "include"));
	cpp[0]     = concat(prefix, LCC_SUFFIX_EXE "cpp");
	com[0]     = concat(prefix, LCC_SUFFIX_EXE "rcc");
	// as has 2 compoennts
	as[0]      = concat(prefix, LCC_SUFFIX_EXE "asl");
	as[4]      = concat(prefix, LCC_SUFFIX_DIR "include");
	ld[0]      = concat(prefix, LCC_SUFFIX_EXE "p2hex");
	peep[0]    = concat(prefix, LCC_SUFFIX_EXE "copt");

	// free(prefix);
	if (exe_absolute_should_free) free(exe_absolute);

	return 0;
}

int option(char *arg) {
	if (strncmp(arg, "-lccdir=", 8) == 0) {
		include[0] = concat("-I", concat(&arg[8], "/include"));
		cpp[0] = concat(&arg[8], "/bin/cpp");
		com[0] = concat(&arg[8], "/bin/rcc");
		as[0] = concat(&arg[8], "/bin/asl");
		as[4] = concat(&arg[8], "/include");
		ld[0]  = concat(&arg[8], "/bin/p2hex");
		peep[0]  = concat(&arg[8], "/bin/copt");
	} else if (strcmp(arg, "-p") == 0 || strcmp(arg, "-pg") == 0) {
		//ld[7] = "/usr/lib/gcrt1.o";
		//ld[18] = "-lgmon";
		;
	} else if (strcmp(arg, "-b") == 0)
		;
	else if (strcmp(arg, "-g") == 0)
		;
	else if (strncmp(arg, "-ld=", 4) == 0)
		//ld[0] = &arg[4];
		;
	else if (strcmp(arg, "-static") == 0) {
		//ld[3] = "-static";
		//ld[4] = "";
		;
	} else
		return 0;
	return 1;
}
