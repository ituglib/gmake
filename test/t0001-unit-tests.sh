#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Module Unit Tests
'

. ./test-lib.sh

test_expect_success 'tandem_ext.h tests' '
	cat <<-EOF >expecting &&
	ToGuardian:
	(null)
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	
	\$data02
	\$data02.subvol
	\$data02.subvol.file
	\\node
	\\node
	\\node.\$data02
	\\node.\$data02.subvol
	\\node.\$data02.subvol.file
	ToOSS:
	(null)
	/E
	/G/data02
	/G/data02/subvol
	/G/data02/subvol/file
	/E/node
	/E/node
	/E/node/G/data02
	/E/node/G/data02/subvol
	/E/node/G/data02/subvol/file
	EOF
	launch_helper "${TANDEM_EXT_HELPER}" node data02 >actual &&
	test_cmp expecting actual
'

test_done
