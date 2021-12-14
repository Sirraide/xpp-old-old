#include "preprocessor.h"

#include "utils.h"

#include <cctype>
#include <cstring>
#include <regex>
#include <string>
#include <utility>
#include <vector>

void ReplaceAll(std::string& str, const std::string& from, const std::string& to) {
	const size_t to_len	  = to.size();
	const size_t from_len = from.size();
	size_t		 where	  = 0;
	for (;;) {
		auto pos = str.find(from, where);
		if (pos == std::string::npos) return;
		str.replace(pos, from_len, to);
		where = pos + to_len;
	}
}

Preprocessor::Preprocessor(const char* program_name, std::string _filename, std::string _file, std::string _prefix1,
	std::string _prefix2, bool _kern_en_dashes, bool _collapse_linebreaks, bool _has_finally)
	: v8(V8Handle(program_name)), filename(std::move(_filename)), file(std::move(_file)),
	  prefix1(std::move(_prefix1)), prefix2(std::move(_prefix2)), kern_en_dashes(_kern_en_dashes),
	  collapse_linebreaks(_collapse_linebreaks), has_finally(_has_finally) {}

std::string VecToStr(const std::vector<std::string>& vec) {
	if (vec.empty()) return "";
	if (vec.size() == 1) return vec[0];
	std::string ret = vec[0];
	for (size_t i = 1; i < vec.size(); i++) ret += ", " + vec[i];
	return ret;
}

std::vector<std::string> StringSplit(const std::string& str) {
	std::vector<std::string> elems;
	std::stringstream		 ss(str);
	while (ss.good()) {
		std::string tmp;
		ss >> tmp;
		elems.push_back(tmp);
	}
	return elems;
}

std::string TrimStr(const std::string& str) {
	size_t start = 0;
	size_t end	 = str.size() - 1;
	while (start < str.size() && isspace(str[start])) start++;
	while (isspace(str[end])) end--;
	return str.substr(start, end - start + 1);
}

enum class EvaluationMode {
	replace,
	no_arg,
	include,
	defun,
	block,
};

void Preprocessor::DoPass(const std::string& prefix) {
	const size_t prefix_len = prefix.size();
	size_t		 last		= 0;

	for (;;) {
		size_t pos = file.find(prefix + "eval", last);
		if (pos == std::string::npos) break;
		size_t end = pos;

		v8("__injectbuf = '';");

		std::string	   signature;
		EvaluationMode mode = EvaluationMode::no_arg;
		while (end < file.size() && file[end] != '\n') end++;
		std::string buf = file.substr(pos, end - pos);

		bool function_definition = buf.starts_with(prefix + "eval defun");
		if (function_definition || buf.starts_with(prefix + "eval begin")) {
			signature	 = buf.substr(buf.find('n') + 1);
			size_t start = end;
			end			 = file.find(prefix + "eval end");
			if (end == std::string::npos) die("%seval begin/defun terminated by end of file", prefix.data());
			buf = file.substr(start, end - start);
			end += strlen("eval end") + prefix_len;
			mode = function_definition ? EvaluationMode::defun : EvaluationMode::block;
		} else if (buf.starts_with(prefix + "eval include")) {
			buf	 = TrimStr(buf.substr(prefix_len + strlen("eval include")));
			buf	 = io::infile(buf, io::perror_and_exit).mmap();
			mode = EvaluationMode::include;
		} else if (buf.starts_with(prefix + "eval replace")) {
			buf	 = buf.substr(prefix_len + strlen("eval replace"));
			mode = EvaluationMode::replace;
		} else if (buf.starts_with(prefix + "eval if")) {
			buf			= buf.substr(prefix_len + strlen("eval if"));
			auto end_if = file.find(prefix + "eval endif", last);
			if (end_if == std::string::npos) die("%seval endif terminated by end of file", prefix.data());

			auto end_if_len = prefix_len + strlen("eval endif");
			auto res		= v8(buf);
			if (res.As<v8::Boolean>()->IsFalse()) end = end_if + end_if_len;
			else file.replace(end_if, end_if_len, "");
		} else buf = buf.substr(prefix_len + strlen("eval"));

		std::string tail = file.substr(end);
		file			 = file.substr(0, pos);
		switch (mode) {
			case EvaluationMode::defun: {
				std::vector<std::string> sig_elems;
				std::stringstream		 ss(std::move(signature));
				std::string				 funcname;
				if (!ss.good()) die("Missing function name after %seval defun", prefix.data());

				ss >> funcname;
				while (ss.good()) {
					std::string tmp;
					ss >> tmp;
					sig_elems.push_back(move(tmp));
				}

				std::string function = std::string("function ")
										   .append(funcname)
										   .append("(")
										   .append(VecToStr(sig_elems))
										   .append("){")
										   .append(buf)
										   .append("}");

				// std::cout << "Function: " << function << "\n";
				v8(function);
			} break;
			case EvaluationMode::replace: {
				std::stringstream ss(std::move(buf));
				std::string		  from, to;
				if (!ss.good()) die("Missing replacement after %seval replace", prefix.data());
				ss >> from;
				if (!ss.good()) die("Missing replacement after %seval replace %s", prefix.data(), from.data());
				ss >> to;
				ReplaceAll(to, "<SP>", " ");
				ReplaceAll(tail, from, to);
			} break;
			case EvaluationMode::no_arg:
			case EvaluationMode::block: {
				std::string code = "{" + buf + "}";
				// std::cout << "Code: " << code << "\n";
				v8(code);
			} break;
			case EvaluationMode::include: {
				file += buf;
			} break;
		}

		auto injected = v8("__injectbuf");
		if (!injected->IsNullOrUndefined()) file += *v8::String::Utf8Value(v8.isolate, injected);
		v8("__injectbuf = ''");
		file += tail;
		last = pos;

		if (mode == EvaluationMode::include && has_finally) CollectFinally();
	}
}

