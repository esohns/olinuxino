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
#include "stdafx.h"

#include "olimex_mod_mpu6050_signalhandler.h"

#include "ace/Log_Msg.h"

#include "common_tools.h"

#include "common_ui_gtk_manager.h"

#include "olimex_mod_mpu6050_macros.h"

Olimex_Mod_MPU6050_SignalHandler::Olimex_Mod_MPU6050_SignalHandler ()
 : inherited (this) // event handler handle
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_SignalHandler::Olimex_Mod_MPU6050_SignalHandler"));

}

Olimex_Mod_MPU6050_SignalHandler::~Olimex_Mod_MPU6050_SignalHandler ()
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_SignalHandler::~Olimex_Mod_MPU6050_SignalHandler"));

}

bool
Olimex_Mod_MPU6050_SignalHandler::handleSignal (int signal_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("Olimex_Mod_MPU6050_SignalHandler::handleSignal"));

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  bool stop_event_dispatching = false;
  bool connect = false;
//  bool abort = false;
  switch (signal_in)
  {
    case SIGINT:
// *PORTABILITY*: on Windows SIGQUIT is not defined
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGQUIT:
#endif
    {
      //ACE_DEBUG ((LM_DEBUG,
      //            ACE_TEXT ("shutting down...\n")));

      // shutdown...
      stop_event_dispatching = true;

      break;
    }
// *PORTABILITY*: on Windows SIGUSRx are not defined
// --> use SIGBREAK (21) and SIGTERM (15) instead...
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGUSR1:
#else
    case SIGBREAK:
#endif
    {
      // (try to) connect...
      connect = true;

      break;
    }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
    case SIGHUP:
    case SIGUSR2:
#endif
    case SIGTERM:
    {
//      // (try to) abort a connection...
//      abort = true;

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("received invalid/unknown signal: \"%S\", aborting\n"),
                  signal_in));
      return false;
    }
  } // end SWITCH

//  // ...abort ?
//  if (abort)
//  {
//    // close any connections...
//    CONNECTIONMANAGER_SINGLETON::instance ()->abortConnections ();
//  } // end IF

  // ...connect ?
  if (connect &&
      inherited::configuration_->interfaceHandle)
  {
    try {
      inherited::configuration_->interfaceHandle->connect (inherited::configuration_->peerAddress);
    } catch (...) {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in Olimex_Mod_MPU6050_IConnector_t::connect(), aborting\n")));

      return false;
    }
  } // end IF

  // ...shutdown ?
  if (stop_event_dispatching)
  {
    // stop everything, i.e.
    // - leave reactor event loop handling signals, sockets, (maintenance) timers...
    // --> (try to) terminate in a well-behaved manner

    // step1: close open connection attempt(s)
    if (inherited::configuration_->interfaceHandle)
    {
      try {
        inherited::configuration_->interfaceHandle->abort ();
      } catch (...) {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("caught exception in Olimex_Mod_MPU6050_IConnector_t::abort(), aborting\n")));

        return false;
      }
    } // end IF

    // step2: stop GTK event dispatch ?
    if (!inherited::configuration_->consoleMode)
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();

    // step3: stop reactor (&& proactor, if applicable)
    Common_Tools::finalizeEventDispatch (true,                                   // stop reactor ?
                                         !inherited::configuration_->useReactor, // stop proactor ?
                                         -1);                                    // group ID (--> don't block !)
  } // end IF

  return true;
}
