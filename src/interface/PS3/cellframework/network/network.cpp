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





#include "network.hpp"
#include <cell/sysmodule.h>
#include <netex/net.h>

namespace Network {

NetworkInterface::NetworkInterface()
{
   if (ref() == 0)
   {
      int rc;
      rc = cellSysmoduleLoadModule(CELL_SYSMODULE_NET);
      rc = sys_net_initialize_network();
   }
   ref()++;
}

NetworkInterface::~NetworkInterface()
{
   if (ref() == 1)
   {
      sys_net_finalize_network();
      cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
   }
   ref()--;
}

}
