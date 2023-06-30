# GMake - GNU Make Guardian Port Manual

This manual describes the GNU Make Guardian port extensions and differences
from the main GNU Make program. The [GNU Make Manual](https://www.gnu.org/software/make/manual/make.html)
describes the program in detail. This manual describes only the extensions.

GMake is a program for automatically determining which parts of software
programs need to be recompiled and runs the commands to perform the compilation
of each out of date component. This port runs in the Guardian personality of
the NonStop J-series and L-series platforms and is subject to the capabilities
available on those platforms.

This edition, last updated on 30 January 2023, was written for the 4.3g8
version of GMake, based on GNU Make 4.3. There have been many contributors to
GMake including Hewlett-Packard Enterprise LLC, ITUGLIB Engineering Team - part
of Connect Inc., and Nexbridge Inc.

Copyright &copy; 2020-2023, ITUGLIB Engineering Team. Permission is granted to
copy, distribute and/or modify this document under the terms of the GNU Free
Documentation License, Version 1.3 or any later version published by the
Free Software Foundation; with no Invariant Sections, with the Front-Cover
Texts being "GMake - GNU Make Guardian Port Manual". A copy of the license is
included in the section entitled "GNU Free Documentation License."

## Overview

GMake is a program for automatically determining which parts of software
programs need to be recompiled and runs the commands to perform the compilation
of each out of date component.

This manual describes the extensions for HPE NonStop to GNU Make, called GMake,
which was implemented by Richard Stallman and Roland McGrath and ported to HPE
NonStop by a number of people. GNU Make development since Version 3.76 has been
handled by Paul D. Smith. Extensions development and porting since Version 3.77
has been handled by Randall S. Becker of the ITUGLIB Engineering Team, part of
the Connect Community, with time donated by Nexbridge Inc.

GMake conforms to section 6.2 of IEEE Standard 1003.2-1992 (POSIX.2) except as
might occur out of necessity for variants to support NonStop Guardian interfaces
and APIs.

## Variations from GNU Make

### Command Options

The `--legacy-cc` option, or `PARAM LEGACY-CC 1` will cause the original GMAKE
port behaviour to be used, where `CC=1` will always be considered an ignored
warning. Otherwise, a non-zero completion code is considered an error.

The `--search-define=define-name` specifies the name of a CLASS SEARCH DEFINE
variable GMAKE uses for resolving program file names. If unspecified, the SEARCH
DEFINE is assumed to be named `=PMSEARCHLIST`. Do not specify the extra `=`
character when using this command option. GMAKE will look for any unqualified
file names using the sub-volumes specified in this SEARCH DEFINE.

### Interpretation

Because the Guardian operating system is case insensitive, file name
interpretation may have issues. File `a` is interpreted as equivalent to `A`
by GMake.

Special characters, like `\` and `$` have special meaning in
Guardian file names. Node names may not be interpreted correctly and should
be avoided or tested before use.

`$` values in Guardian refer to disk volumes. The use of `$` should be clearly
disambiguated from GMake variables using `()` characters.

### Variable Substitution

Variables in GNU Make are referenced with using either of the following forms:

    $V
    $(VARIABLE)

In GMake, the first format, a `$` followed by a letter, is not interpreted as
a variable. Instead, all tokens represented by a `$` followed by letters are
considered to be disk volumes in the Guardian space. The `$V` form should not
be used. The `$(VARIABLE)` form should be used for all variable references.

### Special Variables

Guardian MAP DEFINE values, if they are in the GMake process context, can be
referenced using the form:

    $=DEFINE

In this case, the variable will be substituted with the value of the `FILE`
attribute of the MAP DEFINE.

This variable form can also be specified on the `GMAKE` command line when
referencing targets.

### Predefined Rules

Many of the predefined rules in GNU Make have no effect in GMake because files
in Guardian have no file extensions.

### Built-in Commands

There are two built-ins that allow some compatibility with GNU Make semantics
without having direct shell access: `echo`; and `rm`.

#### `echo`

The `echo` command prints out strings supplied on the command line. The syntax
of the command is:

    echo [ string ] ...

`string` can be quoted with single or double quotes. The quotes are stripped off
before being printed.

The built-in `echo` command does not support any Guardian run options.

#### `rm`

The `rm` command removes files from a supplied set of patterns. This is more
flexible than using `FUP`, which requires commas between sets of files. The
syntax of the command is:

    rm [ file-pattern | file-name ] ...

`file-pattern` is a valid file pattern containing `*` and/or `?` wildcard
characters. Any files matching the supplied pattern are removed. If the pattern
does not match any existing files, no error is reported and no file is purged. 

`file-name` is a valid file name without any wildcards. This argument is always
assumed to match a file, so if the file does not exist, GMake reports an error
`11` (not found).

If a file cannot be purged, whether directly specified or matched via a pattern,
the reason error code is reported. 
 
The built-in `rm` command does not support any Guardian run options.

### Predefined Variables

Many of the predefined variables in GNU Make have no effect in GMake because
files in Guardian have no file extensions.

The `$*` predefined has no meaning in GMake.

Any of the `$(@D)` or `$(@F)` predefines have no meaning in GMake.

Other substitution forms may also not work in GMake.

### Predefined Functions

Many of the predefined functions are not appropriate in the Guardian space. This
section describes specific limitations but is not necessarily complete.

The `$(wildcard pattern)` function works for Guardian file name patterns. The
patterns must be fully-qualified optionally including the Expand node name and
must resolve to file names only. An example of this function usage is:

    $(wildcard $data01.mysubvol.file*)

The semantics of the pattern specifications of `*` and `?` are the same as for
the standard GNU Make.  

## Building GMAKE

`GMAKE` is build in OSS using `/usr/coreutils/bin/make`.

### L-Series

To build on L-Series, use make, and specify a Guardian target file.

    make -f Makefile.NSX.OSS GMAKE_EXE=/G/.../.../gmake

### J-Series

To build on J-Series, use make, and specify a Guardian target file.

    make -f Makefile.NSE.OSS GMAKE_EXE=/G/.../.../gmake

### Workstations

Makefiles are supplied for NSDEE and scripted builds. The Makefiles for the
platforms are `Makefile.NSX.Win` and `Makefile.NSE.Win`. Because of a
requirement to use the `$system.zsutver.psvgth` file for the
`TOS_VERSION_FULL` variable, which is not available to cross compilers, the
value reported by this variable will not include the third version identifier on
workstation builds - although you can override Make variables for `INCLUDE`
and `CFLAGS` to point to that file by turning off the `-D OMIT_PSVGT`. 

## Execution Differences

### Load Handling

The `GMAKE` program handles the `load` and `-load` commands slightly
differently from standard GNU Make. On Guardian, the separator is assumed to
be `.` instead of `/`.
 
### Error Handling

The `GMAKE` program differs from the standard GNU Make in terms of how
completion codes are handled. Because of differences in completion codes
generated by compilers, handling has deviated. The original GMAKE port always
considered completion code 1 as an ignored warning. This has not always been
a desirable outcome. In the case of the DDL compiler, there is no way to
suppress `CC=1` even if warnings are ignored. As a result, `GMAKE`
considers `CC=1` generated by DDL to be ignored in all cases. Other
processes generating `CC=1` are treated consistently with GNU Make and will
cause the recipe to fail. The `--legacy-cc` option, or `PARAM LEGACY-CC 1`
will cause the original GMAKE port behaviour to be used, where `CC=1` will
always be considered an ignored warning.

### Predefined Variables

The following predefined variables are added to GMake:

| Variable  | Default            | Meaning                                                    |
| --------- | ------------------ | -----------------------------------------------------------|
| `TNS_PLATFORM` | `E`,`X`       | Operating system on which GMAKE is running.                |
| `TOS_VERSION` | `J06`,`L21`, etc.       | Operating system release identifier.                   |
| `TOS_VERSION_FULL` | `J06.23.01`,`L21.02.00`, etc. | Full operating system release version.    |
| `SYSVOL` | `$SYSTEM.SYSTEM` | The default location of system programs.                   |
| `OSHARGS`| `-osstty`         | Default arguments for OSH, specified in the $(SH) variable.|
| `SH`      | `$(SYSVOL).OSH`  | The default location of OSH.                               |
| `ARFLAGS`  | `rv`                  | AR program standard arguments.                            |
| `AR`        | `$(SYSVOL).AR`      | Object Archive program location.                           |
| `AXCEL`     | `$(SYSVOL).AXCEL`  | Non-Native Object Acceleration program location.           |
| `BIND`     | `$(SYSVOL).BIND`     | BINDER program object location.                           |
| `CC`        | `$(SYSVOL).C`        | C compiler program location.                              |
| `COBEX`    | `$(SYSVOL).XCOBEX0`   | COBEX program location. `XCOBEX0` on J-series.         |
| `COBOL`    | `$(SYSVOL).XCOBOL`    | Native COBOL program location. `ECOBOL` on J-series.    |
| `COBOL85`  | `$(SYSVOL).COBOL85` | COBOL85 compiler program location.                        |
| `CCOMP`    | `$(SYSVOL).CCOMP`    | CCOMP compiler program location.                          |
| `CFRONT`   | `$(SYSVOL).CFRONT`   | CFRONT preprocessor program location.                     |
| `CPP`       | `$(SYSVOL).C`        | C++ compiler program location.                            |
| `CPPCOMP`  | `$(SYSVOL).CPPCOMP`  | C++ compiler  program location.                          |
| `CTOEDIT`  | `$(SYSVOL).CTOEDIT`  | C-to-Edit conversion program location.                    |
| `DDL`       | `$(SYSVOL).DDL`      | DDL compiler program location.                            |
| `ENABLE`   | `$(SYSVOL).ENABLE`   | ENABLE  program location.                                 |
| `EDIT`     | `$(SYSVOL).EDIT`      | EDIT utility program location.                           |
| `EDITTOC`  | `$(SYSVOL).EDITTOC`  | Edit-to-C conversion program location.                    |
| `FORTRAN`  | `$(SYSVOL).FORTRAN`  | FORTRAN compiler program location.                        |
| `FUP`      | `$(SYSVOL).FUP`       | FUP utility program location.                             |
| `LD`       | `$(SYSVOL).XLD`        | Linker program location. `ELD` on J-series.             |
| `NMC`       | `$(SYSVOL).NMC`      | NMC compiler program location. J-series only.             |
| `OCA`      | `$(SYSVOL).OCA`       | OCA program location.                                     |
| `OSH`       | `$(SYSVOL).OSH $(OSHARGS)` | The default location of OSH and arguments.        |
| `PATHCOM`  | `$(SYSVOL).PATHCOM`   | PATHCOM program location.                                |
| `PDMCOM`   | `$(SYSVOL).PDMCOM`    | PDMCOM program location.                                 |
| `PTAL`     | `$(SYSVOL).XPTAL`     | PTAL program location. `EPTAL` on J-series              |
| `SCOBOL`   | `$(SYSVOL).SCOBOLX`  | SCOBOL compiler program location.                         |
| `SCOBOLX`  | `$(SYSVOL).SCOBOLX`  | Alternate SCOBOL compiler program location.               |
| `SCUP`     | `$(SYSVOL).SCUP`      | SCUP utility program location.                           |
| `TACL`     | `$(SYSVOL).TACL`      | TACL interpreter program location.                        |
| `TAL`       | `$(SYSVOL).TAL`      | TAL compiler program location.                            |
| `TEMPL`    | `$(SYSVOL).TEMPL`     | TEMPL compiler program location.                          |
| `TEMPLI`   | `$(SYSVOL).TEMPLI`    | Template installer program location.                     |
| `TFORM`    | `$(SYSVOL).TFORM`     | TFORM documenter program location.                        |
| `TGAL`     | `$(SYSVOL).TGAL`      | TGAL documenter program location.                         |
| `SQLCI`    | `$(SYSVOL).SQLCI`     | SQLCI interpreter program location.                       |
| `SQLCOMP`  | `$(SYSVOL).SQLCOMP`  | SQLCOMP compiler program location.                        |
| `SPOOLCOM` | `$(SYSVOL).SPOOLCOM` | SPOOLCOM program location.                                |
| `VPROC`    | `$(SYSVOL).VPROC`     | VPROC utility program location.                           |
| `NSGITVOL` | `$SYSTEM.NSGIT`       | Default NSGit installation subvolume                     |
| `GMAKEDEP` | `$(NSGITVOL).GMAKEDEP` | GMAKEDEP dependency generator program location.         |
| `NSGIT`    | `$(NSGITVOL).NSGIT`   | NSGit program location.                                  |

More variables will be defined in future releases.

### Predefined Functions

Some substitution functions are not supported by GMake. Please check
individual functions before committing to their use.

#### `$(add_define define-attributes)`

The `add_define` function adds a Guardian define into the process context of
the GMake process. The added DEFINE is usable as a DEFINE variable in rules and
recipes. It is also passed to any process started by the GMake process. DEFINE
names always must begin with an `=` followed by a combination of 1 to 24
letters, numbers, carets or underscore. The `add_define` function can create
many different DEFINE classes:

`$(add_define =define,map,file)` function creates a MAP DEFINE.

`$(add_define =define,catalog,sub-volume)` function creates a CATALOG
DEFINE.

`$(add_define =define,spool,location[,attribute=value]...)` function
creates a SPOOL DEFINE with a location. Any SPOOL define attribute can be
specified to this function.

`$(add_define =define,search,(subvol|relsubvol)=value...)` function
creates a SEARCH DEFINE. The `subvol` and `relsubvol` attributes are
automatically numbered and can appear multiple times.

The `$(define_add)` function is an alias to `$(add_define)` for historical
reasons.

#### `$(delete_define define|**)`

The `delete_define` function removes one or all Guardian DEFINES in the PFS
of the GMake process.

`$(delete_define =define)` removes a specific DEFINE. No error is reported
if the specified DEFINE does not exist.

`$(delete_define **)` removes all DEFINEs. No error is reported if there
are no current DEFINEs.

The `$(define_delete)` is an alias to `$(delete_define)` for historical
reasons.

#### `$(assign name,file)`

The `assign` function adds a Guardian `ASSIGN` to the process context. The
initial set of `ASSIGN` values are loaded when GMAKE starts. This function can
modify or add an `ASSIGN`.

Note that functions are evaluated as a set before programs are run, so clearing
and setting `ASSIGNs` should be specified before running programs in a recipe.
See also `$(clear_assign)` below.

**Example:**

    object: source
            $(param SWAPVOL $SWAP)
            $(TAL) /in $</ $@

#### `$(clear_assign name)`

The `clear_assign` function removes one or all Guardian `ASSIGNs` from the
process context. If `name` is `*`, all `ASSIGNs` are removed. If the specified
`ASSIGN` does not exist, no error is reported.

Note that functions are evaluated as a set before programs are run, so clearing
and setting `ASSIGNs` should be specified before running programs in a recipe.

**Example:**

    object: source
            $(clear_assign *)
            $(assign SSV0 $SYSTEM.SYSTEM)
            $(TAL) /in $</ $@

#### `$(delay time units)`

The `$(delay time units)` function causes the current recipe to delay for
some period of time. This is similar to calling sleep in OSS. _time_ must be a
positive integer. _units_ can be one of: **microseconds** (the default);
**milliseconds**; **seconds**; **minutes**; or **hours**.

#### `$(param name value)`

The `param` function adds a Guardian `PARAM` to the process context. The
initial set of `PARAM` values are loaded when GMAKE starts. This function can
modify or add a `PARAM`.

Note that functions are evaluated as a set before programs are run, so clearing
and setting `PARAMs` should be specified before running programs in a recipe.
See also `$(clear_param)` below.

**Example:**

    object: source
            $(param SWAPVOL $SWAP)
            $(TAL) /in $</ $@

#### `$(clear_param name)`

The `clear_param` function removes one or all Guardian `PARAMs` from the
process context. If `name` is `*`, all `PARAMs` are removed. If the specified
param does not exist, no error is reported.

Note that functions are evaluated as a set before programs are run, so clearing
and setting `PARAMs` should be specified before running programs in a recipe.

**Example:**

    object: source
            $(clear_param *)
            $(param SWAPVOL $SWAP)
            $(TAL) /in $</ $@

#### `$(pname file)`

The `pname` function converts a Guardian file name into an OSS path. The
Guardian name can be fully or partially qualified and does not need to exist.
The resulting OSS path will always be fully qualified. If the EXPAND node in
the Guardian name is the same as the local node, the OSS path starts with `/G`,
otherwise the OSS path starts with `/E`. The function is similar to the OSS
command `pname -s guardian-file`.

**Example:**

    SRC = $VOL.SUBVOL.FILE
    OSS_SRC = $(pname $(SRC))

#### `$(shell command)`

The shell command function is implemented in GMake. This function executes the
specified command in a TACL process, captures the output, and returns the
result. Typically, the result should only be a single line of output. You can
assign the result to a variable, for example:

    VERSION:=$(shell nsgit describe --long --first-parent)

captures the current version from the most recent git tag in the local NSGit
repository. Caution must be used with the `$(shell)` function as not all
programs are compatible with its use. Some OSH functions are known to cause
problems for the `$(shell)` function and may not work as intended.

It is recommended that the `:=` operator be used in conjunction with the
`$(shell)` function so that the evaluation is done in isolation instead of as
part of a recipe. 

#### `$(vcompare version1,version2)`

The `vcompare` command function compares two version strings and reports
whether the versions are equal, less, or greater than each other. If `version1`
is less than `version1`, `-1` is returned. If the two are equal, `0` is
returned. If `version1` is greater than `version2`, `1` is returned. If the
two versions do not contain blanks, it is legal to use a blank separator instead
of a comma between the two. `vcompare` checks each dot(`.`) or space(` `)
separated segment of each version. If the segment contains only digits, a
numerical comparison is used, so `1.100` is greater than `1.1`. If the two
numbers are equal but different in length, like `01` and `1`, the numbers are
considered different with the longer string being considered less because it
would have to have leading zeros. TOS versions can be compared. The following
is a usage example:

    ifeq ($vcompare $(TOS_VERSION_FULL),L20.10.00),1)

checks whether the operating system is more recent than `L20.10.00`.

### Shell Control

GMake does not, by default, use the OSS shell to execute commands. Instead,
commands are executed directly using the `PROCESS_LAUNCH_` method. Commands
should be specified as they are in TACL, using the fully qualified object file
names, for example:

    $SYSTEM.SYSTEM.TAL/IN source,OUT $s.#hold/

Object file names can be unqualified if a CLASS SEARCH DEFINE is provided to
GMAKE as described above.

Each step in a recipe must be an executable program that can be run directly.
If you need to run a TACL MACRO, you must use TACL explicitly with the MACRO
run through an `IN` file. The following run options are currently supported and
are all optional:

| Option  | Argument         | Meaning                                                                      |
| ------- | ---------------- | ---------------------------------------------------------------------------- |
|   `IN`  | file            | The file to be used as an input file to the program. This is generally used. |
|  `OUT`  | file or spooler | The file or location to be used to capture output. This is optional.         |
| `TERM`  | file            | The file to be used as an home terminal to the program. This is optional.    |
|  `CPU`  | number          | The CPU on which the program will be run.                                    |
| `NAME`  | pname           | The name of the process to be run.                                           |
| `DEBUG` |                 | Causes the program to be started in DEBUG mode.                              |

The `NAME` option is always used, by default. Specifying `/NAME/` without a
process name will use a system generated name, as would not specifying the
option at all. `/NAME $pn1/` would create a process with the name `$pn` instead
of a system generated name.

### Special Commands

The `PARAM` command is supported by `GMAKE` in recipes. `PARAMs` can be
quoted or unquoted:

    PARAM SWAPVOL $SYSTEM

The `ASSIGN` command is also supported in recipes. `ASSIGNs` can specify
either a sub-volume or a file. As with the `$(assign)` function, extended
attributes are not supported.

    ASSIGN SSV0,$SYSTEM.SYSTEM

The `CLEAR` command can be used to remove either `ASSIGNs` or `PARAMs` as
follows:

    CLEAR PARAM ALL
    CLEAR PARAM SWAPVOL
    CLEAR ASSIGN ALL
    CLEAR ASSIGN SSV0

### Running OSS Commands

When running OSS commands, use the `$(SH)` predefined variable to launch commands.
`$(SH)` expands to `$SYSTEM.SYSTEM.OSH`. The `-osstty` argument should
generally be used when running OSS commands. It is automatically included in the
`$(OSHARGS)` variable and referenced by `$(SH)`, for example:

    $(SH) -c "echo Running a build"

expands to:

    $SYSTEM.SYSTEM.OSH -osstty -c "echo Running a build"

## Archives

Archive targets are specified in GMake the same way as in GNU Make, as follows:

    archive(MEMBER): source1 source2 ...

The caveat for archives is that member names are case insensitive and should
be specified in upper case, but that is not required.

## Extensions

The following extensions are currently defined for GMake:

* Guardian DEFINE variable expansion.
* Program execution using `PROCESS_LAUNCH_` instead of shell.
* Recognition of disk and process names instead of single character variables.

## Appendix A

Appendix A GNU Free Documentation License
Version 1.3, 3 November 2008

Copyright (C) 2000, 2001, 2002, 2007, 2008 Free Software Foundation, Inc.
https://fsf.org/

Everyone is permitted to copy and distribute verbatim copies
of this license document, but changing it is not allowed.

### PREAMBLE

The purpose of this License is to make a manual, textbook, or other functional and useful document free in the sense of freedom: to assure everyone the effective freedom to copy and redistribute it, with or without modifying it, either commercially or noncommercially. Secondarily, this License preserves for the author and publisher a way to get credit for their work, while not being considered responsible for modifications made by others.

This License is a kind of "copyleft", which means that derivative works of the document must themselves be free in the same sense. It complements the GNU General Public License, which is a copyleft license designed for free software.

We have designed this License in order to use it for manuals for free software, because free software needs free documentation: a free program should come with manuals providing the same freedoms that the software does. But this License is not limited to software manuals; it can be used for any textual work, regardless of subject matter or whether it is published as a printed book. We recommend this License principally for works whose purpose is instruction or reference.

### 1. APPLICABILITY AND DEFINITIONS

This License applies to any manual or other work, in any medium, that contains a notice placed by the copyright holder saying it can be distributed under the terms of this License. Such a notice grants a world-wide, royalty-free license, unlimited in duration, to use that work under the conditions stated herein. The "Document", below, refers to any such manual or work. Any member of the public is a licensee, and is addressed as "you". You accept the license if you copy, modify or distribute the work in a way requiring permission under copyright law.

A "Modified Version" of the Document means any work containing the Document or a portion of it, either copied verbatim, or with modifications and/or translated into another language.

A "Secondary Section" is a named appendix or a front-matter section of the Document that deals exclusively with the relationship of the publishers or authors of the Document to the Document's overall subject (or to related matters) and contains nothing that could fall directly within that overall subject. (Thus, if the Document is in part a textbook of mathematics, a Secondary Section may not explain any mathematics.) The relationship could be a matter of historical connection with the subject or with related matters, or of legal, commercial, philosophical, ethical or political position regarding them.

The "Invariant Sections" are certain Secondary Sections whose titles are designated, as being those of Invariant Sections, in the notice that says that the Document is released under this License. If a section does not fit the above definition of Secondary then it is not allowed to be designated as Invariant. The Document may contain zero Invariant Sections. If the Document does not identify any Invariant Sections then there are none.

The "Cover Texts" are certain short passages of text that are listed, as Front-Cover Texts or Back-Cover Texts, in the notice that says that the Document is released under this License. A Front-Cover Text may be at most 5 words, and a Back-Cover Text may be at most 25 words.

A "Transparent" copy of the Document means a machine-readable copy, represented in a format whose specification is available to the general public, that is suitable for revising the document straightforwardly with generic text editors or (for images composed of pixels) generic paint programs or (for drawings) some widely available drawing editor, and that is suitable for input to text formatters or for automatic translation to a variety of formats suitable for input to text formatters. A copy made in an otherwise Transparent file format whose markup, or absence of markup, has been arranged to thwart or discourage subsequent modification by readers is not Transparent. An image format is not Transparent if used for any substantial amount of text. A copy that is not "Transparent" is called "Opaque".

Examples of suitable formats for Transparent copies include plain ASCII without markup, Texinfo input format, LaTeX input format, SGML or XML using a publicly available DTD, and standard-conforming simple HTML, PostScript or PDF designed for human modification. Examples of transparent image formats include PNG, XCF and JPG. Opaque formats include proprietary formats that can be read and edited only by proprietary word processors, SGML or XML for which the DTD and/or processing tools are not generally available, and the machine-generated HTML, PostScript or PDF produced by some word processors for output purposes only.

The "Title Page" means, for a printed book, the title page itself, plus such following pages as are needed to hold, legibly, the material this License requires to appear in the title page. For works in formats which do not have any title page as such, "Title Page" means the text near the most prominent appearance of the work's title, preceding the beginning of the body of the text.

The "publisher" means any person or entity that distributes copies of the Document to the public.

A section "Entitled XYZ" means a named subunit of the Document whose title either is precisely XYZ or contains XYZ in parentheses following text that translates XYZ in another language. (Here XYZ stands for a specific section name mentioned below, such as "Acknowledgements", "Dedications", "Endorsements", or "History".) To "Preserve the Title" of such a section when you modify the Document means that it remains a section "Entitled XYZ" according to this definition.

The Document may include Warranty Disclaimers next to the notice which states that this License applies to the Document. These Warranty Disclaimers are considered to be included by reference in this License, but only as regards disclaiming warranties: any other implication that these Warranty Disclaimers may have is void and has no effect on the meaning of this License.

### 2. VERBATIM COPYING

You may copy and distribute the Document in any medium, either commercially or noncommercially, provided that this License, the copyright notices, and the license notice saying this License applies to the Document are reproduced in all copies, and that you add no other conditions whatsoever to those of this License. You may not use technical measures to obstruct or control the reading or further copying of the copies you make or distribute. However, you may accept compensation in exchange for copies. If you distribute a large enough number of copies you must also follow the conditions in section 3.

You may also lend copies, under the same conditions stated above, and you may publicly display copies.

### 3. COPYING IN QUANTITY

If you publish printed copies (or copies in media that commonly have printed covers) of the Document, numbering more than 100, and the Document's license notice requires Cover Texts, you must enclose the copies in covers that carry, clearly and legibly, all these Cover Texts: Front-Cover Texts on the front cover, and Back-Cover Texts on the back cover. Both covers must also clearly and legibly identify you as the publisher of these copies. The front cover must present the full title with all words of the title equally prominent and visible. You may add other material on the covers in addition. Copying with changes limited to the covers, as long as they preserve the title of the Document and satisfy these conditions, can be treated as verbatim copying in other respects.

If the required texts for either cover are too voluminous to fit legibly, you should put the first ones listed (as many as fit reasonably) on the actual cover, and continue the rest onto adjacent pages.

If you publish or distribute Opaque copies of the Document numbering more than 100, you must either include a machine-readable Transparent copy along with each Opaque copy, or state in or with each Opaque copy a computer-network location from which the general network-using public has access to download using public-standard network protocols a complete Transparent copy of the Document, free of added material. If you use the latter option, you must take reasonably prudent steps, when you begin distribution of Opaque copies in quantity, to ensure that this Transparent copy will remain thus accessible at the stated location until at least one year after the last time you distribute an Opaque copy (directly or through your agents or retailers) of that edition to the public.

It is requested, but not required, that you contact the authors of the Document well before redistributing any large number of copies, to give them a chance to provide you with an updated version of the Document.

### 4. MODIFICATIONS

You may copy and distribute a Modified Version of the Document under the conditions of sections 2 and 3 above, provided that you release the Modified Version under precisely this License, with the Modified Version filling the role of the Document, thus licensing distribution and modification of the Modified Version to whoever possesses a copy of it. In addition, you must do these things in the Modified Version:

  1. Use in the Title Page (and on the covers, if any) a title distinct from that of the Document, and from those of previous versions (which should, if there were any, be listed in the History section of the Document). You may use the same title as a previous version if the original publisher of that version gives permission.
  2. List on the Title Page, as authors, one or more persons or entities responsible for authorship of the modifications in the Modified Version, together with at least five of the principal authors of the Document (all of its principal authors, if it has fewer than five), unless they release you from this requirement.
  3. State on the Title page the name of the publisher of the Modified Version, as the publisher.
  4. Preserve all the copyright notices of the Document.
  5. Add an appropriate copyright notice for your modifications adjacent to the other copyright notices.
  6. Include, immediately after the copyright notices, a license notice giving the public permission to use the Modified Version under the terms of this License, in the form shown in the Addendum below.
  7. Preserve in that license notice the full lists of Invariant Sections and required Cover Texts given in the Document's license notice.
  8. Include an unaltered copy of this License.
  9. Preserve the section Entitled "History", Preserve its Title, and add to it an item stating at least the title, year, new authors, and publisher of the Modified Version as given on the Title Page. If there is no section Entitled "History" in the Document, create one stating the title, year, authors, and publisher of the Document as given on its Title Page, then add an item describing the Modified Version as stated in the previous sentence.
  10. Preserve the network location, if any, given in the Document for public access to a Transparent copy of the Document, and likewise the network locations given in the Document for previous versions it was based on. These may be placed in the "History" section. You may omit a network location for a work that was published at least four years before the Document itself, or if the original publisher of the version it refers to gives permission.
  11. For any section Entitled "Acknowledgements" or "Dedications", Preserve the Title of the section, and preserve in the section all the substance and tone of each of the contributor acknowledgements and/or dedications given therein.
  12. Preserve all the Invariant Sections of the Document, unaltered in their text and in their titles. Section numbers or the equivalent are not considered part of the section titles.
  13. Delete any section Entitled "Endorsements". Such a section may not be included in the Modified Version.
  14. Do not retitle any existing section to be Entitled "Endorsements" or to conflict in title with any Invariant Section.
  15. Preserve any Warranty Disclaimers.

If the Modified Version includes new front-matter sections or appendices that qualify as Secondary Sections and contain no material copied from the Document, you may at your option designate some or all of these sections as invariant. To do this, add their titles to the list of Invariant Sections in the Modified Version's license notice. These titles must be distinct from any other section titles.

You may add a section Entitled "Endorsements", provided it contains nothing but endorsements of your Modified Version by various parties; for example, statements of peer review or that the text has been approved by an organization as the authoritative definition of a standard.

You may add a passage of up to five words as a Front-Cover Text, and a passage of up to 25 words as a Back-Cover Text, to the end of the list of Cover Texts in the Modified Version. Only one passage of Front-Cover Text and one of Back-Cover Text may be added by (or through arrangements made by) any one entity. If the Document already includes a cover text for the same cover, previously added by you or by arrangement made by the same entity you are acting on behalf of, you may not add another; but you may replace the old one, on explicit permission from the previous publisher that added the old one.

The author(s) and publisher(s) of the Document do not by this License give permission to use their names for publicity for or to assert or imply endorsement of any Modified Version.

### 5. COMBINING DOCUMENTS

You may combine the Document with other documents released under this License, under the terms defined in section 4 above for modified versions, provided that you include in the combination all of the Invariant Sections of all of the original documents, unmodified, and list them all as Invariant Sections of your combined work in its license notice, and that you preserve all their Warranty Disclaimers.

The combined work need only contain one copy of this License, and multiple identical Invariant Sections may be replaced with a single copy. If there are multiple Invariant Sections with the same name but different contents, make the title of each such section unique by adding at the end of it, in parentheses, the name of the original author or publisher of that section if known, or else a unique number. Make the same adjustment to the section titles in the list of Invariant Sections in the license notice of the combined work.

In the combination, you must combine any sections Entitled "History" in the various original documents, forming one section Entitled "History"; likewise combine any sections Entitled "Acknowledgements", and any sections Entitled "Dedications". You must delete all sections Entitled "Endorsements."

### 6. COLLECTIONS OF DOCUMENTS

You may make a collection consisting of the Document and other documents released under this License, and replace the individual copies of this License in the various documents with a single copy that is included in the collection, provided that you follow the rules of this License for verbatim copying of each of the documents in all other respects.

You may extract a single document from such a collection, and distribute it individually under this License, provided you insert a copy of this License into the extracted document, and follow this License in all other respects regarding verbatim copying of that document.

### 7. AGGREGATION WITH INDEPENDENT WORKS

A compilation of the Document or its derivatives with other separate and independent documents or works, in or on a volume of a storage or distribution medium, is called an "aggregate" if the copyright resulting from the compilation is not used to limit the legal rights of the compilation's users beyond what the individual works permit. When the Document is included in an aggregate, this License does not apply to the other works in the aggregate which are not themselves derivative works of the Document.

If the Cover Text requirement of section 3 is applicable to these copies of the Document, then if the Document is less than one half of the entire aggregate, the Document's Cover Texts may be placed on covers that bracket the Document within the aggregate, or the electronic equivalent of covers if the Document is in electronic form. Otherwise they must appear on printed covers that bracket the whole aggregate.

### 8. TRANSLATION

Translation is considered a kind of modification, so you may distribute translations of the Document under the terms of section 4. Replacing Invariant Sections with translations requires special permission from their copyright holders, but you may include translations of some or all Invariant Sections in addition to the original versions of these Invariant Sections. You may include a translation of this License, and all the license notices in the Document, and any Warranty Disclaimers, provided that you also include the original English version of this License and the original versions of those notices and disclaimers. In case of a disagreement between the translation and the original version of this License or a notice or disclaimer, the original version will prevail.

If a section in the Document is Entitled "Acknowledgements", "Dedications", or "History", the requirement (section 4) to Preserve its Title (section 1) will typically require changing the actual title.

### 9. TERMINATION

You may not copy, modify, sublicense, or distribute the Document except as expressly provided under this License. Any attempt otherwise to copy, modify, sublicense, or distribute it is void, and will automatically terminate your rights under this License.

However, if you cease all violation of this License, then your license from a particular copyright holder is reinstated (a) provisionally, unless and until the copyright holder explicitly and finally terminates your license, and (b) permanently, if the copyright holder fails to notify you of the violation by some reasonable means prior to 60 days after the cessation.

Moreover, your license from a particular copyright holder is reinstated permanently if the copyright holder notifies you of the violation by some reasonable means, this is the first time you have received notice of violation of this License (for any work) from that copyright holder, and you cure the violation prior to 30 days after your receipt of the notice.

Termination of your rights under this section does not terminate the licenses of parties who have received copies or rights from you under this License. If your rights have been terminated and not permanently reinstated, receipt of a copy of some or all of the same material does not give you any rights to use it.

### 10. FUTURE REVISIONS OF THIS LICENSE

The Free Software Foundation may publish new, revised versions of the GNU Free Documentation License from time to time. Such new versions will be similar in spirit to the present version, but may differ in detail to address new problems or concerns. See https://www.gnu.org/copyleft/.

Each version of the License is given a distinguishing version number. If the Document specifies that a particular numbered version of this License "or any later version" applies to it, you have the option of following the terms and conditions either of that specified version or of any later version that has been published (not as a draft) by the Free Software Foundation. If the Document does not specify a version number of this License, you may choose any version ever published (not as a draft) by the Free Software Foundation. If the Document specifies that a proxy can decide which future versions of this License can be used, that proxy's public statement of acceptance of a version permanently authorizes you to choose that version for the Document.

### 11. RELICENSING

"Massive Multiauthor Collaboration Site" (or "MMC Site") means any World Wide Web server that publishes copyrightable works and also provides prominent facilities for anybody to edit those works. A public wiki that anybody can edit is an example of such a server. A "Massive Multiauthor Collaboration" (or "MMC") contained in the site means any set of copyrightable works thus published on the MMC site.

"CC-BY-SA" means the Creative Commons Attribution-Share Alike 3.0 license published by Creative Commons Corporation, a not-for-profit corporation with a principal place of business in San Francisco, California, as well as future copyleft versions of that license published by that same organization.

"Incorporate" means to publish or republish a Document, in whole or in part, as part of another Document.

An MMC is "eligible for relicensing" if it is licensed under this License, and if all works that were first published under this License somewhere other than this MMC, and subsequently incorporated in whole or in part into the MMC, (1) had no cover texts or invariant sections, and (2) were thus incorporated prior to November 1, 2008.

The operator of an MMC Site may republish an MMC contained in the site under CC-BY-SA on the same site at any time before August 1, 2009, provided the MMC is eligible for relicensing.
