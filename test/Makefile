# GMAKE test runner
#
# Copyright (c) 2020 Randall S. Becker
#

DEST ?= .
SHELL_PATH ?= $(SHELL)
TEST_SHELL_PATH ?= $(SHELL_PATH)
TAR ?= $(TAR)
RM ?= /bin/rm -f
DEFAULT_TEST_TARGET ?= test
HELPERS = 
LDFLAGS=-g
CC=c99
SED=sed
FGREP=fgrep

TEST_RESULTS_DIRECTORY = test-results

# Shell quote;
SHELL_PATH_SQ = $(subst ','\'',$(SHELL_PATH))
TEST_SHELL_PATH_SQ = $(subst ','\'',$(TEST_SHELL_PATH))
TEST_RESULTS_DIRECTORY_SQ = $(subst ','\'',$(TEST_RESULTS_DIRECTORY))

T = $(sort $(wildcard t[0-9][0-9][0-9][0-9]-*.sh))
THELPERS = $(sort $(filter-out $(T),$(wildcard *.sh)))

all: $(HELPERS) $(DEFAULT_TEST_TARGET)

helpers: $(HELPERS)

test: pre-clean
	$(PREAMBLE_VARS) $(MAKE) aggregate-results-and-cleanup

failed:
	@failed=$$(cd '$(TEST_RESULTS_DIRECTORY_SQ)' && \
		grep -l '^failed [1-9]' *.counts | \
		sed -n 's/\.counts$$/.sh/p') && \
	test -z "$$failed" || $(MAKE) $$failed

$(T):
	@echo "*** $@ ***"; '$(TEST_SHELL_PATH_SQ)' $@ $(GMAKE_TEST_OPTS)

pre-clean:
	$(RM) -f $(HELPERS) *.o *.cunit
	$(RM) -r '$(TEST_RESULTS_DIRECTORY_SQ)'
	$(PREAMBLE_VARS) $(MAKE) helpers

aggregate-results-and-cleanup: $(T)
	$(PREAMBLE_VARS) $(MAKE) helpers
	$(PREAMBLE_VARS) $(MAKE) aggregate-results
	$(PREAMBLE_VARS) $(MAKE) clean

aggregate-results:
	for f in '$(TEST_RESULTS_DIRECTORY_SQ)'/t*-*.counts; do \
		echo "$$f"; \
	done | '$(SHELL_PATH_SQ)' ./aggregate-results.sh

%.exe: %.o
	$(CC) $(LDFLAGS) -o $@ $^

%.txe: %.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.cunit: %.c
	@echo "// Start Generated Test List" > $@
	$(FGREP) CUNIT_SETUP $< | \
		$(SED) '1,$$s/CUNIT_SETUP\([^(]*\).*$$/\t\1();/' >> $@
	$(FGREP) CUNIT_TEST $< | \
		$(SED) '1,$$s/CUNIT_TEST\([^(]*\).*$$/\t\1();/' >> $@
	$(FGREP) CUNIT_CLEANUP $< | \
		$(SED) '1,$$s/CUNIT_CLEANUP\([^(]*\).*$$/\t\1();/' >> $@
	@echo "// End Generated Test List" >> $@

.PHONY: pre-clean $(T) aggregate-results clean
