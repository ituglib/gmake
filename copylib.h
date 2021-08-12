#pragma once

/*
 * TANDEM COPYLIB support extensions. This DLL module is designed as a plugin for
 * GMAKE managed by ITUGLIB. The contents of the COPYLIB DLL are under commercial
 * license.
 * @author Randall S. Becker
 * @copyright Copyright (c) 2021 Nexbridge Inc. All rights reserved. Proprietary and
 * confidential except as specified in the GMake License terms. This header file
 * is contributed to the GMake GNUMake fork. The implementation is proprietary
 * and distinct from the GMake and GNUMake code base.
 */
typedef long int (*copylib_member_func_t)(const void *entry,
		const void *arg);

/* DLL Symbols - Do not modify this section. */
#define COPYLIB_EXTENSION_DLL "gmakecpy"
#define COPYLIB_EXTENSION_DLL_QUALIFIED "$system.nsgit.gmakecpy"
typedef void (*copylib_set_debug_level_function)(int level);
#define COPYLIB_SET_DEBUG_LEVEL "copylib_set_debug_level"
typedef long int (*copylib_reindex_function)(const void *copylib, const void *sections);
#define COPYLIB_REINDEX "copylib_reindex"
typedef void (*copylib_set_cache_add_function)(const char *(*fcn)(const char *));
#define COPYLIB_SET_CACHE_ADD "copylib_set_cache_add"
typedef long int (*copylib_member_date_1_function)(const void *entry, const void *name);
#define COPYLIB_MEMBER_DATE_1 "copylib_member_date_1"
typedef long int (*copylib_glob_match_function)(const void *entry, const void *arg);
#define COPYLIB_GLOB_MATCH "copylib_glob_match"
typedef int (*is_copylib_function)(const char *copylibdir);
#define IS_COPYLIB "is_copylib"
typedef long int (*copylib_scan_function)(const char *copylibdir, copylib_member_func_t function, const void *arg);
#define COPYLIB_SCAN "copylib_scan"
typedef int (*copylib_member_touch_function)(const char *copylibdir, const char *reqname);
#define COPYLIB_MEMBER_TOUCH "copylib_member_touch"
typedef int (*copylib_report_version_function)(const char *precede);
#define COPYLIB_REPORT_VERSION "copylib_report_version"

#if ! defined (COPYLIB_SUPPRESS_EXTERNALIZATION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

#ifdef COPYLIB_EXTERN
extern dlHandle handleCopylib;
extern int copylibChecked;
#define IS_COPYLIB_ENABLED (handleCopylib != NULL)
extern copylib_set_debug_level_function copylib_set_debug_level_func;
extern copylib_reindex_function copylib_reindex_func;
extern copylib_set_cache_add_function copylib_set_cache_add_func;
extern copylib_member_date_1_function copylib_member_date_1_func;
extern copylib_glob_match_function copylib_glob_match_func;
extern is_copylib_function is_copylib_func;
extern copylib_scan_function copylib_scan_func;
extern copylib_member_touch_function copylib_member_touch_func;
extern copylib_report_version_function copylib_report_version_func;
extern void close_copylib_dll(void);
extern void open_copylib_dll(void);
#else
dlHandle handleCopylib = NULL;
int copylibChecked = 0;
#define IS_COPYLIB_ENABLED (handleCopylib != NULL)
copylib_set_debug_level_function copylib_set_debug_level_func;
copylib_reindex_function copylib_reindex_func;
copylib_set_cache_add_function copylib_set_cache_add_func;
copylib_member_date_1_function copylib_member_date_1_func;
copylib_glob_match_function copylib_glob_match_func;
is_copylib_function is_copylib_func;
copylib_scan_function copylib_scan_func;
copylib_member_touch_function copylib_member_touch_func;
copylib_report_version_function copylib_report_version_func;

/* release DLL resources. */
void close_copylib_dll(void)
{
  if (handleCopylib)
    {
      if (ISDB(DB_BASIC))
        printf("dlclose of handle 0x%x\n", handleCopylib);
      dlclose(handleCopylib);
      handleCopylib = NULL;
      copylib_set_debug_level_func = NULL;
      copylib_reindex_func = NULL;
      copylib_set_cache_add_func = NULL;
      copylib_member_date_1_func = NULL;
      copylib_glob_match_func = NULL;
      is_copylib_func = NULL;
      copylib_scan_func = NULL;
      copylib_member_touch_func = NULL;
      copylib_report_version_func = NULL;
      copylibChecked = 0;
    }
}

