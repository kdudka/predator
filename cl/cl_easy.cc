/*
 * Copyright (C) 2010 Kamil Dudka <kdudka@redhat.com>
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

#include "config.h"
#include "cl_easy.hh"

#include <cl/cl_msg.hh>
#include <cl/easy.hh>

#include "cl_storage.hh"

#include <string>

class ClEasy: public ClStorageBuilder {
    public:
        ClEasy(const char *configString):
            configString_(configString)
        {
            CL_DEBUG("ClEasy initialized: \"" << configString << "\"");
        }

    protected:
        virtual void run(CodeStorage::Storage &stor) {
            CL_DEBUG("ClEasy is calling peer...");
            clEasyRun(stor, configString_.c_str());
        }

    private:
        std::string configString_;
};


// /////////////////////////////////////////////////////////////////////////////
// interface, see cl_easy.hh for details
ICodeListener* createClEasy(const char *configString) {
    return new ClEasy(configString);
}
