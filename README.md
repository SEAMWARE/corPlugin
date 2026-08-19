# corPlugin — Generic Plugin-Loading Library

A small, dependency-light wrapper around `dlopen`/`dlsym` for C: it resolves a
short plugin **name** to a `.so` path under a configurable base directory, opens it,
looks up a named register symbol, and tracks every handle so they can all be closed
at shutdown. The coraine NGSI-LD context broker uses it to load its database,
temporal and API plugins, but the library knows nothing about any of those — it is a
generic loader.

- **Version:** 0.1.0
- **Language:** C
- **License:** [Apache License 2.0](LICENSE) — Copyright 2026 Seamware

## Features

- **Base-dir resolution** — a configurable plugin root with an environment-variable
  override, set once at startup.
- **Name → path** — `category[/subcategory]/name` maps to
  `<base>/<category>/<subcategory>/<name>.so`; a name containing `/` is treated as
  an explicit path and used verbatim.
- **Open + symbol lookup** — `dlopen` a `.so` and `dlsym` a register symbol in one
  call, with a caller-supplied error buffer.
- **Handle tracking** — every opened handle is remembered and released together via
  a single `corPluginCloseAll()`.
- **Directory scan** — enumerate the available plugin names in a directory (used to
  fill in `--help` text for plugin-selecting CLI args).

## API reference

```c
#include "corPlugin/corPlugin.h"
```

```c
// Library version string
const char* corPluginVersion(void);

// Call once at startup, before anything else. The env var (if set) overrides
// defaultDir at runtime. Returns the resolved base directory.
const char* corPluginSetBaseDir(const char* defaultDir, const char* envVarName);
const char* corPluginBaseDir(void);

// dlopen `path` and dlsym `symbolName`. Returns the symbol pointer, or NULL on
// failure (writing a message into errorBuf). The handle is tracked internally.
void* corPluginOpen(const char* path, const char* symbolName, char* errorBuf, int errorBufSize);

// dlclose every handle opened so far.
void  corPluginCloseAll(void);

// Resolve a short name to a full .so path:
//   category/subcategory/name -> {base}/{category}/{subcategory}/{name}.so
//   subcategory == NULL        -> {base}/{category}/{name}.so
//   name contains '/'          -> used as-is (explicit path)
void  corPluginResolve(const char* baseDir, const char* category,
                      const char* subcategory, const char* name,
                      char* pathOut, int pathSize);

// Scan a directory for *.so and return the names (suffix stripped) joined by `sep`,
// e.g. "dummy|mongoc". Returns NULL if the dir is missing or has no .so files.
const char* corPluginScanNames(const char* dirPath, char sep, char* buf, int bufSize);

// Set a CLI arg's help text to the list of plugin names found under baseDir/subDir.
void  corPluginArgUpdate(const char* argLongName, const char* subDir);
```

## Usage example

A host application that loads a DB plugin exporting `void dbRegister(DbDriver*)`:

```c
#include "corPlugin/corPlugin.h"

// 1. Establish the plugin root (env var overrides the default at runtime)
corPluginSetBaseDir("/opt/seamware/plugins", "SEAMWARE_PLUGIN_DIR");

// 2. Resolve "mongoc" → /opt/seamware/plugins/db/currentState/mongoc.so
char path[512];
corPluginResolve(corPluginBaseDir(), "db", "currentState", "mongoc", path, sizeof(path));

// 3. Open it and look up the register symbol
char  err[256];
void* sym = corPluginOpen(path, "dbRegister", err, sizeof(err));
if (sym == NULL)
  errorExit("plugin load failed: %s", err);

typedef void (*DbRegisterFunc)(DbDriver*);
((DbRegisterFunc) sym)(&db);   // hand the driver struct to the plugin to fill

// … run …

// 4. Release every opened handle at shutdown
corPluginCloseAll();
```

Because resolution is just string-building, a developer can also point straight at
a build tree by passing a path-like name (anything with a `/`), bypassing the base
dir entirely — handy for testing a freshly built `.so` without installing it.

## Building

```bash
make            # build libcorPlugin.a (+ .so)
make ci         # clean + install
make di         # debug + install
```

`libcorPlugin.a` links statically into its consumers. The consuming binary should be
linked so that plugin `.so`s can resolve shared symbols back into it at load time
(e.g. `-rdynamic`), since plugins typically call into the host rather than
re-linking shared libraries.

## Dependencies

- [`kargs`](https://gitlab.com/kzangeli/kargs) — CLI argument handling (used by `corPluginArgUpdate`)
- `dl` — the dynamic-loader (`dlopen`/`dlsym`/`dlclose`)
- `pthread`
