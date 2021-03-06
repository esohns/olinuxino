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

#ifndef OLIMEX_MOD_MPU6050_SESSIONMESSAGE_H
#define OLIMEX_MOD_MPU6050_SESSIONMESSAGE_H

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_messageallocatorheap_base.h"
#include "stream_session_message_base.h"

#include "olimex_mod_mpu6050_message.h"
#include "olimex_mod_mpu6050_stream_common.h"
#include "olimex_mod_mpu6050_types.h"

// forward declarations
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;

class Olimex_Mod_MPU6050_SessionMessage
 : public Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Olimex_Mod_MPU6050_StreamSessionData_t,
                                      Olimex_Mod_MPU6050_UserData,
                                      Stream_ControlMessageType,
                                      Olimex_Mod_MPU6050_Message>
{
  // enable access to private ctor(s)...
  friend class Stream_MessageAllocatorHeapBase_T<Stream_AllocatorConfiguration,
                                                 Olimex_Mod_MPU6050_ControlMessage_t,
                                                 Olimex_Mod_MPU6050_Message,
                                                 Olimex_Mod_MPU6050_SessionMessage>;

 public:
  // *NOTE*: assume lifetime responsibility for the second argument !
  Olimex_Mod_MPU6050_SessionMessage (Stream_SessionMessageType,                // session message type
                                     Olimex_Mod_MPU6050_StreamSessionData_t*&, // session data handle
                                     Olimex_Mod_MPU6050_UserData*);            // user data handle
  // *NOTE*: to be used by message allocators...
  Olimex_Mod_MPU6050_SessionMessage (ACE_Allocator*); // message allocator
  Olimex_Mod_MPU6050_SessionMessage (ACE_Data_Block*, // data block
                                     ACE_Allocator*); // message allocator
  virtual ~Olimex_Mod_MPU6050_SessionMessage ();

  // override from ACE_Message_Block
  // *WARNING*: any children need to override this as well
  virtual ACE_Message_Block* duplicate (void) const;

 private:
  typedef Stream_SessionMessageBase_T<Stream_AllocatorConfiguration,
                                      Stream_SessionMessageType,
                                      Olimex_Mod_MPU6050_StreamSessionData_t,
                                      Olimex_Mod_MPU6050_UserData,
                                      Stream_ControlMessageType,
                                      Olimex_Mod_MPU6050_Message> inherited;

  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_SessionMessage ())
  // copy ctor (to be used by duplicate())
  Olimex_Mod_MPU6050_SessionMessage (const Olimex_Mod_MPU6050_SessionMessage&);
  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_SessionMessage& operator= (const Olimex_Mod_MPU6050_SessionMessage&))
};

#endif
