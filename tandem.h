#ifndef _MAKE_TANDEM_H_
#define _MAKE_TANDEM_H_

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
#endif /* _MAKE_TANDEM_H_ */
