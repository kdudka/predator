// dummy implementation for internals
#ifndef H_GUARD_PT_TEST_H
#define H_GUARD_PT_TEST_H

#include "pointsto_assert.hh"

#define ___cl_pt_points_loc_y(varNameA, varNameB)                           \
    PT_ASSERT(PT_ASSERT_MAY_POINT_LOCAL, varNameA, varNameB)
#define ___cl_pt_points_loc_n(varNameA, varNameB)                           \
    PT_ASSERT(PT_ASSERT_MAY_NOT_POINT_LOCAL, varNameA, varNameB)
#define ___cl_pt_points_glob_y(varA, varB)                                  \
    PT_ASSERT(PT_ASSERT_MAY_POINT, &varA, &varB);
#define ___cl_pt_points_glob_n(varA, varB)                                  \
    PT_ASSERT(PT_ASSERT_MAY_NOT_POINT, &varA, &varB);
#define ___cl_pt_is_pointed_y(var)                                          \
    PT_ASSERT(PT_ASSERT_MAY_BE_POINTED, &var);
#define ___cl_pt_is_pointed_n(var)                                          \
    PT_ASSERT(PT_ASSERT_MAY_NOT_BE_POINTED, &var);

// check if the variable does exist in points-to graph of concrete function
#define ___cl_pt_exists_local_y(varName)                                    \
    PT_ASSERT(PT_ASSERT_EXISTS_LOCAL, varName)
#define ___cl_pt_exists_local_n(varName)                                    \
    PT_ASSERT(PT_ASSERT_EXISTS_LOCAL_NOT, varName)

// call this when you expect that the construction of points-to graph will fail
// (we implicitly expect that the building will not fail)
#define ___cl_pt_build_fail() \
    PT_ASSERT(PT_ASSERT_BUILD_FAIL)


void PT_ASSERT(enum PTAssertType, ...);

#endif /* H_GUARD_PT_TEST_H */
