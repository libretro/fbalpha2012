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
 * NetLogger.cpp
 *
 *	inspired by: http://github.com/lousyphreak/scummvm-ps3/blob/ps3/backends/platform/ps3/netdbg/net.cpp
 *
 *  Created on: Nov 1, 2010
 *      Author: halsafar
 */
#include <types.h>
#include "NetLogger.h"
#include "../network/TCPSocket.hpp"
#include <string.h>


NetLogger::NetLogger(const char *host, uint16_t port)
{
   sock = new Network::TCPSocket(host, port);
}

NetLogger::~NetLogger()
{
   delete sock;
}

void NetLogger::printf(const char* fmt, ...)
{
	va_list args;
	va_start(args,fmt);
	vsnprintf(_sendbuf,4000,fmt, args);
	va_end(args);
   sock->send(_sendbuf, strlen(_sendbuf));
}

