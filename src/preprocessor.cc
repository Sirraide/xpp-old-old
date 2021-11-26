#include "preprocessor.h"

#include "utils.h"

#include <cstring>
#include <regex>
#include <string>
#include <utility>
#include <vector>

void ReplaceAll(std::string &str, const std::string &from, const std::string &to) {
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

Preprocessor::Preprocessor(const char *program_name, std::string _file, std::string _prefix1, std::string _prefix2, bool _kern_en_dashes)
	: v8(V8Handle(program_name)), file(std::move(_file)), prefix1(std::move(_prefix1)), prefix2(std::move(_prefix2)), kern_en_dashes(_kern_en_dashes) {
}

std::string VecToStr(const std::vector<std::string> &vec) {
	if (vec.empty()) return "";
	if (vec.size() == 1) return vec[0];
	std::string ret = vec[0];
	for (size_t i = 1; i < vec.size(); i++) ret += ", " + vec[i];
	return ret;
}

std::vector<std::string> StringSplit(const std::string &str) {
	std::vector<std::string> elems;
	std::stringstream		 ss(str);
	while (ss.good()) {
		std::string tmp;
		ss >> tmp;
		elems.push_back(tmp);
	}
	return elems;
}

void Preprocessor::DoPass(const std::string &prefix) {
	const size_t prefix_len = prefix.size();
	size_t		 last		= 0;

	for (;;) {
		size_t pos = file.find(prefix + "eval", last);
		if (pos == std::string::npos) break;
		size_t end = pos;

		v8("__injectbuf = '';");

		std::string signature;
		bool		replace = false;
		while (end < file.size() && file[end] != '\n') end++;
		std::string buf = file.substr(pos, end - pos);

		bool defun = buf.starts_with(prefix + "eval defun");
		if (defun || buf.starts_with(prefix + "eval begin")) {
			signature	 = buf.substr(buf.find('n') + 1);
			size_t start = end;
			end			 = file.find(prefix + "eval end");
			if (end == std::string::npos) die("%seval begin/defun terminated by end of file", prefix.data());
			buf = file.substr(start, end - start);
			end += strlen("eval end") + prefix_len;
		} else if (buf.starts_with(prefix + "eval replace")) {
			buf		= buf.substr(prefix_len + strlen("eval replace"));
			replace = true;
		} else buf = buf.substr(prefix_len + strlen("eval"));

		std::string tail = file.substr(end);
		file			 = file.substr(0, pos);
		if (defun) {
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
		} else if (replace) {
			std::stringstream ss(std::move(buf));
			std::string		  from, to;
			if (!ss.good()) die("Missing replacement after %seval replace", prefix.data());
			ss >> from;
			if (!ss.good()) die("Missing replacement after %seval replace %s", prefix.data(), from.data());
			ss >> to;
			ReplaceAll(to, "<SP>", " ");
			ReplaceAll(tail, from, to);
		} else {
			std::string code = "{" + buf + "}";
			// std::cout << "Code: " << code << "\n";
			v8(code);
			auto injected = v8("__injectbuf");
			if (!injected->IsNullOrUndefined()) file += *v8::String::Utf8Value(v8.isolate, injected);
		}
		file += tail;
		last = pos;
	}
}

void Preprocessor::KernEnDashes() {
	std::regex r("(\\d)(–|--)(\\d)");
	file = std::regex_replace(file, r, "$1\\kern1pt--\\kern1pt$3");
}

std::string &Preprocessor::operator()() {
	V8HANDLE_INIT(v8);
	v8("let __injectbuf = '';\n"
	   "function inject(str) { __injectbuf += str; return ''; }");
	DoPass(prefix1);
	DoPass(prefix2);
	if (kern_en_dashes) KernEnDashes();
	return file;
}
