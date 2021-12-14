#ifndef XPP_JS_H
#define XPP_JS_H

#include <optional>
#include <string>
#include <v8.h>

#define V8HANDLE_INIT(handle_name)                         \
	v8::Isolate::Scope isolate_scope(handle_name.isolate); \
	v8::HandleScope	   handle_scope(handle_name.isolate);  \
	handle_name.CreateContext();                           \
	v8::Context::Scope ctx_scope(handle_name.ctx);         \
	handle_name.init(&isolate_scope, &ctx_scope)

using value = v8::Local<v8::Value>;

class V8Handle {
	std::unique_ptr<v8::Platform> platform;
	v8::Isolate::CreateParams	  params;
	v8::Isolate::Scope*			  isolate_scope;
	v8::Context::Scope*			  ctx_scope;

	void Exception(v8::TryCatch* try_catch) const;

public:
	v8::Isolate*				  isolate;
	v8::Local<v8::ObjectTemplate> global;
	v8::Local<v8::Context>		  ctx;
	explicit V8Handle(const char* program_name);
	~V8Handle();

	V8Handle(const V8Handle&) = delete;
	V8Handle(V8Handle&&)	  = delete;
	V8Handle& operator=(const V8Handle&) = delete;
	V8Handle& operator=(V8Handle&&) = delete;

	void CreateContext();
	auto operator()(const std::string& code) const -> v8::Local<v8::Value>;
	void init(
		v8::Isolate::Scope* _isolate_scope,
		v8::Context::Scope* _ctx_scope //
	);
};

#endif // XPP_JS_H
