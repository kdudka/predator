#define BOOST_AUTO_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_NO_MAIN

#include "dll.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/debug.hpp>

BOOST_AUTO_TEST_SUITE(test_dll)
BOOST_AUTO_TEST_CASE(basic_test) {
    // BOOST_CHECK(v != 0);
    // BOOST_CHECK_EQUAL(cnt, 12);
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
