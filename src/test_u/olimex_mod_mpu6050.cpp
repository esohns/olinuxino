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

#include "stdafx.h"

#if defined (VALGRIND_USE)
#include "valgrind/memcheck.h"
#endif // VALGRIND_USE

#include <iostream>
#include <sstream>

#if defined (ENABLE_NLS)
#include "locale.h"
#include "libintl.h"
#endif // ENABLE_NLS

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "gettext.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/ACE.h"
#include "ace/Get_Opt.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "ace/Init_ACE.h"
#endif
#include "ace/Log_Msg.h"
#include "ace/OS.h"
#include "ace/OS_main.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "ace/POSIX_Proactor.h"
#endif
#include "ace/Profile_Timer.h"
#include "ace/Time_Value.h"

#if defined (HAVE_CONFIG_H)
#include "olinuxino_config.h"
#endif // HAVE_CONFIG_H

#include "common_defines.h"
#include "common_file_tools.h"

#include "common_event_tools.h"

#include "common_log_tools.h"

#include "common_ui_defines.h"
// #include "common_ui_glade_definition.h"
#include "common_ui_gtk_manager_common.h"
#include "common_ui_gtk_builder_definition.h"

#include "stream_allocatorheap.h"

#include "net_defines.h"

#include "net_client_connector.h"
#include "net_client_asynchconnector.h"

#include "olimex_mod_mpu6050_callbacks.h"
#include "olimex_mod_mpu6050_defines.h"
#include "olimex_mod_mpu6050_eventhandler.h"
#include "olimex_mod_mpu6050_macros.h"
#include "olimex_mod_mpu6050_network.h"
#include "olimex_mod_mpu6050_signalhandler.h"
#include "olimex_mod_mpu6050_stream.h"
#include "olimex_mod_mpu6050_types.h"

const char stream_name_io_string_[] = ACE_TEXT_ALWAYS_CHAR ("NetIOStream");
const char stream_name_string_[] = ACE_TEXT_ALWAYS_CHAR ("Stream");

