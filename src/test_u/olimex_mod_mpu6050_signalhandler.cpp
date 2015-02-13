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

#include "common_timer_manager.h"

#include "net_common_tools.h"

#include "client_GTK_manager.h"

#include "olimex_mod_mpu6050_macros.h"
#include "olimex_mod_mpu6050_types.h"

Olimex_Mod_MPU6050_SignalHandler::Olimex_Mod_MPU6050_SignalHandler (long actionTimerID_in,
                                                                    const ACE_INET_Addr& peerSAP_in,
                                                                    Net_Client_IConnector* connector_in,
                                                                    // ---------
                                                                    bool useReactor_in)
 : inherited (this,          // event handler handle
              useReactor_in) // use reactor ?
 , actionTimerID_ (actionTimerID_in)
 , peerAddress_ (peerSAP_in)
 , connector_ (connector_in)
 , useReactor_ (useReactor_in)
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

  bool stop_event_dispatching = false;
  bool connect = false;
  bool abort = false;
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
      // (try to) abort a connection...
      abort = true;

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

  // ...abort ?
  if (abort)
  {
    // release an existing connection...
    CONNECTIONMANAGER_SINGLETON::instance ()->abortOldestConnection ();
  } // end IF

  // ...connect ?
  if (connect)
  {
    try
    {
      connector_->connect (peerAddress_);
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in RPG_Net_IConnector::connect(), aborting\n")));

      return false;
    }
  } // end IF

  // ...shutdown ?
  if (stop_event_dispatching)
  {
    // stop everything, i.e.
    // - leave reactor event loop handling signals, sockets, (maintenance) timers...
    // --> (try to) terminate in a well-behaved manner

    // step1: stop all open connections

    // stop action timer (might spawn new connections otherwise)
    if (actionTimerID_ >= 0)
    {
      const void* act = NULL;
      if (COMMON_TIMERMANAGER_SINGLETON::instance ()->cancel (actionTimerID_,
                                                              &act) <= 0)
      {
        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("failed to cancel timer (ID: %d): \"%m\", aborting\n"),
                    actionTimerID_));

        // clean up
        actionTimerID_ = -1;

        return false;
      } // end IF

      // clean up
      actionTimerID_ = -1;
    } // end IF
    try
    {
      connector_->abort ();
    }
    catch (...)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("caught exception in RPG_Net_IConnector::abort(), aborting\n")));

      return false;
    }
    CONNECTIONMANAGER_SINGLETON::instance ()->stop ();
    CONNECTIONMANAGER_SINGLETON::instance ()->abortConnections ();
    // *IMPORTANT NOTE*: as long as connections are inactive (i.e. events are
    // dispatched by reactor thread(s), there is no real reason to wait here)
    //CONNECTIONMANAGER_SINGLETON::instance ()->waitConnections ();

    // step2: stop GTK event dispatch
    CLIENT_GTK_MANAGER_SINGLETON::instance ()->stop ();

    // step3: stop reactor (&& proactor, if applicable)
    Net_Common_Tools::finiEventDispatch (true,         // stop reactor ?
                                         !useReactor_, // stop proactor ?
                                         -1);          // group ID (--> don't block !)
  } // end IF

  return true;
}