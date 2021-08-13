/**
 * @file
 * Handle Tandem extensions.
 * @author HPE Development
 * @author Randall S. Becker
 * @copyright Some parts of this file are under Copyright (c) 2021 Nexbridge Inc.
 * All rights reserved. Proprietary and confidential. Disclosure without written
 * permission violates international laws and will be prosecuted.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <limits.h>
#include <cextdecs.h>
#include <dlaunch.h>
#include <tal.h>
#ifdef _OSS_HOST
#include <zsysc>
#else
#include <zsysc.h>
#endif
/* #include <zspic> */
/* #include <zgrdc> */
#define _XOPEN_SOURCE /* getopt */
#include <unistd.h> /* getopt */

#include "makeint.h"
#include "debug.h"
#include "tandem.h"

typedef struct ParamEntry_ {
	char *name;
	char *value;
} ParamEntry;

typedef struct AssignEntry_ {
	char *name;
	assign_msg_type *value;
} AssignEntry;

static size_t _num_params = 0;
static ParamEntry *_params = NULL;

static size_t _num_assigns = 0;
static AssignEntry *_assigns = NULL;

/**
 * Upshift a string.
 * @param string the string to upshift.
 * @return a pointer to the start of the string.
 */
static char *_strupr(char *string) {
	for (size_t i=0;string[i]; i++) {
		string[i] = toupper(string[i]);
	}
	return string;
}

static short get_param_msg_local(param_msg_type *pmt, short *plen) {
	if (_num_params > 0) {
		char *s;
		size_t index;
		memset(pmt, 0, sizeof(param_msg_type));
		pmt->msg_code = -3; /* Param Message. */
		s = pmt->parameters;
		/* parameters: 0 - name len, 1 for n - name, n + 1: val len, n + 2: val */
		for (index = 0; index < _num_params; index++) {
			size_t remaining = sizeof(pmt->parameters)
					- (size_t) (s - pmt->parameters);
			size_t length;
			ParamEntry *entry = _params + index;

			if (strlen(entry->name) + strlen(entry->value) + 2 > remaining) {
				OS(message, 0, "Insufficient environment space for %s",
						entry->name);
				continue;
			}
			(pmt->num_params)++;
			length = strlen(entry->name);
			*s++ = (unsigned char) length;
			strncpy(s, entry->name, length);
			s += length;
			length = strlen(entry->value);
			*s++ = (unsigned char) length;
			strncpy(s, entry->value, length);
			s += length;
		}
		*plen = sizeof(*pmt) - sizeof(pmt->parameters) + (s - pmt->parameters);
		return 0;
	}
	return -1;
}

static short parse_assign_file(const char *value, char *filename) {
	short error, length;
	char fullName[ZSYS_VAL_LEN_FILENAME + 1];

	error = FILENAME_RESOLVE_(value, (short) strlen(value), fullName,
			(short) (sizeof(fullName) - 1), &length,
			0x0003 /* Keep Subvol, Upshift */);
	if (error)
		return error;
	fullName[length] = '\0';

	memset(filename, ' ', 24);
	error = FILENAME_DECOMPOSE_(fullName, (short) strlen(fullName),
			(filename+0), 8, &length, 0);
	error = FILENAME_DECOMPOSE_(fullName, (short) strlen(fullName),
			(filename+8), 8, &length, 1);
	error = FILENAME_DECOMPOSE_(fullName, (short) strlen(fullName),
			(filename+16), 8, &length, 2);
	if (memcmp(filename+16, "        ", 8) == 0) {
		/* Special subvolume mangling. Looks like a file. */
		memcpy(filename+16, filename+8, 8);
		memset(filename+8, ' ', 8);
	}
	return error;
}

void tandem_set_param(const char *name, const char *value) {
	ParamEntry *entry;
	size_t index;

	for (index = 0; index < _num_params; index++) {
		entry = _params + index;
		if (strcasecmp(entry->name, name) == 0) {
			// It is this entry.
			free(entry->value);
			entry->value = xstrdup(value);
			if (ISDB(DB_BASIC))
				printf("PARAM %s %s replaced\n", entry->name, entry->value);
			return;
		}
	}
	/* If we get here, the param is new */
	_params = realloc(_params, sizeof(ParamEntry) * (_num_params + 1));
	entry = _params + _num_params;
	memset(entry, 0, sizeof(ParamEntry));
	entry->name = xstrdup(name);
	_strupr(entry->name);
	entry->value = xstrdup(value);
	_num_params++;

	if (ISDB(DB_BASIC))
		printf("PARAM %s %s added\n", entry->name, entry->value);
}

void tandem_clear_param(const char *name) {
	ParamEntry *entry;
	size_t index;

	if (strcmp(name, "*") == 0) {
		for (index = 0; index < _num_params; index++) {
			entry = _params + index;
			if (ISDB(DB_BASIC))
				printf("PARAM %s removed\n", entry->name);
			free(entry->name);
			free(entry->value);
		}
		_num_params = 0;
	} else {
		for (index = 0; index < _num_params; index++) {
			entry = _params + index;
			if (strcasecmp(entry->name, name) == 0) {
				// It is this entry to remove.
				if (ISDB(DB_BASIC))
					printf("PARAM %s removed\n", entry->name);
				free(entry->name);
				free(entry->value);

				if (index != _num_params - 1) {
					memmove(entry, entry + 1,
							sizeof(ParamEntry) * (_num_params - index - 1));
				}
				_num_params--;
				break;
			}
		}
	}
}

