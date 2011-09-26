/******************************************************************************* 
 *  -- Cellframework -  Open framework to abstract the common tasks related to
 *                      PS3 application development.
 *
 *  Copyright (C) 2010
 *       Hans-Kristian Arntzen
 *       Stephen A. Damm
 *       Daniel De Matteis
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/





/*
 * ILogger.h
 *
 *  Created on: Nov 1, 2010
 *      Author: halsafar
 */
#ifndef ILOGGER_H_
#define ILOGGER_H_

#include <types.h>
#include <sys/types.h>

class ILogger
{
public:
	virtual ~ILogger() {}

	virtual int32_t init() = 0;
	virtual void printf(const char* fmt, ...) = 0;
	virtual void flush() = 0;
	virtual void close() = 0;
};


#endif
