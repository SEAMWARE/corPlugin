#ifndef SWPLUGIN_SWPLUGIN_H_
#define SWPLUGIN_SWPLUGIN_H_

//
// FILE            swPlugin.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
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
// swPluginVersion - return the library version string
//
extern const char* swPluginVersion(void);



// -----------------------------------------------------------------------------
//
// swPluginSetBaseDir - set the plugin base directory and env var override
//
// Call once at startup before any other swPlugin function.
// The env var (if set) overrides defaultDir at runtime.
// Returns the resolved base directory.
//
extern const char* swPluginSetBaseDir(const char* defaultDir, const char* envVarName);



// -----------------------------------------------------------------------------
//
// swPluginBaseDir - get the current plugin base directory
//
extern const char* swPluginBaseDir(void);



// -----------------------------------------------------------------------------
//
// swPluginOpen - dlopen a .so file and look up a symbol by name
//
// Returns the symbol pointer on success, NULL on failure.
// Tracks handles internally for cleanup via swPluginCloseAll().
// On failure, writes an error message to errorBuf (if not NULL).
//
extern void* swPluginOpen(const char* path, const char* symbolName, char* errorBuf, int errorBufSize);



// -----------------------------------------------------------------------------
//
// swPluginCloseAll - dlclose all previously opened plugin handles
//
extern void swPluginCloseAll(void);



// -----------------------------------------------------------------------------
//
// swPluginScanNames - scan a directory for .so files, return names as separator-delimited string
//
// Writes names (with .so suffix stripped) joined by `sep` into buf.
// E.g. swPluginScanNames(".../db/currentState", '|', buf, 256) -> "dummy|mongoc"
// Returns buf on success, NULL if directory doesn't exist or has no .so files.
//
extern const char* swPluginScanNames(const char* dirPath, char sep, char* buf, int bufSize);



// -----------------------------------------------------------------------------
//
// swPluginResolve - resolve a short name to a full .so path
//
// category/subcategory/name  -> {baseDir}/{category}/{subcategory}/{name}.so
// If subcategory is NULL:      {baseDir}/{category}/{name}.so
// If name contains '/', it's treated as a full path and returned as-is.
//
extern void swPluginResolve(const char* baseDir, const char* category, const char* subcategory, const char* name, char* pathOut, int pathSize);



// -----------------------------------------------------------------------------
//
// swPluginArgUpdate - update a CLI arg's description with available plugin names
//
// Scans baseDir/subDir for .so files and sets the named arg's description
// to the list of plugin names (e.g. "dummy|mongoc").
//
extern void swPluginArgUpdate(const char* argLongName, const char* subDir);

#endif  // SWPLUGIN_SWPLUGIN_H_
