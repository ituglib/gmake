#!/bin/sh
#
# Copyright (c) 2020 Make Project
#

test_description='Test the test infrastructure
'

. ./test-lib.sh

try_local_x () {
	local x="local" &&
	echo "$x"
}

test_expect_success 'verify that the running shell supports "local"' '
	x="notlocal" &&
	echo "local" >expected1 &&
	try_local_x >actual1 &&
	test_cmp expected1 actual1 &&
	echo "notlocal" >expected2 &&
	echo "$x" >actual2 &&
	test_cmp expected2 actual2
'

################################################################
# Test harness
test_expect_success 'success is reported like this' '
	:
'

_run_sub_test_lib_test_common () {
	neg="$1" name="$2" descr="$3" # stdin is the body of the test code
	shift 3
	mkdir "$name" &&
	(
		# Pretend we're not running under a test harness, whether we
		# are or not. The test-lib output depends on the setting of
		# this variable, so we need a stable setting under which to run
		# the sub-test.
		unset HARNESS_ACTIVE &&
		cd "$name" &&
		cat >"$name.sh" <<-EOF &&
		#!$SHELL_PATH

		test_description='$descr (run in sub test-lib)

		This is run in a sub test-lib so that we do not get incorrect
		passing metrics
		'

		# Point to the t/test-lib.sh, which isn't in ../ as usual
		. "\$TEST_DIRECTORY"/test-lib.sh
		EOF
		cat >>"$name.sh" &&
		chmod +x "$name.sh" &&
		export TEST_DIRECTORY &&
		TEST_OUTPUT_DIRECTORY=$(pwd) &&
		export TEST_OUTPUT_DIRECTORY &&
		if test -z "$neg"
		then
			./"$name.sh" "$@" >out 2>err
		else
			!  ./"$name.sh" "$@" >out 2>err
		fi
	)
}

run_sub_test_lib_test () {
	_run_sub_test_lib_test_common '' "$@"
}

run_sub_test_lib_test_err () {
	_run_sub_test_lib_test_common '!' "$@"
}

check_sub_test_lib_test () {
	name="$1" # stdin is the expected output from the test
	(
		cd "$name" &&
		! test -s err &&
		sed -e 's/^> //' -e 's/Z$//' >expect &&
		test_cmp expect out
	)
}

check_sub_test_lib_test_err () {
	name="$1" # stdin is the expected output from the test
	# expected error output is in descriptior 3
	(
		cd "$name" &&
		sed -e 's/^> //' -e 's/Z$//' >expect.out &&
		test_cmp expect.out out &&
		sed -e 's/^> //' -e 's/Z$//' <&3 >expect.err &&
		test_cmp expect.err err
	)
}

test_expect_success 'pretend we have a fully passing test suite' "
	run_sub_test_lib_test full-pass '3 passing tests' <<-\\EOF &&
	for i in 1 2 3
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test full-pass <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 - passing test #3
	> # passed all 3 test(s)
	> 1..3
	EOF
"

test_expect_success 'pretend we have a partially passing test suite' "
	test_must_fail run_sub_test_lib_test \
		partial-pass '2/3 tests passing' <<-\\EOF &&
	test_expect_success 'passing test #1' 'true'
	test_expect_success 'failing test #2' 'false'
	test_expect_success 'passing test #3' 'true'
	test_done
	EOF
	check_sub_test_lib_test partial-pass <<-\\EOF
	> ok 1 - passing test #1
	> not ok 2 - failing test #2
	#	false
	> ok 3 - passing test #3
	> # failed 1 among 3 test(s)
	> 1..3
	EOF
"

test_expect_success 'pretend we have a known breakage' "
	run_sub_test_lib_test failing-todo 'A failing TODO test' <<-\\EOF &&
	test_expect_success 'passing test' 'true'
	test_expect_failure 'pretend we have a known breakage' 'false'
	test_done
	EOF
	check_sub_test_lib_test failing-todo <<-\\EOF
	> ok 1 - passing test
	> not ok 2 - pretend we have a known breakage # TODO known breakage
	> # still have 1 known breakage(s)
	> # passed all remaining 1 test(s)
	> 1..2
	EOF
"