void
do_printVersion (const std::string& programName_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::do_printVersion"));

  // step1: program version
  //   std::cout << programName_in << ACE_TEXT(" : ") << VERSION << std::endl;
  std::cout << programName_in
            << ACE_TEXT_ALWAYS_CHAR (": ")
#ifdef HAVE_CONFIG_H
            << olinuxino_VERSION_MAJOR
            << ACE_TEXT_ALWAYS_CHAR (".")
            << olinuxino_VERSION_MINOR
            << ACE_TEXT_ALWAYS_CHAR (".")
            << olinuxino_VERSION_MICRO
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

void
do_printUsage (const std::string& programName_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::do_printUsage"));

  // enable verbatim boolean output
  std::cout.setf (ios::boolalpha);

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  std::cout << ACE_TEXT ("usage: ")
            << programName_in
            << ACE_TEXT (" [OPTIONS]")
            << std::endl
            << std::endl;
  std::cout << ACE_TEXT ("currently available options:")
            << std::endl;
  std::cout << ACE_TEXT ("-c           : client mode [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
  std::cout << ACE_TEXT ("-g           : (netlink) multicast group [")
            << OLIMEX_MOD_MPU6050_DEFAULT_NETLINK_GROUP
            << ACE_TEXT ("]")
            << std::endl;
#endif
  std::cout << ACE_TEXT ("-l           : log to a file [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-r           : use reactor [")
            << OLIMEX_MOD_MPU6050_USE_REACTOR
            << ACE_TEXT ("]")
            << std::endl;
  std::cout << ACE_TEXT ("-s [IPv4]    : server address[:port]")
            << std::endl;
  std::cout << ACE_TEXT ("-t           : trace information [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string UI_file = path;
  UI_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  UI_file += ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_DEFINITION_FILE_NAME);
  std::cout << ACE_TEXT ("-u[[STRING]] : (libglade) interface definition file [\"")
            << UI_file
            << ACE_TEXT_ALWAYS_CHAR ("\"] {\"\" --> no GUI}")
            << std::endl;
  std::cout << ACE_TEXT ("-v           : print version information and exit [")
            << false
            << ACE_TEXT ("]")
            << std::endl;
}

bool
do_processArguments (int argc_in,
                     ACE_TCHAR** argv_in, // cannot be const...
                     bool& clientMode_out,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                     unsigned int& netlinkGroup_out,
#endif
                     bool& logToFile_out,
                     bool& useReactor_out,
                     ACE_INET_Addr& peerAddress_out,
                     bool& traceInformation_out,
                     std::string& interfaceDefinitionFile_out,
                     bool& printVersionAndExit_out)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::do_processArguments"));

  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  // initialize results
  clientMode_out              = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  netlinkGroup_out            = OLIMEX_MOD_MPU6050_DEFAULT_NETLINK_GROUP;
#endif
  logToFile_out               = false;
  useReactor_out              = OLIMEX_MOD_MPU6050_USE_REACTOR;
  traceInformation_out        = false;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  interfaceDefinitionFile_out = path;
  interfaceDefinitionFile_out += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  interfaceDefinitionFile_out +=
    ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_DEFINITION_FILE_NAME);
  printVersionAndExit_out     = false;

  ACE_Get_Opt argumentParser (argc_in,
                              argv_in,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              ACE_TEXT ("clrs:tu::v"),
#else
                              ACE_TEXT ("cg:lrs:tu::v"),
#endif
                              1,                          // skip command name
                              1,                          // report parsing errors
                              ACE_Get_Opt::PERMUTE_ARGS,  // ordering
                              0);                         // for now, don't use long options

  int option = 0;
  std::stringstream converter;
  while ((option = argumentParser ()) != EOF)
  {
    switch (option)
    {
      case 'c':
      {
        clientMode_out = true;
        break;
      }
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
      case 'g':
      {
        converter.str (ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ()));
        converter >> netlinkGroup_out;

        break;
      }
#endif
      case 'l':
      {
        logToFile_out = true;
        break;
      }
      case 'r':
      {
        useReactor_out = true;
        break;
      }
      case 's':
      {
        int result = -1;
        std::string address = ACE_TEXT_ALWAYS_CHAR (argumentParser.opt_arg ());
        if (address.find (':') != std::string::npos)
          result = peerAddress_out.set (address.c_str (), 0);
        else
          result = peerAddress_out.set (OLIMEX_MOD_MPU6050_DEFAULT_PORT,
                                        address.c_str (),
                                        1,
                                        0);
        if (result == -1)
        {
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to ACE_INET_Addr::set(\"%s\"): \"%m\", aborting\n"),
                      ACE_TEXT (argumentParser.opt_arg ())));
          return false;
        } // end IF
        break;
      }
      case 't':
      {
        traceInformation_out = true;
        break;
      }
      case 'u':
      {
        ACE_TCHAR* opt_arg = argumentParser.opt_arg ();
        if (opt_arg)
          interfaceDefinitionFile_out = ACE_TEXT_ALWAYS_CHAR (opt_arg);
        else
          interfaceDefinitionFile_out.clear ();
        break;
      }
      case 'v':
      {
        printVersionAndExit_out = true;
        break;
      }
      // error handling
      case ':':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("option \"%c\" requires an argument, aborting\n"),
                    argumentParser.opt_opt ()));
        return false;
      }
      case '?':
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("unrecognized option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.last_option ())));
        return false;
      }
      case 0:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("found long option \"%s\", aborting\n"),
                    ACE_TEXT (argumentParser.long_option ())));
        return false;
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("parse error, aborting\n")));
        return false;
      }
    } // end SWITCH
  } // end WHILE

  return true;
}

void
do_initializeSignals (bool useReactor_in,
                      bool allowUserRuntimeStats_in,
                      ACE_Sig_Set& signals_out,
                      ACE_Sig_Set& ignoredSignals_out)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::do_initializeSignals"));

  int result = -1;

  // initialize return value(s)
  result = signals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF
  result = ignoredSignals_out.empty_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Sig_Set::empty_set(): \"%m\", returning\n")));
    return;
  } // end IF

  // *PORTABILITY*: on Microsoft Windows (TM) most signals are not defined,
  //                and ACE_Sig_Set::fill_set() doesn't really work as specified
  //                --> add valid signals (see <signal.h>)...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  signals_out.sig_add (SIGINT);            // 2       /* interrupt */
  signals_out.sig_add (SIGILL);            // 4       /* illegal instruction - invalid function image */
  signals_out.sig_add (SIGFPE);            // 8       /* floating point exception */
  //  signals_out.sig_add(SIGSEGV);          // 11      /* segment violation */
  signals_out.sig_add (SIGTERM);           // 15      /* Software termination signal from kill */
  if (allowUserRuntimeStats_in)
  {
    signals_out.sig_add (SIGBREAK);        // 21      /* Ctrl-Break sequence */
    ignoredSignals_out.sig_add (SIGBREAK); // 21      /* Ctrl-Break sequence */
  } // end IF
  signals_out.sig_add (SIGABRT);           // 22      /* abnormal termination triggered by abort call */
  signals_out.sig_add (SIGABRT_COMPAT);    // 6       /* SIGABRT compatible with other platforms, same as SIGABRT */
