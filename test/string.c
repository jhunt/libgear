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

#include <stdarg.h>

static void assert_auto_string(struct string *s, const char *value)
{
	char buf[256];

	snprintf(buf, 256, "String is set to %s", value);
	assert_str_eq(buf, s->raw, value);

	snprintf(buf, 256, "String is %u chars long", strlen(value));
	assert_int_eq(buf, s->len, strlen(value));
}

static void assert_stringlist(struct stringlist *sl, const char *name, size_t n, ...)
{
	va_list args;
	size_t i;
	char *str;
	char buf[128];

	snprintf(buf, 128, "%s has %d strings", name, n);
	assert_int_eq(buf, sl->num, n);

	va_start(args, n);
	for (i = 0; i < n; i++) {
		str = va_arg(args, char *);
		snprintf(buf, 128, "%s->strings[%d] == '%s'", name, i, str);
		assert_str_eq(buf, sl->strings[i], str);
	}
	va_end(args);

	snprintf(buf, 128, "%s->strings[%d] is NULL", name, sl->num);
	assert_null(buf, sl->strings[sl->num]);
}

static int setup_list_item(struct stringlist *list, const char *item)
{
	char buf[128];
	int ret;

	snprintf(buf, 128, "adding item '%s' to list", item);
	assert_int_eq(buf, 0, ret = stringlist_add(list, item));
	return ret;
}

static struct stringlist* setup_list(const char *first, ...)
{
	va_list args;
	struct stringlist *list;
	const char *item;

	list = stringlist_new(NULL);
	assert_not_null("stringlist_new returns a non-NULL pointer", list);

	if (setup_list_item(list, first) != 0) { return list; }

	va_start(args, first);
	while ( (item = va_arg(args, const char*)) != NULL) {
		if (setup_list_item(list, item) != 0) { break; }
	}
	va_end(args);
	return list;
}

/**********************************************************************/

NEW_TEST(string_interpolation)
{
	char buf[8192];
	struct hash *context;

	const char *tests[][2] = {
		{ "string with no references",
		  "string with no references" },

		{ "string with simple (a-z) ref: $ref1",
		  "string with simple (a-z) ref: this is a reference" },

		{ "multi.level.fact = ${multi.level.fact}",
		  "multi.level.fact = MULTILEVEL" },

		{ "Unknown variable: $unknown",
		  "Unknown variable: " },

		{ "Unknown CREF: ${unknown.cref}",
		  "Unknown CREF: " },

		{ "Escaped ref: \\$ref1",
		  "Escaped ref: $ref1" },

		{ "Multiple refs: $ref1, $ref1 and $name",
		  "Multiple refs: this is a reference, this is a reference and Clockwork" },

		{ "Dollar sign at end of string: \\$",
		  "Dollar sign at end of string: $" },

		{ "Bare dollar sign at end: $",
		  "Bare dollar sign at end: " },

		{ NULL, NULL }
	};

	test("STRING: Interpolation");

	context = hash_new();
	assert_not_null("(test sanity) hash_new must return a valid hash", context);
	if (!context) { return; }

	hash_set(context, "ref1", "this is a reference");
	hash_set(context, "name", "Clockwork");
	hash_set(context, "multi.level.fact", "MULTILEVEL");
	hash_set(context, "kernel_version", "2.6");

	size_t i;
	for (i = 0; tests[i][0]; i++) {
		string_interpolate(buf, 8192, tests[i][0], context);
		assert_str_eq("interpolate(tpl) == str", tests[i][1], buf);
	}

	hash_free(context);
}

NEW_TEST(string_interpolate_short_stroke)
{
	char buf[512]; // extra-large
	struct hash *context;

	test("STRING: Interpolation (short-stroke the buffer)");
	context = hash_new();
	hash_set(context, "ref", "1234567890abcdef");

	string_interpolate(buf, 8, "$ref is 16 characters long", context);
	assert_str_eq("interpolated value cut short", buf, "1234567");

	hash_free(context);
}

NEW_TEST(string_automatic)
{
	struct string *s = string_new(NULL, 0);

	test("STRING: Automatic strings");
	assert_auto_string(s, "");

	test("STRING: Append another string");
	assert_int_eq("string_append returns 0", string_append(s, "Hello,"), 0);
	assert_auto_string(s, "Hello,");

	test("STRING: Append a character");
	assert_int_eq("string_append1 returns 0", string_append1(s, ' '), 0);
	assert_auto_string(s, "Hello, ");

	test("STRING: Append a second string");
	assert_int_eq("string_append returns 0", string_append(s, "World!"), 0);
	assert_auto_string(s, "Hello, World!");

	string_free(s);
}

