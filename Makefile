#
# Copyright 2011 James Hunt <james@jameshunt.us>
#
# This file is part of libgear, a C framework library.
#
# libgear is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libgear is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libgear.  If not, see <http://www.gnu.org/licenses/>.
#

CFLAGS  := -fPIC -g -Wall -lc -lctest -I.
COVER   := -fprofile-arcs -ftest-coverage

LCOV    := lcov --directory . --base-directory .
GENHTML := genhtml --prefix $(shell dirname `pwd`)
CDOC    := cdoc

SONAME  := libgear.so
VERSION := 1.0.0

test_o  := test/hash.o
test_o  += test/path.o
test_o  += test/string.o
test_o  += test/pack.o
test_o  += test/list.o

############################################################

default: libgear.so

install: default
	install -o root -g root -m 0644 $(SONAME).$(VERSION) /usr/lib
	install -o root -g root -m 0644 gear.h /usr/include
	ldconfig

.PHONY: clean
clean:
	rm -f *.o test/*.o lcov.info lib*.so* test/run
	find . -name '*.gc??' | xargs rm -f
	rm -rf doc/api/* doc/coverage/*

test: test/run
	@LD_LIBRARY_PATH=. ./test/run

coverage:
	$(LCOV) --capture -o $@.tmp
	$(LCOV) --remove $@.tmp > lcov.info
	rm -f $@.tmp
	rm -rf doc/coverage
	mkdir -p doc
	$(GENHTML) -o doc/coverage lcov.info


apidocs:
	mkdir -p doc/api
	rm -rf doc/api/*
	$(CDOC) --root doc/api *.c *.h -v

############################################################

libgear.so: hash.o log.o path.o string.o pack.o
	$(CC) -shared -Wl,-soname,$(SONAME) -o $@.$(VERSION) $+
	ln -sf $@.$(VERSION) $@

test/run: test/run.o $(test_o) gear.o
	$(CC) $(CFLAGS) $(COVER) -o $@ $+

gear.o: hash.c log.c path.c string.c pack.c
	$(CC) $(CFLAGS) $(COVER) -combine -c -o $@ $+