#else
  result = signals_out.fill_set ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
      ACE_TEXT ("failed to ACE_Sig_Set::fill_set(): \"%m\", returning\n")));
    return;
  } // end IF
  // *NOTE*: cannot handle some signals --> registration fails for these...
  signals_out.sig_del (SIGKILL);           // 9       /* Kill signal */
  // ---------------------------------------------------------------------------
  if (!allowUserRuntimeStats_in)
  {
    signals_out.sig_del (SIGUSR1);         // 10      /* User-defined signal 1 */
    ignoredSignals_out.sig_add (SIGUSR1);  // 10      /* User-defined signal 1 */
} // end IF
  // *NOTE* core dump on SIGSEGV
  signals_out.sig_del (SIGSEGV);           // 11      /* Segmentation fault: Invalid memory reference */
  // *NOTE* don't care about SIGPIPE
  signals_out.sig_del (SIGPIPE);           // 12      /* Broken pipe: write to pipe with no readers */
  signals_out.sig_del (SIGSTOP);           // 19      /* Stop process */

  // *IMPORTANT NOTE*: "...NPTL makes internal use of the first two real-time
  //                   signals (see also signal(7)); these signals cannot be
  //                   used in applications. ..." (see 'man 7 pthreads')
  // --> on POSIX platforms, make sure that ACE_SIGRTMIN == 34
  //  for (int i = ACE_SIGRTMIN;
  //       i <= ACE_SIGRTMAX;
  //       i++)
  //    signals_out.sig_del (i);

  if (!useReactor_in)
  {
    ACE_Proactor* proactor_p = ACE_Proactor::instance ();
    ACE_ASSERT (proactor_p);
    ACE_POSIX_Proactor* proactor_impl_p =
      dynamic_cast<ACE_POSIX_Proactor*> (proactor_p->implementation ());
    ACE_ASSERT (proactor_impl_p);
    if (proactor_impl_p->get_impl_type () == ACE_POSIX_Proactor::PROACTOR_SIG)
      signals_out.sig_del (COMMON_EVENT_PROACTOR_SIG_RT_SIGNAL);
  } // end IF
#endif

  // *NOTE*: gdb sends some signals (when running in an IDE ?)
  //         --> remove signals (and let IDE handle them)
  // *TODO*: clean this up
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#if defined (DEBUG_DEBUGGER)
  //  signals_out.sig_del (SIGINT);
  signals_out.sig_del (SIGCONT);
  signals_out.sig_del (SIGHUP);
#endif
#endif

  // *TODO*: improve valgrind support
#ifdef OLINUXINO_ENABLE_VALGRIND_SUPPORT
  // *NOTE*: valgrind uses SIGRT32 (--> SIGRTMAX ?) and apparently will not work
  // if the application installs its own handler (see documentation)
  if (RUNNING_ON_VALGRIND)
    signals_inout.sig_del (SIGRTMAX);     // 64
#endif
}

void
do_work (int argc_in,
         ACE_TCHAR** argv_in,
         bool clientMode_in,
         const ACE_INET_Addr& peerAddress_in,
#if !defined (ACE_WIN32) && !defined (ACE_WIN64)
         const Net_Netlink_Addr& netlinkAddress_in,
#endif
         bool useReactor_in,
         const std::string& interfaceDefinitionFile_in,
         ////////////////////////////////
         struct Olimex_Mod_MPU6050_GTK_CBData& CBData_in,
         ////////////////////////////////
         const ACE_Sig_Set& signalSet_in,
         const ACE_Sig_Set& ignoredSignalSet_in,
         Common_SignalActions_t& previousSignalActions_inout)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::do_work"));

  int result = false;

  // step1: initialize configuration data
  struct Olimex_Mod_MPU6050_Configuration configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Olimex_Mod_MPU6050_NetlinkConfiguration netlink_configuration;
#endif
  struct Olimex_Mod_MPU6050_UserData user_data;
  user_data.configuration = &configuration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Olimex_Mod_MPU6050_NetlinkUserData netlink_user_data;
  netlink_user_data.configuration = &netlink_configuration;