test_expect_success 'pretend we have fixed a known breakage' "
	run_sub_test_lib_test passing-todo 'A passing TODO test' <<-\\EOF &&
	test_expect_failure 'pretend we have fixed a known breakage' 'true'
	test_done
	EOF
	check_sub_test_lib_test passing-todo <<-\\EOF
	> ok 1 - pretend we have fixed a known breakage # TODO known breakage vanished
	> # 1 known breakage(s) vanished; please update test(s)
	> 1..1
	EOF
"

test_expect_success 'pretend we have fixed one of two known breakages (run in sub test-lib)' "
	run_sub_test_lib_test partially-passing-todos \
		'2 TODO tests, one passing' <<-\\EOF &&
	test_expect_failure 'pretend we have a known breakage' 'false'
	test_expect_success 'pretend we have a passing test' 'true'
	test_expect_failure 'pretend we have fixed another known breakage' 'true'
	test_done
	EOF
	check_sub_test_lib_test partially-passing-todos <<-\\EOF
	> not ok 1 - pretend we have a known breakage # TODO known breakage
	> ok 2 - pretend we have a passing test
	> ok 3 - pretend we have fixed another known breakage # TODO known breakage vanished
	> # 1 known breakage(s) vanished; please update test(s)
	> # still have 1 known breakage(s)
	> # passed all remaining 1 test(s)
	> 1..3
	EOF
"

test_expect_success 'pretend we have a pass, fail, and known breakage' "
	test_must_fail run_sub_test_lib_test \
		mixed-results1 'mixed results #1' <<-\\EOF &&
	test_expect_success 'passing test' 'true'
	test_expect_success 'failing test' 'false'
	test_expect_failure 'pretend we have a known breakage' 'false'
	test_done
	EOF
	check_sub_test_lib_test mixed-results1 <<-\\EOF
	> ok 1 - passing test
	> not ok 2 - failing test
	> #	false
	> not ok 3 - pretend we have a known breakage # TODO known breakage
	> # still have 1 known breakage(s)
	> # failed 1 among remaining 2 test(s)
	> 1..3
	EOF
"

test_expect_success 'pretend we have a mix of all possible results' "
	test_must_fail run_sub_test_lib_test \
		mixed-results2 'mixed results #2' <<-\\EOF &&
	test_expect_success 'passing test' 'true'
	test_expect_success 'passing test' 'true'
	test_expect_success 'passing test' 'true'
	test_expect_success 'passing test' 'true'
	test_expect_success 'failing test' 'false'
	test_expect_success 'failing test' 'false'
	test_expect_success 'failing test' 'false'
	test_expect_failure 'pretend we have a known breakage' 'false'
	test_expect_failure 'pretend we have a known breakage' 'false'
	test_expect_failure 'pretend we have fixed a known breakage' 'true'
	test_done
	EOF
	check_sub_test_lib_test mixed-results2 <<-\\EOF
	> ok 1 - passing test
	> ok 2 - passing test
	> ok 3 - passing test
	> ok 4 - passing test
	> not ok 5 - failing test
	> #	false
	> not ok 6 - failing test
	> #	false
	> not ok 7 - failing test
	> #	false
	> not ok 8 - pretend we have a known breakage # TODO known breakage
	> not ok 9 - pretend we have a known breakage # TODO known breakage
	> ok 10 - pretend we have fixed a known breakage # TODO known breakage vanished
	> # 1 known breakage(s) vanished; please update test(s)
	> # still have 2 known breakage(s)
	> # failed 3 among remaining 7 test(s)
	> 1..10
	EOF
"

test_expect_success 'test --verbose' '
	test_must_fail run_sub_test_lib_test \
		test-verbose "test verbose" --verbose <<-\EOF &&
	test_expect_success "passing test" true
	test_expect_success "test with output" "echo foo"
	test_expect_success "failing test" false
	test_done
	EOF
	mv test-verbose/out test-verbose/out+ &&
	grep -v "^Initialized empty" test-verbose/out+ >test-verbose/out &&
	check_sub_test_lib_test test-verbose <<-\EOF
	> expecting success: true
	> ok 1 - passing test
	> Z
	> expecting success: echo foo
	> foo
	> ok 2 - test with output
	> Z
	> expecting success: false
	> not ok 3 - failing test
	> #	false
	> Z
	> # failed 1 among 3 test(s)
	> 1..3
	EOF
