/*
 * Copyright (C) 2013 Kamil Dudka <kdudka@redhat.com>
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

#ifndef H_GUARD_GLCONF_H
#define H_GUARD_GLCONF_H

#include "config.h"

#include <string>

namespace FixedPoint {
    class StateByInsn;
}

namespace GlConf {

struct Options {
    bool trackUninit;       ///< enable/disable @b track_uninit @b mode
    bool oomSimulation;     ///< enable/disable @b oom @b simulation mode
    bool memLeakIsError;    ///< treat memory leak as an error
    bool skipUserPlots;     ///< ignore all ___sl_plot*() calls
    int errorRecoveryMode;  ///< @copydoc config.h::SE_ERROR_RECOVERY_MODE
    bool verifierErrorIsError; ///< treat reaching __VERIFIER_error() as error
    std::string errLabel;   ///< if not empty, treat reaching the label as error
    bool allowCyclicTraceGraph; ///< create node with two parents on entailment
    int allowThreeWayJoin;  ///< @copydoc config.h::SE_ALLOW_THREE_WAY_JOIN
    bool forbidHeapReplace; ///< @copydoc config.h::SE_FORBID_HEAP_REPLACE
    int intArithmeticLimit; ///< @copydoc config.h::SE_INT_ARITHMETIC_LIMIT
    int joinOnLoopEdgesOnly;///< @copydoc config.h::SE_JOIN_ON_LOOP_EDGES_ONLY
    int stateLiveOrdering;  ///< @copydoc config.h::SE_STATE_ON_THE_FLY_ORDERING
    bool detectContainers;  ///< detect containers and operations over them
    FixedPoint::StateByInsn *fixedPoint;  ///< fixed-point plotter (0 if unused)

    Options();
};

extern Options data;

void loadConfigString(const std::string &);

} // namespace GlConf

#endif /* H_GUARD_GLCONF_H */