#endif

  Olimex_Mod_MPU6050_EventHandler event_handler (&CBData_in,
                                                 interfaceDefinitionFile_in.empty ());
  std::string module_name = ACE_TEXT_ALWAYS_CHAR ("MessageHandler");
  Olimex_Mod_MPU6050_Module_MessageHandler_Module event_handler_module (NULL,
                                                                        module_name.c_str ());

  Stream_AllocatorHeap_T<ACE_MT_SYNCH,
                         struct Stream_AllocatorConfiguration> heap_allocator;
  struct Stream_AllocatorConfiguration allocator_configuration;
  heap_allocator.initialize (allocator_configuration);
  Olimex_Mod_MPU6050_MessageAllocator_t message_allocator (OLIMEX_MOD_MPU6050_MAXIMUM_NUMBER_OF_INFLIGHT_MESSAGES,
                                                           &heap_allocator,
                                                           true); // block

  Olimex_Mod_MPU6050_ConnectionManager_t* connectionManager_p =
    OLIMEX_MOD_MPU6050_CONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (connectionManager_p);
  connectionManager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                   ACE_Time_Value::zero);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Olimex_Mod_MPU6050_NetlinkConnectionManager_t* netlinkConnectionManager_p =
    OLIMEX_MOD_MPU6050_NETLINKCONNECTIONMANAGER_SINGLETON::instance ();
  ACE_ASSERT (netlinkConnectionManager_p);
  if (!clientMode_in)
    netlinkConnectionManager_p->initialize (std::numeric_limits<unsigned int>::max (),
                                            ACE_Time_Value::zero);
#endif

  Olimex_Mod_MPU6050_Stream_t stream;
  Olimex_Mod_MPU6050_AsynchStream_t asynch_stream;

  // ******************* socket configuration data ****************************
  configuration.connectionConfiguration.allocatorConfiguration =
    &allocator_configuration;
  configuration.connectionConfiguration.socketConfiguration.peerAddress =
    peerAddress_in;
  configuration.connectionConfiguration.socketConfiguration.bufferSize =
    OLIMEX_MOD_MPU6050_SOCKET_RECEIVE_BUFFER_SIZE;
  configuration.connectionConfiguration.streamConfiguration =
    &configuration.streamConfiguration;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  netlink_configuration.connectionConfiguration.socketConfiguration.address =
    netlinkAddress_in;
  netlink_configuration.connectionConfiguration.socketConfiguration.protocol =
    OLIMEX_MOD_MPU6050_NETLINK_PROTOCOL;
  netlink_configuration.connectionConfiguration.streamConfiguration =
    &netlink_configuration.streamConfiguration;
#endif

  configuration.connectionConfiguration.messageAllocator =
    &message_allocator;
  configuration.connectionConfiguration.statisticReportingInterval =
    OLIMEX_MOD_MPU6050_STATISTICS_REPORTING_INTERVAL;
  configuration.connectionConfiguration.userData = &user_data;
  configuration.connectionConfiguration.useThreadPerConnection = false;

  // ******************** stream configuration data ***************************
  configuration.moduleHandlerConfiguration.connectionManager =
    connectionManager_p;
  configuration.moduleHandlerConfiguration.consoleMode =
    interfaceDefinitionFile_in.empty ();
  Stream_Net_StreamConnectionConfigurations_t connection_configurations_a;
  configuration.moduleHandlerConfiguration.connectionConfigurations =
    &connection_configurations_a;
  configuration.moduleHandlerConfiguration.stream = &stream;
  if (!useReactor_in)
    configuration.moduleHandlerConfiguration.stream = &asynch_stream;
  configuration.moduleHandlerConfiguration.subscriber = &event_handler;

  struct Olimex_Mod_MPU6050_StreamConfiguration stream_configuration_s;
  stream_configuration_s.messageAllocator = &message_allocator;
  stream_configuration_s.notificationStrategy = NULL;
  stream_configuration_s.module = &event_handler_module;
  stream_configuration_s.printFinalReport = false;
  configuration.streamConfiguration.initialize (configuration.moduleConfiguration,
                                                configuration.moduleHandlerConfiguration,
                                                stream_configuration_s);
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  struct Olimex_Mod_MPU6050_NetlinkStreamConfiguration netlink_stream_configuration_s;
  netlink_stream_configuration_s.messageAllocator = &message_allocator;
  netlink_stream_configuration_s.notificationStrategy = NULL;
  netlink_stream_configuration_s.module = &event_handler_module;
  netlink_stream_configuration_s.printFinalReport = false;
  netlink_configuration.streamConfiguration.initialize (netlink_configuration.moduleConfiguration,
                                                        netlink_configuration.moduleHandlerConfiguration,
                                                        netlink_stream_configuration_s);
