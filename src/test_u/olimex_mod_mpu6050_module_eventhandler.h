/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
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

#ifndef OLIMEX_MOD_MPU6050_MODULE_EVENTHANDLER_H
#define OLIMEX_MOD_MPU6050_MODULE_EVENTHANDLER_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_streammodule_base.h"

#include "stream_misc_messagehandler.h"

#include "olimex_mod_mpu6050_message.h"
#include "olimex_mod_mpu6050_sessionmessage.h"
#include "olimex_mod_mpu6050_stream_common.h"
#include "olimex_mod_mpu6050_types.h"

//// forward declaration(s)
//class Olimex_Mod_MPU6050_SessionMessage;
//class Olimex_Mod_MPU6050_Message;

class Olimex_Mod_MPU6050_Module_EventHandler
 : public Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                         Olimex_Mod_MPU6050_ControlMessage_t,
                                         Olimex_Mod_MPU6050_Message,
                                         Olimex_Mod_MPU6050_SessionMessage,
                                         Stream_SessionId_t,
                                         Olimex_Mod_MPU6050_SessionData>
{
 public:
  Olimex_Mod_MPU6050_Module_EventHandler ();
  virtual ~Olimex_Mod_MPU6050_Module_EventHandler ();

  // implement Common_IClone_T
  virtual Stream_Module_t* clone ();

 private:
  typedef Stream_Module_MessageHandler_T<ACE_MT_SYNCH,
                                         Common_TimePolicy_t,
                                         Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                                         Olimex_Mod_MPU6050_ControlMessage_t,
                                         Olimex_Mod_MPU6050_Message,
                                         Olimex_Mod_MPU6050_SessionMessage,
                                         Stream_SessionId_t,
                                         Olimex_Mod_MPU6050_SessionData> inherited;

  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Module_EventHandler (const Olimex_Mod_MPU6050_Module_EventHandler&))
  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Module_EventHandler& operator= (const Olimex_Mod_MPU6050_Module_EventHandler&))
};

// declare module
DATASTREAM_MODULE_INPUT_ONLY (Olimex_Mod_MPU6050_SessionData,                // session data type
                              Stream_SessionMessageType,                     // session event type
                              Olimex_Mod_MPU6050_ModuleHandlerConfiguration, // module handler configuration type
                              Olimex_Mod_MPU6050_IStreamNotify_t,            // stream notification interface type
                              Olimex_Mod_MPU6050_Module_EventHandler);       // writer type

#endif
