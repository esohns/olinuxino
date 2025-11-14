/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef OLIMEX_MOD_MPU6050_MODULES_COMMON_H
#define OLIMEX_MOD_MPU6050_MODULES_COMMON_H

#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_queue_source.h"
#include "stream_misc_messagehandler.h"

#include "stream_net_source.h"

#include "stream_stat_statistic_report.h"

#include "olimex_mod_mpu6050_message.h"
#include "olimex_mod_mpu6050_network.h"
#include "olimex_mod_mpu6050_sessionmessage.h"
#include "olimex_mod_mpu6050_stream_common.h"
#include "olimex_mod_mpu6050_types.h"

typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Olimex_Mod_MPU6050_ControlMessage_t,
                                    Olimex_Mod_MPU6050_Message,
                                    Olimex_Mod_MPU6050_SessionMessage,
                                    struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Olimex_Mod_MPU6050_StreamState,
                                    Olimex_Mod_MPU6050_RuntimeStatistic_t,
                                    Olimex_Mod_MPU6050_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    Olimex_Mod_MPU6050_Connector_t,
                                    struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_Module_SocketHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Olimex_Mod_MPU6050_SessionData,                // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_source_module_name_string,
                              Olimex_Mod_MPU6050_IStreamNotify_t,                   // stream notification interface type
                              Olimex_Mod_MPU6050_Module_SocketHandler);             // writer type
typedef Stream_Module_Net_SourceH_T<ACE_MT_SYNCH,
                                    Olimex_Mod_MPU6050_ControlMessage_t,
                                    Olimex_Mod_MPU6050_Message,
                                    Olimex_Mod_MPU6050_SessionMessage,
                                    struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                    enum Stream_ControlType,
                                    enum Stream_SessionMessageType,
                                    struct Olimex_Mod_MPU6050_StreamState,
                                    Olimex_Mod_MPU6050_RuntimeStatistic_t,
                                    Olimex_Mod_MPU6050_SessionManager_t,
                                    Common_Timer_Manager_t,
                                    Olimex_Mod_MPU6050_AsynchConnector_t,
                                    struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_Module_AsynchSocketHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Olimex_Mod_MPU6050_SessionData,                // session data type
                              enum Stream_SessionMessageType,                       // session event type
                              struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration, // module handler configuration type
                              libacestream_default_net_source_module_name_string,
                              Olimex_Mod_MPU6050_IStreamNotify_t,                   // stream notification interface type
                              Olimex_Mod_MPU6050_Module_AsynchSocketHandler);       // writer type

typedef Stream_Statistic_StatisticReport_ReaderTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                                      Olimex_Mod_MPU6050_ControlMessage_t,
                                                      Olimex_Mod_MPU6050_Message,
                                                      Olimex_Mod_MPU6050_SessionMessage,
                                                      enum Olimex_Mod_MPU6050_MessageType,
                                                      Olimex_Mod_MPU6050_RuntimeStatistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_Module_Statistic_ReaderTask_t;
typedef Stream_Statistic_StatisticReport_WriterTask_T<ACE_MT_SYNCH,
                                                      Common_TimePolicy_t,
                                                      struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                                      Olimex_Mod_MPU6050_ControlMessage_t,
                                                      Olimex_Mod_MPU6050_Message,
                                                      Olimex_Mod_MPU6050_SessionMessage,
                                                      enum Olimex_Mod_MPU6050_MessageType,
                                                      Olimex_Mod_MPU6050_RuntimeStatistic_t,
                                                      Common_Timer_Manager_t,
                                                      struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_Module_Statistic_WriterTask_t;
DATASTREAM_MODULE_DUPLEX (struct Olimex_Mod_MPU6050_SessionData,                // session data type
                          enum Stream_SessionMessageType,                       // session event type
                          struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration, // module handler configuration type
                          libacestream_default_stat_report_module_name_string,
                          Olimex_Mod_MPU6050_IStreamNotify_t,                   // stream notification interface type
                          Olimex_Mod_MPU6050_Module_Statistic_ReaderTask_t,     // reader type
                          Olimex_Mod_MPU6050_Module_Statistic_WriterTask_t,     // writer type
                          Olimex_Mod_MPU6050_Module_RuntimeStatistic);          // name

typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                       Common_TimePolicy_t,
                                       struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                       Olimex_Mod_MPU6050_ControlMessage_t,
                                       Olimex_Mod_MPU6050_Message,
                                       Olimex_Mod_MPU6050_SessionMessage,
                                       struct Olimex_Mod_MPU6050_SessionData,
                                       struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_Module_MessageHandler;
DATASTREAM_MODULE_INPUT_ONLY (struct Olimex_Mod_MPU6050_SessionData,                       // session data type
                              enum Stream_SessionMessageType,                              // session event type
                              struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,        // module handler configuration type
                              libacestream_default_misc_messagehandler_module_name_string, // module name
                              Olimex_Mod_MPU6050_IStreamNotify_t,                          // stream notification interface type
                              Olimex_Mod_MPU6050_Module_MessageHandler);                   // writer type

#endif
