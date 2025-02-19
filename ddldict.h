#pragma once

#include <stdint.h>

/*
 * TANDEM DDLDICT support extensions. This DLL module is designed as a plugin for
 * GMAKE managed by ITUGLIB. The contents of the DDLDICT DLL are under commercial
 * license.
 * @author Randall S. Becker
 * @copyright Copyright (c) 2021,2025 Nexbridge Inc. All rights reserved. Proprietary and
 * confidential except as specified in the GMake License terms. This header file
 * is contributed to the GMake GNUMake fork. The implementation is proprietary
 * and distinct from the GMake and GNUMake code base.
 */
typedef long int (*ddldict_member_func_t)(const void *entry,
		const void *arg);
typedef int64_t (*ddldict_member_func_64_t)(const void *entry,
		const void *arg);

/* DLL Symbols - Do not modify this section. */
#define DDLDICT_EXTENSION_DLL "gmakeddl"
#define DDLDICT_EXTENSION_DLL_QUALIFIED "$system.nsgit.gmakeddl"
typedef void (*ddldict_set_debug_level_function)(int level);
#define DDLDICT_SET_DEBUG_LEVEL "ddldict_set_debug_level"
typedef void (*ddldict_set_cache_add_function)(const char *(*fcn)(const char *));
#define DDLDICT_SET_CACHE_ADD "ddldict_set_cache_add"
typedef long int (*ddldict_member_date_1_function)(const void *entry, const void *name);
#define DDLDICT_MEMBER_DATE_1 "ddldict_member_date_1"
typedef int64_t (*ddldict_member_date_64_function)(const void *entry, const void *name);
#define DDLDICT_MEMBER_DATE_64 "ddldict_member_date_64"
typedef long int (*ddldict_glob_match_function)(const void *entry, const void *arg);
#define DDLDICT_GLOB_MATCH "ddldict_glob_match"
typedef int (*is_ddldict_function)(const char *ddldictdir);
#define IS_DDLDICT "is_ddldict"
typedef long int (*ddldict_scan_function)(const char *ddldictdir, ddldict_member_func_t function, const void *arg);
#define DDLDICT_SCAN "ddldict_scan"
typedef int64_t (*ddldict_scan_64_function)(const char *ddldictdir, ddldict_member_func_64_t function, const void *arg);
#define DDLDICT_SCAN_64 "ddldict_scan_64"
typedef int (*ddldict_member_touch_function)(const char *ddldictdir, const char *reqname);
#define DDLDICT_MEMBER_TOUCH "ddldict_member_touch"
typedef int (*ddldict_report_version_function)(const char *precede);
#define DDLDICT_REPORT_VERSION "ddldict_report_version"

#if ! defined (DDLDICT_SUPPRESS_EXTERNALIZATION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

#ifdef DDLDICT_EXTERN
extern dlHandle handleDdlDict;
extern int ddldictChecked;
#define IS_DDLDICT_ENABLED (handleDdlDict != NULL)
extern ddldict_set_debug_level_function ddldict_set_debug_level_func;
extern ddldict_set_cache_add_function ddldict_set_cache_add_func;
extern ddldict_member_date_1_function ddldict_member_date_1_func;
extern ddldict_member_date_64_function ddldict_member_date_64_func;

extern ddldict_glob_match_function ddldict_glob_match_func;
extern is_ddldict_function is_ddldict_func;
extern ddldict_scan_function ddldict_scan_func;
extern ddldict_scan_64_function ddldict_scan_64_func;
extern ddldict_member_touch_function ddldict_member_touch_func;
extern ddldict_report_version_function ddldict_report_version_func;
extern void close_ddldict_dll(void);
extern void open_ddldict_dll(void);
#else
dlHandle handleDdlDict = NULL;
int ddldictChecked = 0;
#define IS_DDLDICT_ENABLED (handleDdlDict != NULL)
ddldict_set_debug_level_function ddldict_set_debug_level_func;
ddldict_set_cache_add_function ddldict_set_cache_add_func;
ddldict_member_date_1_function ddldict_member_date_1_func;
ddldict_member_date_64_function ddldict_member_date_64_func;
ddldict_glob_match_function ddldict_glob_match_func;
is_ddldict_function is_ddldict_func;
ddldict_scan_function ddldict_scan_func;
ddldict_scan_64_function ddldict_scan_64_func;
ddldict_member_touch_function ddldict_member_touch_func;
ddldict_report_version_function ddldict_report_version_func;

/* release DLL resources. */
void close_ddldict_dll(void)
{
  if (handleDdlDict)
    {
      if (ISDB(DB_BASIC))
        printf("dlclose of handle 0x%x\n", handleDdlDict);
      dlclose(handleDdlDict);
      handleDdlDict = NULL;
      ddldict_set_debug_level_func = NULL;
      ddldict_set_cache_add_func = NULL;
      ddldict_member_date_1_func = NULL;
      ddldict_member_date_64_func = NULL;
      ddldict_glob_match_func = NULL;
      is_ddldict_func = NULL;
      ddldict_scan_func = NULL;
      ddldict_scan_64_func = NULL;
      ddldict_member_touch_func = NULL;
      ddldict_report_version_func = NULL;
      ddldictChecked = 0;
    }
}

