#ifndef XPP_PREPROCESSOR_H
#define XPP_PREPROCESSOR_H

#include "js.h"

#include <string>

class Preprocessor {
	V8Handle v8;

	std::string file;
	std::string prefix1		   = "%#";
	std::string prefix2		   = "%##";
	bool		kern_en_dashes = false;

	void DoPass(const std::string &prefix);
	void KernEnDashes();

public:
	Preprocessor(
		const char *program_name,
		std::string file,
		std::string prefix1,
		std::string prefix2,
		bool		kern_en_dashes //
	);
	std::string &operator()();
};

#endif // XPP_PREPROCESSOR_H
