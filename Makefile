# WARNING: This Makefile is used only for code analysis and should not be used
# for product builds.
# DO NOT DELETE THIS LINE -- make depend uses it

VPROC=$(shell sh ./version.sh MAIN L01)

# Set the systype flag.
SYSTYPE_FLAG = 

# Set up compiler and its flags
CC=gcc
INCLUDES=-I. -I./glob
CFLAGS=$(SYSTYPE_FLAG) -D INCLUDEDIR=0 -D LIBDIR=0 -D _POSIX_SOURCE -D HAVE_CONFIG_H -D HAVE_SYS_SIGLIST $(INCLUDES) 
GMAKE_EXE=gmake
LIBS=-ldl
LDFLAGS=$(SYSTYPE_FLAG)

.PHONY: version.o

# List of object files
objects = alloca.o  \
          ar.o \
          arscan.o \
          commands.o \
          default.o \
          dir.o \
          expand.o \
          file.o \
          function.o \
          getloadavg.o \
          getopt1.o \
          getopt.o \
          guile.o \
          hash.o \
          implicit.o \
          job.o \
          load.o \
          main.o \
          misc.o \
          output.o \
          remake.o \
          read.o \
          remote-stub.o \
          rule.o \
          signame.o \
          strcache.o \
          tandem.o \
          tandem_ext.o \
          variable.o \
          version.o \
          vpath.o \
          glob/glob.o \
          glob/fnmatch.o

# Link line
$(GMAKE_EXE) : $(objects)
	$(CC) $(LDFLAGS) -o $(GMAKE_EXE) $(objects) $(LIBS)

default : $(GMAKE_EXE)

# Compilation line
%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<
glob/%.o: glob/%.c
	$(CC) -c $(CFLAGS) -o $@ $<

version.o: version.c
	$(CC) -c $(CFLAGS) -DVPROC=$(VPROC) -o $@ $<

# Dependency details
alloca.o       : config.h
ar.o           : makeint.h config.h signame.h filedef.h dep.h glob/fnmatch.h
arscan.o       : makeint.h config.h signame.h 
commands.o     : makeint.h config.h signame.h dep.h filedef.h variable.h job.h commands.h
default.o      : makeint.h config.h signame.h rule.h dep.h filedef.h job.h commands.h variable.h
dir.o          : makeint.h config.h signame.h glob/glob.h
expand.o       : makeint.h config.h signame.h filedef.h job.h commands.h variable.h rule.h
file.o         : makeint.h config.h signame.h dep.h filedef.h job.h commands.h variable.h
function.o     : makeint.h config.h signame.h filedef.h variable.h dep.h job.h commands.h
getloadavg.o   : config.h
getopt1.o      : config.h getopt.h
getopt.o       : config.h getopt.h
guile.o        : makeint.h gnumake.h debug.h filedef.h dep.h variable.h
hash.o         : makeint.h hash.h
implicit.o     : makeint.h config.h signame.h rule.h dep.h filedef.h
job.o          : makeint.h config.h signame.h job.h filedef.h commands.h variable.h
load.o         : makeint.h #include debug.h filedef.h variable.h
main.o         : ver.c makeint.h config.h signame.h dep.h filedef.h variable.h job.h commands.h getopt.h
misc.o         : makeint.h config.h signame.h dep.h
output.o       : makeint.h job.h output.h
read.o         : makeint.h config.h signame.h filedef.h job.h commands.h dep.h
remake.o       : makeint.h config.h signame.h dep.h filedef.h job.h commands.h variable.h rule.h glob/glob.h
remote-stub.o  : makeint.h config.h signame.h filedef.h job.h commands.h
rule.o         : makeint.h config.h signame.h dep.h filedef.h job.h commands.h variable.h rule.h
signame.o      : config.h signame.h
strcache.o     : makeint.h hash.h
tandem.o       : makeint.h
tandem_ext.o   : makeint.h
variable.o     : makeint.h config.h signame.h dep.h filedef.h job.h commands.h variable.h
version.o      : config.h
vpath.o        : makeint.h config.h signame.h filedef.h variable.h
glob/glob.o    : config.h glob/fnmatch.h glob/glob.h
glob/fnmatch.o : config.h glob/fnmatch.h

# Cleaning related.
.PHONY : clean
clean :
	rm -f $(GMAKE_EXE) $(objects)

clobber: clean
