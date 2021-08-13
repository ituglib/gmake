#!/bin/sh
#
# Copyright (c) 2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Test copylib extensions
'

. ./test-lib.sh

test_expect_success 'simple ddldict compile' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: dict(odf-v2.id)

dict(odf-v2.id): \$system.system.ddschema foo
        \$(DDL) /IN \$<,OUT =SPOOL/ DICT ${MAKE_SUBVOL_PREFIX_GUARDIAN}${this_test} !
//
EOF
	edit_loader foo <<-EOF > /dev/null &&
a

//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.DDL /IN \$system.system.ddschema,OUT =SPOOL/ DICT ${MAKE_SUBVOL_PREFIX_GUARDIAN}${this_test} !
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest compile - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success 'recompile - changed foo' '
	edit_loader foo <<-EOF > /dev/null &&
a

//
EOF
	launch_spoolcom_delete $this_test >/dev/null &&
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.DDL /IN \$system.system.ddschema,OUT =SPOOL/ DICT ${MAKE_SUBVOL_PREFIX_GUARDIAN}${this_test} !
	EOF
	test_cmp expecting actual
'

test_expect_success 'ddldict member touch' '
	edit_loader foo <<-EOF > /dev/null &&
a

//
EOF
	launch_make --touch > actual &&
	cat >expecting <<-EOF &&
	touch dict(odf-v2.id)
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest compile after touch - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_done
