#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Test $shell 
'

. ./test-lib.sh

test_expect_success 'Test $shell simple case' '
	edit_loader makefile <<-EOF &&
a
SHELLOUT:=\$(shell #OUTPUT ABV)
.PHONY: always
always:
        \$(TAL)/IN makeSRC1,TERM \$NULL/;DEFINE SPR=\$(SHELLOUT)
//
EOF
	edit_loader makesrc1 <<-EOF &&
a
?nolist, source \$SYSTEM.SYSTEM.EXTDECS0(STOP)

PROC M MAIN;
BEGIN
   CALL STOP;
END;
//
EOF
	launch_make --legacy-cc > capture &&
	fgrep ABV < capture | sed "s/^  *//g" > actual &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.TAL/IN makeSRC1,TERM \$NULL/;DEFINE SPR=ABV
	EOF
	test_cmp expecting actual
'

test_expect_success 'Test $shell who case' '
	edit_loader makefile <<-EOF &&
dq!a
a
SHELLSING:=\$(shell #OUTPUT ABV)
SHELLOUT:=\$(shell who)
.PHONY: always
always:
        \$(info COMMENT \$(SHELLSING))
        \$(info COMMENT \$(SHELLOUT))
        \$(info COMMENT)
//
EOF
	launch_make > capture &&
	fgrep COMMENT < capture | sed "2s/:.*$//" > actual &&
	cat >expecting <<-EOF &&
	COMMENT ABV
	COMMENT Home terminal
	COMMENT
	EOF
	test_cmp expecting actual
'

test_done
