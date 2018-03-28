#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libplatform/libplatform.h"
#include "v8.h"
using namespace v8;

#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/filesystem.hpp>
using namespace std;
using namespace boost::filesystem;

#include <vector>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <numeric> 

std::vector<string> getSourceText(std::string path_dir);

int _tmain(int argc, _TCHAR* argv[])
{	
	std::vector<string> lines = getSourceText(".\\javascript");
	std:string code = std::accumulate(lines.begin(), lines.end(), std::string{}, [&](string& acc, string& val){
		return acc += " \n " + val;
	});

	// Initialize V8.
	Platform* platform = platform::CreateDefaultPlatform();
	V8::InitializePlatform(platform);
	V8::Initialize();
	// Create a new Isolate and make it the current one.
	Isolate::CreateParams create_params;
	create_params.array_buffer_allocator =
		v8::ArrayBuffer::Allocator::NewDefaultAllocator();
	Isolate* isolate = Isolate::New(create_params);
	{
		Isolate::Scope isolate_scope(isolate);
		// Create a stack-allocated handle scope.
		HandleScope handle_scope(isolate);
		// Create a new context.
		Local<Context> context = Context::New(isolate);
		// Enter the context for compiling and running the hello world script.
		Context::Scope context_scope(context);
		// Create a string containing the JavaScript source code.
		Local<String> source =
			String::NewFromUtf8(isolate, code.c_str(),
			NewStringType::kNormal).ToLocalChecked();
		// Compile the source code.
		Local<Script> script = Script::Compile(context, source).ToLocalChecked();
		// Run the script to get the result.
		auto result = script->Run(context);  

		Handle<v8::Object> global = context->Global();
		Handle<v8::Value> value = global->Get(v8::String::NewFromUtf8(isolate, "multiply"));

		if (value->IsFunction()) {
			Handle<v8::Function> func = v8::Handle<v8::Function>::Cast(value);
			Handle<Value> args[2];
			int val1 = 45;
			int val2 = 3;
			args[0] = v8::String::NewFromUtf8(isolate, std::to_string(val1).c_str());
			args[1] = v8::String::NewFromUtf8(isolate, std::to_string(val2).c_str());

			Handle<Value> js_result = func->Call(global, 2, args);

			if (js_result->IsInt32()) {
				int32_t result = js_result->ToInt32()->Int32Value();
				// do something with the result
				printf("%d * %d = %d\n", val1, val2, result);
			}
		}
	}

	// Dispose the isolate and tear down V8.
	isolate->Dispose();
	V8::Dispose();
	V8::ShutdownPlatform();
	delete platform;
	delete create_params.array_buffer_allocator;

	system("pause");

	return 0;
}

std::vector<string> getSourceText(std::string path_dir)
{
	std::vector<string> res;
	path p(path_dir);
	std::vector<directory_entry> v; // To save the file names in a vector.

	if (is_directory(p))
	{
		copy(directory_iterator(p), directory_iterator(), back_inserter(v));
		//std::cout << p << " is a directory containing:\n";
		for (std::vector<directory_entry>::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			//std::cout << (*it).path().string() << endl;
			std::vector<string> partial = getSourceText((*it).path().string());
			res.insert(res.end(), partial.begin(), partial.end());
		}
	}
	else
	{
		//   if is file
		boost::iostreams::stream<boost::iostreams::mapped_file_source> str(path_dir);
		if (!str.eof())
		{
			for (std::string x; std::getline(str, x, '\n');)
			{
				res.push_back(x);
			}
		}		
	}

	return res;

}