#endif

  configuration.userData = &user_data;

  // step2: initialize connection manager
  struct Olimex_Mod_MPU6050_Configuration configuration_2;
  configuration_2.connectionConfiguration =
    configuration.connectionConfiguration;
  configuration_2.connectionConfiguration.streamConfiguration =
    &configuration_2.streamConfiguration;
  configuration_2.moduleHandlerConfiguration =
    configuration.moduleHandlerConfiguration;
  struct Olimex_Mod_MPU6050_StreamConfiguration stream_configuration_2 =
    stream_configuration_s;
  stream_configuration_2.module = NULL;
  configuration_2.streamConfiguration.initialize (configuration_2.moduleConfiguration,
                                                  configuration_2.moduleHandlerConfiguration,
                                                  stream_configuration_2);
  connectionManager_p->set (configuration_2.connectionConfiguration,
                            &user_data); // passed to all handlers
  connection_configurations_a.insert (std::make_pair (ACE_TEXT_ALWAYS_CHAR ("CamSource"),
                                                      &configuration_2.connectionConfiguration));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Olimex_Mod_MPU6050_NetlinkConfiguration netlink_configuration_2;
  netlink_configuration_2.connectionConfiguration =
    netlink_configuration.connectionConfiguration;
  netlink_configuration_2.connectionConfiguration.streamConfiguration =
    &netlink_configuration_2.streamConfiguration;
  netlink_configuration_2.moduleHandlerConfiguration =
    netlink_configuration.moduleHandlerConfiguration;
  struct Olimex_Mod_MPU6050_NetlinkStreamConfiguration netlink_stream_configuration_2 =
    netlink_stream_configuration_s;
  netlink_stream_configuration_2.module = NULL;
  netlink_configuration_2.streamConfiguration.initialize (netlink_configuration_2.moduleConfiguration,
                                                          netlink_configuration_2.moduleHandlerConfiguration,
                                                          netlink_stream_configuration_2);
  if (!clientMode_in)
  {
    netlinkConnectionManager_p->set (netlink_configuration_2.connectionConfiguration,
                                     &netlink_user_data); // passed to all handlers
  } // end IF
#endif

  // step3: initialize event dispatch
  struct Common_EventDispatchConfiguration dispatch_configuration_s;
  dispatch_configuration_s.dispatch =
    useReactor_in ? COMMON_EVENT_DISPATCH_REACTOR : COMMON_EVENT_DISPATCH_PROACTOR;
  dispatch_configuration_s.numberOfProactorThreads = useReactor_in ? 0 : 1;
  dispatch_configuration_s.numberOfReactorThreads = useReactor_in ? 1 : 0;
  if (!Common_Event_Tools::initializeEventDispatch (dispatch_configuration_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Tools::initializeEventDispatch(), returning\n")));
    return;
  } // end IF

  // step4: initialize signal handling
  struct Olimex_Mod_MPU6050_SignalHandlerConfiguration signalhandler_configuration;
  signalhandler_configuration.consoleMode = interfaceDefinitionFile_in.empty ();
  signalhandler_configuration.peerAddress = peerAddress_in;
  signalhandler_configuration.useReactor = useReactor_in;
  Olimex_Mod_MPU6050_SignalHandler signal_handler;
  if (!signal_handler.initialize (signalhandler_configuration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handler, returning\n")));
    return;
  } // end IF
  if (!Common_Signal_Tools::initialize (useReactor_in ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR,
                                        signalSet_in,
                                        ignoredSignalSet_in,
                                        &signal_handler,
                                        previousSignalActions_inout))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize signal handling, returning\n")));
    return;
  } // end IF

  // step5: start stream
  if (!configuration.moduleHandlerConfiguration.stream->initialize (configuration.streamConfiguration))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize processing stream, returning\n")));

    // clean up
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    delete iconnector_p;
//#else
//    if (clientMode_in)
//      delete iconnector_p;
//    else
//      delete netlink_iconnector_p;
//#endif

    return;
  } // end IF
  configuration.moduleHandlerConfiguration.stream->start ();
  if (!configuration.moduleHandlerConfiguration.stream->isRunning ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start processing stream, returning\n")));

    // clean up
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    delete iconnector_p;
//#else
//    if (clientMode_in)
//      delete iconnector_p;
//    else
//      delete netlink_iconnector_p;
//#endif

    return;
  } // end IF
  // *WARNING*: from this point on, clean up any remote connections !

  // step6: start event loop(s):
  // - catch SIGINT/SIGQUIT/SIGTERM/... signals (connect / perform orderly shutdown)
  // [- signal timer expiration to perform server queries] (see above)

  // step6a: start GTK event loop ?
  if (!interfaceDefinitionFile_in.empty ())
  {
    COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->start ();
    ACE_Time_Value delay (0,
                          OLIMEX_MOD_MPU6050_UI_INITIALIZATION_DELAY);
    result = ACE_OS::sleep (delay);
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_OS::sleep(%#T): \"%m\", continuing\n"),
                  &delay));
    if (!COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->isRunning ())
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to start GTK event dispatch, returning\n")));

      // clean up
      configuration.moduleHandlerConfiguration.stream->stop (true);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    delete iconnector_p;
