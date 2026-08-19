#ifndef CORPLUGIN_H_
#define CORPLUGIN_H_

//
// FILE            corPlugin.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//



// -----------------------------------------------------------------------------
//
// corPluginVersion - return the library version string
//
extern const char* corPluginVersion(void);



// -----------------------------------------------------------------------------
//
// corPluginSetBaseDir - set the plugin base directory and env var override
//
// Call once at startup before any other corPlugin function.
// The env var (if set) overrides defaultDir at runtime.
// Returns the resolved base directory.
//
extern const char* corPluginSetBaseDir(const char* defaultDir, const char* envVarName);



// -----------------------------------------------------------------------------
//
// corPluginBaseDir - get the current plugin base directory
//
extern const char* corPluginBaseDir(void);



// -----------------------------------------------------------------------------
//
// corPluginOpen - dlopen a .so file and look up a symbol by name
//
// Returns the symbol pointer on success, NULL on failure.
// Tracks handles internally for cleanup via corPluginCloseAll().
// On failure, writes an error message to errorBuf (if not NULL).
//
extern void* corPluginOpen(const char* path, const char* symbolName, char* errorBuf, int errorBufSize);



// -----------------------------------------------------------------------------
//
// corPluginCloseAll - dlclose all previously opened plugin handles
//
extern void corPluginCloseAll(void);



// -----------------------------------------------------------------------------
//
// corPluginScanNames - scan a directory for .so files, return names as separator-delimited string
//
// Writes names (with .so suffix stripped) joined by `sep` into buf.
// E.g. corPluginScanNames(".../db/currentState", '|', buf, 256) -> "dummy|mongoc"
// Returns buf on success, NULL if directory doesn't exist or has no .so files.
//
extern const char* corPluginScanNames(const char* dirPath, char sep, char* buf, int bufSize);



// -----------------------------------------------------------------------------
//
// corPluginResolve - resolve a short name to a full .so path
//
// category/subcategory/name  -> {baseDir}/{category}/{subcategory}/{name}.so
// If subcategory is NULL:      {baseDir}/{category}/{name}.so
// If name contains '/', it's treated as a full path and returned as-is.
//
extern void corPluginResolve(const char* baseDir, const char* category, const char* subcategory, const char* name, char* pathOut, int pathSize);



// -----------------------------------------------------------------------------
//
// corPluginArgUpdate - update a CLI arg's description with available plugin names
//
// Scans baseDir/subDir for .so files and sets the named arg's description
// to the list of plugin names (e.g. "dummy|mongoc").
//
extern void corPluginArgUpdate(const char* argLongName, const char* subDir);

#endif  // CORPLUGIN_H_
