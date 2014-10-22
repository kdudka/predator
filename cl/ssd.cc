/*
 * Copyright (C) 2009 Kamil Dudka <kdudka@redhat.com>
 *
 * This file is part of ssd (Standard Stream Decorators)
 *
 * ssd is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * ssd is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ssd.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ssd.h"
#include <unistd.h>

#ifndef HAVE_ISATTY
#warning macro HAVE_ISATTY is not set, defaulting to 1
#   define HAVE_ISATTY 1
#endif

#if HAVE_ISATTY
#include <fstream>
#include <sstream>
namespace {

int readTracerPid()
{
    int tracerPid = 0;

    std::fstream inStr("/proc/self/status", std::ios::in);
    if (inStr) {
        // find the corresponding line in the table
        std::string token;
        while (inStr >> token && (token != "TracerPid:"))
            ;

        inStr >> tracerPid;
        inStr.close();
    }

    return tracerPid;
}

bool cgdbAttached()
{
    const int tracerPid = readTracerPid();
    if (!tracerPid)
        // we are not being traced
        return false;

    // read the command-line arguments of the process that traces us
    std::ostringstream ss;
    ss << "/proc/" << tracerPid << "/cmdline";
    std::fstream inStr(ss.str().c_str(), std::ios::in);
    if (!inStr)
        return false;

    // match the pattern that cgdb uses to run gdb
    const char pattern[] = "gdb\0--nw";
    for (unsigned i = 0U; i < sizeof pattern; ++i) {
        char c;
        if (inStr >> c && (c == pattern[i]))
            continue;

        // pattern mismatch
        inStr.close();
        return false;
    }

    inStr.close();
    return /* found */ true;
}
} // namespace
#endif

namespace ssd {

// /////////////////////////////////////////////////////////////////////////////
// ColorConsole implementation
bool ColorConsole::enabled_ = false;

void ColorConsole::enable(bool value)
{
    enabled_ = value;
}

bool ColorConsole::isEnabled()
{
    return enabled_;
}

void ColorConsole::enableForTerm(int fd)
{
#if HAVE_ISATTY
    enabled_ = isatty(fd) && !cgdbAttached();
#endif
}

void ColorConsole::enableIfCoutIsTerm()
{
    ColorConsole::enableForTerm(STDOUT_FILENO);
}

void ColorConsole::enableIfCerrIsTerm()
{
    ColorConsole::enableForTerm(STDERR_FILENO);
}

void ColorConsole::enableForTerm()
{
    ColorConsole::enableIfCoutIsTerm();
    if (enabled_)
        ColorConsole::enableIfCerrIsTerm();
}

// /////////////////////////////////////////////////////////////////////////////
// Color implementation
std::ostream& operator<< (std::ostream &stream, const Color &color)
{
    static const char ESC = '\033';
    if (!ColorConsole::isEnabled())
        return stream;

    stream << ESC;
    switch (color.color) {
        case C_NO_COLOR:     stream << "[0m";   break;
        case C_BLUE:         stream << "[0;34m"; break;
        case C_GREEN:        stream << "[0;32m"; break;
        case C_CYAN:         stream << "[0;36m"; break;
        case C_RED:          stream << "[0;31m"; break;
        case C_PURPLE:       stream << "[0;35m"; break;
        case C_BROWN:        stream << "[0;33m"; break;
        case C_LIGHT_GRAY:   stream << "[0;37m"; break;
        case C_DARK_GRAY:    stream << "[1;30m"; break;
        case C_LIGHT_BLUE:   stream << "[1;34m"; break;
        case C_LIGHT_GREEN:  stream << "[1;32m"; break;
        case C_LIGHT_CYAN:   stream << "[1;36m"; break;
        case C_LIGHT_RED:    stream << "[1;31m"; break;
        case C_LIGHT_PURPLE: stream << "[1;35m"; break;
        case C_YELLOW:       stream << "[1;33m"; break;
        case C_WHITE:        stream << "[1;37m"; break;
    }

    return stream;
}

// /////////////////////////////////////////////////////////////////////////////
// Colorize implementation
Colorize::Colorize(std::ostream &stream, EColor color):
    stream_(stream)
{
    stream_ << Color(color);
}

Colorize::~Colorize()
{
    stream_ << Color(C_NO_COLOR);
}

std::ostream& Colorize::stream()
{
    return stream_;
}

} // namespace ssd
