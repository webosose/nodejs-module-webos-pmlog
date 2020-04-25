// Copyright (c) 2010-2018 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <node.h>
#include <iostream>
#include <syslog.h>
#include <v8.h>

#if HAS_PMLOGLIB
#include <PmLogLib.h>
#endif

#include "pmloglib.js.h"

using namespace v8;
using namespace std;

#if !HAS_PMLOGLIB
void LogString(int level, const char* label, const char *msgId, const char* stringToLog)
{
    static bool sysLogOpened = false;
    if (!sysLogOpened) {
        openlog(label, 0, 0);
        sysLogOpened = true;
    }
    syslog(level, "%s", stringToLog);
}
static void LogKeyValueString(int level, const char *label, const char *msgId, const char *keyValues, const char *freeText)
{
    static bool sysLogOpened = false;
    if (!sysLogOpened) {
        openlog(label, 0, 0);
        sysLogOpened = true;
    }
    syslog(level, "%s %s %s", msgId, keyValues, freeText);
}
#else
static void LogString(int level, const char* label, const char *msgId, const char* stringToLog)
{
        PmLogContext jsContext;
        PmLogGetContext(label, &jsContext);

	switch(level) {
	default:
		PmLogPrintInfo(jsContext, "%s", stringToLog);
		break;
	case LOG_WARNING:
		PmLogPrintWarning(jsContext, "%s", stringToLog);
		break;
	case LOG_ERR:
		PmLogPrintError(jsContext, "%s", stringToLog);
		break;
	}
}

static void LogKeyValueString(int level, const char *label, const char *msgId, const char *keyValues, const char *freeText)
{
        PmLogContext jsContext;
        PmLogGetContext(label, &jsContext);
        PmLogString(jsContext, static_cast<PmLogLevel>(level), msgId, keyValues, freeText);
}

#endif

static void LogWrapper(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    if (args.Length() != 4) {
        args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Invalid number of parameters, 3 expected."))));
        return;
    }
    
    String::Utf8Value label(isolate, args[0]);
    int logLevel = args[1]->IntegerValue(isolate->GetCurrentContext()).FromMaybe(0);
    String::Utf8Value msgId(isolate, args[2]);
    String::Utf8Value stringToLog(isolate, args[3]);
    LogString(logLevel, *label, *msgId, *stringToLog);
    args.GetReturnValue().Set(args[2]);
}

static void LogKeyValueWrapper(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    if (args.Length() < 2) {
        args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Minimum 2 parameters expected"))));
        return;
    }

    if (args.Length() > 5) {
        args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Not more than 5 parameters expected"))));
        return;
    }
    
    String::Utf8Value label(isolate, args[0]);
    int logLevel = args[1]->IntegerValue(isolate->GetCurrentContext()).FromMaybe(0);
    const char *mid = NULL;
    const char *kv = NULL;
    const char *ft = NULL;

    if (!args[1]->IsNumber()) {
        args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(
            v8::String::NewFromUtf8(isolate, "Logging level must be an integer"))));
        return;
    }

    if(logLevel != kPmLogLevel_Debug) {

	String::Utf8Value msgId(isolate, args[2]);
	String::Utf8Value keyValues(isolate, args[3]);
	String::Utf8Value freeText(isolate, args[4]);

        if (!args[2]->IsNull() && !args[2]->IsUndefined()) {
            mid = *msgId;
        } else {
            args.GetReturnValue().Set(isolate->ThrowException(v8::Exception::Error(
                v8::String::NewFromUtf8(isolate, "msgId is required for info and higher log levels"))));
            return;
        }
        if (!args[3]->IsNull() && !args[3]->IsUndefined()) {
            kv = *keyValues;
        }
        if (!args[4]->IsNull() && !args[4]->IsUndefined()) {
            ft = *freeText;
        }
	LogKeyValueString(logLevel, *label, mid, kv, ft);
        args.GetReturnValue().Set(args[4]);
        return;
    }
    else {

	String::Utf8Value freeText(isolate, args[2]);
        if (!args[2]->IsNull() && !args[2]->IsUndefined()) {
	    ft = *freeText;
	}
        LogKeyValueString(logLevel, *label, mid, kv, ft);
        args.GetReturnValue().Set(args[2]);
        return;
    }
}

extern "C" void
init(Local<Object> target)
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    HandleScope scope(isolate);
    Local<FunctionTemplate> logFunction = FunctionTemplate::New(isolate, LogWrapper);
    target->Set(v8::String::NewFromUtf8(isolate, "_logString", v8::String::kInternalizedString),
                logFunction->GetFunction(isolate->GetCurrentContext()).ToLocalChecked());
    Local<FunctionTemplate> logKeyValueFunction = FunctionTemplate::New(isolate, LogKeyValueWrapper);
    target->Set(
        v8::String::NewFromUtf8(isolate, "_logKeyValueString", v8::String::kInternalizedString),
        logKeyValueFunction->GetFunction(isolate->GetCurrentContext()).ToLocalChecked());
    target->Set(
        v8::String::NewFromUtf8(isolate, "LOG_CRITICAL", v8::String::kInternalizedString),
        Integer::New(isolate, kPmLogLevel_Critical));
    target->Set(
        v8::String::NewFromUtf8(isolate, "LOG_ERR",      v8::String::kInternalizedString),
        Integer::New(isolate, kPmLogLevel_Error));
    target->Set(
        v8::String::NewFromUtf8(isolate, "LOG_WARNING",  v8::String::kInternalizedString),
        Integer::New(isolate, kPmLogLevel_Warning));
    target->Set(
        v8::String::NewFromUtf8(isolate, "LOG_INFO",     v8::String::kInternalizedString),
        Integer::New(isolate, kPmLogLevel_Info));
    target->Set(
        v8::String::NewFromUtf8(isolate, "LOG_DEBUG",    v8::String::kInternalizedString),
        Integer::New(isolate, kPmLogLevel_Debug));
    Local<String> scriptText = v8::String::NewFromUtf8(isolate,
                                                       (const char*)pmloglib_js,
                                                       String::kNormalString,
                                                       pmloglib_js_len);
	ScriptOrigin *scriptOrigin = new ScriptOrigin(String::NewFromUtf8(isolate, "pmloglib.js"));
    Local<Script> script = Script::Compile(isolate->GetCurrentContext(),
        scriptText, scriptOrigin).ToLocalChecked();
    if (!script.IsEmpty()) {
        Local<Value> v = script->Run(isolate->GetCurrentContext()).ToLocalChecked();
        Local<Function> f = Local<Function>::Cast(v);
        Local<Value> argv[1];
        argv[0] = target;
        f->Call(isolate->GetCurrentContext(), Null(isolate), 1, &argv[0]);
    } else {
        cerr << "Script was empty." << endl;
    }
}

NODE_MODULE(pmloglib, init)