//#else
//    if (clientMode_in)
//      delete iconnector_p;
//    else
//      delete netlink_iconnector_p;
//#endif

      return;
    } // end IF
  } // end IF

  // step6b: initialize worker(s)
  struct Common_EventDispatchState dispatch_state_s;
  dispatch_state_s.configuration = &dispatch_configuration_s;
  if (!Common_Event_Tools::startEventDispatch (dispatch_state_s))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to start event dispatch, returning\n")));
//		{ // synch access
//			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard  (CBData_in.lock);

//			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin ();
//					 iterator != CBData_in.event_source_ids.end ();
//					 iterator++)
//				g_source_remove (*iterator);
//		} // end lock scope
    if (!interfaceDefinitionFile_in.empty ())
      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();

    // clean up
    configuration.moduleHandlerConfiguration.stream->stop (true);
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//    delete iconnector_p;
//#else
//    if (clientMode_in)
//      delete iconnector_p;
//    else
//      delete netlink_iconnector_p;
//#endif

    return;
  } // end IF

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("started event dispatch...\n")));

//  // step7: connect
//  ACE_HANDLE handle = ACE_INVALID_HANDLE;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//  handle = iconnector_p->connect (peerAddress_in);
//#else
//  if (clientMode_in)
//    handle = iconnector_p->connect (peerAddress_in);
//  else
//    handle = netlink_iconnector_p->connect (netlinkAddress_in);
//#endif
//  if (handle == ACE_INVALID_HANDLE)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to connect, aborting\n")));
//    Common_Tools::finalizeEventDispatch (useReactor_in,
//                                         !useReactor_in,
//                                         group_id);
//    //		{ // synch access
//    //			ACE_Guard<ACE_Recursive_Thread_Mutex> aGuard  (CBData_in.lock);
//
//    //			for (Net_GTK_EventSourceIDsIterator_t iterator = CBData_in.event_source_ids.begin ();
//    //					 iterator != CBData_in.event_source_ids.end ();
//    //					 iterator++)
//    //				g_source_remove (*iterator);
//    //		} // end lock scope
//    if (!interfaceDefinitionFile_in.empty ())
//      COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->stop ();
//
//    // clean up
//    configuration.moduleHandlerConfiguration.stream->stop (true);
//    delete iconnector_p;
//#if defined (ACE_WIN32) || defined (ACE_WIN64)
//#else
//    if (clientMode_in)
//      delete iconnector_p;
//    else
//      delete netlink_iconnector_p;
//#endif
//
//    return;
//  }

  // step8: dispatch events
  Common_Event_Tools::dispatchEvents (dispatch_state_s);

  // step9: clean up
  configuration.moduleHandlerConfiguration.stream->stop (true);
//  connectionManager_p->stop ();
//  connectionManager_p->abort ();
//  connectionManager_p->wait ();
  //delete iconnector_p;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
//  netlinkConnectionManager_p->stop ();
//  netlinkConnectionManager_p->abort ();
//  netlinkConnectionManager_p->wait ();
//  if (!clientMode_in)
//    delete netlink_iconnector_p;
#endif
//  result = event_handler_module.close ();
//  if (result == -1)
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to ACE_Module::close(): \"%m\", continuing\n"),
//                event_handler_module.name ()));

  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("finished working...\n")));
}

int
ACE_TMAIN (int argc_in,
           ACE_TCHAR** argv_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::main"));

  int result = -1;

  // step0: initialize ACE
  // *PORTABILITY*: on Windows, ACE needs initialization...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::init ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::init(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif

#if defined (OLINUXINO_ENABLE_VALGRIND_SUPPORT)
  if (RUNNING_ON_VALGRIND)
    ACE_DEBUG ((LM_INFO,
                ACE_TEXT ("running on valgrind...\n")));
#endif

  // *PROCESS PROFILE*
  ACE_Profile_Timer process_profile;
  // start profile timer...
  process_profile.start ();

  // step1: process commandline options (if any)
  std::string configuration_path =
    Common_File_Tools::getWorkingDirectory ();
#if defined (DEBUG_DEBUGGER)
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("..");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("src");
  configuration_path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  configuration_path += ACE_TEXT_ALWAYS_CHAR ("test_u");
#endif // #ifdef DEBUG_DEBUGGER

  bool client_mode            = false;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  unsigned int netlink_group  = OLIMEX_MOD_MPU6050_DEFAULT_NETLINK_GROUP;
#endif
  bool log_to_file            = false;
  bool use_reactor            = OLIMEX_MOD_MPU6050_USE_REACTOR;
  ACE_INET_Addr peer_address (static_cast<u_short> (OLIMEX_MOD_MPU6050_DEFAULT_PORT),
                              static_cast<ACE_UINT32> (INADDR_ANY));
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  Net_Netlink_Addr netlink_address;
  pid_t pid = ACE_OS::getpid ();
  netlink_address.set (pid, OLIMEX_MOD_MPU6050_DEFAULT_NETLINK_GROUP);
#endif
  bool trace_information      = false;
  std::string path = configuration_path;
  path += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  path += ACE_TEXT_ALWAYS_CHAR (COMMON_LOCATION_CONFIGURATION_SUBDIRECTORY);
  std::string interface_definition_file = path;
  interface_definition_file += ACE_DIRECTORY_SEPARATOR_CHAR_A;
  interface_definition_file +=
    ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_DEFINITION_FILE_NAME);
  bool print_version_and_exit = false;
  if (!do_processArguments (argc_in,
                            argv_in,
                            client_mode,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
                            netlink_group,
#endif
                            log_to_file,
                            use_reactor,
                            peer_address,
                            trace_information,
                            interface_definition_file,
                            print_version_and_exit))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to do_processArguments(), aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, ACE needs finalization...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  netlink_address.set (pid, netlink_group);
