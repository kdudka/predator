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


#ifndef H_GUARD_SYM_DEBUG_H
#define H_GUARD_SYM_DEBUG_H

#include "symplot.hh"

#include <iomanip>
#include <string>
#include <sstream>

#define LOCAL_DEBUG_PLOTTER(name, enabled_by_default)                          \
    static bool __ldp_enabled_##name = static_cast<bool>(enabled_by_default);  \
    static int __ldp_cnt_total_##name = -1;                                    \
    static int __ldp_cnt_steps_##name;                                         \
    static std::string __ldp_plot_name_##name;                                 \
                                                                               \
    void __ldp_enable_##name() {                                               \
        ::__ldp_enabled_##name = true;                                         \
    }                                                                          \
                                                                               \
    void __ldp_disable_##name() {                                              \
        ::__ldp_enabled_##name = false;                                        \
    }                                                                          \
                                                                               \
    void __ldp_init_##name(std::string plot_name) {                            \
        ++::__ldp_cnt_total_##name;                                            \
        ::__ldp_cnt_steps_##name = 0;                                          \
        ::__ldp_plot_name_##name = plot_name;                                  \
    }                                                                          \
                                                                               \
    void __ldp_plot_##name(const SymHeap &sh, std::string *pName = 0) {        \
        if (!::__ldp_enabled_##name)                                           \
            return;                                                            \
                                                                               \
        std::ostringstream str;                                                \
                                                                               \
        str << #name "-" << std::setfill('0') << std::setw(4)                  \
            << ::__ldp_cnt_total_##name                                        \
            << "-" << ::__ldp_plot_name_##name                                 \
            << "-" << std::setfill('0') << std::setw(4)                        \
            << (::__ldp_cnt_steps_##name++);                                   \
                                                                               \
       plotHeap(sh, str.str().c_str(), /* loc */ 0, pName);                    \
    }                                                                          \

#define LDP_INIT(plotter, plot_name) __ldp_init_##plotter(plot_name)
#define LDP_PLOT(plotter, sh)        __ldp_plot_##plotter(sh)
#define LDP_PLOTN(plotter, sh, pName)__ldp_plot_##plotter(sh, pName)
#define LDP_ENABLE(plotter)          __ldp_enable_##plotter()
#define LDP_DISABLE(plotter)         __ldp_disable_##plotter()

#endif /* H_GUARD_SYM_DEBUG_H */
