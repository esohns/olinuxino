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

#ifndef OLIMEX_MOD_MPU6050_MESSAGE_H
#define OLIMEX_MOD_MPU6050_MESSAGE_H

#include <string>

#include "ace/Global_Macros.h"

#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_message_base.h"
#include "stream_messageallocatorheap_base.h"

//#include "olimex_mod_mpu6050_sessionmessage.h"
//#include "olimex_mod_mpu6050_types.h"

// forward declaration(s)
class ACE_Allocator;
class ACE_Data_Block;
class ACE_Message_Block;
class Olimex_Mod_MPU6050_SessionMessage;

enum Olimex_Mod_MPU6050_MessageType
{
  OLIMEX_MOD_MPU6050_MESSAGE_INVALID = -1,
  OLIMEX_MOD_MPU6050_MESSAGE_SENSOR_DATA,
  ///////////////////////////////////////
  OLIMEX_MOD_MPU6050_MESSAGE_MAX
};

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType> Olimex_Mod_MPU6050_ControlMessage_t;

class Olimex_Mod_MPU6050_Message
 : public Stream_MessageBase_T<Stream_DataBase_T<enum Olimex_Mod_MPU6050_MessageType>,
                               enum Stream_MessageType,
                               enum Olimex_Mod_MPU6050_MessageType>
{
  // enable access to specific private ctors...
  friend class Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                                 struct Stream_AllocatorConfiguration,
                                                 Olimex_Mod_MPU6050_ControlMessage_t,
                                                 Olimex_Mod_MPU6050_Message,
                                                 Olimex_Mod_MPU6050_SessionMessage>;

 public:
  virtual ~Olimex_Mod_MPU6050_Message ();

  virtual enum Olimex_Mod_MPU6050_MessageType command () const; // return value: message type
  static std::string CommandTypeToString (enum Olimex_Mod_MPU6050_MessageType);

  // overrides from ACE_Message_Block
  // --> create a "shallow" copy that references the same datablock
  // *NOTE*: this uses the allocator (if any) to create a new message
  virtual ACE_Message_Block* duplicate (void) const;

 protected:
  // copy ctor to be used by duplicate() and child classes
  // --> uses an (incremented refcount of) the same datablock ("shallow copy")
  Olimex_Mod_MPU6050_Message (const Olimex_Mod_MPU6050_Message&);

 private:
  typedef Stream_MessageBase_T<Stream_DataBase_T<enum Olimex_Mod_MPU6050_MessageType>,
                               enum Stream_MessageType,
                               enum Olimex_Mod_MPU6050_MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Message ())
  // *NOTE*: to be used by allocators...
  Olimex_Mod_MPU6050_Message (Stream_SessionId_t, // session id
                              ACE_Data_Block*,    // data block to use
                              ACE_Allocator*,     // message allocator
                              bool = true);       // increment running message counter ?
  ACE_UNIMPLEMENTED_FUNC (Olimex_Mod_MPU6050_Message& operator= (const Olimex_Mod_MPU6050_Message&))
};

#endif
