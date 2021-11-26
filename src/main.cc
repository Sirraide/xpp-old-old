#include "utils.h"

#include <cstdarg>
#include <cstring>
#include <iostream>
#include <string>
#include "js.h"

std::string infile;
std::string ofile;
std::string pass1_prefix;
std::string pass2_prefix;
bool		kern_en_dashes = false;

__attribute__((noreturn)) void die(const char *format, ...) noexcept {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	exit(1);
}

__attribute__((noreturn)) void usage(FILE *stream) noexcept {
	fflush(stream);
	fprintf(stream,
		"Usage: xpp <file> [options]\n"
		"\n"
		"Options:\n"
		"    -o, --output <file>    output to <file> instead of stdout\n"
		"    -p, --prefix <str>     set the prefix for pass 1\n"
		"    -2, --prefix2 <str>    set the prefix for pass 2\n");
	exit(stream == stderr ? 1 : 0);
}

#define FLAG(name, var)
#define OPTION(name, expr)                                             \
	if (!strcmp(argv[i], name)) {                                \
		i++;                                                           \
		if (i == argc) die("Missing argument after '" name "'"); \
		infile = argv[i];                                              \
		continue;                                                      \
	}

void parse_args(int argc, char **argv) {
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--prefix")) {
			i++;
			if (i == argc) die("Missing argument <str> after '--prefix'");
			infile = argv[i];
			continue;
		}
	}
}

int main(int argc, char **argv) {
	V8HANDLE(v8);

	v8("let out = 'bla'");
	v8("println(out)");

	//parse_args(argc, argv);
	//usage(stdout);
	return 0;
}

