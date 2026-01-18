#!/bin/sh
#
# Copyright (c) 2020-2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Test POBJ extensions
'

. ./test-lib.sh

test_lazy_prereq POBJDLL '
	test -f `dirname $PRODUCT_BUILD`/gmakeext
'

test_expect_success POBJDLL 'simple pobj compile' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: pobjdir(nsgreqs)

pobjdir(nsgreqs): nsgreqs
        \$(SCOBOLX)/IN \$<,OUT =SPOOL,TERM \$NULL/pobj
//
EOF
	edit_loader nsgreqs <<-EOF > /dev/null &&
a
?SYMBOLS
?COMPILE
 IDENTIFICATION DIVISION.
 PROGRAM-ID.    NSGREQS.
 AUTHOR.        STUDENT.
 DATE-WRITTEN.  05/06/2019.
 ENVIRONMENT DIVISION.
 CONFIGURATION SECTION.
 SOURCE-COMPUTER. TANDEM/16.
 OBJECT-COMPUTER. TANDEM/16, TERMINAL IS T16-6530.
 SPECIAL-NAMES.
     F16-KEY is F16,
     PROTECTED is PROTECTED.

 DATA DIVISION.

 WORKING-STORAGE SECTION.

 01  SCR-BUF.
     02   HELLO-WORLD-WS   PIC X(12) VALUE "Hello World!".

 SCREEN SECTION.
 01  NSGREQ-SCR SIZE 24, 80.
     02  S-MSG AT 05, 34
           PIC X(12)
           FROM HELLO-WORLD-WS PROTECTED.
     02  FILLER AT 23, 10 VALUE "F16 - EXIT".

/
 PROCEDURE DIVISION.
 CONTROL-MAIN.
    DISPLAY BASE NSGREQ-SCR.
    DISPLAY NSGREQ-SCR.
     ACCEPT NSGREQ-SCR
         UNTIL F16-KEY.
 CONTROL-EXIT.
    EXIT PROGRAM.
//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.SCOBOLX/IN nsgreqs,OUT =SPOOL,TERM \$NULL/pobj
	EOF
	test_cmp expecting actual
'

test_expect_success POBJDLL 'retest compile - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success POBJDLL 'recompile - changed source' '
	sleep 2 &&
	edit_loader nsgreqs <<-EOF > /dev/null &&
a

//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.SCOBOLX/IN nsgreqs,OUT =SPOOL,TERM \$NULL/pobj
	EOF
	test_cmp expecting actual
'

test_expect_success POBJDLL 'pobj member touch' '
	sleep 2 &&
	edit_loader nsgreqs <<-EOF > /dev/null &&
a

//
EOF
	sleep 2 &&
	launch_make --touch > actual &&
	cat >expecting <<-EOF &&
	touch pobjdir(nsgreqs)
	EOF
	test_cmp expecting actual
'

test_expect_success POBJDLL 'retest compile after touch - should not run' '
	sleep 2 &&
	launch_make > capture &&
	cat capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_done
