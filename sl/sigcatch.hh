/*
 * Copyright (C) 2011 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_SIGCATCH_H
#define H_GUARD_SIGCATCH_H

// import signal numbers as part of the sigcatch.hh API
#include <signal.h>

class SignalCatcher {
    public:
        static bool install(int signum);
        static bool cleanup();

        static bool caught(int signum);
        static bool caught(int *signum = 0);

    private:
        /// library class
        SignalCatcher();
};

#endif /* H_GUARD_SIGCATCH_H */