short tandem_set_assign(const char *name, const char *value) {
	AssignEntry *entry;
	size_t index;
	short error;

	for (index = 0; index < _num_assigns; index++) {
		entry = _assigns + index;
		if (strcasecmp(entry->name, name) == 0) {
			// It is this entry.
			error = parse_assign_file(value,
					&(entry->value->filename.whole[0]));
			if (error != 0)
				return error;
			if (ISDB(DB_BASIC)) {
				char fullName[ZSYS_VAL_LEN_FILENAME + 1];
				short length = FNAMECOLLAPSE(
						(short *) (entry->value->filename.whole), fullName);
				fullName[length] = '\0';
				printf("ASSIGN %s %s replaced\n", entry->name, fullName);
			}
			return 0;
		}
	}
	/* If we get here, the assign is new */
	_assigns = realloc(_assigns, sizeof(AssignEntry) * (_num_assigns + 1));
	entry = _assigns + _num_assigns;
	memset(entry, 0, sizeof(AssignEntry));
	entry->name = xstrdup(name);
	_strupr(entry->name);
	entry->value = calloc(1, sizeof(assign_msg_type));
	entry->value->msg_code = -2;
	memset(&(entry->value->logical_unit_name), ' ',
			sizeof(entry->value->logical_unit_name));
	entry->value->logical_unit_name.prognamelen = 0;
	entry->value->logical_unit_name.filenamelen = strlen(entry->name);
	memcpy(entry->value->logical_unit_name.filename, entry->name,
			strlen(entry->name));
	error = parse_assign_file(value, &(entry->value->filename.whole[0]));
	if (error != 0) {
		free(entry->name);
		free(entry->value);
		return error;
	}
	entry->value->field_mask = 0x80000000; /* Only the file name */
	_num_assigns++;

	if (ISDB(DB_BASIC)) {
		char fullName[ZSYS_VAL_LEN_FILENAME + 1];
		short length = FNAMECOLLAPSE(
				(short *) (entry->value->filename.whole), fullName);
		fullName[length] = '\0';
		printf("ASSIGN %s %s added\n", entry->name, fullName);
	}
	return 0;
}

void tandem_clear_assign(const char *name) {
	AssignEntry *entry;
	size_t index;

	if (strcmp(name, "*") == 0) {
		for (index = 0; index < _num_assigns; index++) {
			entry = _assigns + index;
			if (ISDB(DB_BASIC))
				printf("ASSIGN %s removed\n", entry->name);
			free(entry->name);
			free(entry->value);
		}
		_num_assigns = 0;
	} else {
		for (index = 0; index < _num_assigns; index++) {
			entry = _assigns + index;
			if (strcasecmp(entry->name, name) == 0) {
				// It is this entry to remove.
				if (ISDB(DB_BASIC))
					printf("ASSIGN %s removed\n", entry->name);
				free(entry->name);
				free(entry->value);

				if (index != _num_assigns - 1) {
					memmove(entry, entry + 1,
							sizeof(AssignEntry) * (_num_assigns - index - 1));
				}
				_num_assigns--;
				break;
			}
		}
	}
}

void tandem_initialize(void) {
	short rc, plen, index;
	short num;
	param_msg_type pmt;
	assign_msg_type amt;
	char *s, *t;

	rc = get_param_msg(&pmt, &plen); /* all params are in one message */
	if (rc == 0) {
		_params = calloc(1, sizeof(ParamEntry));
		s = &(pmt.parameters[0]);
		for (index=0; index<pmt.num_params; index++) {
			short name_size = *s++;
			short value_size;
			ParamEntry *entry;

			t = s+name_size;
			value_size = *t++;
			_params = realloc(_params, sizeof(ParamEntry)*(_num_params+1));
			
			entry = _params + _num_params;
			memset(entry, 0, sizeof(ParamEntry));
			entry->name = malloc(name_size+1);
			strncpy(entry->name, s, name_size);
			entry->name[name_size] = '\0';
			entry->value = malloc(value_size+1);
			strncpy(entry->value, t, value_size);
			entry->value[value_size] = '\0';
			_num_params++;
			s = t+value_size;
		}
	} else {
		_params = calloc(1, sizeof(ParamEntry));
	}

#ifdef _GUARDIAN_TARGET
	num = get_max_assign_msg_ordinal(); /* number of assigns */
#else
	/* NOTE: This needs to be fixed for OSS build. This will not happen      */
	/*       in this fork.                                                   */
	/* Currently we don't build/support this version of gmake in oss         */
	/* If at all one ports this gmake to oss, this variable and associated   */
	/* code needs to be fixed                                                */
	num = 0;
#endif
	if (num == 0) {
		_assigns = calloc(1, sizeof(AssignEntry));
	} else {
		_assigns = calloc(num, sizeof(AssignEntry));
	}
	if (ISDB(DB_BASIC))
		printf("launch_proc get_max_assign_msg_ordinal returned %d\n", num);
	for (index = 1; index <= num; index++) { /* send each one */
		AssignEntry *entry = _assigns + (index - 1);
		rc = get_assign_msg((short) index, &amt);
		if (ISDB(DB_BASIC)) {
			char filename3[ZSYS_VAL_LEN_FILENAME];
			short size1, error;

			printf(
					"launch_proc get_assign_msg returned %d, %d, %.*s, %.*s, "
							"0x%x, %.24s\n", rc, amt.msg_code,
					amt.logical_unit_name.prognamelen,
					amt.logical_unit_name.progname,
					amt.logical_unit_name.filenamelen,
					amt.logical_unit_name.filename, amt.field_mask,
					amt.filename.whole);
			/* in out outmax outlen */
			error = OLDFILENAME_TO_FILENAME_((short *) amt.filename.whole,
					filename3, sizeof(filename3), &size1);
			printf("launch_proc OLDFILENAME_TO_FILENAME_ returned %d\n",
					error);
			if (!error) {
				filename3[size1] = 0;
				printf("launch_proc filename = %.*s, size = %d\n", size1,
						filename3, size1);
			}
		}
		entry->name = malloc(amt.logical_unit_name.filenamelen+1);
		strncpy(entry->name, amt.logical_unit_name.filename, amt.logical_unit_name.filenamelen);
		entry->name[amt.logical_unit_name.filenamelen] = '\0';
		entry->value = malloc(sizeof(assign_msg_type));
		memcpy(entry->value, &amt, sizeof(amt));
		_num_assigns++;
	}
}

