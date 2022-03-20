#ifndef _MAKE_TANDEM_H_
#define _MAKE_TANDEM_H_

/**
 * The set of launch options. These can be OR'd together.
 */
enum {
	/** No options selected. */
	LAUNCH_PROC_NONE = 0,
	/** Set the IN file to $RECEIVE. */
	LAUNCH_PROC_IN_RECEIVE = 0x0001,
	/** Set the OUT file to this process. */
	LAUNCH_PROC_OUT_PARENT = 0x0002,
	/** Force the use of TACL in this process. */
	LAUNCH_PROC_USE_TACL = 0x0004,
} Launch_Proc_Options;

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
 * <pre>
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
 * </pre>
 * @param argv the set of arguments with a NULL value terminating.
 * @param envp the set of environmetn variables with a NULL value terminating.
 * @param capture the optional buffer in which to capture output.
 * @param capture_len the maximum size of the <b>capture</b> buffer.
 * @param options one of a set of options controlling activity of the child.
 * @return the completion code of the launch.
*/
int launch_proc(char *argv[], char *envp[], char *capture, size_t capture_len,
		int options);

/**
 * Reference a define and substitute it into the resulting string.
 * @param ptr the pointer to the output string.
 * @param string the pointer to the input string position. This is updated after the string is consumed.
 * @return the new position at or right of ptr.
 */
char *
reference_define (char *ptr, char **string);

/**
 * Resolve a define into a new string.
 * @param name the define to resolve.
 * @return a pointer to a buffer containing the name.
 */
char *
resolve_define (const char *name);

/**
 * Check whether the exit code of a command should be reported as an ignored tandem warning
 * @param exit_code the exit code reported.
 * @param command the command supplied.
 * @return 1 if the command should be reported as a warning - 0 if the command should fail.
 */
int
ignore_tandem_warning(int exit_code, const char *command);

/**
 * Resolve a subvolume into an OSS path.
 * @param path is the path to convert, like $data01.m2, or m2.
 * @return the resolved path or the original path.
 */
char *resolve_subvolume(char *path);
#endif /* _MAKE_TANDEM_H_ */
