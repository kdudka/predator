/*
 * Copyright (C) 2009 Kamil Dudka <kamil@dudka.cz>
 *
 * This file is part of ssd (Standard Stream Decorators).
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

#ifndef SSD_H
#define SSD_H

#include <iostream>

/**
 * Colorize standard output stream [dirty hack]
 * @param STREAM Stream to colorize
 * @param COLOR A color to colorize stream with.
 */
#define SSD_COLORIZE(STREAM, COLOR) (ssd::Colorize(STREAM, COLOR).stream())

/**
 * Colorize standard output stream [dirty hack]
 * @param COLOR A color to colorize stream with.
 * @param CONTENT Content to colorize.
 */
#define SSD_INLINE_COLOR(COLOR, CONTENT) ssd::Color(COLOR) << CONTENT \
    << Color(C_NO_COLOR)

namespace ssd {

    /**
     * Console output colors enumeration. Zero means default color.
     */
    enum EColor {
        C_NO_COLOR = 0,
        C_BLUE,
        C_GREEN,
        C_CYAN,
        C_RED,
        C_PURPLE,
        C_BROWN,
        C_LIGHT_GRAY,
        C_DARK_GRAY,
        C_LIGHT_BLUE,
        C_LIGHT_GREEN,
        C_LIGHT_CYAN,
        C_LIGHT_RED,
        C_LIGHT_PURPLE,
        C_YELLOW,
        C_WHITE
    };

    /**
     * Library class to turn on/off console color output.
     * @attention This class is @b not thread-safe.
     */
    class ColorConsole {
        public:
            /**
             * Enable/disable console color output.
             * @param If true, enable console color output, disable otherwise.
             */
            static void enable(bool value = true);

            /**
             * Return true if console color output is enabled.
             */
            static bool isEnabled();

            /**
             * Enable console color output if std::cout and std::cerr are
             * connected to terminal.
             * @note This method is implemented only if (HAVE_ISATTY == 1).
             */
            static void enableForTerm();

            /**
             * Enable console color output if given file descriptor is connected
             * to terminal.  @note This method is implemented only if
             * (HAVE_ISATTY == 1).
             */
            static void enableForTerm(int fd);

            /**
             * Enable console color output if std::cout is connected
             * to terminal.
             * @note This method is implemented only if (HAVE_ISATTY == 1).
             */
            static void enableIfCoutIsTerm();

            /**
             * Enable console color output if std::cerr is connected
             * to terminal.
             * @note This method is implemented only if (HAVE_ISATTY == 1).
             */
            static void enableIfCerrIsTerm();

        private:
            ColorConsole();
            static bool enabled_;
    };

    /**
     * Color manipulator for standard output stream.
     */
    struct Color {
        EColor color;

        /**
         * @param color_ Color to colorize stream with.
         */
        Color(EColor color_):
            color(color_)
        {
        }
    };

    /**
     * Colorize standard output stream using Color manipulator.
     */
    std::ostream& operator<< (std::ostream &, const Color &);

    /**
     * Temporarily colorize standard output stream.
     * @attention @b dirty @b hack
     * @note used by macro SSD_COLORIZE
     */
    class Colorize {
        public:
            Colorize(std::ostream&, EColor);
            ~Colorize();
            std::ostream& stream();

        private:
            Colorize(const Colorize &);
            Colorize& operator= (const Colorize &);

            std::ostream &stream_;
    };
}

#endif /* SSD_H */
