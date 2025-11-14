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
#include <string>

#include "ace/Log_Msg.h"

#include "olimex_mod_mpu6050_defines.h"
#include "olimex_mod_mpu6050_macros.h"
#include "olimex_mod_mpu6050_stream_common.h"

template <typename SourceModuleType>
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::Olimex_Mod_MPU6050_Stream_T ()
 : inherited ()
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::Olimex_Mod_MPU6050_Stream_T"));

}

template <typename SourceModuleType>
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::~Olimex_Mod_MPU6050_Stream_T ()
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::~Olimex_Mod_MPU6050_Stream_T"));

  // *NOTE*: this implements an ordered shutdown on destruction...
  inherited::shutdown ();
}

template <typename SourceModuleType>
bool
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::load (Stream_ILayout* layout_inout,
                                                     bool& delete_out)
{
  STREAM_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::load"));

  Stream_Module_t* module_p = NULL;
  ACE_NEW_RETURN (module_p,
                  SourceModuleType (this,
                                    ACE_TEXT_ALWAYS_CHAR ("CamSource")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  ACE_NEW_RETURN (module_p,
                  Olimex_Mod_MPU6050_Module_RuntimeStatistic_Module (this,
                                                                     ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")),
                  false);
  layout_inout->append (module_p, NULL, 0);
  module_p = NULL;

  delete_out = true;

  return true;
}

template <typename SourceModuleType>
bool
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::initialize (const typename inherited::CONFIGURATION_T& configuration_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::initialize"));

  bool setup_pipeline = configuration_in.configuration_->setupPipeline;
  bool reset_setup_pipeline = false;
  struct Olimex_Mod_MPU6050_SessionData* session_data_p = NULL;
  Olimex_Mod_MPU6050_SessionManager_t* session_manager_p =
    Olimex_Mod_MPU6050_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);

  // allocate a new session state, reset stream
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    false;
  reset_setup_pipeline = true;
  if (!inherited::initialize (configuration_in))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Base_T::initialize(), aborting\n"),
                ACE_TEXT (stream_name_string_)));
    goto error;
  } // end IF
  const_cast<inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline =
    setup_pipeline;
  reset_setup_pipeline = false;

  // sanity check(s)
  session_data_p =
    &const_cast<struct Olimex_Mod_MPU6050_SessionData&> (session_manager_p->getR (inherited::id_));

  // -------------------------------------------------------------

  if (inherited::configuration_->configuration_->setupPipeline)
    if (!inherited::setup (NULL))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to setup pipeline, aborting\n")));
      goto error;
    } // end IF

  inherited::isInitialized_ = true;
  //   inherited::dump_state();

  return true;

error:
  if (reset_setup_pipeline)
    const_cast<typename inherited::CONFIGURATION_T&> (configuration_in).configuration_->setupPipeline = setup_pipeline;

  return false;
}

template <typename SourceModuleType>
void
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::ping ()
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::ping"));

//  Net_Module_ProtocolHandler* protocolHandler_impl = NULL;
//  protocolHandler_impl = dynamic_cast<Net_Module_ProtocolHandler*> (protocolHandler_.writer ());
//  if (!protocolHandler_impl)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("dynamic_cast<Net_Module_ProtocolHandler> failed, returning\n")));

//    return;
//  } // end IF

//  // delegate to this module...
//  protocolHandler_impl->handleTimeout (NULL);

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}

template <typename SourceModuleType>
bool
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::collect (Olimex_Mod_MPU6050_RuntimeStatistic_t& data_out)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::collect"));

  int result = -1;
  Olimex_Mod_MPU6050_SessionManager_t* session_manager_p =
    Olimex_Mod_MPU6050_SessionManager_t::SINGLETON_T::instance ();
  ACE_ASSERT (session_manager_p);
  struct Olimex_Mod_MPU6050_SessionData* session_data_p =
    &const_cast<struct Olimex_Mod_MPU6050_SessionData&> (session_manager_p->getR (inherited::id_));
  Stream_Module_t* module_p =
    const_cast<Stream_Module_t*> (inherited::find (ACE_TEXT_ALWAYS_CHAR ("RuntimeStatistic")));
  if (!module_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to retrieve \"%s\" module handle, aborting\n"),
                ACE_TEXT ("RuntimeStatistic")));
    return false;
  } // end IF
  Olimex_Mod_MPU6050_Module_Statistic_WriterTask_t* statistic_impl_p =
    dynamic_cast<Olimex_Mod_MPU6050_Module_Statistic_WriterTask_t*> (module_p->writer ());
  if (!statistic_impl_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("dynamic_cast<Olimex_Mod_MPU6050_Module_Statistic_WriterTask_t> failed, aborting\n")));
    return false;
  } // end IF

  // synch access
  if (session_data_p->lock)
  {
    result = session_data_p->lock->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::acquire(): \"%m\", aborting\n")));
      return false;
    } // end IF
  } // end IF

  session_data_p->currentStatistic.timeStamp = COMMON_TIME_NOW;

  // delegate to the statistic module
  bool result_2 = false;
  try {
    result_2 = statistic_impl_p->collect (data_out);
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("caught exception in Common_IStatistic_T::collect(), continuing\n")));
  }
  if (!result)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_IStatistic_T::collect(), aborting\n")));
  else
    session_data_p->currentStatistic = data_out;

  if (session_data_p->lock)
  {
    result = session_data_p->lock->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  return result_2;
}

template <typename SourceModuleType>
void
Olimex_Mod_MPU6050_Stream_T<SourceModuleType>::report () const
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_Stream_T::report"));

  ACE_ASSERT (inherited::state_.currentSessionData);

  ACE_DEBUG ((LM_INFO,
              ACE_TEXT ("*** [session: %u] RUNTIME STATISTICS ***\n--> Stream Statistics @ %#D<--\n (data) messages: %u\n dropped messages: %u\n bytes total: %.0f\n*** RUNTIME STATISTICS ***\\END\n"),
              inherited::state_.currentSessionData->sessionId,
              &(inherited::state_.currentSessionData->lastCollectionTimeStamp),
              inherited::state_.currentSessionData->currentStatistic.dataMessages,
              inherited::state_.currentSessionData->currentStatistic.droppedFrames,
              inherited::state_.currentSessionData->currentStatistic.bytes));
}