/* open and load DDLDICT dll symbols */
void open_ddldict_dll(void)
{
  if (!ddldictChecked)
    {
      handleDdlDict = dlopen(DDLDICT_EXTENSION_DLL, RTLD_NOW);
      if (ISDB(DB_BASIC))
        printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handleDdlDict);
      if (!handleDdlDict)
        {
          handleDdlDict = dlopen(DDLDICT_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
          if (ISDB(DB_BASIC))
            printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handleDdlDict);
        }
      if (handleDdlDict)
        {
          atexit(close_ddldict_dll);
          ddldict_set_debug_level_func =
            (ddldict_set_debug_level_function) dlsym(handleDdlDict, DDLDICT_SET_DEBUG_LEVEL);
          ddldict_set_cache_add_func =
            (ddldict_set_cache_add_function) dlsym(handleDdlDict, DDLDICT_SET_CACHE_ADD);
          ddldict_member_date_1_func =
            (ddldict_member_date_1_function) dlsym(handleDdlDict, DDLDICT_MEMBER_DATE_1);
          ddldict_member_date_64_func =
            (ddldict_member_date_64_function) dlsym(handleDdlDict, DDLDICT_MEMBER_DATE_64);
          ddldict_glob_match_func =
            (ddldict_glob_match_function) dlsym(handleDdlDict, DDLDICT_GLOB_MATCH);
          is_ddldict_func =
            (is_ddldict_function) dlsym(handleDdlDict, IS_DDLDICT);
          ddldict_scan_func =
            (ddldict_scan_function) dlsym(handleDdlDict, DDLDICT_SCAN);
          ddldict_scan_64_func =
            (ddldict_scan_64_function) dlsym(handleDdlDict, DDLDICT_SCAN_64);
          ddldict_member_touch_func =
            (ddldict_member_touch_function) dlsym(handleDdlDict, DDLDICT_MEMBER_TOUCH);
          ddldict_report_version_func =
            (ddldict_report_version_function) dlsym(handleDdlDict, DDLDICT_REPORT_VERSION);

          ddldict_set_debug_level_func(db_level);
          ddldict_set_cache_add_func(strcache_add);

          if (ISDB(DB_BASIC))
            printf("ddldict DLL symbols loaded\n");
        }
      ddldictChecked = 1;
    }
}
#endif
#endif

#if ! defined (DDLDICT_SUPPRESS_EXTERNALIZATION_VERSION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

/**
 * Load only the required symbols for version reporting.
 * @param precede the string to use as a prefix for reporting.
 */
static void ddldict_report_version_dll(const char *precede)
{
   dlHandle handleDdlDict = dlopen(DDLDICT_EXTENSION_DLL, RTLD_NOW);
   if (ISDB(DB_BASIC))
     printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handleDdlDict);
   if (!handleDdlDict)
     {
       handleDdlDict = dlopen(DDLDICT_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
       if (ISDB(DB_BASIC))
         printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handleDdlDict);
     }
   if (handleDdlDict)
     {
	   ddldict_set_debug_level_function ddldict_set_debug_level_func =
         (ddldict_set_debug_level_function) dlsym(handleDdlDict, DDLDICT_SET_DEBUG_LEVEL);
       ddldict_set_debug_level_func(db_level);

       ddldict_report_version_function ddldict_report_version_func =
         (ddldict_report_version_function) dlsym(handleDdlDict, DDLDICT_REPORT_VERSION);

       if (ddldict_report_version_func)
           ddldict_report_version_func(precede);

       dlclose(handleDdlDict);
     }
}
#endif

#if defined (DDLDICT_INTERNALS)
/*
 * DLL references internal to the module. The interfaces are provided for
 * consistency and code review purposes to ensure the externals symbols above
 * can be reconciled with the internal symbols below.
 */
void ddldict_set_debug_level(int level);

void ddldict_set_cache_add(const char *(*fcn)(const char *));

long int
ddldict_member_date_1 (const void *entry, const void *name);

int64_t
ddldict_member_date_64 (const void *entry, const void *name);

long int
ddldict_glob_match (const void *entry, const void *arg);

/* Is the specified ddldictdir an actual DDLDICT? */
int
is_ddldict(const char *ddldictdir);

long int
ddldict_scan (const char *ddldictdir, ddldict_member_func_t function, const void *arg);

int64_t
ddldict_scan_64 (const char *ddldictdir, ddldict_member_func_64_t function, const void *arg);

int
ddldict_member_touch (const char *ddldictdir, const char *reqname);
#endif /* DDLDICT_INTERNALS */
