#ifndef _MAKE_TANDEM_H_
#define _MAKE_TANDEM_H_

/**
 * Initialize internal variables for the tandem module.
 */
void tandem_initialize(void);

/**
 * Set the value of a parameter.
 * @param name the parameter name.
 * @param value the parameter value.
 */
void tandem_set_param(const char *name, const char *value);

/**
 * Clear the value of a parameter.
 * @param name the parameter name.
 */
void tandem_clear_param(const char *name);

/**
 * Set the value of an assign.
 * @param name the assign name.
 * @param value the assign file/subvolume.
 * @return 0 if the assign could be parsed, a GUARDIAN error otherwise.
 */
short tandem_set_assign(const char *name, const char *value);

/**
 * Clear the value of a parameter.
 * @param name the parameter name.
 */
void tandem_clear_assign(const char *name);

/**
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
int launch_proc(char *argv[], char *envp[]);

/**
 * Check whether the exit code of a command should be reported as an ignored tandem warning
 * @param exit_code the exit code reported.
 * @param command the command supplied.
 * @return 1 if the command should be reported as a warning - 0 if the command should fail.
 */
int
ignore_tandem_warning(int exit_code, const char *command);
#endif /* _MAKE_TANDEM_H_ */
