/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of predator.
 *
 * predator is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * predator is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with predator.  If not, see <http://www.gnu.org/licenses/>.
 */

#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include "dll.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/debug.hpp>

#include <iostream>

#include <signal.h>

inline void chk_msg(const char *msg) {
    std::cerr << "--- checking: " << msg << std::endl;
}

inline void chk_dll_size(dll_t *list, size_t size)
{
    BOOST_CHECK_EQUAL(size, dll_size(list));
    if (size) {
        BOOST_CHECK_NE(DLL_NULL, dll_beg(list));
        BOOST_CHECK_NE(DLL_NULL, dll_end(list));
    } else {
        BOOST_CHECK(dll_empty(list));
        BOOST_CHECK_EQUAL(DLL_NULL, dll_beg(list));
        BOOST_CHECK_EQUAL(DLL_NULL, dll_end(list));
    }

    if (1 < size)
        BOOST_CHECK_NE(dll_beg(list), dll_end(list));

    // traverse forward
    size_t i = 0;
    dll_item_t *item;
    for (item = dll_beg(list); item; item = dll_next(item))
        ++i;
    BOOST_CHECK_EQUAL(size, i);

    // traverse backward
    i = 0;
    for (item = dll_end(list); item; item = dll_prev(item))
        ++i;
    BOOST_CHECK_EQUAL(size, i);
}

void dll_die(const char *msg)
{
    BOOST_CHECK_MESSAGE(false, msg);
}

BOOST_AUTO_TEST_SUITE(test_dll)
BOOST_AUTO_TEST_CASE(basic_test) {
    dll_data_t data;
    // TODO: initialize, then modify and also check the data

    chk_msg("dll_init");
    dll_t list;
    dll_init(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_back");
    dll_push_back(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_push_front");
    dll_push_front(&list, &data);
    chk_dll_size(&list, 2);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_front");
    dll_push_front(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_pop_back");
    dll_pop_back(&list, &data);
    chk_dll_size(&list, 0);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_destroy");
    dll_destroy(&list);
    chk_dll_size(&list, 0);

    chk_msg("dll_push_back");
    dll_item *item = dll_push_back(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_insert_before");
    item = dll_insert_before(&list, item, &data);
    chk_dll_size(&list, 2);

    chk_msg("dll_insert_after");
    item = dll_insert_after(&list, item, &data);
    chk_dll_size(&list, 3);

    chk_msg("dll_remove");
    dll_remove(&list, dll_next(dll_prev(dll_prev(dll_end(&list)))));
    chk_dll_size(&list, 2);

    chk_msg("dll_pop_front");
    dll_pop_front(&list, &data);
    chk_dll_size(&list, 1);

    chk_msg("dll_remove");
    dll_remove(&list, dll_beg(&list));
    chk_dll_size(&list, 0);
}
BOOST_AUTO_TEST_SUITE_END()

int main(int argc, char* argv[])
{
    using namespace boost::debug;
    using namespace boost::unit_test;

    // FIXME: not supported by gcc?
    detect_memory_leaks(true);

    return unit_test_main(&init_unit_test, argc, argv);
}
