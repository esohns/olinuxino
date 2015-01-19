/*  I2C kernel module driver test for the Olimex MOD-MPU6050 UEXT module
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

#if defined(_MSC_VER)
#include "stdafx.h"
#endif

#include <iostream>
#include <sstream>

#if defined(ENABLE_NLS)
#include <locale.h>
#include <libintl.h>
#endif
#include "gettext.h"

#include "ace/OS.h"
#include "ace/OS_main.h"
#include "ace/ACE.h"
#if defined(_MSC_VER)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"

#ifdef HAVE_CONFIG_H
#include "olinuxino_config.h"
#endif

void
do_printVersion (const std::string& programName_in)
{
  // step1: program version
  //   std::cout << programName_in << ACE_TEXT(" : ") << VERSION << std::endl;
  std::cout << programName_in
            << ACE_TEXT_ALWAYS_CHAR (": ")
#ifdef HAVE_CONFIG_H
            << OLINUXINO_VERSION
#else
            << ACE_TEXT_ALWAYS_CHAR ("N/A")
#endif
            << std::endl;

  std::ostringstream version_number;
  // step2: ACE version
  // *NOTE*: cannot use ACE_VERSION, as it doesn't contain the (potential) beta
  // version number (this is needed, as the library soname is compared to this
  // string)
//  version_number.str ("");
  version_number << ACE::major_version ();
  version_number << ACE_TEXT_ALWAYS_CHAR (".");
  version_number << ACE::minor_version ();
  version_number << ACE_TEXT_ALWAYS_CHAR (".");
  version_number << ACE::beta_version ();
  std::cout << ACE_TEXT ("ACE: ")
    //             << ACE_VERSION
            << version_number.str ()
            << std::endl;
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR** argv_in)
{
  // step0: init ACE
  // *PORTABILITY*: on Windows, ACE needs initialization...
#if defined (_MSC_VER)
  if (ACE::init () == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));

    return EXIT_FAILURE;
  } // end IF
#endif

//  // step1: process commandline options (if any)
//  if (mode == MODE_SHOW_VERSION)
//  {
//    do_printVersion (ACE::basename (argv_in[0]));

//    // *PORTABILITY*: on Windows, ACE needs finalization...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

//    return EXIT_SUCCESS;
//  } // end IF

//  // step2: initialize logging and/or tracing
//  std::string log_file;
//  const Configuration_t& configuration =
//    SPLOT_CONFIGURATION_SINGLETON::instance ()->get ();
//  if (configuration.debug)
//  {
//    log_file = getHomeDirectory ();
//    log_file += ACE_DIRECTORY_SEPARATOR_STR;
//    log_file += ACE_TEXT_ALWAYS_CHAR (CONFIG_LOG_FILE);
//  } // end IF
//  if (!initLogging (ACE_TEXT_ALWAYS_CHAR (ACE::basename (argv_in[0])),        // program name
//                    log_file,                                                 // logfile
//                    false,                                                    // log to syslog ?
//                    false,                                                    // trace messages ?
//                    SPLOT_CONFIGURATION_SINGLETON::instance() ->get ().debug, // debug messages ?
//                    NULL))                                                    // logger
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to initLogging(), aborting\n")));

//    // *PORTABILITY*: on Windows, need to fini ACE...
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    if (ACE::fini () == -1)
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
//#endif

//    return EXIT_FAILURE;
//  } // end IF

#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
  setlocale (LC_ALL, "");
#endif
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);
#endif

#ifdef __linux__
  // by default, disable SDL's use of DGA mouse. If SDL_VIDEO_X11_DGAMOUSE is
  // set however, use default value.
  ACE_OS::setenv ("SDL_VIDEO_X11_DGAMOUSE", "0", false);
#endif

  // *PORTABILITY*: on Windows, must fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if (ACE::fini () == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));

    return EXIT_FAILURE;
  } // end IF
#endif

  return EXIT_SUCCESS;
} // end main
