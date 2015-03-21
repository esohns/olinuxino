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

#ifndef OLIMEX_MOD_MPU6050_STREAM_H
#define OLIMEX_MOD_MPU6050_STREAM_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common.h"

#include "stream_base.h"
#include "stream_common.h"

#include "net_configuration.h"
#include "net_stream_common.h"

#include "olimex_mod_mpu6050_message.h"
#include "olimex_mod_mpu6050_sessionmessage.h"
#include "olimex_mod_mpu6050_stream_common.h"

class Olimex_Mod_MPU6050_Stream
 : public Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_State_t,
                        Stream_Statistic_t,
                        Net_UserData_t,
                        Net_StreamSessionData_t,
                        Olimex_Mod_MPU6050_SessionMessage,
                        Olimex_Mod_MPU6050_Message>
{
 public:
  Olimex_Mod_MPU6050_Stream ();
  virtual ~Olimex_Mod_MPU6050_Stream ();

  // initialize stream
  bool initialize (unsigned int,                       // session ID
                   const Stream_Configuration_t&,      // configuration
                   const Net_ProtocolConfiguration_t&, // protocol configuration
                   const Net_UserData_t&);             // user data

  // *TODO*: re-consider this API
  void ping ();

  // implement Common_IStatistic_T
  // *NOTE*: these delegate to runtimeStatistic_
  virtual bool collect (Stream_Statistic_t&); // return value: statistic data
  virtual void report () const;

 private:
  typedef Stream_Base_T<ACE_MT_SYNCH,
                        Common_TimePolicy_t,
                        Stream_State_t,
                        Stream_Statistic_t,
                        Net_UserData_t,
                        Net_StreamSessionData_t,
                        Olimex_Mod_MPU6050_SessionMessage,
                        Olimex_Mod_MPU6050_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Stream (const Olimex_Mod_MPU6050_Stream&));
  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Stream& operator= (const Olimex_Mod_MPU6050_Stream&));

  // finalize stream
  // *NOTE*: need this to clean up queued modules if something goes wrong during
  //         initialize () !
  bool finalize (const Stream_Configuration_t&); // configuration

  // modules
  Olimex_Mod_MPU6050_Module_SocketHandler_Module    socketHandler_;
  Olimex_Mod_MPU6050_Module_RuntimeStatistic_Module runtimeStatistic_;
};

#endif
