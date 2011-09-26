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
 * Logger.h
 *
 *  Created on: Nov 1, 2010
 *      Author: halsafar
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <memory>
#include <iostream>

#include "ILogger.h"
#include "NetLogger.h"


// LOGGER FACTORY, ONE INSTANCE
class Logger
{
public:
	static ILogger& GetLogger();
private:
	static std::auto_ptr<ILogger> _netLogger;
};

// Defines used for completely abstract use of the logger subsystem

#ifdef CELL_DEBUG
	#define LOG_INIT() Logger::GetLogger().init()
#else
	#define LOG_INIT()
#endif


#ifdef CELL_DEBUG
	#define LOG_CLOSE() Logger::GetLogger().close()
#else
	#define LOG_CLOSE()
#endif


#ifdef CELL_DEBUG
	#define LOG(fmt, args...) Logger::GetLogger().printf(fmt, ##args)
#else
	#define LOG(fmt, args...)
#endif

#ifdef CELL_DEBUG
	#define LOG_DBG(fmt, args...) Logger::GetLogger().printf("%s:%d:"fmt, __FILE__, __LINE__, ##args)
#else
	#define LOG_DBG(fmt, args...)
#endif

#ifdef CELL_DEBUG
	#define LOG_ERR(fmt, args...) Logger::GetLogger().printf("%s:%d:"fmt, __FILE__, __LINE__, ##args)
#else
	#define LOG_ERR(fmt, args...)
#endif

#ifdef CELL_DEBUG
	#define LOG_WRN(fmt, args...) Logger::GetLogger().printf("%s:%d:"fmt, __FILE__, __LINE__, ##args)
#else
	#define LOG_WRN(fmt, args...)
#endif

#endif /* LOGGER_H_ */
