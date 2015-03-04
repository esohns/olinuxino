﻿/*  I2C kernel module driver for the Olimex MOD-MPU6050 UEXT module
    (see https://www.olimex.com/Products/Modules/Sensors/MOD-MPU6050/open-source-hardware,
         http://www.invensense.com/mems/gyro/mpu6050.html)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef OLIMEX_MOD_MPU6050_NETWORK_H
#define OLIMEX_MOD_MPU6050_NETWORK_H

#include "ace/Global_Macros.h"
#include "ace/INET_Addr.h"
#include "ace/Singleton.h"
#include "ace/Synch.h"
#include "ace/SOCK_Dgram.h"

#include "stream_common.h"

#include "net_asynch_udpsockethandler.h"
#include "net_configuration.h"
#include "net_connection_manager.h"
#include "net_itransportlayer.h"
#include "net_stream_asynch_udpsocket_base.h"
#include "net_stream_common.h"
#include "net_stream_udpsocket_base.h"
#include "net_udpconnection.h"
#include "net_udpsockethandler.h"

#include "net_client_asynchconnector.h"
#include "net_client_connector.h"
#include "net_client_iconnector.h"

#include "olimex_mod_mpu6050_stream.h"

typedef Net_StreamUDPSocketBase_T<Net_Configuration_t,
                                  Net_StreamSessionData_t,
                                  Net_IInetTransportLayer_t,
                                  Stream_Statistic_t,
                                  Olimex_Mod_MPU6050_Stream,
                                  Net_UDPSocketHandler_T<ACE_SOCK_DGRAM> > Olimex_Mod_MPU6050_UDPHandler_t;
typedef Net_StreamAsynchUDPSocketBase_T<Net_Configuration_t,
                                        Net_StreamSessionData_t,
                                        Net_IInetTransportLayer_t,
                                        Stream_Statistic_t,
                                        Olimex_Mod_MPU6050_Stream,
                                        ACE_SOCK_DGRAM,
                                        Net_AsynchUDPSocketHandler> Olimex_Mod_MPU6050_AsynchUDPHandler_t;

typedef Net_Client_IConnector_T<ACE_INET_Addr,
                                Net_Configuration_t> Olimex_Mod_MPU6050_IConnector_t;

typedef Net_Client_Connector_T<ACE_INET_Addr,
                               Net_Configuration_t,
                               Net_StreamSessionData_t,
                               Net_IInetTransportLayer_t,
                               Net_UDPConnection_T<Net_StreamSessionData_t,
                                                   Olimex_Mod_MPU6050_UDPHandler_t> > Olimex_Mod_MPU6050_Connector_t;
typedef Net_Client_AsynchConnector_T<ACE_INET_Addr,
                                     Net_Configuration_t,
                                     Net_StreamSessionData_t,
                                     Net_IInetTransportLayer_t,
                                     Net_AsynchUDPConnection_T<Net_StreamSessionData_t,
                                                               Olimex_Mod_MPU6050_AsynchUDPHandler_t> > Olimex_Mod_MPU6050_AsynchConnector_t;

typedef Net_Connection_Manager_T<Net_Configuration_t,
                                 Net_StreamSessionData_t,
                                 Net_IInetTransportLayer_t,
                                 Stream_Statistic_t> Olimex_Mod_MPU6050_ConnectionManager_t;
typedef ACE_Singleton<Olimex_Mod_MPU6050_ConnectionManager_t,
                      ACE_Recursive_Thread_Mutex> CONNECTIONMANAGER_SINGLETON;

#endif // #ifndef OLIMEX_MOD_MPU6050_NETWORK_H
