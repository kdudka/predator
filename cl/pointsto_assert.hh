/*
 * Copyright (C) 2012 Pavel Raiskup <pavel@raiskup.cz>
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

#ifndef H_GUARD_PT_ASSERT_HH
#define H_GUARD_PT_ASSERT_HH

enum PTAssertType {
    /// ask if exists PT-relation between two operands among all functions
    PT_ASSERT_MAY_POINT = 0,
    PT_ASSERT_MAY_NOT_POINT,

    /// ask if concrete variable may be pointed among all functions
    PT_ASSERT_MAY_BE_POINTED,
    PT_ASSERT_MAY_NOT_BE_POINTED,

    /// ask if exists PT-relation between two operands in concrete function
    PT_ASSERT_MAY_POINT_LOCAL,
    PT_ASSERT_MAY_NOT_POINT_LOCAL,

    /// ask whether the variable name exists in concrete function
    PT_ASSERT_EXISTS_LOCAL,
    PT_ASSERT_EXISTS_LOCAL_NOT,

    /// setup flag that we expect the particular PT-build will fail
    PT_ASSERT_BUILD_FAIL
};

#endif /* H_GUARD_PT_ASSERT_H */
