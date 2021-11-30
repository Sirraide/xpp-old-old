#include "utils.h"

__attribute__((noreturn)) void die(const char *format, ...) noexcept {
	va_list args;
	va_start(args, format);
	fprintf(stderr, "\033[31m");
	vfprintf(stderr, format, args);
	fprintf(stderr, "\033[0m\n");
	va_end(args);
	exit(1);
}

__attribute__((noreturn)) void usage(FILE *stream) noexcept {
	fflush(stream);
	fprintf(stream,
		"Usage: xpp <file> [options]\n"
		"\n"
		"Options:\n"
		"    --help                 display this help message\n"
		"    -o, --output <file>    output to <file> instead of stdout\n"
		"    -p, --prefix <str>     set the prefix for pass 1\n"
		"    -2, --prefix2 <str>    set the prefix for pass 2\n"
		"    --kern-en-dashes       insert kerning around '--' if surrounded by digits\n");
	exit(stream == stderr ? 1 : 0);
}