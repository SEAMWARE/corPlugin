//
// FILE            swPlugin.c
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
#include <dlfcn.h>                                    // dlopen, dlsym, dlclose, dlerror
#include <dirent.h>                                   // opendir, readdir, closedir
#include <stdio.h>                                    // snprintf, fprintf
#include <stdlib.h>                                   // getenv
#include <string.h>                                   // strcmp, strchr, strncpy, strlen, memcpy
#include <stddef.h>                                   // NULL
#include <unistd.h>                                   // access

#include "kargs/KArg.h"                               // KArg, KARGS_END
#include "kargs/KArgInfo.h"                           // KArgInfo
#include "kargs/kargsInit.h"                          // kargInfoV

#include "swPlugin/version.h"                         // SWPLUGIN_VERSION
#include "swPlugin/swPlugin.h"                        // Own interface



// -----------------------------------------------------------------------------
//
// Plugin handle tracking (for dlclose on shutdown)
//
#define PLUGIN_HANDLES_MAX  32

static void*  pluginHandles[PLUGIN_HANDLES_MAX];
static int    pluginHandleCount = 0;



// -----------------------------------------------------------------------------
//
// Base directory state (set by swPluginSetBaseDir)
//
static const char* baseDir = NULL;



// -----------------------------------------------------------------------------
//
// swPluginVersion -
//
const char* swPluginVersion(void)
{
  return SWPLUGIN_VERSION;
}



// -----------------------------------------------------------------------------
//
// swPluginSetBaseDir -
//
const char* swPluginSetBaseDir(const char* defaultDir, const char* envVarName)
{
  if (envVarName != NULL)
  {
    const char* envDir = getenv(envVarName);

    if (envDir != NULL)
    {
      baseDir = envDir;
      return baseDir;
    }
  }

  baseDir = defaultDir;
  return baseDir;
}



// -----------------------------------------------------------------------------
//
// swPluginBaseDir -
//
const char* swPluginBaseDir(void)
{
  return baseDir;
}



// -----------------------------------------------------------------------------
//
// swPluginOpen -
//
void* swPluginOpen(const char* path, const char* symbolName, char* errorBuf, int errorBufSize)
{
  if (errorBuf != NULL && errorBufSize > 0)
    errorBuf[0] = 0;

  if (access(path, F_OK) != 0)
  {
    if (errorBuf != NULL)
      snprintf(errorBuf, errorBufSize, "not found");
    return NULL;
  }

  void* handle = dlopen(path, RTLD_NOW);
  if (handle == NULL)
  {
    if (errorBuf != NULL)
      snprintf(errorBuf, errorBufSize, "dlopen(%s) failed: %s", path, dlerror());
    return NULL;
  }

  // Track handle for cleanup
  if (pluginHandleCount < PLUGIN_HANDLES_MAX)
    pluginHandles[pluginHandleCount++] = handle;

  void* sym = dlsym(handle, symbolName);
  if (sym == NULL)
  {
    if (errorBuf != NULL)
      snprintf(errorBuf, errorBufSize, "dlsym(%s, %s) failed: %s", path, symbolName, dlerror());
    dlclose(handle);
    pluginHandleCount--;
    return NULL;
  }

  return sym;
}



// -----------------------------------------------------------------------------
//
// swPluginCloseAll -
//
void swPluginCloseAll(void)
{
  for (int i = 0; i < pluginHandleCount; i++)
  {
    if (pluginHandles[i] != NULL)
    {
      dlclose(pluginHandles[i]);
      pluginHandles[i] = NULL;
    }
  }

  pluginHandleCount = 0;
}



// -----------------------------------------------------------------------------
//
// swPluginScanNames -
//
const char* swPluginScanNames(const char* dirPath, char sep, char* buf, int bufSize)
{
  DIR* dir = opendir(dirPath);
  if (dir == NULL)
    return NULL;

  int pos   = 0;
  int count = 0;
  struct dirent* entry;

  while ((entry = readdir(dir)) != NULL)
  {
    const char* name = entry->d_name;
    int len = strlen(name);

    if (len < 4 || strcmp(name + len - 3, ".so") != 0)
      continue;

    if (count > 0 && pos < bufSize - 1)
      buf[pos++] = sep;

    int nameLen = len - 3;  // strip ".so"
    if (pos + nameLen < bufSize)
    {
      memcpy(buf + pos, name, nameLen);
      pos += nameLen;
    }

    count++;
  }

  closedir(dir);

  if (count == 0)
    return NULL;

  buf[pos] = '\0';
  return buf;
}



// -----------------------------------------------------------------------------
//
// swPluginResolve -
//
void swPluginResolve(const char* pluginBaseDir, const char* category, const char* subcategory, const char* name, char* pathOut, int pathSize)
{
  if (strchr(name, '/') != NULL)
  {
    strncpy(pathOut, name, pathSize - 1);
    pathOut[pathSize - 1] = '\0';
    return;
  }

  if (subcategory != NULL)
    snprintf(pathOut, pathSize, "%s/%s/%s/%s.so", pluginBaseDir, category, subcategory, name);
  else
    snprintf(pathOut, pathSize, "%s/%s/%s.so", pluginBaseDir, category, name);
}



// -----------------------------------------------------------------------------
//
// swPluginArgUpdate - update a CLI arg's description with available plugin names
//
#define ARG_UPDATE_MAX  8

void swPluginArgUpdate(const char* argLongName, const char* subDir)
{
  static char  namesBufV[ARG_UPDATE_MAX][256];
  static int   namesBufIx = 0;

  if (baseDir == NULL || namesBufIx >= ARG_UPDATE_MAX)
    return;

  char dirPath[512];
  snprintf(dirPath, sizeof(dirPath), "%s/%s", baseDir, subDir);

  char* buf = namesBufV[namesBufIx];
  const char* names = swPluginScanNames(dirPath, '|', buf, sizeof(namesBufV[0]));

  if (names == NULL)
    return;

  namesBufIx++;

  for (int i = 0; kargInfoV[i].type != KaEnd; i++)
  {
    if (kargInfoV[i].longName != NULL && strcmp(kargInfoV[i].longName, argLongName) == 0)
    {
      kargInfoV[i].description = buf;
      break;
    }
  }
}