/* open and load COPYLIB dll symbols */
void open_copylib_dll(void)
{
  if (!copylibChecked)
    {
      handleCopylib = dlopen(COPYLIB_EXTENSION_DLL, RTLD_NOW);
      if (ISDB(DB_BASIC))
        printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handleCopylib);
      if (!handleCopylib)
        {
          handleCopylib = dlopen(COPYLIB_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
          if (ISDB(DB_BASIC))
            printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handleCopylib);
        }
      if (handleCopylib)
        {
          atexit(close_copylib_dll);
          copylib_set_debug_level_func =
            (copylib_set_debug_level_function) dlsym(handleCopylib, COPYLIB_SET_DEBUG_LEVEL);
          copylib_reindex_func =
            (copylib_reindex_function) dlsym(handleCopylib, COPYLIB_REINDEX);
          copylib_set_cache_add_func =
            (copylib_set_cache_add_function) dlsym(handleCopylib, COPYLIB_SET_CACHE_ADD);
          copylib_member_date_1_func =
            (copylib_member_date_1_function) dlsym(handleCopylib, COPYLIB_MEMBER_DATE_1);
          copylib_glob_match_func =
            (copylib_glob_match_function) dlsym(handleCopylib, COPYLIB_GLOB_MATCH);
          is_copylib_func =
            (is_copylib_function) dlsym(handleCopylib, IS_COPYLIB);
          copylib_scan_func =
            (copylib_scan_function) dlsym(handleCopylib, COPYLIB_SCAN);
          copylib_member_touch_func =
            (copylib_member_touch_function) dlsym(handleCopylib, COPYLIB_MEMBER_TOUCH);
          copylib_report_version_func =
            (copylib_report_version_function) dlsym(handleCopylib, COPYLIB_REPORT_VERSION);

          copylib_set_debug_level_func(db_level);
          copylib_set_cache_add_func(strcache_add);

          if (ISDB(DB_BASIC))
            printf("copylib DLL symbols loaded\n");
        }
      copylibChecked = 1;
    }
}
#endif
#endif

#if ! defined (COPYLIB_SUPPRESS_EXTERNALIZATION_VERSION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

/**
 * Load only the required symbols for version reporting.
 * @param precede the string to use as a prefix for reporting.
 */
static void copylib_report_version_dll(const char *precede)
{
   dlHandle handleCopylib = dlopen(COPYLIB_EXTENSION_DLL, RTLD_NOW);
   if (ISDB(DB_BASIC))
     printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handleCopylib);
   if (!handleCopylib)
     {
       handleCopylib = dlopen(COPYLIB_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
       if (ISDB(DB_BASIC))
         printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handleCopylib);
     }
   if (handleCopylib)
     {
	   copylib_set_debug_level_function copylib_set_debug_level_func =
         (copylib_set_debug_level_function) dlsym(handleCopylib, COPYLIB_SET_DEBUG_LEVEL);
       copylib_set_debug_level_func(db_level);

       copylib_report_version_function copylib_report_version_func =
         (copylib_report_version_function) dlsym(handleCopylib, COPYLIB_REPORT_VERSION);

       if (copylib_report_version_func)
           copylib_report_version_func(precede);

       dlclose(handleCopylib);
     }
}
#endif

#if defined (COPYLIB_INTERNALS)
/*
 * DLL references internal to the module. The interfaces are provided for
 * consistency and code review purposes to ensure the externals symbols above
 * can be reconciled with the internal symbols below.
 */
void copylib_set_debug_level(int level);

void copylib_set_cache_add(const char *(*fcn)(const char *));

long int
copylib_member_date_1 (const void *entry, const void *name);

long int
copylib_glob_match (const void *entry, const void *arg);

/* Is the specified copylibdir an actual COPYLIB? */
int
is_copylib(const char *copylibdir);

long int
copylib_scan (const char *copylibdir, copylib_member_func_t function, const void *arg);

int
copylib_member_touch (const char *copylibdir, const char *reqname);
#endif /* COPYLIB_INTERNALS */
