
#include "preprocessor.h"
#include "utils.h"

#include <cstring>
#include <string>

#define FLAG(name, expr, ...)     \
	if (!strcmp(argv[i], name)) { \
		__VA_ARGS__ __VA_OPT__(   \
			:) expr;              \
		continue;                 \
	}
#define OPTION(name, expr, ...)                                               \
	if (!strcmp(argv[i], name)) {                                             \
		__VA_ARGS__ __VA_OPT__(                                               \
			:)                                                                \
			i++;                                                              \
		if (i >= argc) die("Missing argument after \033[33m%s", argv[i - 1]); \
		expr;                                                                 \
		continue;                                                             \
	}
#define ALIAS(name, label) \
	if (!strcmp(argv[i], name)) goto label;
int main(int argc, char** argv) {
	std::string infile;
	std::string ofile;
	std::string prefix1				= "%#";
	std::string prefix2				= "%##";
	bool		kern_en_dashes		= false;
	bool		collapse_linebreaks = false;
	bool		has_finally			= false;
	bool		skip_pass2			= false;

	for (int i = 1; i < argc; i++) {
		OPTION("--prefix", prefix1 = argv[i], prefix)
		OPTION("--prefix2", prefix2 = argv[i], prefix2)
		OPTION("--output", ofile = argv[i], output)
		FLAG("--help", usage(stdout))
		FLAG("--kern-en-dashes", kern_en_dashes = true)
		FLAG("--collapse-linebreaks", collapse_linebreaks = true)
		FLAG("--finally", has_finally = true)
		FLAG("--skip-pass2", skip_pass2 = true)
		ALIAS("-p", prefix)
		ALIAS("-2", prefix2)
		ALIAS("-o", output)
		if (!infile.empty())
			die("Unknown option \033[33m%s\033[31m\nMay only specify at most one file", argv[i]);
		infile = argv[i];
	}
	if (infile.empty()) die("No file specified");
	if (ofile.empty()) {
		size_t pos = infile.find(".tex");
		if (pos != std::string::npos) {
			ofile = infile;
			ofile.replace(pos, 4, ".preprocessed.tex");
		} else ofile = infile + ".preprocessed.tex";
	}

	std::string file = io::infile(infile, io::perror_and_exit).mmap();
	file			 = Preprocessor(argv[0], infile, file, prefix1, prefix2, kern_en_dashes, collapse_linebreaks, has_finally, skip_pass2)();
	io::ofile(ofile, io::perror_and_exit).write(file);
}
