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
 * NetLogger.h
 *
 *  Created on: Nov 1, 2010
 *      Author: halsafar
 */

#ifndef NETLOGGER_H_
#define NETLOGGER_H_

#include "ILogger.h"
#include "../network/TCPSocket.hpp"
#include "../network/network.hpp"

#define SEND_BUFFER_SIZE 4096

class NetLogger : public ILogger
{
public:
	NetLogger(const char *host, uint16_t port);
	virtual ~NetLogger();
	virtual void printf(const char* fmt, ...);
   int32_t init() { return sock->alive() ? 0 : 1; }
   virtual void flush() {}
   virtual void close() {}
private:
   Network::TCPSocket *sock;
	char _sendbuf[SEND_BUFFER_SIZE];
};

#endif /* NETLOGGER_H_ */