'

test_expect_success 'test --verbose-only' '
	test_must_fail run_sub_test_lib_test \
		test-verbose-only-2 "test verbose-only=2" \
		--verbose-only=2 <<-\EOF &&
	test_expect_success "passing test" true
	test_expect_success "test with output" "echo foo"
	test_expect_success "failing test" false
	test_done
	EOF
	check_sub_test_lib_test test-verbose-only-2 <<-\EOF
	> ok 1 - passing test
	> Z
	> expecting success: echo foo
	> foo
	> ok 2 - test with output
	> Z
	> not ok 3 - failing test
	> #	false
	> # failed 1 among 3 test(s)
	> 1..3
	EOF
'

test_expect_success 'MAKE_SKIP_TESTS' "
	(
		MAKE_SKIP_TESTS='make.2' && export MAKE_SKIP_TESTS &&
		run_sub_test_lib_test make-skip-tests-basic \
			'MAKE_SKIP_TESTS' <<-\\EOF &&
		for i in 1 2 3
		do
			test_expect_success \"passing test #\$i\" 'true'
		done
		test_done
		EOF
		check_sub_test_lib_test make-skip-tests-basic <<-\\EOF
		> ok 1 - passing test #1
		> ok 2 # skip passing test #2 (MAKE_SKIP_TESTS)
		> ok 3 - passing test #3
		> # passed all 3 test(s)
		> 1..3
		EOF
	)
"

test_expect_success 'MAKE_SKIP_TESTS several tests' "
	(
		MAKE_SKIP_TESTS='make.2 make.5' && export MAKE_SKIP_TESTS &&
		run_sub_test_lib_test make-skip-tests-several \
			'MAKE_SKIP_TESTS several tests' <<-\\EOF &&
		for i in 1 2 3 4 5 6
		do
			test_expect_success \"passing test #\$i\" 'true'
		done
		test_done
		EOF
		check_sub_test_lib_test make-skip-tests-several <<-\\EOF
		> ok 1 - passing test #1
		> ok 2 # skip passing test #2 (MAKE_SKIP_TESTS)
		> ok 3 - passing test #3
		> ok 4 - passing test #4
		> ok 5 # skip passing test #5 (MAKE_SKIP_TESTS)
		> ok 6 - passing test #6
		> # passed all 6 test(s)
		> 1..6
		EOF
	)
"

test_expect_success 'MAKE_SKIP_TESTS sh pattern' "
	(
		MAKE_SKIP_TESTS='make.[2-5]' && export MAKE_SKIP_TESTS &&
		run_sub_test_lib_test make-skip-tests-sh-pattern \
			'MAKE_SKIP_TESTS sh pattern' <<-\\EOF &&
		for i in 1 2 3 4 5 6
		do
			test_expect_success \"passing test #\$i\" 'true'
		done
		test_done
		EOF
		check_sub_test_lib_test make-skip-tests-sh-pattern <<-\\EOF
		> ok 1 - passing test #1
		> ok 2 # skip passing test #2 (MAKE_SKIP_TESTS)
		> ok 3 # skip passing test #3 (MAKE_SKIP_TESTS)
		> ok 4 # skip passing test #4 (MAKE_SKIP_TESTS)
		> ok 5 # skip passing test #5 (MAKE_SKIP_TESTS)
		> ok 6 - passing test #6
		> # passed all 6 test(s)
		> 1..6
		EOF
	)
"

