/* dlfcn stub for the Xbox 360 build of libdvdread.
   No dynamic loading exists on the platform, so dlopen always fails and
   libdvdread falls back to its plain file-based input functions
   (unencrypted discs / pre-decrypted images only). */
#ifndef LIBDVD_X360_DLFCN_H
#define LIBDVD_X360_DLFCN_H

#define RTLD_LAZY 1

static void* dlopen(const char* filename, int flag) { return NULL; }
static void* dlsym(void* handle, const char* name) { return NULL; }
static int dlclose(void* handle) { return 0; }
static const char* dlerror(void) { return "dlopen unsupported on Xbox 360"; }

#endif /* LIBDVD_X360_DLFCN_H */