#endif

  // step2: validate configuration
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  if ((!interface_definition_file.empty () && !Common_File_Tools::isReadable (interface_definition_file)))
#else
  if ((!interface_definition_file.empty () && !Common_File_Tools::isReadable (interface_definition_file)) ||
      (!netlink_group || (netlink_group > sizeof (unsigned int) * 8)))
#endif
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("invalid configuration, aborting\n")));

    do_printUsage (ACE::basename (argv_in[0]));

    // *PORTABILITY*: on Windows, ACE needs finalization...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
  else
    if (!client_mode)
      netlink_address.set (ACE_OS::getpid (),
                           (1 << (netlink_group - 1)));
#endif

  // step3: initialize logging and/or tracing
  ACE_TCHAR buffer_a[PATH_MAX];
  if (!ACE_OS::getcwd (buffer_a, sizeof (ACE_TCHAR[PATH_MAX])))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_OS::getcwd(): \"%m\", aborting\n")));

    // *PORTABILITY*: on Windows, ACE needs finalization...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  std::string log_file = ACE_TEXT_ALWAYS_CHAR (buffer_a);
  log_file += ACE_DIRECTORY_SEPARATOR_STR;
  log_file += ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_LOG_FILE_NAME);
  if (!log_to_file)
    log_file.clear ();
  if (!Common_Log_Tools::initialize (ACE::basename (argv_in[0]),
                                     log_file,
                                     false,
                                     trace_information,
                                     true,
                                     NULL))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to initialize logging, aborting\n")));

    // *PORTABILITY*: on Windows, ACE needs finalization...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step4: (pre-)initialize signal handling
  ACE_Sig_Set signal_set (false);
  ACE_Sig_Set ignored_signal_set (false);
  do_initializeSignals (use_reactor,
                        true,
                        signal_set,
                        ignored_signal_set);
  Common_SignalActions_t previous_signal_actions;
  ACE_Sig_Set previous_signal_mask (false);
  if (!Common_Signal_Tools::preInitialize (signal_set,
                                           use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR,
                                           true,
                                           false,
                                           previous_signal_actions,
                                           previous_signal_mask))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Common_Signal_Tools::preInitialize(), aborting\n")));

    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF

  // step4: initialize NLS
#ifdef ENABLE_NLS
#ifdef HAVE_LOCALE_H
  setlocale (LC_ALL, "");