test_expect_success '--run basic' "
	run_sub_test_lib_test run-basic \
		'--run basic' --run='1 3 5' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-basic <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 # skip passing test #2 (--run)
	> ok 3 - passing test #3
	> ok 4 # skip passing test #4 (--run)
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with a range' "
	run_sub_test_lib_test run-range \
		'--run with a range' --run='1-3' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-range <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 - passing test #3
	> ok 4 # skip passing test #4 (--run)
	> ok 5 # skip passing test #5 (--run)
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with two ranges' "
	run_sub_test_lib_test run-two-ranges \
		'--run with two ranges' --run='1-2 5-6' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-two-ranges <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 # skip passing test #4 (--run)
	> ok 5 - passing test #5
	> ok 6 - passing test #6
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with a left open range' "
	run_sub_test_lib_test run-left-open-range \
		'--run with a left open range' --run='-3' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-left-open-range <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 - passing test #3
	> ok 4 # skip passing test #4 (--run)
	> ok 5 # skip passing test #5 (--run)
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with a right open range' "
	run_sub_test_lib_test run-right-open-range \
		'--run with a right open range' --run='4-' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-right-open-range <<-\\EOF
	> ok 1 # skip passing test #1 (--run)
	> ok 2 # skip passing test #2 (--run)
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 - passing test #6
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with basic negation' "
	run_sub_test_lib_test run-basic-neg \
		'--run with basic negation' --run='"'!3'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-basic-neg <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 - passing test #6
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run with two negations' "
	run_sub_test_lib_test run-two-neg \
		'--run with two negations' --run='"'!3 !6'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-two-neg <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run a range and negation' "
	run_sub_test_lib_test run-range-and-neg \
		'--run a range and negation' --run='"'-4 !2'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-range-and-neg <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 # skip passing test #2 (--run)
	> ok 3 - passing test #3
	> ok 4 - passing test #4
	> ok 5 # skip passing test #5 (--run)
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run range negation' "
	run_sub_test_lib_test run-range-neg \
		'--run range negation' --run='"'!1-3'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-range-neg <<-\\EOF
	> ok 1 # skip passing test #1 (--run)
	> ok 2 # skip passing test #2 (--run)
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 - passing test #6
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run include, exclude and include' "
	run_sub_test_lib_test run-inc-neg-inc \
		'--run include, exclude and include' \
		--run='"'1-5 !1-3 2'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-inc-neg-inc <<-\\EOF
	> ok 1 # skip passing test #1 (--run)
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run include, exclude and include, comma separated' "
	run_sub_test_lib_test run-inc-neg-inc-comma \
		'--run include, exclude and include, comma separated' \
		--run=1-5,\!1-3,2 <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-inc-neg-inc-comma <<-\\EOF
	> ok 1 # skip passing test #1 (--run)
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 - passing test #4
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run exclude and include' "
	run_sub_test_lib_test run-neg-inc \
		'--run exclude and include' \
		--run='"'!3- 5'"' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-neg-inc <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 - passing test #2
	> ok 3 # skip passing test #3 (--run)
	> ok 4 # skip passing test #4 (--run)
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run empty selectors' "
	run_sub_test_lib_test run-empty-sel \
		'--run empty selectors' \
		--run='1,,3,,,5' <<-\\EOF &&
	for i in 1 2 3 4 5 6
	do
		test_expect_success \"passing test #\$i\" 'true'
	done
	test_done
	EOF
	check_sub_test_lib_test run-empty-sel <<-\\EOF
	> ok 1 - passing test #1
	> ok 2 # skip passing test #2 (--run)
	> ok 3 - passing test #3
	> ok 4 # skip passing test #4 (--run)
	> ok 5 - passing test #5
	> ok 6 # skip passing test #6 (--run)
	> # passed all 6 test(s)
	> 1..6
	EOF
"

test_expect_success '--run invalid range start' "
	run_sub_test_lib_test_err run-inv-range-start \
		'--run invalid range start' \
		--run='a-5' <<-\\EOF &&
	test_expect_success \"passing test #1\" 'true'
	test_done
	EOF
	check_sub_test_lib_test_err run-inv-range-start \
		<<-\\EOF_OUT 3<<-\\EOF_ERR
	> FATAL: Unexpected exit with code 1
	EOF_OUT
	> error: --run: invalid non-numeric in range start: 'a-5'
	EOF_ERR
"

test_expect_success '--run invalid range end' "
	run_sub_test_lib_test_err run-inv-range-end \
		'--run invalid range end' \
		--run='1-z' <<-\\EOF &&
	test_expect_success \"passing test #1\" 'true'
	test_done
	EOF
	check_sub_test_lib_test_err run-inv-range-end \
		<<-\\EOF_OUT 3<<-\\EOF_ERR
	> FATAL: Unexpected exit with code 1
	EOF_OUT
	> error: --run: invalid non-numeric in range end: '1-z'
	EOF_ERR
