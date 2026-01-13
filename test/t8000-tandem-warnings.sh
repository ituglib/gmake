#!/bin/sh
#
# Copyright (c) 2021 Nexbridge Inc. All Rights Reserved.
#

test_description='Tandem warning handling
'

. ./test-lib.sh

case `uname -r` in
J*)
	CCOMPILER='$SYSTEM.SYSTEM.NMC'
	;;
L*)
	CCOMPILER='$SYSTEM.SYSTEM.CCOMP'
	;;
*)
	test_failure
	;;
esac

test_expect_success 'simple DDL compile' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: cout

cout: ddlsrc
        \$(DDL) /IN \$<,OUT =SPOOL,TERM \$NULL/ dict !,nosave
//
EOF
	edit_loader ddlsrc <<-EOF > /dev/null &&
a
?c cout !

def def1.
  2 field1     type character 64.
  2 field2     type binary 16 redefines field1.
end.
//
EOF
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	\$SYSTEM.SYSTEM.DDL /IN ddlsrc,OUT =SPOOL,TERM \$NULL/ dict !,nosave
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest compile - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success 'C compile, with warnings as errors' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: obj

obj: csrc
        ${CCOMPILER} /IN \$<,OUT =SPOOL,TERM \$NULL/ \$@
//
EOF

	edit_loader csrc <<-EOF > /dev/null &&
dq!a
a
#include <stdio.h>
static int bob = 0;
int main(int argc, char **argv) {
	printf("Hello world\n");
}
//
EOF
	test_expect_code 2 launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	${CCOMPILER} /IN csrc,OUT =SPOOL,TERM \$NULL/ obj
	EOF
	test_cmp expecting actual
'

test_expect_success 'C compile, with warnings ignored legacy' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: obj

obj: csrc
        ${CCOMPILER} /IN \$<,OUT =SPOOL,TERM \$NULL/ \$@
//
EOF
	sleep 2 &&
	edit_loader csrc <<-EOF > /dev/null &&
dq!a
a
#include <stdio.h>
static int bob = 0;
int main(int argc, char **argv) {
	printf("Hello world\n");
}
//
EOF
	launch_make --legacy-cc > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	${CCOMPILER} /IN csrc,OUT =SPOOL,TERM \$NULL/ obj
	EOF
	test_cmp expecting actual
'

test_expect_success 'retest c compile - should not run' '
	launch_make > capture &&
	sed "1,\$s/^.*:/:/" < capture > actual &&
	echo ": Nothing to be done for ${QUOTE}all${QUOTE}." >expecting &&
	test_cmp expecting actual
'

test_expect_success 'C compile, with warnings ignored new approach' '
	edit_loader makefile <<-EOF > /dev/null &&
dq!a
a
\$(define_delete =SPOOL)
\$(define_add =SPOOL,spool,\$s.#$this_test)

all: obj

obj: csrc
        _${CCOMPILER} /IN \$<,OUT =SPOOL,TERM \$NULL/ \$@
        @echo Got past the compile warning without failing
//
EOF
	sleep 2 &&
	edit_loader csrc <<-EOF > /dev/null &&
dq!a
a
#include <stdio.h>
static int bob = 0;
int main(int argc, char **argv) {
	printf("Hello world\n");
}
//
EOF
	launch_make > actual &&
	launch_spoolcom_delete $this_test >/dev/null &&
	cat >expecting <<-EOF &&
	_${CCOMPILER} /IN csrc,OUT =SPOOL,TERM \$NULL/ obj
	Got past the compile warning without failing
	EOF
	test_cmp expecting actual
'

test_done
