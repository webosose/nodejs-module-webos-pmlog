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

var pmloglib = require('pmloglib');

var o = {
    name: "Nutmeg",
    sex: "Female"
}

pmloglib.info("Info: Hello, %s", "Rob");
pmloglib.warn("Warn: Hello, %s", "Rob");
pmloglib.error("Error: Hello, %s", "Rob");
pmloglib.error("Error: Hello, %j", o, 3, [1,2], {a:1});
pmloglib.log("Log: Hello, %s", "Rob");
pmloglib.log("Log: Hello, %d", 23);

var context = new pmloglib.Context("com.lge.mycomponent");
context.log(pmloglib.LOG_CRITICAL, "APPCRASH", {"APP_NAME": "Facebook", "APP_ID": 12}, "Facebook app crashed, restart application");
context.log(pmloglib.LOG_ERR, "APPCRASH", {"APP_NAME": "Facebook", "APP_ID": 12}, "Facebook app crashed, restart application");
context.log(pmloglib.LOG_WARNING, "APPCRASH", {"APP_NAME": "Facebook", "APP_ID": 12}, "Facebook app crashed, restart application");
context.log(pmloglib.LOG_INFO, "APPCRASH", {"APP_NAME": "Facebook", "APP_ID": 12}, "Facebook app crashed, restart application");
context.log(pmloglib.LOG_DEBUG, "APPCRASH", {"APP_NAME": "Facebook", "APP_ID": 12}, "Facebook app crashed, restart application");

var console = new pmloglib.Console("com.lge.myComponent");
console.info("info message");
console.log("log message");
console.warn("warn message");
console.error("error message");