#endif
  bindtextdomain (PACKAGE, LOCALEDIR);
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);
#endif

  // step5: initialize GTK UI
  struct Olimex_Mod_MPU6050_GTK_CBData gtk_cb_data;
  gtk_cb_data.clientMode = client_mode;
  ACE_OS::memset (&gtk_cb_data.clientSensorBias, 0, sizeof (struct SensorBias));
  gtk_cb_data.openGLDoubleBuffered =
    OLIMEX_MOD_MPU6050_OPENGL_DOUBLE_BUFFERED;
  ACE_OS::memset (gtk_cb_data.temperature, 0, sizeof (gfloat[OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE * 2]));

  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
  //CBData_in.UIState->gladeXML[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
  //  std::make_pair (interface_definition_file, static_cast<GladeXML*> (NULL));
  state_r.builders[ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN)] =
    std::make_pair (interface_definition_file, static_cast<GtkBuilder*> (NULL));
  gtk_cb_data.UIState = &state_r;

  Common_UI_GtkBuilderDefinition_t ui_definition;
  ui_definition.initialize (state_r);
  Common_UI_GTK_Configuration_t gtk_configuration;
  gtk_configuration.argc = argc_in;
  gtk_configuration.argv = argv_in;
  gtk_configuration.CBData = &gtk_cb_data;
  gtk_configuration.definition = &ui_definition;
  gtk_configuration.eventHooks.initHook = idle_initialize_ui_cb;
  gtk_configuration.eventHooks.finiHook = idle_finalize_ui_cb;
  COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->initialize (gtk_configuration);

  // step6: run program ?
  ACE_High_Res_Timer timer;
  timer.start ();
  if (print_version_and_exit)
    do_printVersion (ACE::basename (argv_in[0]));
  else
    do_work (argc_in,
             argv_in,
             client_mode,
             peer_address,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
             netlink_address,
#endif
             use_reactor,
             interface_definition_file,
             ////////////////////////////
             gtk_cb_data,
             ////////////////////////////
             signal_set,
             ignored_signal_set,
             previous_signal_actions);

  // debug info
  timer.stop ();
  ACE_Time_Value working_time;
  timer.elapsed_time (working_time);
  std::string working_time_string =
    Common_Timer_Tools::periodToString (working_time);
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("total working time (h:m:s.us): \"%s\"...\n"),
              ACE_TEXT (working_time_string.c_str ())));

  // debug info
  process_profile.stop ();
  ACE_Profile_Timer::ACE_Elapsed_Time elapsed_time;
  elapsed_time.real_time = 0.0;
  elapsed_time.user_time = 0.0;
  elapsed_time.system_time = 0.0;
  if (process_profile.elapsed_time (elapsed_time) == -1)
  {
    ACE_DEBUG ((LM_ERROR,
               ACE_TEXT ("failed to ACE_Profile_Timer::elapsed_time: \"%m\", aborting\n")));

    Common_Signal_Tools::finalize (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR,
                                   previous_signal_actions,
                                   previous_signal_mask);
    Common_Log_Tools::finalize ();
    // *PORTABILITY*: on Windows, fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
    result = ACE::fini ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE::fini(): \"%m\", continuing\n")));
#endif

    return EXIT_FAILURE;
  } // end IF
  ACE_Profile_Timer::Rusage elapsed_rusage;
  ACE_OS::memset (&elapsed_rusage, 0, sizeof (ACE_Profile_Timer::Rusage));
  process_profile.elapsed_rusage (elapsed_rusage);
  ACE_Time_Value user_time (elapsed_rusage.ru_utime);
  ACE_Time_Value system_time (elapsed_rusage.ru_stime);
  std::string user_time_string = Common_Timer_Tools::periodToString (user_time);
  std::string system_time_string =
    Common_Timer_Tools::periodToString (system_time);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              ACE_TEXT (user_time_string.c_str ()),
              ACE_TEXT (system_time_string.c_str ())));
#else
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT (" --> Process Profile <--\nreal time = %A seconds\nuser time = %A seconds\nsystem time = %A seconds\n --> Resource Usage <--\nuser time used: %s\nsystem time used: %s\nmaximum resident set size = %d\nintegral shared memory size = %d\nintegral unshared data size = %d\nintegral unshared stack size = %d\npage reclaims = %d\npage faults = %d\nswaps = %d\nblock input operations = %d\nblock output operations = %d\nmessages sent = %d\nmessages received = %d\nsignals received = %d\nvoluntary context switches = %d\ninvoluntary context switches = %d\n"),
              elapsed_time.real_time,
              elapsed_time.user_time,
              elapsed_time.system_time,
              user_time_string.c_str (),
              system_time_string.c_str (),
              elapsed_rusage.ru_maxrss,
              elapsed_rusage.ru_ixrss,
              elapsed_rusage.ru_idrss,
              elapsed_rusage.ru_isrss,
              elapsed_rusage.ru_minflt,
              elapsed_rusage.ru_majflt,
              elapsed_rusage.ru_nswap,
              elapsed_rusage.ru_inblock,
              elapsed_rusage.ru_oublock,
              elapsed_rusage.ru_msgsnd,
              elapsed_rusage.ru_msgrcv,
              elapsed_rusage.ru_nsignals,
              elapsed_rusage.ru_nvcsw,
              elapsed_rusage.ru_nivcsw));
#endif

  // step6: clean up
  Common_Signal_Tools::finalize (use_reactor ? COMMON_SIGNAL_DISPATCH_REACTOR : COMMON_SIGNAL_DISPATCH_PROACTOR,
                                 previous_signal_actions,
                                 previous_signal_mask);
  Common_Log_Tools::finalize ();

  // *PORTABILITY*: on Windows, must fini ACE...
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE::fini ();
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE::fini(): \"%m\", aborting\n")));
    return EXIT_FAILURE;
  } // end IF
#endif

  return EXIT_SUCCESS;
} // end main