"

test_expect_success '--run invalid selector' "
	run_sub_test_lib_test_err run-inv-selector \
		'--run invalid selector' \
		--run='1?' <<-\\EOF &&
	test_expect_success \"passing test #1\" 'true'
	test_done
	EOF
	check_sub_test_lib_test_err run-inv-selector \
		<<-\\EOF_OUT 3<<-\\EOF_ERR
	> FATAL: Unexpected exit with code 1
	EOF_OUT
	> error: --run: invalid non-numeric in test selector: '1?'
	EOF_ERR
"


test_set_prereq HAVEIT
haveit=no
test_expect_success HAVEIT 'test runs if prerequisite is satisfied' '
	test_have_prereq HAVEIT &&
	haveit=yes
'
donthaveit=yes
test_expect_success DONTHAVEIT 'unmet prerequisite causes test to be skipped' '
	donthaveit=no
'
if test $haveit$donthaveit != yesyes
then
	say "bug in test framework: prerequisite tags do not work reliably"
	exit 1
fi

test_set_prereq HAVETHIS
haveit=no
test_expect_success HAVETHIS,HAVEIT 'test runs if prerequisites are satisfied' '
	test_have_prereq HAVEIT &&
	test_have_prereq HAVETHIS &&
	haveit=yes
'
donthaveit=yes
test_expect_success HAVEIT,DONTHAVEIT 'unmet prerequisites causes test to be skipped' '
	donthaveit=no
'
donthaveiteither=yes
test_expect_success DONTHAVEIT,HAVEIT 'unmet prerequisites causes test to be skipped' '
	donthaveiteither=no
'
if test $haveit$donthaveit$donthaveiteither != yesyesyes
then
	say "bug in test framework: multiple prerequisite tags do not work reliably"
	exit 1
fi

test_lazy_prereq LAZY_TRUE true
havetrue=no
test_expect_success LAZY_TRUE 'test runs if lazy prereq is satisfied' '
	havetrue=yes
'
donthavetrue=yes
test_expect_success !LAZY_TRUE 'missing lazy prereqs skip tests' '
	donthavetrue=no
'

if test "$havetrue$donthavetrue" != yesyes
then
	say 'bug in test framework: lazy prerequisites do not work'
	exit 1
fi

test_lazy_prereq LAZY_FALSE false
nothavefalse=no
test_expect_success !LAZY_FALSE 'negative lazy prereqs checked' '
	nothavefalse=yes
'

havefalse=yes
test_expect_success LAZY_FALSE 'missing negative lazy prereqs will skip' '
	havefalse=no
'

if test $test_platform != IBM
then
if test "$nothavefalse$havefalse" != yesyes
then
	say 'bug in test framework: negative lazy prerequisites do not work'
	exit 1
fi
fi

clean=no
test_expect_success 'tests clean up after themselves' '
	test_when_finished clean=yes
'

if test $clean != yes
then
	say "bug in test framework: basic cleanup command does not work reliably"
	exit 1
fi

test_expect_failure 'tests clean up even on failures' "
	test_must_fail run_sub_test_lib_test \
		failing-cleanup 'Failing tests with cleanup commands' <<-\\EOF &&
	test_expect_success 'tests clean up even after a failure' '
		touch clean-after-failure &&
		test_when_finished rm clean-after-failure &&
		(exit 1)
	'
	test_expect_success 'failure to clean up causes the test to fail' '
		test_when_finished \"(exit 2)\"
	'
	test_done
	EOF
	check_sub_test_lib_test failing-cleanup <<-\\EOF
	> not ok 1 - tests clean up even after a failure
	> #	Z
	> #	touch clean-after-failure &&
	> #	test_when_finished rm clean-after-failure &&
	> #	(exit 1)
	> #	Z
	> not ok 2 - failure to clean up causes the test to fail
	> #	Z
	> #	test_when_finished \"(exit 2)\"
	> #	Z
	> # failed 2 among 2 test(s)
	> 1..2
	EOF
"

test_done
