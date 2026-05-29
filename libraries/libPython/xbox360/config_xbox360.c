/*
 * config_xbox360.c
 *
 * Built-in module table for Python 3.4.10 on Xbox 360.
 * Only includes modules that are feasible on the Xbox 360 platform.
 */

#include "Python.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Core built-in modules */
extern PyObject* PyInit_array(void);
extern PyObject* PyInit_binascii(void);
extern PyObject* PyInit_cmath(void);
extern PyObject* PyInit_errno(void);
extern PyObject* PyInit_gc(void);
extern PyObject* PyInit_math(void);
extern PyObject* PyInit__md5(void);
extern PyObject* PyInit__operator(void);
extern PyObject* PyInit__sha1(void);
extern PyObject* PyInit__sha256(void);
extern PyObject* PyInit__sha512(void);
extern PyObject* PyInit_time(void);
extern PyObject* PyInit__thread(void);
extern PyObject* PyInit__codecs(void);
extern PyObject* PyInit__weakref(void);
extern PyObject* PyInit__random(void);
extern PyObject* PyInit_itertools(void);
extern PyObject* PyInit__collections(void);
extern PyObject* PyInit__heapq(void);
extern PyObject* PyInit__bisect(void);
extern PyObject* PyInit__symtable(void);
extern PyObject* PyInit__csv(void);
extern PyObject* PyInit__sre(void);
extern PyObject* PyInit_parser(void);
extern PyObject* PyInit__struct(void);
extern PyObject* PyInit__datetime(void);
extern PyObject* PyInit__functools(void);
extern PyObject* PyInit__json(void);
extern PyObject* PyInit_xxsubtype(void);
extern PyObject* PyInit_zipimport(void);
extern PyObject* PyInit__lsprof(void);
extern PyObject* PyInit__ast(void);
extern PyObject* PyInit__io(void);
extern PyObject* PyInit__pickle(void);
extern PyObject* PyInit_atexit(void);
extern PyObject* _PyWarnings_Init(void);
extern PyObject* PyInit__string(void);
extern PyObject* PyInit__stat(void);
extern PyObject* PyInit__opcode(void);

/* CJK codecs */
extern PyObject* PyInit__multibytecodec(void);
extern PyObject* PyInit__codecs_cn(void);
extern PyObject* PyInit__codecs_hk(void);
extern PyObject* PyInit__codecs_iso2022(void);
extern PyObject* PyInit__codecs_jp(void);
extern PyObject* PyInit__codecs_kr(void);
extern PyObject* PyInit__codecs_tw(void);

/* Additional modules matching xbmc4xbox */
extern PyObject* PyInit__sqlite3(void);
extern PyObject* PyInit_zlib(void);
extern PyObject* PyInit_pyexpat(void);
extern PyObject* PyInit__elementtree(void);
extern PyObject* PyInit_unicodedata(void);
extern PyObject* PyInit_select(void);

extern PyObject* PyMarshal_Init(void);
extern PyObject* PyInit_imp(void);
extern PyObject* PyInit_nt(void);

struct _inittab _PyImport_Inittab[] = {
    {"nt", PyInit_nt},  /* Required by importlib bootstrap */
    {"array", PyInit_array},
    {"_ast", PyInit__ast},
    {"binascii", PyInit_binascii},
    {"cmath", PyInit_cmath},
    {"errno", PyInit_errno},
    {"gc", PyInit_gc},
    {"math", PyInit_math},
    {"_operator", PyInit__operator},
    {"_md5", PyInit__md5},
    {"_sha1", PyInit__sha1},
    {"_sha256", PyInit__sha256},
    {"_sha512", PyInit__sha512},
    {"time", PyInit_time},
#ifdef WITH_THREAD
    {"_thread", PyInit__thread},
#endif
    {"_codecs", PyInit__codecs},
    {"_weakref", PyInit__weakref},
    {"_random", PyInit__random},
    {"_bisect", PyInit__bisect},
    {"_heapq", PyInit__heapq},
    {"_lsprof", PyInit__lsprof},
    {"itertools", PyInit_itertools},
    {"_collections", PyInit__collections},
    {"_symtable", PyInit__symtable},
    {"_csv", PyInit__csv},
    {"_sre", PyInit__sre},
    {"parser", PyInit_parser},
    {"_struct", PyInit__struct},
    {"_datetime", PyInit__datetime},
    {"_functools", PyInit__functools},
    {"_json", PyInit__json},
    {"xxsubtype", PyInit_xxsubtype},
    {"zipimport", PyInit_zipimport},

    /* CJK codecs */
    {"_multibytecodec", PyInit__multibytecodec},
    {"_codecs_cn", PyInit__codecs_cn},
    {"_codecs_hk", PyInit__codecs_hk},
    {"_codecs_iso2022", PyInit__codecs_iso2022},
    {"_codecs_jp", PyInit__codecs_jp},
    {"_codecs_kr", PyInit__codecs_kr},
    {"_codecs_tw", PyInit__codecs_tw},

    /* Additional modules (matching xbmc4xbox) */
    {"_sqlite3", PyInit__sqlite3},
    {"zlib", PyInit_zlib},
    {"pyexpat", PyInit_pyexpat},
    {"_elementtree", PyInit__elementtree},
    {"unicodedata", PyInit_unicodedata},
    {"select", PyInit_select},

    /* Core runtime modules */
    {"marshal", PyMarshal_Init},
    {"_imp", PyInit_imp},

    /* Built-in module names for sys */
    {"builtins", NULL},
    {"sys", NULL},
    {"_warnings", _PyWarnings_Init},
    {"_string", PyInit__string},

    {"_io", PyInit__io},
    {"_pickle", PyInit__pickle},
    {"atexit", PyInit_atexit},
    {"_stat", PyInit__stat},
    {"_opcode", PyInit__opcode},

    /* Sentinel */
    {0, 0}
};

#ifdef __cplusplus
}
#endif