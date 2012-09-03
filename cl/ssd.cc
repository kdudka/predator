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

void ColorConsole::enableForTerm()
{
#if HAVE_ISATTY
    enabled_ = isatty(STDOUT_FILENO) && isatty(STDERR_FILENO);
#endif
}

void ColorConsole::enableForTerm(int fd)
{
#if HAVE_ISATTY
    enabled_ = isatty(fd);
#endif
}

void ColorConsole::enableIfCoutIsTerm()
{
#if HAVE_ISATTY
    enabled_ = isatty(STDOUT_FILENO);
#endif
}

void ColorConsole::enableIfCerrIsTerm()
{
#if HAVE_ISATTY
    enabled_ = isatty(STDERR_FILENO);
#endif
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
