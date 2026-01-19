#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Warning and error suppresion
'

. ./test-lib.sh

test_expect_success 'C compile - always builds' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        \$(CCOMP)/IN A,TERM \$NULL/OBJ1
//
EOF
	edit_loader a <<-EOF &&
dq!a
a
#include <stdio.h>
int main(int argc, char **argv) {
return 0;
}
//
EOF
	launch_make
'

test_expect_success 'C compile - no-rebuild' '
	launch_make
'

test_expect_success 'C compile - always builds with warning' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        _\$(CCOMP)/IN B,TERM \$NULL/OBJ2
//
EOF
	edit_loader b <<-EOF &&
dq!a
a
#include <stdio.h>
int main(int argc, char **argv) {
}
//
EOF
	launch_make
'

test_expect_success 'C compile, always with error' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        -\$(CCOMP)/IN C,TERM \$NULL/OBJ3
//
EOF
	edit_loader c <<-EOF &&
dq!a
a
#include <stdio.h>
int main(int argc, char **argv) {
//
EOF
	launch_make
'

test_expect_success 'echo - no error indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        echo Hello World
//
EOF
	launch_make
'

test_expect_success 'echo - no echo indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        @echo Hello World
//
EOF
	launch_make
'

test_expect_success 'echo - error indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        -echo Hello World
//
EOF
	launch_make
'

test_expect_success 'echo - warning indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        _echo Hello World
//
EOF
	launch_make
'

test_expect_success 'rm - no error indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        rm rm1
//
EOF
	edit_loader rm1 <<-EOF &&
dq!a
e
EOF
	launch_make
'

test_expect_success 'rm - no rm indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        @rm rm2
//
EOF
	edit_loader rm2 <<-EOF &&
dq!a
e
EOF
	launch_make
'

test_expect_success 'rm - error indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        rm rm3
        -rm rm3
//
EOF
	edit_loader rm3 <<-EOF &&
dq!a
e
EOF
	launch_make
'

test_expect_success 'rm - warning indicator' '
	edit_loader makefile <<-EOF &&
dq!a
a
all:
        _rm rm4
//
EOF
	edit_loader rm4 <<-EOF &&
dq!a
e
EOF
	launch_make
'

test_done
