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

#ifndef H_GUARD_CL_FACTORY_H
#define H_GUARD_CL_FACTORY_H

/**
 * @file cl_factory.hh
 * ClFactory - factory for ICodeListener objects
 */

class ICodeListener;

/**
 * factory responsible for creating ICodeListener objects
 *
 * see config.h::DEBUG_CL_FACTORY
 *
 * @todo proper documentation for ClFactory config_string
 */
class ClFactory {
    public:
        ClFactory();
        ~ClFactory();

        /**
         * create desired ICodeListener object
         * @copydoc cl_code_listener_create()
         */
        ICodeListener* create(const char *config_string);

    private:
        struct Private;
        Private *d;
};

#endif /* H_GUARD_CL_FACTORY_H */
