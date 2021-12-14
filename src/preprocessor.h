#ifndef XPP_PREPROCESSOR_H
#define XPP_PREPROCESSOR_H

#include "js.h"

#include <string>
#include <utility>

class Preprocessor {
	V8Handle v8;

	std::string filename;
	std::string file;
	std::string finally;
	std::string prefix1				= "%#";
	std::string prefix2				= "%##";
	bool		kern_en_dashes		= false;
	bool		collapse_linebreaks = false;
	bool		has_finally			= false;

	void DoPass(const std::string& prefix);
	void KernEnDashes();
	void CollapseLinebreaks();
	std::pair<size_t, size_t> FindFinally();
	void EmitFinally();
	void CollectFinally();
	void DoCollectFinally(const std::string& prefix);

public:
	Preprocessor(
		const char* program_name,
		std::string filename,
		std::string file,
		std::string prefix1,
		std::string prefix2,
		bool		kern_en_dashes,
		bool		collapse_linebreaks,
		bool		has_finally //
	);
	std::string& operator()();
};

#endif // XPP_PREPROCESSOR_H