NEW_TEST(string_extension)
{
	/* Insanely low block size */
	struct string *s = string_new(NULL, 2);

	test("STRING: Buffer extension (2-byte blocks)");
	assert_auto_string(s, "");
	assert_int_eq("s->bytes is 2 (minimum buffer)", s->bytes, 2);

	assert_int_eq("Can add 1 char successfully", string_append1(s, 'a'), 0);
	assert_int_eq("s->bytes is 2 (0+1+NUL)", s->bytes, 2);

	assert_int_eq("Can add 'BBB' successfully", string_append(s, "BBB"), 0);
	assert_auto_string(s, "aBBB");
	assert_int_eq("s->bytes is 6 (0+1+3+NUL)", s->bytes, 6);

	string_free(s);
}

NEW_TEST(string_initial_value) {
	struct string *s;

	test("STRING: Initial Value");
	s = string_new("Clockwork Rocks Work!", 0);
	assert_auto_string(s, "Clockwork Rocks Work!");

	string_free(s);
}

NEW_TEST(string_free_null)
{
	struct string *s;

	test("STRING: string_free(NULL)");
	s = NULL; string_free(s);
	assert_null("string_free(NULL) doesn't segfault", s);
}

NEW_TEST(stringlist_init)
{
	struct stringlist *sl;

	sl = stringlist_new(NULL);

	test("stringlist: Initialization Routines");
	assert_not_null("stringlist_new returns a non-null pointer", sl);
	assert_int_eq("a new stringlist is empty", sl->num, 0);
	assert_int_ne("a new stringlist has extra capacity", sl->len, 0);

	stringlist_free(sl);
}

NEW_TEST(stringlist_init_with_data)
{
	struct stringlist *sl;
	char *data[33];
	char buf[32];
	size_t i;

	for (i = 0; i < 32; i++) {
		snprintf(buf, 32, "data%u", i);
		data[i] = strdup(buf);
	}
	data[32] = NULL;

	sl = stringlist_new(data);
	test("stringlist: Initialization (with data)");
	assert_int_eq("stringlist should have 32 strings", sl->num, 32);
	assert_int_gt("stringlist should have more than 32 slots", sl->len, 32);
	assert_str_eq("spot-check strings[0]", sl->strings[0], "data0");
	assert_str_eq("spot-check strings[14]", sl->strings[14], "data14");
	assert_str_eq("spot-check strings[28]", sl->strings[28], "data28");
	assert_str_eq("spot-check strings[31]", sl->strings[31], "data31");
	assert_null("strings[32] should be a NULL terminator", sl->strings[32]);

	stringlist_free(sl);
	for (i = 0; i < 32; i++) {
		free(data[i]);
	}
}

NEW_TEST(stringlist_dup)
{
	struct stringlist *orig, *dup;
	char *data[5];
	char buf[32];
	size_t i;

	for (i = 0; i < 4; i++) {
		snprintf(buf, 32, "data%u", i);
		data[i] = strdup(buf);
	}
	data[4] = NULL;

	orig = stringlist_new(data);
	dup  = stringlist_dup(orig);

	assert_int_eq("original stringlist has 4 strings", orig->num, 4);
	assert_int_eq("duplicate stringlist has 4 strings", orig->num, 4);

	assert_str_eq("original->strings[0]",  orig->strings[0], "data0");
	assert_str_eq("duplicate->strings[0]", dup->strings[0], "data0");

	assert_str_eq("original->strings[1]",  orig->strings[1], "data1");
	assert_str_eq("duplicate->strings[1]", dup->strings[1], "data1");

	assert_str_eq("original->strings[2]",  orig->strings[2], "data2");
	assert_str_eq("duplicate->strings[2]", dup->strings[2], "data2");

	assert_str_eq("original->strings[3]",  orig->strings[3], "data3");
	assert_str_eq("duplicate->strings[3]", dup->strings[3], "data3");

	stringlist_free(orig);
	stringlist_free(dup);

	for (i = 0; i < 4; i++) {
		free(data[i]);
	}
}

