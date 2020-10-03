#ifdef __TANDEM
#define NOLIST nolist
#else
#define NOLIST
#endif
#include <stdio.h> NOLIST
#include <stdlib.h> NOLIST
#include <string.h> NOLIST
#include <strings.h> NOLIST
#include <ctype.h> NOLIST
#include <limits.h> NOLIST
#ifdef _GUARDIAN_HOST
#pragma MAPINCLUDE "cextdecs.h" = "cextdecs"
#pragma MAPINCLUDE "zsysc.h" = "zsysc"
#endif
#include <cextdecs.h> NOLIST
#include <dlaunch.h> NOLIST
#include <tal.h> NOLIST
#include <zsysc.h> NOLIST
/* #include <zspic> NOLIST */
/* #include <zgrdc> NOLIST */
#define _XOPEN_SOURCE /* getopt */
#include <unistd.h> NOLIST /* getopt */

#define PROCDEATH_PREMATURE 3 /* Proc Calls App. C Completion Codes: file? */

/* capture calls to unsupported functions */

int vfork() { DEBUG(); return -1; }

#ifndef HAVE_CONFIG_H /* for testing without gmake */
int debug_flag = 0;
int launch_proc(char *argv[]);

int main(int argc, char *argv[])
{
#define NEWARGVMAX 99
  char *stdinenv = NULL;
  short error, typeinfo[5];
  int newargc, lerror;
  char *newargv[NEWARGVMAX + 1], buf[128], *bufp, *argp;

  int optchar;
  extern char *optarg;
  extern int optind;

  while ((optchar = getopt(argc, argv, "d")) != EOF)
    switch (optchar) {
    case 'd': debug_flag = 1; break;
    default: printf("%s [-d] { /in file/ | cmd  ... }\n", argv[0]); return 1;
    }

  if (argc < optind + 1) {
    stdinenv = getenv("STDIN");
    error = FILE_GETINFOBYNAME_(stdinenv, (short) strlen(stdinenv), typeinfo);
    if (error) {
      printf("Could not get info (FILE_GETINFOBYNAME_) on stdin: %s\n",
             stdinenv);
      return -1;
    }
    if (typeinfo[0] != 3 /* disk */ || typeinfo[3] != 0 /* unstructured */) {
      printf("%s [-d] { /in file/ | cmd  ... }\n", argv[0]);
      return 1;
    }
    /* input file supplied */
    bufp = fgets(buf, sizeof(buf), stdin);
    while(bufp) { /* build newargv */
      printf("%s", buf);
      argp = strtok(buf, " \n");
      if (strncmp(buf, "==", 2) && buf[0] != '\n') { /* not comment or blank */
        newargc = 0;
        while(argp) {
          if (newargc >= NEWARGVMAX) {
            printf("Too many (100) words in cmd:\n%s", buf);
            return -1;
          }
          newargv[newargc++] = argp;
          argp = strtok(NULL, " \n");
        } /* end while(argp) */
        newargv[newargc] = NULL; /* mark end of newargv */
        lerror = launch_proc(newargv);
        if (lerror); /* suppress warning */
      } /* end if (strcmp(buf, "==") && buf[0] != '\n') */
      bufp = fgets(buf, sizeof(buf), stdin);
    } /* end while(bufp) */
    return lerror;
  } else
    return launch_proc(&argv[optind]);
}
#else
   extern int debug_flag;
#endif

/*
  Launch a process to run the specified command...

  Need to:
  decide what run-options to support: see tacl :utils tacl cmds and funcs run -
  CPU      DEBUG    DEFMODE  EXTSWAP  HIGHPIN  IN       INLINE   INSPECT  INV
  JOBID    LIB      MEM      NAME     NOWAIT   OUT      OUTV     PFS      PRI
  STATUS   SWAP     TERM          or  WINDOW

  currently handle 'in', 'out', 'debug', 'cpu', and 'name [selected name]'

  /get startup msg, (default subvol, infile, outfile, params)
  /rebuild cmd line
  /parse cmd for: pgm, /run-options/ and param-set
  /modify startup message per in, out, debug, cpu and name
   run-options and param-set,
  /modify plist per other supported run-options,
  /process_launch_
  /get launchee olist.z_procname + len,
  /open launchee,
  /send startup msg,
  /check error for 70 or reply for 0x80/0x40: params/assigns requested
  /if requested, get and send param msg,
  /if requested, get and send assign msgs,
  /monitor launchee for completion to get it's return code.
 */

