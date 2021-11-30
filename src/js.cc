#include "js.h"

#include "utils.h"

#include <cassert>
#include <libplatform/libplatform.h>

using namespace v8;

static void PrintImpl(const FunctionCallbackInfo<Value> &args) {
	for (int i = 0; i < args.Length(); i++) {
		HandleScope scope(args.GetIsolate());
		printf("%s", *String::Utf8Value(args.GetIsolate(), args[i]));
	}
	fflush(stdout);
}

static void PrintlnImpl(const FunctionCallbackInfo<Value> &args) {
	PrintImpl(args);
	printf("\n");
}

V8Handle::V8Handle(const char *program_name) {
	V8::InitializeICUDefaultLocation(program_name);
	V8::InitializeExternalStartupData(program_name);
	platform = platform::NewDefaultPlatform();
	V8::InitializePlatform(platform.get());
	V8::Initialize();

	params.array_buffer_allocator = ArrayBuffer::Allocator::NewDefaultAllocator();
	isolate						  = Isolate::New(params);
}

void V8Handle::init(v8::Isolate::Scope *_isolate_scope, v8::Context::Scope *_ctx_scope) {
	isolate_scope = _isolate_scope;
	ctx_scope	  = _ctx_scope;
}

Local<Value> V8Handle::operator()(const std::string &code) const {
	TryCatch	  try_catch(isolate);
	ScriptOrigin  origin(isolate, String::NewFromUtf8(isolate, "(xpp)").ToLocalChecked());
	Local<Script> script;
	Local<String> src = String::NewFromUtf8(isolate, code.data()).ToLocalChecked();

	if (!Script::Compile(ctx, src, &origin).ToLocal(&script)) Exception(&try_catch);

	Local<Value> result;
	if (!script->Run(ctx).ToLocal(&result)) {
		assert(try_catch.HasCaught());
		Exception(&try_catch);
	}

	assert(!try_catch.HasCaught());
	return result;
}

V8Handle::~V8Handle() {
	isolate->Dispose();
	V8::Dispose();
	V8::ShutdownPlatform();
	delete params.array_buffer_allocator;
}
void V8Handle::CreateContext() {
	global = ObjectTemplate::New(isolate);
	global->Set(isolate, "print", FunctionTemplate::New(isolate, PrintImpl));
	global->Set(isolate, "println", FunctionTemplate::New(isolate, PrintlnImpl));
	ctx = Context::New(isolate, nullptr, global);
}

__attribute__((noreturn)) void V8Handle::Exception(v8::TryCatch *try_catch) const {
	HandleScope		  handle_scope(isolate);
	String::Utf8Value exception(isolate, try_catch->Exception());
	Local<Message>	  message = try_catch->Message();

	if (message.IsEmpty()) fprintf(stderr, "%s\n", *exception);
	else {
		Local<Value> stack_trace;
		if (try_catch->StackTrace(ctx).ToLocal(&stack_trace)
			&& stack_trace->IsString()
			&& stack_trace.As<String>()->Length() > 0)
			fprintf(stderr, "%s\n", *String::Utf8Value(isolate, stack_trace));

		fprintf(stderr, "%s:%i: %s\n",
			*String::Utf8Value(isolate, message->GetScriptOrigin().ResourceName()),
			message->GetLineNumber(ctx).FromJust(),
			*exception);
		fprintf(stderr, "%s\n",
			*String::Utf8Value(isolate, message->GetSourceLine(ctx).ToLocalChecked()));
		for (int i = 0, end = message->GetStartColumn(ctx).FromJust(); i < end; i++) fprintf(stderr, " ");
		for (int i = 0, end = message->GetEndColumn(ctx).FromJust(); i < end; i++) fprintf(stderr, "~");
		fprintf(stderr, "\n");
	}

	exit(1);
}