NEW_TEST(stringlist_basic_add_remove_search)
{
	struct stringlist *sl;

	sl = stringlist_new(NULL);

	test("stringlist: Basic Add/Remove/Search");
	assert_not_null("have a non-null stringlist", sl);

	assert_int_eq("add 'first string' to string list", stringlist_add(sl, "first string"), 0);
	assert_int_eq("add 'second string' to string list", stringlist_add(sl, "second string"), 0);

	assert_stringlist(sl, "sl", 2, "first string", "second string");

	assert_int_eq("search for 'first string' succeeds", stringlist_search(sl, "first string"), 0);
	assert_int_eq("search for 'second string' succeeds", stringlist_search(sl, "second string"), 0);
	assert_int_ne("search for 'no such string' fails", stringlist_search(sl, "no such string"), 0);

	assert_int_eq("removal of 'first string' succeeds", stringlist_remove(sl, "first string"), 0);
	assert_stringlist(sl, "sl", 1, "second string");

	assert_int_eq("search for 'second string' still succeeds", stringlist_search(sl, "second string"), 0);

	stringlist_free(sl);
}

NEW_TEST(stringlist_add_all)
{
	struct stringlist *sl1, *sl2;

	sl1 = setup_list("lorem", "ipsum", "dolor", NULL);
	sl2 = setup_list("sit", "amet", "consectetur", NULL);

	test("stringlist: Combination of string lists");
	assert_stringlist(sl1, "pre-combine sl1", 3, "lorem", "ipsum", "dolor");
	assert_stringlist(sl2, "pre-combine sl2", 3, "sit", "amet", "consectetur");

	assert_int_eq("combine sl1 and sl2 successfully", stringlist_add_all(sl1, sl2), 0);
	assert_stringlist(sl1, "post-combine sl1", 6, "lorem", "ipsum", "dolor", "sit", "amet", "consectetur");
	assert_stringlist(sl2, "post-combine sl2", 3, "sit", "amet", "consectetur");

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_add_all_with_expansion)
{
	struct stringlist *sl1, *sl2;
	int total = 0;
	int capacity = 0;

	test("stringlist: add_all with capacity expansion");
	sl1 = setup_list("a", "b", "c", "d", "e", "f", "g",
	                 "h", "i", "j", "k", "l", "m", "n", NULL);
	assert_stringlist(sl1, "sl1", 14,
		"a", "b", "c", "d", "e", "f", "g",
		"h", "i", "j", "k", "l", "m", "n");

	sl2 = setup_list("o", "p", "q", NULL);
	assert_stringlist(sl2, "sl2", 3, "o", "p", "q");

	total = sl1->num + sl2->num;
	capacity = sl1->len;
	assert_int_gt("combined len should be > capacity", total, capacity);
	assert_int_eq("add_all succeeds", stringlist_add_all(sl1, sl2), 0);
	assert_stringlist(sl1, "combined", 17,
		"a", "b", "c", "d", "e", "f", "g",
		"h", "i", "j", "k", "l", "m", "n",
		"o", "p", "q");
	assert_int_ge("new capacity >= old capacity", sl1->len, capacity);

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_remove_all)
{
	struct stringlist *sl1, *sl2;

	sl1 = setup_list("lorem", "ipsum", "dolor", NULL);
	sl2 = setup_list("ipsum", "dolor", "sit", NULL);

	test("stringlist: Removal of string lists");
	assert_stringlist(sl1, "pre-remove sl1", 3, "lorem", "ipsum", "dolor");
	assert_stringlist(sl2, "pre-remove sl2", 3, "ipsum", "dolor", "sit");

	assert_int_eq("remove sl2 from sl1 successfully", stringlist_remove_all(sl1, sl2), 0);
	assert_stringlist(sl1, "post-remove sl1", 1, "lorem");
	assert_stringlist(sl2, "post-remove sl2", 3, "ipsum", "dolor", "sit");

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_expansion)
{
	struct stringlist *sl;
	size_t max, i;
	char buf[32];
	char assertion[128];

	sl = stringlist_new(NULL);
	test("stringlist: Automatic List Expansion");
	assert_not_null("have a non-null stringlist", sl);

	max = sl->len;
	for (i = 0; i < max * 2 + 1; i++) {
		snprintf(buf, 32, "string%u", i);
		snprintf(assertion, 128, "add 'string%u' to stringlist", i);
		assert_int_eq(assertion, stringlist_add(sl, buf), 0);

		snprintf(assertion, 128, "stringlist should have %u strings", i + 1);
		assert_int_eq(assertion, sl->num, i + 1);
	}

	assert_int_ne("sl->len should have changed", sl->len, max);

	stringlist_free(sl);
}