int launch_proc(char *argv[])
{
   process_launch_parms_def plist = P_L_DEFAULT_PARMS_;
   short error, errordet, olistlen, filenum, rc, slen, plen, num, index;
   short size1, len, countread, doparams = 0, doassigns = 0, param_len;
   int wrerror, procrc = -1, running;
   short homelen, oldhome[12], remotepgm = 0;
   short namespecified = 0, in_found = 0, out_found = 0;
   zsys_ddl_smsg_proccreate_def olist;
   char buf[2048], cbuf[LINE_MAX], cmd[48];
   char filename3[ZSYS_VAL_LEN_FILENAME], *bptr, *cptr;
   char *s1ptr = NULL, *s2ptr = NULL, hometerm[48], inoutfilename[48];
   char *sepptr = NULL, process_name[ZSYS_VAL_LEN_PROCESSNAME + 1];
   char *nextptr = NULL;
   char separators[] = " ,/";
   char mynodename[10], pgmnodename[ZSYS_VAL_LEN_SYSTEMNAME];
   char olddefaults[48], newdefaults[48];
   startup_msg_type *smt;
   assign_msg_type amt;
   param_msg_type pmt;
   zsys_ddl_smsg_def *smsg = (zsys_ddl_smsg_def *) buf;
   zsys_ddl_smsg_procdeath_def *spdmsg = &smsg->u_z_msg.z_procdeath;

   smt = (startup_msg_type *) malloc(sizeof(startup_msg_type));

   if (smt == NULL) {
     printf("launch_proc malloc failed\n");
     return PROCDEATH_PREMATURE;
   }

   rc = get_startup_msg(smt, &slen);
   if (debug_flag)
     printf("launch_proc get_startup_msg returned "
            "%d, len %d, code %d, '%.16s', '%.24s', '%.24s', '%s'\n",
            rc, slen, smt->msg_code, smt->defaults.whole, smt->infile.whole,
            smt->outfile.whole, smt->param);
   if (rc) {
     printf("launch_proc get_startup_msg failed\n");
     return PROCDEATH_PREMATURE;
   }

   /*
    * Assume that /IN and /OUT are specified without any
    * arguments.  And then later put in the appropriate values
    * if they were either specified with a value, or not
    * specified at all.
    */
   memset(smt->infile.whole, ' ', sizeof(smt->infile.whole));
   memset(smt->outfile.whole, ' ', sizeof(smt->outfile.whole));

   if (debug_flag)
     printf("lauch_proc infile = '%.24s', outfile = '%.24s'\n",
             smt->infile.whole, smt->outfile.whole);

   /* build command buffer */
   strcpy(cbuf, argv[0]);
   for (index = 1; argv[index]; index++) {
     strcat(cbuf, " ");
     strcat(cbuf, argv[index]);
   }

   /*
     valid command forms:
     cmd /run-options/ param-set
     cmd
     cmd param-set
     cmd /in, out/
     cmd /in infile, out outfile/
     cmd / in infile, out outfile /
     cmd/in infile, out outfile/
     cmd/in infile, out outfile/param-set
     cmd /in infile, out outfile/ param-set

     combined with:
     cpu cpu_number, debug, name [name_of_process]

     rules:
     run-options must be enclosed in slashes
     run-options must preceed param-set

   */

   bptr = cbuf;
   if (debug_flag)
     printf("launch_proc cbuf = '%s'\n", cbuf);
   s1ptr = strchr(bptr, '/'); /* look for first slash */
   if (s1ptr)
     s2ptr = strchr(s1ptr + 1, '/'); /* look for second slash */

   bptr = strtok(bptr, " /"); /* look for blank or slash */
   if (!bptr) {
     printf("launch_proc Could not find blank or slash\n");
     return PROCDEATH_PREMATURE;
   }
   strcpy(cmd, bptr); /* cmdname /in file, out file/ arg1, arg2, argn */
   if (debug_flag)
     printf("launch_proc cmd = '%s' at 0x%x\n", cmd, cmd);

   /* handle remote program if needed */
   if (cmd[0] == '\\') {
     /* get our node name */
     rc = NODENUMBER_TO_NODENAME_( /* number */ , mynodename,
                                   sizeof(mynodename), &size1);
     if (rc) {
       printf("launch_proc NODENUMBER_TO_NODENAME_ failed\n");
       return PROCDEATH_PREMATURE;
     }
     mynodename[size1] = NULL;
     /* get program node name */
     rc = FILENAME_DECOMPOSE_(cmd, (short) strlen(cmd), pgmnodename,
                              sizeof(pgmnodename), &size1, -1);
     if (rc) {
       printf("launch_proc cmd FILENAME_DECOMPOSE_ failed\n");
       return PROCDEATH_PREMATURE;
     }
     pgmnodename[size1] = NULL;
     /* compare our node name with program name name */
     if (debug_flag)
       printf("launch_proc node name = '%s', pgm node name = '%s'\n",
              mynodename, pgmnodename);
     if (strcasecmp(mynodename, pgmnodename)) {
      remotepgm = 1;
      /* get defaults in new format which adds node name */
      rc = OLDFILENAME_TO_FILENAME_((short *) smt->defaults.whole, newdefaults,
                                   sizeof(newdefaults), &size1);
      if (rc) {
        printf("launch_proc defaults OLDFILENAME_TO_FILENAME_ failed\n");
        return PROCDEATH_PREMATURE;
      }
      newdefaults[size1] = NULL;
      /* put new format defaults back in startup message with node */
      rc = FILENAME_TO_OLDFILENAME_(newdefaults, (short) strlen(newdefaults),
                                   (short *) olddefaults);
      if (rc) {
        printf("launch_proc defaults FILENAME_TO_OLDFILENAME_ failed\n");
        return PROCDEATH_PREMATURE;
      }
      memcpy(smt->defaults.whole, olddefaults, sizeof(smt->defaults.whole));
     }
   } /* if (cmd[0] == '\\') */

   if (s2ptr) { /* run-options (in file, out file, etc.) */
     if (debug_flag)
       printf("launch_proc last slash addr = 0x%x\n", s2ptr);
     while(1) { /* second slash is end of run-options */

       /* Get location just past our current string */
       nextptr = bptr + strlen(bptr) + 1;

       /* See if we've gone past the trailing slash */
       cptr = nextptr;
       while(*cptr == ' ')
         cptr++;
       if (debug_flag)
         printf("launch_proc last token start/end addrs = 0x%x, 0x%x\n",
                bptr, cptr);
       if (cptr >= s2ptr) {
         bptr = (cptr == s2ptr) ? cptr + 1 : cptr; /* move past slash */
         break;
       }

       /*
        * Not past end of run options, so seach from
        * just past any separators immediately adjacent
        * to our string, and locate the succeeding separator.
        */
       cptr = nextptr + strspn(nextptr, separators);
       sepptr = strpbrk(cptr, ",/");

       /* All is okay, so go and get our next token */
       bptr = strtok(NULL, separators);

       if (debug_flag) {
         printf("launch_proc bptr = '%s', sepptr = '%s', cptr = '%s'\n",
                 bptr, sepptr, cptr);
         printf("launch_proc next token = '%s' at 0x%x\n", bptr, bptr);
       }

       /*
        * Now look at our token to see what we've got.
        */
       if (!strncasecmp(bptr, "IN", strlen("IN"))) {
         in_found = 1;

         /*
          * If (1) no more separators or (2) separator has been
          * nulled out or (3) a separator immediately follows
          * this string, then we have no infile name.
          */
         if ((sepptr == NULL) || (*sepptr == '\0') ||
             (strspn(bptr + strlen(bptr) +1, ",/")) ) {
           if (debug_flag)
             printf ("launch_proc not setting infile name\n");
         } else {
           /* in inlen out */
           bptr = strtok(NULL, separators);

           if (debug_flag)
             printf("launch_proc infile = '%s' at 0x%x\n", bptr, bptr);
           /* need to add node if remote pgm */
           if (remotepgm) {
             rc = FILENAME_DECOMPOSE_(bptr, (short) strlen(bptr),
                                      inoutfilename,
                                      sizeof(inoutfilename), &size1,
                                      -1, 1);
             if (rc) {
               printf("launch_proc FILENAME_DECOMPOSE_ failed\n");
               return PROCDEATH_PREMATURE;
             }
             inoutfilename[size1] = NULL;
           } else {
             strcpy(inoutfilename, bptr);
           }
           error = FILENAME_TO_OLDFILENAME_(inoutfilename,
                                            (short) strlen(inoutfilename),
                                            (short *) filename3);
           if (error) {
             printf("launch_proc FILENAME_TO_OLDFILENAME_ returned %d\n",
                    error);
             return PROCDEATH_PREMATURE;
           }
           memcpy(smt->infile.whole, filename3, sizeof(smt->infile.whole));
         }
       } else if (!strncasecmp(bptr, "OUT", strlen("OUT"))) {
         out_found = 1;
         /*
          * If (1) no more separators or (2) separator has been
          * nulled out or (3) a separator immediately follows
          * this string, then we have no outfile name.
          */
         if ((sepptr == NULL) || (*sepptr == '\0') ||
             (strspn(bptr + strlen(bptr) +1, ",/")) ) {
           if (debug_flag)
             printf ("launch_proc not setting outfile name\n");
         } else {
           /* in inlen out */
           bptr = strtok(NULL, separators);

           if (debug_flag)
             printf("launch_proc outfile = '%s' at 0x%x\n", bptr, bptr);
           /* need to add node if remote pgm */
           if (remotepgm) {
             rc = FILENAME_DECOMPOSE_(bptr, (short) strlen(bptr),
                                      inoutfilename,
                                      sizeof(inoutfilename), &size1,
                                      -1, 1);
             if (rc) {
               printf("launch_proc FILENAME_DECOMPOSE_ failed\n");
               return PROCDEATH_PREMATURE;
             }
             inoutfilename[size1] = NULL;
           } else {
             strcpy(inoutfilename, bptr);
           }

           error = FILENAME_TO_OLDFILENAME_(inoutfilename,
                                            (short) strlen(inoutfilename),
                                            (short *) filename3);
           if (error) {
             printf("launch_proc FILENAME_TO_OLDFILENAME_ returned %d\n",
                    error);
             return PROCDEATH_PREMATURE;
           }
           memcpy(smt->outfile.whole, filename3, sizeof(smt->outfile.whole));
         }
       } else if (!strncasecmp(bptr, "CPU", strlen("CPU"))) {
         bptr = strtok(NULL, separators);

         if ( (bptr == NULL) || (sepptr < bptr) ) {
           /* We have a comma or slash or eol prior to our input file arg */
           printf("launch_proc missing cpu argument, found: %s\n",
                   ((bptr == NULL) ? "NULL": bptr));
           return PROCDEATH_PREMATURE;
         }

         if (debug_flag)
           printf("launch_proc cpu = '%s' at 0x%x\n", bptr, bptr);

         plist.cpu = atoi(bptr);

       } else if (!strncasecmp(bptr, "DEBUG", strlen("DEBUG"))) {

         if (debug_flag)
           printf("launch_proc debug is set\n");

         plist.debug_options = ZSYS_VAL_PCREATOPT_RUND;

       } else if (!strncasecmp(bptr, "NAME", strlen("NAME"))) {
         /*
          * If (1) no more separators or (2) separator has been
          * nulled out or (3) a separator immediately follows
          * this string, then we have no name.
          */
         if ((sepptr == NULL) || (*sepptr == '\0') ||
             (strspn(bptr + strlen(bptr) +1, ",/")) ) {
           if (debug_flag)
             printf ("launch_proc using generated process name\n");
         } else {
           namespecified = 1;

           bptr = strtok(NULL, separators);

           plist.name_options = ZSYS_VAL_PCREATOPT_NAMEINCALL;
           if (strlen(bptr) > ZSYS_VAL_LEN_PROCESSNAME) {
             printf("launch_proc process name must be < %d char\n",
                    ZSYS_VAL_LEN_PROCESSNAME);
             return PROCDEATH_PREMATURE;
           } else {
             strcpy(process_name, bptr);
             plist.process_name = process_name;
             plist.process_name_len = strlen(process_name);

             if (debug_flag){
               printf("launch_proc using '%s' process name\n",
                         process_name);
             }
           }
         }
       } else {
         printf("launch_proc Unsupported run-option '%s' at 0x%x\n",
                bptr, bptr);
         return PROCDEATH_PREMATURE;
       }
     } /* while(!done) */
   } else { /* if (s2ptr) */
     bptr = strtok(NULL, ""); /* advance past the command */
   }

   if ((!in_found) || (!out_found)) {
     /* Get hometerminal information for setting infile and/or outfile */
     rc = PROCESS_GETINFO_( /* phandle */ , /* procfname */, /* procmax */,
                          /* proclen */, /* pri */, /* mom */,
                          hometerm, sizeof(hometerm), &homelen);
     if (rc) {
       printf("launch_proc PROCESS_GETINFO_ failed\n");
       return PROCDEATH_PREMATURE;
     }
     hometerm[homelen] = NULL;
     rc = FILENAME_TO_OLDFILENAME_(hometerm, homelen, oldhome);
     if (rc) {
       printf("launch_proc FILENAME_TO_OLDFILENAME_ failed\n");
       return PROCDEATH_PREMATURE;
     }

     /* Now set the infile and/or outfile to the hometerm */
     if (!in_found)
       memcpy(smt->infile.whole, oldhome, 24);

     if (!out_found)
       memcpy(smt->outfile.whole, oldhome, 24);
   }

   if (debug_flag)
     printf("launch_proc launching infile= '%.24s', outfile= '%.24s'\n",
             smt->infile.whole, smt->outfile.whole);

   if (bptr && *bptr) { /* must be the param-set (arg1; arg2, argn) */
     while(*bptr == ' ')
       bptr++;

     if (strlen(bptr) >= sizeof(smt->param) - 3) {
       /* Need more space for our parameters */
       smt = realloc(smt, sizeof(startup_msg_type) +
                          strlen(bptr) - sizeof(smt->param) + 4);
       if (debug_flag)
         printf("launch_proc realloc size = %d\n",
                 sizeof(startup_msg_type) + strlen(bptr) -
                 sizeof(smt->param) + 4);
       if (smt == NULL) {
         printf("launch_proc realloc failed\n");
         return PROCDEATH_PREMATURE;
       }
     }
     strcpy(smt->param, bptr);
     if (debug_flag)
       printf("launch_proc param = '%s'\n", bptr);
   } else smt->param[0] = NULL;

   param_len = (short) strlen(smt->param);
   if (debug_flag)
     printf("launch_proc param len = %d\n", param_len);
   for (index = param_len; index < (param_len + 3); smt->param[index++] = 0);
   index -= (index % 2) ? 1 : 2;
   if (debug_flag)
     printf("launch_proc new param len = %d\n", index);

   if (!namespecified) {
     /* Set default value for "name" run-option */
     plist.name_options = ZSYS_VAL_PCREATOPT_NAMEDBYSYS; /* 4 chars long */
   }

   plist.program_name = cmd;
   plist.program_name_len = (long) strlen(cmd);
   error = PROCESS_LAUNCH_((void *) &plist , &errordet, (void *) &olist,
                           sizeof(olist), &olistlen);
   if (error) {
     /* ZGRD_VAL_PCREATERR_EXTERNALS 14 */
     if (error == 14) {
       printf("*** PROCESS_LAUNCH_ Warning %d: [%s] has undefined externals, "
              "but was started anyway\n", 14, cmd);
     } else {
       printf("launch_proc PROCESS_LAUNCH_ Error = %d, errordet = %d\n",
              error, errordet);
       return PROCDEATH_PREMATURE;
     }
   }

   if (debug_flag)
     printf("launch_proc Opening %.*s\n",
            olist.z_procname_len, olist.u_z_data.z_procname);
   error = FILE_OPEN_(olist.u_z_data.z_procname, olist.z_procname_len,
                      &filenum);
   if (error) {
     FILE_GETINFO_(filenum, &errordet);
     printf("launch_proc FILE_OPEN_ %s failed with error %d, %d\n",
            olist.u_z_data.z_procname, error, errordet);
     return PROCDEATH_PREMATURE;
   }

   if (debug_flag)
     printf("launch_proc sent get_startup_msg, "
            "len %d, code %d, '%.16s', '%.24s', '%.24s', '%s'\n",
            sizeof(startup_msg_type) - sizeof(smt->param) + index,
            smt->msg_code,
            smt->defaults.whole, smt->infile.whole,
            smt->outfile.whole, smt->param);
   wrerror = WRITEREADX(filenum, (char *) smt,
                        (short) (sizeof(startup_msg_type) -
                                 sizeof(smt->param) + index),
                        sizeof(startup_msg_type), &countread);
   if (wrerror) {
     FILE_GETINFO_(filenum, &errordet);
     if (errordet == 70) {
       if (debug_flag)
         printf("launch_proc WRITEREADX got error 70 (send params/assigns)\n");
       doassigns = 1;
       doparams = 1;
     } else {
       printf("launch_proc WRITEREADX failed with error %d, %d\n",
              wrerror, errordet);
       FILE_CLOSE_(filenum);
       return PROCDEATH_PREMATURE;
     }
   } else if (countread) {
     bptr = (char *) smt;
     doassigns = (short) (*bptr & 0x80);
     doparams = (short) (*bptr & 0x40);
     if (debug_flag)
       printf("launch_proc WRITEREADX Countread was %d, assigns flag was %x, "
              "params flag was %x\n", countread, doassigns, doparams);
   }

   if (doparams) {
     rc = get_param_msg(&pmt, &plen); /* all params are in one message */
     /* parameters: 0 - name len, 1 for n - name, n + 1: val len, n + 2: val */
     if (debug_flag)
       printf("launch_proc get_param_msg returned %d, code %d, params %d\n",
              rc, pmt.msg_code, pmt.num_params);

     if (!rc) {
       wrerror = WRITEX(filenum, (char *) &pmt, plen);
       if (wrerror) {
         FILE_GETINFO_(filenum, &errordet);
         printf("launch_proc WRITEX failed with error %d, %d\n",
                wrerror, errordet);
         FILE_CLOSE_(filenum);
         return PROCDEATH_PREMATURE;
       }
     }
   } /* if (doparams) */

   if (doassigns) {
#ifdef _GUARDIAN_TARGET
     num = get_max_assign_msg_ordinal(); /* number of asigns */
#else
    /* NOTE: This needs to be fixed for OSS build.                           */
    /* Currently we don't build/support this version of gmake in oss         */
    /* If at all one ports this gmake to oss, this variable and associated   */
    /* code needs to be fixed                                                */
    num = 0;
#endif
     if (debug_flag)
       printf("launch_proc get_max_assign_msg_ordinal returned %d\n", num);
     for (index = 1; index <= num; index++) { /* send each one */
       rc = get_assign_msg((short) index, &amt);
       if (debug_flag) {
         printf("launch_proc get_assign_msg returned %d, %d, %.*s, %.*s, "
                "0x%x, %.24s\n",
                rc, amt.msg_code,
                amt.logical_unit_name.prognamelen,
                amt.logical_unit_name.progname,
                amt.logical_unit_name.filenamelen,
                amt.logical_unit_name.filename,
                amt.field_mask, amt.filename.whole);
         /* in out outmax outlen */
         error = OLDFILENAME_TO_FILENAME_((short *) amt.filename.whole,
                                        filename3, sizeof(filename3), &size1);
         printf("launch_proc OLDFILENAME_TO_FILENAME_ returned %d\n", error);
         if (!error) {
           filename3[size1] = 0;
           printf("launch_proc filename = %.*s, size = %d\n",
                  size1, filename3, size1);
         }
       }
       wrerror = WRITEX(filenum, (char *) &amt, sizeof(amt));
       if (wrerror) {
         FILE_GETINFO_(filenum, &errordet);
         printf("launch_proc WRITEX failed with error %d, %d\n",
                wrerror, errordet);
         FILE_CLOSE_(filenum);
         return PROCDEATH_PREMATURE;
       }
     } /* for (index = 1; index <= num; index++) */
   } /* if (doassigns) */

   FILE_CLOSE_(filenum);

   error = FILE_OPEN_("$RECEIVE", (short) strlen("$RECEIVE"),
                      &filenum, /* acc */, /* excl */, 0 /* nowait */,
                      1 /* recv depth */, 0 /* rw */);
   if (error) {
     FILE_GETINFO_(filenum, &errordet);
     printf("launch_proc FILE_OPEN_ $RECEIVE failed with error %d, %d\n",
            error, errordet);
     return PROCDEATH_PREMATURE;
   }

   if (debug_flag)
     printf("launch_proc Waiting for process death message\n");
   running = 1;
   while(running) {
     wrerror = READX(filenum, buf, sizeof(buf), &len);
     if (_status_gt(wrerror)) { /* system (not user) message */
       if (smsg->u_z_msg.z_msgnumber[0] == ZSYS_VAL_SMSG_PROCDEATH) {
         wrerror = CHILD_LOST_(buf, len, (short *) &olist.z_phandle);
         if (wrerror != 4) { /* is this the process we last started? */
           if (debug_flag) {
             printf("launch_proc CHILD_LOST_ returned %d for ", wrerror);
             printf("%.*s which returned %d\n", spdmsg->z_procname.zlen,
                    (char *) ((char *) smsg + spdmsg->z_procname.zoffset),
                    spdmsg->z_completion_code);
           }
         } else {
           if ( spdmsg->z_flags & ZSYS_MSK_PDEATH_ABENDED ) {
             if (debug_flag)
               printf("launch_proc: process abended\n");
           }
           if (debug_flag)
             printf("launch_proc %.*s returned %d\n", spdmsg->z_procname.zlen,
                    (char *) ((char *) smsg + spdmsg->z_procname.zoffset),
                    spdmsg->z_completion_code);
           procrc = spdmsg->z_completion_code;
           running = 0; /* break; */
         }
       } else { /* if (smsg->u_z_msg.z_msgnumber[0] == ...PROCDEATH) */
         if (debug_flag)
           printf("launch_proc READUPDATEX got msgnumber %d\n",
                  smsg->u_z_msg.z_msgnumber[0]);
       }
     } else if (_status_eq(wrerror)) { /* if (_status_gt(wrerror)) */
       if (debug_flag)
         printf("launch_proc READUPDATEX got user message\n");
     } else {
       printf("launch_proc READUPDATEX got error %d\n", wrerror);
     }
   } /* while(running) */

   FILE_CLOSE_(filenum);
   return procrc;
}

/*===========================================================================*/
#pragma page "T0593 GUARDIAN GNU Make- standemc Change Descriptions"
/*===========================================================================*/

/*
------------------------------------------------------------------------------
Yosemite 02/12/02                                                 YODL:BLS0002
  - Fix problem with parameters over-running startup_msg_type
    param array.
  - Fix problem with remote filenames.
  - Handle situation where no infile or outfile are specified.
  - Added support for the following run options:
    cpu cpu_number
    debug
    name [process_name]
------------------------------------------------------------------------------
 End standemc change descriptions
*/
