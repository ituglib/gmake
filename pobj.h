#pragma once

/*
 * SCOBOL POBJ support extensions. This DLL module is designed as a plugin for
 * GMAKE managed by ITUGLIB. The contents of the POBJ DLL are under commercial
 * license.
 * @author Randall S. Becker
 * @copyright Copyright (c) 2021 Nexbridge Inc. All rights reserved. Proprietary and
 * confidential except as specified in the GMake License terms. This header file
 * is contributed to the GMake GNUMake fork. The implementation is proprietary
 * and distinct from the GMake and GNUMake code base.
 */
typedef long int (*pobj_member_func_t)(const void *desc, const void *entry,
		const void *arg);

/* DLL Symbols - Do not modify this section. */
#define POBJ_EXTENSION_DLL "gmakeext"
#define POBJ_EXTENSION_DLL_QUALIFIED "$system.nsgit.gmakeext"
typedef void (*pobj_set_debug_level_function)(int level);
#define POBJ_SET_DEBUG_LEVEL "pobj_set_debug_level"
typedef void (*pobj_set_cache_add_function)(const char *(*fcn)(const char *));
#define POBJ_SET_CACHE_ADD "pobj_set_cache_add"
typedef long int (*pobj_member_date_1_function)(const void *desc, const void *entry, const void *name);
#define POBJ_MEMBER_DATE_1 "pobj_member_date_1"
typedef long int (*pobj_glob_match_function)(const void *desc, const void *entry, const void *arg);
#define POBJ_GLOB_MATCH "pobj_glob_match"
typedef int (*is_pobj_function)(const char *pobjdir);
#define IS_POBJ "is_pobj"
typedef long int (*pobj_scan_function)(const char *pobjdir, pobj_member_func_t function, const void *arg);
#define POBJ_SCAN "pobj_scan"
typedef int (*pobj_member_touch_function)(const char *pobjdir, const char *reqname);
#define POBJ_MEMBER_TOUCH "pobj_member_touch"
typedef int (*pobj_report_version_function)(const char *precede);
#define POBJ_REPORT_VERSION "pobj_report_version"

#if ! defined (POBJ_SUPPRESS_EXTERNALIZATION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

static dlHandle handlePobj = NULL;
static int pobjChecked = 0;
#define IS_POBJ_ENABLED (handlePobj != NULL)
static pobj_set_debug_level_function pobj_set_debug_level_func;
static pobj_set_cache_add_function pobj_set_cache_add_func;
static pobj_member_date_1_function pobj_member_date_1_func;
static pobj_glob_match_function pobj_glob_match_func;
static is_pobj_function is_pobj_func;
static pobj_scan_function pobj_scan_func;
static pobj_member_touch_function pobj_member_touch_func;
static pobj_report_version_function pobj_report_version_func;

/* release DLL resources. */
static void close_pobj_dll(void)
{
  if (handlePobj)
    {
      if (ISDB(DB_BASIC))
        printf("dlclose of handle 0x%x\n", handlePobj);
      dlclose(handlePobj);
      handlePobj = NULL;
      pobj_set_debug_level_func = NULL;
      pobj_set_cache_add_func = NULL;
      pobj_member_date_1_func = NULL;
      pobj_glob_match_func = NULL;
      is_pobj_func = NULL;
      pobj_scan_func = NULL;
      pobj_member_touch_func = NULL;
      pobj_report_version_func = NULL;
      pobjChecked = 0;
    }
}

/* open and load POBJ dll symbols */
static void open_pobj_dll(void)
{
  if (!pobjChecked)
    {
      handlePobj = dlopen(POBJ_EXTENSION_DLL, RTLD_NOW);
      if (ISDB(DB_BASIC))
        printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handlePobj);
      if (!handlePobj)
        {
          handlePobj = dlopen(POBJ_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
          if (ISDB(DB_BASIC))
            printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handlePobj);
        }
      if (handlePobj)
        {
          atexit(close_pobj_dll);
          pobj_set_debug_level_func =
            (pobj_set_debug_level_function) dlsym(handlePobj, POBJ_SET_DEBUG_LEVEL);
          pobj_set_cache_add_func =
            (pobj_set_cache_add_function) dlsym(handlePobj, POBJ_SET_CACHE_ADD);
          pobj_member_date_1_func =
            (pobj_member_date_1_function) dlsym(handlePobj, POBJ_MEMBER_DATE_1);
          pobj_glob_match_func =
            (pobj_glob_match_function) dlsym(handlePobj, POBJ_GLOB_MATCH);
          is_pobj_func =
            (is_pobj_function) dlsym(handlePobj, IS_POBJ);
          pobj_scan_func =
            (pobj_scan_function) dlsym(handlePobj, POBJ_SCAN);
          pobj_member_touch_func =
            (pobj_member_touch_function) dlsym(handlePobj, POBJ_MEMBER_TOUCH);
          pobj_report_version_func =
            (pobj_report_version_function) dlsym(handlePobj, POBJ_REPORT_VERSION);

          pobj_set_debug_level_func(db_level);
          pobj_set_cache_add_func(strcache_add);

          if (ISDB(DB_BASIC))
            printf("pobj DLL symbols loaded\n");
        }
      pobjChecked = 1;
    }
}
#endif

#if ! defined (POBJ_SUPPRESS_EXTERNALIZATION_VERSION)
# include <dlfcn.h>
# include <stdio.h>
# include <stdlib.h>

/**
 * Load only the required symbols for version reporting.
 * @param precede the string to use as a prefix for reporting.
 */
static void pobj_report_version_dll(const char *precede)
{
   dlHandle handlePobj = dlopen(POBJ_EXTENSION_DLL, RTLD_NOW);
   if (ISDB(DB_BASIC))
     printf("dlopen first try reported %d for handle 0x%x\n", dlerror(), handlePobj);
   if (!handlePobj)
     {
       handlePobj = dlopen(POBJ_EXTENSION_DLL_QUALIFIED, RTLD_NOW);
       if (ISDB(DB_BASIC))
         printf("dlopen second try reported %d for handle 0x%x\n", dlerror(), handlePobj);
     }
   if (handlePobj)
     {
	   pobj_set_debug_level_function pobj_set_debug_level_func =
         (pobj_set_debug_level_function) dlsym(handlePobj, POBJ_SET_DEBUG_LEVEL);
       pobj_set_debug_level_func(db_level);

       pobj_report_version_function pobj_report_version_func =
         (pobj_report_version_function) dlsym(handlePobj, POBJ_REPORT_VERSION);

       if (pobj_report_version_func)
           pobj_report_version_func(precede);

       dlclose(handlePobj);
     }
}
#endif

#if defined (POBJ_INTERNALS)
/*
 * DLL references internal to the module. The interfaces are provided for
 * consistency and code review purposes to ensure the externals symbols above
 * can be reconciled with the internal symbols below.
 */
void pobj_set_debug_level(int level);

void pobj_set_cache_add(const char *(*fcn)(const char *));

long int
pobj_member_date_1 (const void *desc, const void *entry, const void *name);

long int
pobj_glob_match (const void *desc, const void *entry, const void *arg);

/* Is the specified pobjdir an actual POBJ? */
int
is_pobj(const char *pobjdir);

long int
pobj_scan (const char *pobjdir, pobj_member_func_t function, const void *arg);

int
pobj_member_touch (const char *pobjdir, const char *reqname);
#endif /* POBJ_INTERNALS */