NEW_TEST(stringlist_remove_nonexistent)
{
	const char *tomato = "tomato";

	struct stringlist *sl;
	sl = setup_list("apple", "pear", "banana", NULL);

	test("stringlist: Remove non-existent");
	assert_stringlist(sl, "pre-remove sl", 3, "apple", "pear", "banana");
	assert_int_ne("removal of 'tomato' from stringlist fails", stringlist_remove(sl, tomato), 0);
	assert_stringlist(sl, "pre-remove sl", 3, "apple", "pear", "banana");

	stringlist_free(sl);
}

NEW_TEST(stringlist_qsort)
{
	const char *a = "alice";
	const char *b = "bob";
	const char *c = "candace";

	struct stringlist *sl;
	test("stringlist: Sorting");
	sl = setup_list(b, c, a, NULL);
	assert_stringlist(sl, "pre-sort sl", 3, "bob", "candace", "alice");

	stringlist_sort(sl, STRINGLIST_SORT_ASC);
	assert_stringlist(sl, "post-sort ASC sl", 3, "alice", "bob", "candace");

	stringlist_sort(sl, STRINGLIST_SORT_DESC);
	assert_stringlist(sl, "post-sort DESC sl", 3, "candace", "bob", "alice");

	stringlist_free(sl);

	test("stringlist: Sorting 1-item string list");
	sl = setup_list(b, NULL);
	assert_stringlist(sl, "pre-sort sl", 1, "bob");

	stringlist_sort(sl, STRINGLIST_SORT_ASC);
	assert_stringlist(sl, "post-sort ASC sl", 1, "bob");
	stringlist_sort(sl, STRINGLIST_SORT_DESC);
	assert_stringlist(sl, "post-sort DESC sl", 1, "bob");

	stringlist_free(sl);
}

NEW_TEST(stringlist_uniq)
{
	const char *a = "alice";
	const char *b = "bob";
	const char *c = "candace";

	struct stringlist *sl;

	test("stringlist: Uniq");
	sl = setup_list(b, c, a, b, b, c, NULL);
	assert_stringlist(sl, "pre-uniq sl", 6, "bob", "candace", "alice", "bob", "bob", "candace");

	stringlist_uniq(sl);
	assert_stringlist(sl, "post-uniq sl", 3, "alice", "bob", "candace");

	stringlist_free(sl);
}

NEW_TEST(stringlist_uniq_already)
{
	const char *a = "alice";
	const char *b = "bob";
	const char *c = "candace";

	struct stringlist *sl;

	test("stringlist: Uniq (no changes needed)");
	sl = setup_list(b, c, a, NULL);
	assert_stringlist(sl, "pre-uniq sl", 3, "bob", "candace", "alice");

	stringlist_uniq(sl);
	assert_stringlist(sl, "post-uniq sl", 3, "alice", "bob", "candace");

	stringlist_free(sl);
}

NEW_TEST(stringlist_diff)
{
	const char *a = "alice";
	const char *b = "bob";
	const char *c = "candace";
	const char *d = "david";
	const char *e = "ethan";

	struct stringlist *sl1, *sl2;

	sl1 = setup_list(b, c, a, NULL);
	sl2 = setup_list(b, c, a, NULL);

	test("stringlist: Diff");
	assert_int_ne("sl1 and sl2 are equivalent", stringlist_diff(sl1, sl2), 0);

	stringlist_add(sl2, d);
	assert_int_eq("after addition of 'david' to sl2, sl1 and sl2 differ", stringlist_diff(sl1, sl2), 0);

	stringlist_add(sl1, e);
	assert_int_eq("after addition of 'ethan' to sl1, sl1 and sl2 differ", stringlist_diff(sl1, sl2), 0);

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_diff_non_uniq)
{
	const char *s1 = "string1";
	const char *s2 = "string2";
	const char *s3 = "string3";

	struct stringlist *sl1, *sl2;

	sl1 = setup_list(s1, s2, s2, NULL);
	sl2 = setup_list(s1, s2, s3, NULL);

	test("stringlist: Diff (non-unique entries)");
	assert_int_eq("sl1 and sl2 are different (forward)", stringlist_diff(sl1, sl2), 0);
	assert_int_eq("sl2 and sl1 are different (reverse)", stringlist_diff(sl2, sl1), 0);

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_diff_single_string)
{
	struct stringlist *sl1, *sl2;
	const char *a = "string a";
	const char *b = "string b";

	sl1 = setup_list(a, NULL);
	sl2 = setup_list(b, NULL);

	test("stringlist: Diff (single string)");
	assert_int_eq("sl1 and sl2 are different (forward)", stringlist_diff(sl1, sl2), 0);
	assert_int_eq("sl2 and sl1 are different (reverse)", stringlist_diff(sl2, sl1), 0);

	stringlist_free(sl1);
	stringlist_free(sl2);
}