#define PROCDEATH_PREMATURE 3 /* Proc Calls App. C Completion Codes: file? */

/* capture calls to unsupported functions */

#ifndef HAVE_CONFIG_H /* for testing without gmake */

int main(int argc, char *argv[]) {
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
		case 'd':
			db_level = DB_BASIC = 1;
			break;
		default:
			printf("%s [-d] { /in file/ | cmd  ... }\n", argv[0]);
			return 1;
		}

	if (argc < optind + 1) {
		stdinenv = getenv("STDIN");
		error = FILE_GETINFOBYNAME_(stdinenv, (short) strlen(stdinenv),
				typeinfo);
		if (error) {
			printf("Could not get info (FILE_GETINFOBYNAME_) on stdin: %s\n",
					stdinenv);
			return -1;
		}
		if (typeinfo[0] != 3 /* disk */|| typeinfo[3] != 0 /* unstructured */) {
			printf("%s [-d] { /in file/ | cmd  ... }\n", argv[0]);
			return 1;
		}
		/* input file supplied */
		bufp = fgets(buf, sizeof(buf), stdin);
		while (bufp) { /* build newargv */
			printf("%s", buf);
			argp = strtok(buf, " \n");
			if (strncmp(buf, "==", 2) && buf[0] != '\n') { /* not comment or blank */
				newargc = 0;
				while (argp) {
					if (newargc >= NEWARGVMAX) {
						printf("Too many (100) words in cmd:\n%s", buf);
						return -1;
					}
					newargv[newargc++] = argp;
					argp = strtok(NULL, " \n");
				} /* end while(argp) */
				newargv[newargc] = NULL; /* mark end of newargv */
				lerror = launch_proc(newargv);
				if (lerror) {
					/* suppress warning for now */
				}
			} /* end if (strcmp(buf, "==") && buf[0] != '\n') */
			bufp = fgets(buf, sizeof(buf), stdin);
		} /* end while(bufp) */
		return lerror;
	} else
		return launch_proc(&argv[optind]);
}
#endif

/**
 * Return my process name as a string.
 */
static char *my_paid(void) {
	short pHandle[10];
	short len;
	static char myname[65];
	PROCESSHANDLE_GETMINE_(pHandle);
	PROCESSHANDLE_TO_FILENAME_(pHandle, myname, (short) (sizeof(myname) - 1),
			&len, 1);
	myname[len] = '\0';
	return myname;
}
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