void Preprocessor::KernEnDashes() {
	std::regex r("(\\d)(–|--)(\\d)");
	file = std::regex_replace(file, r, "$1\\endash$3");
	if (has_finally) finally += R"(\def\endash{\kern1pt--\kern1pt})";
}

void Preprocessor::CollapseLinebreaks() {
	std::regex r("\n\n\n+");
	file = std::regex_replace(file, r, "\n\n");
}

void Preprocessor::DoCollectFinally(const std::string& pr) {
	const std::string directive = pr + "eval finally";
	size_t			  pos		= 0;
	for (;;) {
		pos = file.find(directive, pos);
		if (pos == std::string::npos) return;
		auto eol  = file.find('\n', pos);
		auto pos2 = pos + directive.length() + 1;
		if (eol == std::string::npos) eol++;
		finally += file.substr(pos2, eol - pos2) + "\n";
		file = file.substr(0, pos) + (eol != std::string::npos ? file.substr(eol) : "");
	}
}

void Preprocessor::CollectFinally() {
	DoCollectFinally(prefix1);
	DoCollectFinally(prefix2);
}

std::pair<size_t, size_t> Preprocessor::FindFinally() {
	auto str = prefix1 + "finally";
	auto loc = file.find(str);
	if (loc != std::string::npos) return {loc, str.size()};
	auto str2 = prefix2 + "finally";
	loc		  = file.find(str2);
	if (loc != std::string::npos) return {loc, str2.size()};
	die("Could not find %sfinally or %sfinally", prefix1.data(), prefix2.data());
}

void Preprocessor::EmitFinally() {
	auto [pos, size] = FindFinally();
	file			 = file.replace(pos, size, finally);
}

std::string& Preprocessor::operator()() {
	V8HANDLE_INIT(v8);
	v8("let __FILE__ = '"
		+ filename
		+ "'; let __injectbuf = '';\n"
		  "function inject(str) { __injectbuf += str; return ''; }");
	if (has_finally) CollectFinally();
	DoPass(prefix1);
	DoPass(prefix2);
	if (kern_en_dashes) KernEnDashes();
	if (collapse_linebreaks) CollapseLinebreaks();
	if (has_finally) EmitFinally();
	return file;
}
