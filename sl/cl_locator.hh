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

#ifndef H_GUARD_CL_LOCATOR_H
#define H_GUARD_CL_LOCATOR_H

/**
 * @file cl_locator.hh
 * constructor createClLocator() of the @b "locator" code listener
 */

class ICodeListener;

/**
 * create "locator" ICodeListener implementation
 *
 * There is not configuration for now.
 *
 * @todo proper documentation of the "locator" code listener
 */
ICodeListener* createClLocator(const char *);

#endif /* H_GUARD_CL_LOCATOR_H */