int launch_proc(char *argv[], char *envp[], char *capture, size_t capture_len,
		int options) {
	process_launch_parms_def plist = P_L_DEFAULT_PARMS_;
	short error,
	errordet, olistlen, filenum, rc, slen, plen, num, index;
	short childfilenum = -1;
	short waitingforoutput = 0;
	short written = 0;
	short exitWritten = 0;
	__int32_t tleId = 1;
	short size1, len, countread, doparams = 0, doassigns = 0, param_len;
	size_t size2 = 0;
	int wrerror, procrc = -1, running;
	short homelen, oldhome[12], remotepgm = 0;
	short namespecified = 0, in_found = 0, out_found = 0;
	zsys_ddl_smsg_proccreate_def olist;
	char buf[2048], cbuf[LINE_MAX], cmd[48];
	char filename3[ZSYS_VAL_LEN_FILENAME], *bptr, *cptr;
	char *s1ptr = NULL, *s2ptr = NULL, hometerm[48], inoutfilename[48];
	char sethometerm[48];
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
	short anyfile = -1;
	__int32_t len32 = 0;

	if (strcasecmp(argv[0], "param") == 0) {
		char *arg;
		if (!argv[1] || !argv[2]) {
			printf("Missing param values\n");
			return PROCDEATH_PREMATURE;
		}
		if (argv[3]) {
			printf("Extra param values\n");
			return PROCDEATH_PREMATURE;
		}
		arg = argv[2];
		if (arg[0] == '"') {
			if (arg[strlen(arg)-1] != '"') {
				printf("Missing trailing quote\n");
				return PROCDEATH_PREMATURE;
			}
			arg[strlen(arg)-1] = '\0';
			arg++;
		}
		tandem_set_param(argv[1], arg);
		return 0;
	} else if (strcasecmp(argv[0], "assign") == 0) {
		char *comma;
		if (!argv[1]) {
			printf("Missing assign value\n");
			return PROCDEATH_PREMATURE;
		}
		comma = strchr(argv[1], ',');
		if (!comma) {
			printf("Missing assign value\n");
			return PROCDEATH_PREMATURE;
		}
		*comma++ = '\0';
		if (argv[2]) {
			printf("Extra assign values\n");
			return PROCDEATH_PREMATURE;
		}
		tandem_set_assign(argv[1], comma);
		return 0;
	} else if (strcasecmp(argv[0], "clear") == 0) {
		if (strcasecmp(argv[1], "param") == 0) {
			if (strcasecmp(argv[2], "all") == 0) {
				tandem_clear_param("*");
				return 0;
			} else {
				tandem_clear_param(argv[2]);
				return 0;
			}
			if (argv[3]) {
				printf("Extra param values\n");
				return PROCDEATH_PREMATURE;
			}
		} else if (strcasecmp(argv[1], "assign") == 0) {
			if (strcasecmp(argv[2], "all") == 0) {
				tandem_clear_assign("*");
				return 0;
			} else {
				tandem_clear_assign(argv[2]);
				return 0;
			}
			if (argv[3]) {
				printf("Extra assign values\n");
				return PROCDEATH_PREMATURE;
			}
		} else {
			printf("Unknown clear option %s\n", argv[1]);
			return PROCDEATH_PREMATURE;
		}
	}

	smt = (startup_msg_type *) malloc(sizeof(startup_msg_type));

	if (smt == NULL) {
		printf("launch_proc malloc failed\n");
		return PROCDEATH_PREMATURE;
	}

	rc = get_startup_msg(smt, &slen);
	if (ISDB(DB_BASIC))
		printf("launch_proc get_startup_msg returned "
				"%d, len %d, code %d, '%.16s', '%.24s', '%.24s', '%s'\n", rc,
				slen, smt->msg_code, smt->defaults.whole, smt->infile.whole,
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

	if (ISDB(DB_BASIC))
		printf("lauch_proc infile = '%.24s', outfile = '%.24s'\n",
				smt->infile.whole, smt->outfile.whole);

	/* build command buffer */
	strcpy(cbuf, "");
	if (options & LAUNCH_PROC_USE_TACL) {
		strcat(cbuf, "$SYSTEM.SYSTEM.TACL");
		if (options & LAUNCH_PROC_IN_RECEIVE) {
			if (options & LAUNCH_PROC_OUT_PARENT) {
				strcat(cbuf, "/IN $RECEIVE,OUT ");
				strcat(cbuf, my_paid());
				strcat(cbuf, "/");
			} else {
				strcat(cbuf, "/IN $RECEIVE/");
			}
		} else if (options & LAUNCH_PROC_OUT_PARENT) {
			strcat(cbuf, "/OUT ");
			strcat(cbuf, my_paid());
			strcat(cbuf, "/");
		}
	} else {
		strcat(cbuf, argv[0]);
		for (index = 1; argv[index]; index++) {
			strcat(cbuf, " ");
			strcat(cbuf, argv[index]);
		}
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
	if (ISDB(DB_BASIC))
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
	if (ISDB(DB_BASIC))
		printf("launch_proc cmd = '%s' at 0x%x\n", cmd, cmd);

	/* handle remote program if needed */
	if (cmd[0] == '\\') {
		/* get our node name */
		rc = NODENUMBER_TO_NODENAME_( /* number */, mynodename,
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
		if (ISDB(DB_BASIC))
			printf("launch_proc node name = '%s', pgm node name = '%s'\n",
					mynodename, pgmnodename);
		if (strcasecmp(mynodename, pgmnodename)) {
			remotepgm = 1;
			/* get defaults in new format which adds node name */
			rc = OLDFILENAME_TO_FILENAME_((short *) smt->defaults.whole,
					newdefaults, sizeof(newdefaults), &size1);
			if (rc) {
				printf(
						"launch_proc defaults OLDFILENAME_TO_FILENAME_ failed\n");
				return PROCDEATH_PREMATURE;
			}
			newdefaults[size1] = NULL;
			/* put new format defaults back in startup message with node */
			rc = FILENAME_TO_OLDFILENAME_(newdefaults,
					(short) strlen(newdefaults), (short *) olddefaults);
			if (rc) {
				printf(
						"launch_proc defaults FILENAME_TO_OLDFILENAME_ failed\n");
				return PROCDEATH_PREMATURE;
			}
			memcpy(smt->defaults.whole, olddefaults,
					sizeof(smt->defaults.whole));
		}
	} /* if (cmd[0] == '\\') */

	if (s2ptr) { /* run-options (in file, out file, etc.) */
		if (ISDB(DB_BASIC))
			printf("launch_proc last slash addr = 0x%x\n", s2ptr);
		while (1) { /* second slash is end of run-options */

			/* Get location just past our current string */
			nextptr = bptr + strlen(bptr) + 1;

			/* See if we've gone past the trailing slash */
			cptr = nextptr;
			while (*cptr == ' ')
				cptr++;
			if (ISDB(DB_BASIC))
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

			if (ISDB(DB_BASIC)) {
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
				if ((sepptr == NULL) || (*sepptr == '\0')
						|| (strspn(bptr + strlen(bptr) + 1, ",/"))) {
					if (ISDB(DB_BASIC))
						printf("launch_proc not setting infile name\n");
				} else {
					/* in inlen out */
					bptr = strtok(NULL, separators);

					if (ISDB(DB_BASIC))
						printf("launch_proc infile = '%s' at 0x%x\n", bptr,
								bptr);
					/* need to add node if remote pgm */
					if (remotepgm) {
						rc = FILENAME_DECOMPOSE_(bptr, (short) strlen(bptr),
								inoutfilename, sizeof(inoutfilename), &size1,
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
							(short) strlen(inoutfilename), (short *) filename3);
					if (error) {
						printf(
								"launch_proc FILENAME_TO_OLDFILENAME_ returned %d\n",
								error);
						return PROCDEATH_PREMATURE;
					}
					memcpy(smt->infile.whole, filename3,
							sizeof(smt->infile.whole));
				}
			} else if (!strncasecmp(bptr, "OUT", strlen("OUT"))) {
				out_found = 1;
				/*
				 * If (1) no more separators or (2) separator has been
				 * nulled out or (3) a separator immediately follows
				 * this string, then we have no outfile name.
				 */
				if ((sepptr == NULL) || (*sepptr == '\0')
						|| (strspn(bptr + strlen(bptr) + 1, ",/"))) {
					if (ISDB(DB_BASIC))
						printf("launch_proc not setting outfile name\n");
				} else {
					/* in inlen out */
					bptr = strtok(NULL, separators);

					if (ISDB(DB_BASIC))
						printf("launch_proc outfile = '%s' at 0x%x\n", bptr,
								bptr);
					/* need to add node if remote pgm */
					if (remotepgm) {
						rc = FILENAME_DECOMPOSE_(bptr, (short) strlen(bptr),
								inoutfilename, sizeof(inoutfilename), &size1,
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
							(short) strlen(inoutfilename), (short *) filename3);
					if (error) {
						printf(
								"launch_proc FILENAME_TO_OLDFILENAME_ returned %d\n",
								error);
						return PROCDEATH_PREMATURE;
					}
					memcpy(smt->outfile.whole, filename3,
							sizeof(smt->outfile.whole));
				}
			} else if (!strncasecmp(bptr, "TERM", strlen("TERM"))) {
				out_found = 1;
				/*
				 * If (1) no more separators or (2) separator has been
				 * nulled out or (3) a separator immediately follows
				 * this string, then we have no outfile name.
				 */
				if ((sepptr == NULL) || (*sepptr == '\0')
						|| (strspn(bptr + strlen(bptr) + 1, ",/"))) {
					if (ISDB(DB_BASIC))
						printf("launch_proc not setting term name\n");
				} else {
					/* in inlen out */
					bptr = strtok(NULL, separators);

					if (ISDB(DB_BASIC))
						printf("launch_proc term = '%s' at 0x%x\n", bptr,
								bptr);
					/* need to add node if remote pgm */
					if (remotepgm) {
						rc = FILENAME_DECOMPOSE_(bptr, (short) strlen(bptr),
								sethometerm, sizeof(sethometerm), &size1,
								-1, 1);
						if (rc) {
							printf("launch_proc FILENAME_DECOMPOSE_ failed\n");
							return PROCDEATH_PREMATURE;
						}
						sethometerm[size1] = NULL;
					} else {
						strcpy(sethometerm, bptr);
					}
				}
			} else if (!strncasecmp(bptr, "CPU", strlen("CPU"))) {
				bptr = strtok(NULL, separators);

				if ((bptr == NULL) || (sepptr < bptr)) {
					/* We have a comma or slash or eol prior to our input file arg */
					printf("launch_proc missing cpu argument, found: %s\n",
							((bptr == NULL) ? "NULL" : bptr));
					return PROCDEATH_PREMATURE;
				}

				if (ISDB(DB_BASIC))
					printf("launch_proc cpu = '%s' at 0x%x\n", bptr, bptr);

				plist.cpu = (short) atoi(bptr);

			} else if (!strncasecmp(bptr, "DEBUG", strlen("DEBUG"))) {

				if (ISDB(DB_BASIC))
					printf("launch_proc debug is set\n");

				plist.debug_options = ZSYS_VAL_PCREATOPT_RUND;

			} else if (!strncasecmp(bptr, "NAME", strlen("NAME"))) {
				/*
				 * If (1) no more separators or (2) separator has been
				 * nulled out or (3) a separator immediately follows
				 * this string, then we have no name.
				 */
				if ((sepptr == NULL) || (*sepptr == '\0')
						|| (strspn(bptr + strlen(bptr) + 1, ",/"))) {
					if (ISDB(DB_BASIC))
						printf("launch_proc using generated process name\n");
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
						plist.process_name_len = (short) strlen(process_name);

						if (ISDB(DB_BASIC)) {
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
		rc = PROCESS_GETINFO_( /* phandle */, /* procfname */, /* procmax */,
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

	if (ISDB(DB_BASIC))
		printf("launch_proc launching infile= '%.24s', outfile= '%.24s'\n",
				smt->infile.whole, smt->outfile.whole);

	if (bptr && *bptr) { /* must be the param-set (arg1; arg2, argn) */
		while (*bptr == ' ')
			bptr++;

		if (strlen(bptr) >= sizeof(smt->param) - 3) {
			/* Need more space for our parameters */
			smt = realloc(smt,
					sizeof(startup_msg_type) + strlen(bptr) - sizeof(smt->param)
							+ 4);
			if (ISDB(DB_BASIC))
				printf("launch_proc realloc size = %d\n",
						sizeof(startup_msg_type) + strlen(bptr)
								- sizeof(smt->param) + 4);
			if (smt == NULL) {
				printf("launch_proc realloc failed\n");
				return PROCDEATH_PREMATURE;
			}
		}
		strcpy(smt->param, bptr);
		if (ISDB(DB_BASIC))
			printf("launch_proc param = '%s'\n", bptr);
	} else
		smt->param[0] = NULL;

	param_len = (short) strlen(smt->param);
	if (ISDB(DB_BASIC))
		printf("launch_proc param len = %d\n", param_len);
	for (index = param_len; index < (param_len + 3); smt->param[index++] = 0)
		;
	index -= (index % 2) ? 1 : 2;
	if (ISDB(DB_BASIC))
		printf("launch_proc new param len = %d\n", index);

	if (!namespecified) {
		/* Set default value for "name" run-option */
		plist.name_options = ZSYS_VAL_PCREATOPT_NAMEDBYSYS; /* 4 chars long */
	}

	plist.create_options |= //
			ZSYS_VAL_PCREATOPT_DEFENABLED +
			ZSYS_VAL_PCREATOPT_DEFOVERRIDE;
	plist.program_name = cmd;
	plist.program_name_len = (long) strlen(cmd);
	plist.hometerm_name = sethometerm;
	plist.hometerm_name_len = (long) strlen(sethometerm);
	error = PROCESS_LAUNCH_((void *) &plist, &errordet, (void *) &olist,
			sizeof(olist), &olistlen);
	if (error) {
		/* ZGRD_VAL_PCREATERR_EXTERNALS 14 */
		if (error == 14) {
			printf(
					"*** PROCESS_LAUNCH_ Warning %d: [%s] has undefined externals, "
							"but was started anyway\n", 14, cmd);
		} else {
			printf("launch_proc PROCESS_LAUNCH_ Error = %d, errordet = %d\n",
					error, errordet);
			return PROCDEATH_PREMATURE;
		}
	}

	if (ISDB(DB_BASIC))
		printf("launch_proc Opening %.*s\n", olist.z_procname_len,
				olist.u_z_data.z_procname);
	error = FILE_OPEN_(olist.u_z_data.z_procname, olist.z_procname_len,
			&filenum);
	if (error) {
		FILE_GETINFO_(filenum, &errordet);
		printf("launch_proc FILE_OPEN_ %s failed with error %d, %d\n",
				olist.u_z_data.z_procname, error, errordet);
		return PROCDEATH_PREMATURE;
	}

	if (ISDB(DB_BASIC))
		printf("launch_proc sent get_startup_msg, "
				"len %d, code %d, '%.16s', '%.24s', '%.24s', '%s'\n",
				sizeof(startup_msg_type) - sizeof(smt->param) + index,
				smt->msg_code, smt->defaults.whole, smt->infile.whole,
				smt->outfile.whole, smt->param);
	wrerror = WRITEREADX(filenum, (char *) smt,
			(short) (sizeof(startup_msg_type) - sizeof(smt->param) + index),
			sizeof(startup_msg_type), (unsigned short *) &countread);
	if (wrerror) {
		FILE_GETINFO_(filenum, &errordet);
		if (errordet == 70) {
			if (ISDB(DB_BASIC))
				printf(
						"launch_proc WRITEREADX got error 70 (send params/assigns)\n");
			doassigns = 1;
			doparams = 1;
		} else {
			printf("launch_proc WRITEREADX failed with error %d, %d\n", wrerror,
					errordet);
			FILE_CLOSE_(filenum);
			return PROCDEATH_PREMATURE;
		}
	} else if (countread) {
		bptr = (char *) smt;
		doassigns = (short) (*bptr & 0x80);
		doparams = (short) (*bptr & 0x40);
		if (ISDB(DB_BASIC))
			printf(
					"launch_proc WRITEREADX Countread was %d, assigns flag was %x, "
							"params flag was %x\n", countread, doassigns,
					doparams);
	}

	if (doparams) {
		rc = get_param_msg_local(&pmt, &plen);
		if (ISDB(DB_BASIC))
			printf(
					"launch_proc get_param_msg returned %d, code %d, params %d\n",
					rc, pmt.msg_code, pmt.num_params);

		if (!rc) {
			wrerror = WRITEX(filenum, (char *) &pmt, plen);
			if (wrerror) {
				FILE_GETINFO_(filenum, &errordet);
				printf("launch_proc WRITEX failed with error %d, %d\n", wrerror,
						errordet);
				FILE_CLOSE_(filenum);
				return PROCDEATH_PREMATURE;
			}
		}
	} /* if (doparams) */

	if (doassigns) {
		if (ISDB(DB_BASIC))
			printf("launch_proc get_max_assign_msg_ordinal returned %d\n", _num_assigns);
		for (index = 0; index < _num_assigns; index++) { /* send each one */
			AssignEntry *entry = _assigns + index;
			if (ISDB(DB_BASIC)) {
				printf("launch_proc get_assign_msg returned %d, %.*s, %.*s, "
						"0x%x, %.24s\n", entry->value->msg_code,
						entry->value->logical_unit_name.prognamelen,
						entry->value->logical_unit_name.progname,
						entry->value->logical_unit_name.filenamelen,
						entry->value->logical_unit_name.filename,
						entry->value->field_mask, entry->value->filename.whole);
				/* in out outmax outlen */
				error = OLDFILENAME_TO_FILENAME_(
						(short *) entry->value->filename.whole, filename3,
						sizeof(filename3), &size1);
				printf("launch_proc OLDFILENAME_TO_FILENAME_ returned %d\n",
						error);
				if (!error) {
					filename3[size1] = 0;
					printf("launch_proc filename = %.*s, size = %d\n", size1,
							filename3, size1);
				}
			}
			wrerror = WRITEX(filenum, (char *) entry->value,
					sizeof(*(entry->value)));
			if (wrerror) {
				FILE_GETINFO_(filenum, &errordet);
				printf("launch_proc WRITEX failed with error %d, %d\n", wrerror,
						errordet);
				FILE_CLOSE_(filenum);
				return PROCDEATH_PREMATURE;
			}
		} /* for (index = 0; index <= _num_assigns; index++) */
	} /* if (doassigns) */

	FILE_CLOSE_(filenum);

	error = FILE_OPEN_("$RECEIVE", (short) strlen("$RECEIVE"), &filenum, //
			/* acc */, /* excl */, 1 /* nowait */, 1 /* recv depth */, //
			0 /* rw */);
	if (error) {
		FILE_GETINFO_(filenum, &errordet);
		printf("launch_proc FILE_OPEN_ $RECEIVE failed with error %d, %d\n",
				error, errordet);
		return PROCDEATH_PREMATURE;
	}

	if (capture != 0 && capture_len > 0) {
		capture[0] = '\0';
	}

	if (ISDB(DB_BASIC))
		printf("launch_proc Waiting for process death message\n");
	running = 1;
	if (ISDB(DB_BASIC))
		printf("launch_proc issuing READUPDATEX($RECEIVE)\n");
	READUPDATEX(filenum, buf, sizeof(buf), (unsigned short *) &len);
	while (running) {
		anyfile = -1;
		len32 = 0;
		wrerror = AWAITIOXL(&anyfile, , &len32);
		len = (short) len32;
		if (anyfile == filenum) {
			if (wrerror == 6) { /* system (not user) message */
				if (smsg->u_z_msg.z_msgnumber[0] == ZSYS_VAL_SMSG_PROCDEATH) {
					if (ISDB(DB_BASIC))
						printf("launch_proc PROCDEATH received\n");
					wrerror = CHILD_LOST_(buf, len, (short *) &olist.z_phandle);
					if (wrerror != 4) { /* is this the process we last started? */
						if (ISDB(DB_BASIC)) {
							printf("launch_proc CHILD_LOST_ returned %d for ",
									wrerror);
							printf("%.*s which returned %d\n",
									spdmsg->z_procname.zlen,
									(char *) ((char *) smsg
											+ spdmsg->z_procname.zoffset),
									spdmsg->z_completion_code);
						}
					} else {
						if (spdmsg->z_flags & ZSYS_MSK_PDEATH_ABENDED) {
							if (ISDB(DB_BASIC))
								printf("launch_proc: process abended\n");
						}
						if (ISDB(DB_BASIC))
							printf("launch_proc %.*s returned %d\n",
									spdmsg->z_procname.zlen,
									(char *) ((char *) smsg
											+ spdmsg->z_procname.zoffset),
									spdmsg->z_completion_code);
						procrc = spdmsg->z_completion_code;
						running = 0; /* break; */
					}
				} else if (smsg->u_z_msg.z_msgnumber[0] == ZSYS_VAL_SMSG_OPEN) {
					if (ISDB(DB_BASIC))
						printf("launch_proc OPEN received\n");
					if ((options & LAUNCH_PROC_OUT_PARENT)
							&& childfilenum <= 0) {
						if (ISDB(DB_BASIC))
							printf("launch_proc FILE_OPEN_ initiated to %s\n",
									olist.u_z_data.z_procname);
						error = FILE_OPEN_(olist.u_z_data.z_procname,
								olist.z_procname_len, &childfilenum, //
								/* acc */, /* excl */, /* nowait */2, //
								/* sync */, 0x4000);
						if (error) {
							FILE_GETINFO_(childfilenum, &errordet);
							printf(
									"launch_proc FILE_OPEN_ %s failed with error %d, %d\n",
									olist.u_z_data.z_procname, error, errordet);
							procrc = PROCDEATH_PREMATURE;
							running = 0;
						}
					}
				} else if (smsg->u_z_msg.z_msgnumber[0] == ZSYS_VAL_SMSG_CONTROL) {
					if (ISDB(DB_BASIC))
						printf("launch_proc CONTROL received\n");
				} else if (smsg->u_z_msg.z_msgnumber[0]
						== ZSYS_VAL_SMSG_TIMESIGNAL) {
					if (ISDB(DB_BASIC))
						printf("launch_proc TIMESIGNAL received\n");
					if (options & LAUNCH_PROC_OUT_PARENT) {
						TIMER_STOP_(-1);
						if (ISDB(DB_BASIC))
							printf("launch_proc TLE %d stopped\n", tleId);
						if (!written) {
							strcpy(cbuf, argv[0]);
							for (index = 1; argv[index]; index++) {
								strcat(cbuf, " ");
								strcat(cbuf, argv[index]);
							}
							if (ISDB(DB_BASIC))
								printf(
										"launch_proc %s constructed to go to TACL\n",
										cbuf);
							wrerror = FILE_WRITE64_(childfilenum, (char *) cbuf,
									strlen(cbuf));
							if (wrerror) {
								FILE_GETINFO_(childfilenum, &errordet);
								printf(
										"launch_proc WRITEX failed to TACL with error %d, %d\n",
										wrerror, errordet);
								FILE_CLOSE_(childfilenum);
								procrc = PROCDEATH_PREMATURE;
								running = 0;
							}
							written = 1;
							waitingforoutput = 1;
						}
					}
				} else { /* if (smsg->u_z_msg.z_msgnumber[0] == ...PROCDEATH) */
					if (ISDB(DB_BASIC))
						printf("launch_proc READUPDATEX got msgnumber %d\n",
								smsg->u_z_msg.z_msgnumber[0]);
				}
			} else if (wrerror == 0) {
				buf[len] = '\0';
				if (ISDB(DB_BASIC))
					printf("launch_proc READUPDATEX got user message\n");
				if ((options & LAUNCH_PROC_OUT_PARENT) && waitingforoutput) {
					if (size2 > capture_len) {
						if (ISDB(DB_BASIC))
							printf(
									"launch_proc READUPDATEX got user message discarded\n");
					}
					if (size2 + len > capture_len) {
						if (ISDB(DB_BASIC))
							printf(
									"launch_proc READUPDATEX got user message discarded partial\n");
						len = capture_len - size2;
						buf[len] = '\0';
						strcpy(&(capture[size2]), buf);
						size2 = capture_len;
					} else {
						if (ISDB(DB_BASIC))
							printf("launch_proc appended %s\n", buf);
						strcpy(&(capture[size2]), buf);
						size2 += len;
					}
					if (!exitWritten) {
						char exitBuf[10];
						strcpy(exitBuf, "exit");
						if (ISDB(DB_BASIC))
							printf("launch_proc %s constructed to go to TACL\n",
									exitBuf);
						wrerror = FILE_WRITE64_(childfilenum, exitBuf,
								strlen(exitBuf));
						if (wrerror) {
							FILE_GETINFO_(childfilenum, &errordet);
							printf(
									"launch_proc FILE_WRITE64_failed to TACL with error %d, %d\n",
									wrerror, errordet);
							FILE_CLOSE_(childfilenum);
							procrc = PROCDEATH_PREMATURE;
							running = 0;
						}
						exitWritten = 1;
					}
				} else if ((options & LAUNCH_PROC_OUT_PARENT)
						&& !waitingforoutput) {
					if (tleId == 1) {
						TIMER_START_(1000000, 0, 0, &tleId);
						if (ISDB(DB_BASIC))
							printf("launch_proc TLE %d started\n", tleId);
					}
				} else {
					TIMER_STOP_(tleId);
					if (ISDB(DB_BASIC))
						printf("launch_proc TLE %d stopped\n", tleId);
					if (ISDB(DB_BASIC))
						printf(
								"launch_proc READUPDATEX got user message preamble discarded\n");
				}
			}
			REPLYX();
			if (running) {
				if (ISDB(DB_BASIC))
					printf("launch_proc issuing READUPDATEX($RECEIVE)\n");
				wrerror = READUPDATEX(filenum, buf, sizeof(buf),
						(unsigned short *) &len);
			}
		} else if (anyfile == childfilenum) {
			if (written) {
				if (exitWritten) {
					if (ISDB(DB_BASIC))
						printf(
								"launch_proc FILE_WRITE64_ exit completed on child.\n");
				} else {
					if (ISDB(DB_BASIC))
						printf(
								"launch_proc FILE_WRITE64_ completed on child.\n");
				}
				FILE_CLOSE_(childfilenum);
				childfilenum = -1;
			} else {
				if (ISDB(DB_BASIC))
					printf("launch_proc FILE_OPEN_completed on child.\n");
				// TODO: We could initiate the timer here instead of elsewhere.
			}
		} else {
			printf("launch_proc AWAITIOXL got error %d\n", wrerror);
			procrc = PROCDEATH_PREMATURE;
			running = 0;
		}
	} /* while(running) */

	if (childfilenum > 0) {
		FILE_CLOSE_(childfilenum);
	}
	FILE_CLOSE_(filenum);
	if (ISDB(DB_BASIC))
		printf("launch_proc returning %d\n", procrc);
	return procrc;
}

char *
reference_define(char *ptr, char **string) {
	char defineName[25];
	char *s = defineName;
	char *end = *string;
	short error, length;

	if (*end != '=')
		return ptr;

	*s++ = *end++;
	/* Pull out the DEFINE name */
	for (size_t i = 1; i < sizeof(defineName); i++) {
		if (i == 1) {
			if (!isalpha(*end))
				return ptr;
		}
		if (!isalnum(*end) && *end != '^' && *end != '_')
			break;

		*s++ = *end++;
	}
	*s = '\0';
	/* end points to the character after the define. */
	/* s can now be discarded. */
	/* ptr points to the = to be substituted. */

	error = FILENAME_RESOLVE_(defineName, (short) strlen(defineName), ptr,
	ZSYS_VAL_LEN_FILENAME, &length,
	/* resolve defines */0x0018);
	if (error != 0) {
		printf("FILENAME_RESOLVE_ of DEFINE %s failed with error %d\n",
				defineName, error);
		return ptr;
	}
	ptr[length] = '\0';

	if (ISDB(DB_BASIC))
		printf("Resolving DEFINE %s to %s\n", defineName, ptr);

	ptr += length;
	*string = end - 1; /* backtrack 1 if we resolved stuff */

	return ptr;
}

char *xstrdup(const char *ptr);

char *
resolve_define(const char *name) {
	char defineName[25];
	char filename[ZSYS_VAL_LEN_FILENAME + 1];
	short error, length;

	error = FILENAME_RESOLVE_(name, (short) strlen(name), filename,
	ZSYS_VAL_LEN_FILENAME, &length,
	/* resolve defines */0x0018);
	if (error != 0) {
		printf("FILENAME_RESOLVE_ of DEFINE %s failed with error %d\n", name,
				error);
		return xstrdup(name);
	}
	filename[length] = '\0';

	if (ISDB(DB_BASIC))
		printf("Resolving DEFINE %s to %s\n", defineName, filename);

	return xstrdup(filename);
}

const char *ignoreWarnings[] = {
		"$SYSTEM.SYSTEM.DDL",
		NULL,
};

#define PROCDEATH_WARNING 1 /* Proc Calls App. C Completion Codes: file? */
int ignore_tandem_warning(int exit_code, const char *command_line) {
	if (exit_code != PROCDEATH_WARNING)
		return 1;
	if (legacy_cc)
		return 0;
	for (size_t index=0; ignoreWarnings[index]; index++) {
		const char *s = command_line;
		while (*s == ' ')
			s++;
		const char *t = s;
		while (*t == '$' || isalpha(*t) || *t == '.')
			t++;
		if (strncasecmp(ignoreWarnings[index], s, t-s) == 0)
			return 0;
	}
	return 1;
}

/*===========================================================================*/
#pragma page "T0593 GUARDIAN GNU Make- tandem.c Change Descriptions"
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
 */
