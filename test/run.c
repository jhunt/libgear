/*
  Copyright 2011 James Hunt <james@jameshunt.us>

  This file is part of libgear, a C framework library.

  libgear is free software: you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  libgear is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with libgear.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "test.h"

int main(int argc, char **argv)
{
	TEST_SUITE(list);
	TEST_SUITE(string);
	TEST_SUITE(pack);
	TEST_SUITE(path);
	TEST_SUITE(hash);

	return run_tests(argc, argv);
}