NEW_TEST(stringlist_join)
{
	char *joined = NULL;
	struct stringlist *list = setup_list("item1","item2","item3", NULL);
	struct stringlist *empty = stringlist_new(NULL);

	test("stringlist: Join stringlist with a delimiter");
	free(joined);
	joined = stringlist_join(list, "::");
	assert_str_eq("Check joined string with '::' delimiter", "item1::item2::item3", joined);

	free(joined);
	joined = stringlist_join(list, "");
	assert_str_eq("Check joined string with '' delimiter", "item1item2item3", joined);

	free(joined);
	joined = stringlist_join(list, "\n");
	assert_str_eq("Check joined string with '' delimiter", "item1\nitem2\nitem3", joined);

	free(joined);
	joined = stringlist_join(empty, "!!");
	assert_str_eq("Check empty join", "", joined);

	free(joined);
	stringlist_free(list);
	stringlist_free(empty);
}

NEW_TEST(stringlist_split)
{
	struct stringlist *list;
	char *joined = "apple--mango--pear";
	char *single = "loganberry";
	char *nulls  = "a space    separated  list";

	test("stringlist: Split strings with delimiters");
	list = stringlist_split(joined, strlen(joined), "--", 0);
	assert_stringlist(list, "split by '--'", 3, "apple", "mango","pear");
	stringlist_free(list);

	list = stringlist_split(joined, strlen(joined), "-", 0);
	assert_stringlist(list, "split by '-'", 5, "apple", "", "mango", "", "pear");
	stringlist_free(list);

	list = stringlist_split(single, strlen(single), "/", 0);
	assert_stringlist(list, "split single-entry list string", 1, "loganberry");
	stringlist_free(list);

	list = stringlist_split(single, strlen(""), "/", 0);
	assert_stringlist(list, "split empty list string", 0);
	stringlist_free(list);

	list = stringlist_split(nulls, strlen(nulls), " ", SPLIT_GREEDY);
	assert_stringlist(list, "greedy split",
		4, "a", "space", "separated", "list");
	stringlist_free(list);
}

NEW_TEST(stringlist_free_null)
{
	struct stringlist *sl;

	test("stringlist: stringlist_free(NULL)");
	sl = NULL; stringlist_free(sl);
	assert_null("stringist_free(NULL) doesn't segfault", sl);
}

NEW_TEST(auto_string)
{
	char *s;
	char buf[129];

	test("MEM: string() - normal use");
	s = string("%s: %u 0x%08x", "Clockwork test build", 1025, 1025);
	assert_not_null("string() returns valid pointer", s);
	assert_str_eq("string() formats properly", "Clockwork test build: 1025 0x00000401", s);
	free(s);

	test("MEM: string() - large buffer required");
	memset(buf, 'x', 128); buf[128] = '\0';
	assert_int_eq("buffer should be 128 octets long", 128, strlen(buf));
	s = string("%sA%sB%sC%sD", buf, buf, buf, buf);
	assert_int_eq("s should be 4+(128*4) octets long", 4+(128*4), strlen(s));
	free(s);

}


NEW_SUITE(string)
{
	RUN_TEST(string_interpolation);
	RUN_TEST(string_interpolate_short_stroke);
	RUN_TEST(string_automatic);
	RUN_TEST(string_extension);
	RUN_TEST(string_initial_value);
	RUN_TEST(string_free_null);


	RUN_TEST(stringlist_init);
	RUN_TEST(stringlist_init_with_data);
	RUN_TEST(stringlist_dup);
	RUN_TEST(stringlist_basic_add_remove_search);
	RUN_TEST(stringlist_add_all);
	RUN_TEST(stringlist_add_all_with_expansion);
	RUN_TEST(stringlist_remove_all);
	RUN_TEST(stringlist_expansion);
	RUN_TEST(stringlist_remove_nonexistent);
	RUN_TEST(stringlist_qsort);
	RUN_TEST(stringlist_uniq);
	RUN_TEST(stringlist_uniq_already);

	RUN_TEST(stringlist_diff);
	RUN_TEST(stringlist_diff_non_uniq);
	RUN_TEST(stringlist_diff_single_string);

	RUN_TEST(stringlist_join);
	RUN_TEST(stringlist_split);

	RUN_TEST(stringlist_free_null);

	RUN_TEST(auto_string);
}
