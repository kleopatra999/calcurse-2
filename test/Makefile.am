AUTOMAKE_OPTIONS = foreign

TESTS = \
	true-001.sh \
	run-test-001.sh \
	run-test-002.sh \
	todo-001.sh \
	todo-002.sh \
	todo-003.sh \
	day-001.sh \
	day-002.sh \
	day-003.sh \
	range-001.sh \
	range-002.sh \
	range-003.sh \
	appointment-001.sh \
	appointment-002.sh \
	appointment-003.sh \
	appointment-004.sh \
	appointment-005.sh \
	appointment-006.sh \
	appointment-007.sh \
	appointment-008.sh \
	appointment-009.sh \
	appointment-010.sh \
	appointment-011.sh \
	appointment-012.sh \
	appointment-013.sh \
	appointment-014.sh \
	appointment-015.sh \
	appointment-016.sh \
	appointment-017.sh \
	appointment-018.sh \
	appointment-019.sh \
	appointment-020.sh \
	appointment-021.sh \
	appointment-022.sh \
	event-001.sh \
	event-002.sh \
	event-003.sh \
	event-004.sh \
	event-005.sh \
	event-006.sh \
	next-001.sh \
	search-001.sh \
	bug-002.sh \
	recur-001.sh \
	recur-002.sh \
	recur-003.sh \
	recur-004.sh \
	recur-005.sh

TESTS_ENVIRONMENT = \
	CALCURSE='$(top_builddir)/src/calcurse' \
	DATA_DIR='$(top_srcdir)/test/data/'

AM_CFLAGS = -std=c99 -pedantic -D_POSIX_C_SOURCE=200809L

check_PROGRAMS = run-test
check_SCRIPTS = $(TESTS)

run_test_SOURCES = run-test.c

EXTRA_DIST = \
	$(TESTS) \
	data/apts \
	data/apts-bug-002 \
	data/apts-recur \
	data/conf \
	data/todo
