/*
 * Copyright (C) 2012 Jiri Simacek
 *
 * This file is part of forester.
 *
 * forester is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * forester is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with forester.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file config.h
 * various compile-time options
 */

#ifndef CONFIG_H
#define CONFIG_H

/**
 * set reference count tracking treshold (default is 2)
 */
#define FA_REF_CNT_TRESHOLD					2

/**
 * set real reference count tracking treshold (default is 1)
 */
#define FA_REAL_REF_CNT_TRESHOLD			1

/**
 * allow folding of nested structures (default is 1)
 */
#define FA_ALLOW_FOLDING					1

/**
 * overapproximate when folding (default is 0)
 */
#define FA_BOX_APPROXIMATION				1

/**
 * should we restart evry time a new box is encountered (default is 1)
 */
#define FA_RESTART_AFTER_BOX_DISCOVERY		(1 + FA_BOX_APPROXIMATION)

/**
 * enable fusion when computing abstraction (default is 1)
 */
#define FA_FUSION_ENABLED					1

#endif /* CONFIG_H */
