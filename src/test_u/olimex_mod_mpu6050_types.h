/*  I2C kernel module driver for the Olimex MOD-MPU6050 UEXT module
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

#ifndef OLIMEX_MOD_MPU6050_TYPES_H
#define OLIMEX_MOD_MPU6050_TYPES_H

#include <deque>
#include <list>

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#include "GL/gl.h"
#include "glm/glm.hpp"

#include "ace/Global_Macros.h"
#include "ace/OS.h"
#include "ace/Synch_Traits.h"
#include "ace/Time_Value.h"

#include "gtk/gtk.h"
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
//#else
//#error GTK 3 supports OpenGL natively from 3.16.0 onward; for earlier versions, try gtkglarea (see: git://git.gnome.org/gtkglarea)
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,16,0) */
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gdkgl.h"
#include "gtkgl/gtkglarea.h"
#endif /* GTKGLAREA_SUPPORT */
#else
#include "gdk/gdkgl.h"
#endif /* GTK_CHECK_VERSION (x,0,0) */
// #include "glade/glade.h"

#include "common_gl_shader.h"

#include "common_time_common.h"

#include "common_ui_gtk_common.h"

#include "stream_base.h"
#include "stream_common.h"
#include "stream_control_message.h"
#include "stream_isessionnotify.h"
#include "stream_session_manager.h"
#include "stream_statemachine_control.h"

#include "net_common.h"
#include "net_connection_configuration.h"
#include "net_iconnection.h"
#include "net_iconnectionmanager.h"
#include "net_iconnector.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "net_netlinksockethandler.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "olimex_mod_mpu6050_defines.h"
#include "olimex_mod_mpu6050_stream_common.h"
#include "olimex_mod_mpu6050_sessionmessage.h"

// forward declarations
class Olimex_Mod_MPU6050_Message;
class Stream_IAllocator;
struct Olimex_Mod_MPU6050_SessionData;

enum Olimex_Mod_MPU6050_Event
{
  OLIMEX_MOD_MPU6050_EVENT_INVALID = -1,
  OLIMEX_MOD_MPU6050_EVENT_CONNECT,
  OLIMEX_MOD_MPU6050_EVENT_DISCONNECT,
  OLIMEX_MOD_MPU6050_EVENT_MESSAGE,
  OLIMEX_MOD_MPU6050_EVENT_SESSION_MESSAGE,
  ///////////////////////////////////////
  OLIMEX_MOD_MPU6050_EVENT_MAX
};
typedef std::deque<Olimex_Mod_MPU6050_Event> Olimex_Mod_MPU6050_Events_t;
typedef Olimex_Mod_MPU6050_Events_t::const_iterator Olimex_Mod_MPU6050_EventsIterator_t;

typedef std::deque<Olimex_Mod_MPU6050_Message*> Olimex_Mod_MPU6050_Messages_t;
typedef Olimex_Mod_MPU6050_Messages_t::const_iterator Olimex_Mod_MPU6050_MessagesIterator_t;

typedef Stream_ControlMessage_T<enum Stream_ControlType,
                                enum Stream_ControlMessageType> Olimex_Mod_MPU6050_ControlMessage_t;

typedef Stream_MessageAllocatorHeapBase_T<ACE_MT_SYNCH,
                                          struct Stream_AllocatorConfiguration,
                                          Olimex_Mod_MPU6050_ControlMessage_t,
                                          Olimex_Mod_MPU6050_Message,
                                          Olimex_Mod_MPU6050_SessionMessage> Olimex_Mod_MPU6050_MessageAllocator_t;

typedef Stream_INotify_T<Stream_SessionMessageType> Olimex_Mod_MPU6050_IStreamNotify_t;
typedef Stream_ISessionDataNotify_T<struct Olimex_Mod_MPU6050_SessionData,
                                    enum Stream_SessionMessageType,
                                    Olimex_Mod_MPU6050_Message,
                                    Olimex_Mod_MPU6050_SessionMessage> Olimex_Mod_MPU6050_Notification_t;
typedef std::list<Olimex_Mod_MPU6050_Notification_t*> Olimex_Mod_MPU6050_Subscribers_t;
typedef Olimex_Mod_MPU6050_Subscribers_t::iterator Olimex_Mod_MPU6050_SubscribersIterator_t;

struct SensorBias
{
  gfloat ax_bias;
  gfloat ay_bias;
  gfloat az_bias;
  gfloat gx_bias;
  gfloat gy_bias;
  gfloat gz_bias;
};

struct Camera
{
  //glm::vec3 position;
  //glm::vec3 looking_at;
  //glm::vec3 up;

  float zoom;
  glm::vec3 rotation;
  glm::vec3 translation;
  int last[2];
};

struct Olimex_Mod_MPU6050_Configuration;
struct Olimex_Mod_MPU6050_UserData
 : Stream_UserData
{
  Olimex_Mod_MPU6050_UserData ()
   : Stream_UserData ()
   , configuration (NULL)
  {}

  Olimex_Mod_MPU6050_Configuration* configuration;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Olimex_Mod_MPU6050_NetlinkConfiguration;
struct Olimex_Mod_MPU6050_NetlinkUserData
 : Olimex_Mod_MPU6050_UserData
{
  Olimex_Mod_MPU6050_NetlinkUserData ()
   : Olimex_Mod_MPU6050_UserData ()
   , configuration (NULL)
  {}

  Olimex_Mod_MPU6050_NetlinkConfiguration* configuration;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Olimex_Mod_MPU6050_Configuration;
typedef Stream_Statistic Olimex_Mod_MPU6050_RuntimeStatistic_t;
struct Olimex_Mod_MPU6050_ConnectionState
 : Net_StreamConnectionState
{
  Olimex_Mod_MPU6050_ConnectionState ()
   : Net_StreamConnectionState ()
   , configuration (NULL)
   , status (NET_CONNECTION_STATUS_INVALID)
   , currentStatistic ()
   , userData (NULL)
  {}

  // *TODO*: consider making this a separate entity (i.e. a pointer)
  Olimex_Mod_MPU6050_Configuration*     configuration;

  Net_Connection_Status                 status;

  Olimex_Mod_MPU6050_RuntimeStatistic_t currentStatistic;

  Olimex_Mod_MPU6050_UserData*          userData;
};

struct Olimex_Mod_MPU6050_Configuration;
struct Olimex_Mod_MPU6050_ConnectionState;
typedef Net_IConnection_T<ACE_INET_Addr,
                          struct Olimex_Mod_MPU6050_ConnectionState,
                          Net_StreamStatistic_t> Olimex_Mod_MPU6050_IConnection_t;
struct Olimex_Mod_MPU6050_NetlinkConfiguration;
struct Olimex_Mod_MPU6050_NetlinkConnectionState;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_IConnection_T<Net_Netlink_Addr,
                          struct Olimex_Mod_MPU6050_NetlinkConnectionState,
                          Net_StreamStatistic_t> Olimex_Mod_MPU6050_INetlinkConnection_t;
#endif

typedef Stream_Configuration_T<//stream_name_string_,
                               struct Olimex_Mod_MPU6050_StreamConfiguration,
                               struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration> Olimex_Mod_MPU6050_StreamConfiguration_t;
class Olimex_Mod_MPU6050_ConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<Olimex_Mod_MPU6050_StreamConfiguration_t,
                                              NET_TRANSPORTLAYER_UDP>
{
 public:
  Olimex_Mod_MPU6050_ConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T<Olimex_Mod_MPU6050_StreamConfiguration_t,
                                         NET_TRANSPORTLAYER_UDP> ()
   , userData (NULL)
  {}

  Olimex_Mod_MPU6050_UserData* userData;
};

typedef Net_IConnector_T<ACE_INET_Addr,
                         Olimex_Mod_MPU6050_ConnectionConfiguration> Olimex_Mod_MPU6050_IConnector_t;
typedef Net_IAsynchConnector_T<ACE_INET_Addr,
                               Olimex_Mod_MPU6050_ConnectionConfiguration> Olimex_Mod_MPU6050_IAsynchConnector_t;
struct Olimex_Mod_MPU6050_StreamState;
struct Olimex_Mod_MPU6050_StreamConfiguration;
struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration;
struct Olimex_Mod_MPU6050_SessionData;
typedef Stream_SessionData_T<struct Olimex_Mod_MPU6050_SessionData> Olimex_Mod_MPU6050_StreamSessionData_t;
extern const char stream_name_string_[];
typedef Stream_Session_Manager_T<ACE_MT_SYNCH,
                                 enum Stream_SessionMessageType,
                                 struct Stream_SessionManager_Configuration,
                                 struct Olimex_Mod_MPU6050_SessionData,
                                 Olimex_Mod_MPU6050_RuntimeStatistic_t,
                                 struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_SessionManager_t;
typedef Stream_Base_T<ACE_MT_SYNCH,
                      Common_TimePolicy_t,
                      stream_name_string_,
                      enum Stream_ControlType,
                      enum Stream_SessionMessageType,
                      enum Stream_StateMachine_ControlState,
                      struct Olimex_Mod_MPU6050_StreamState,
                      struct Olimex_Mod_MPU6050_StreamConfiguration,
                      Olimex_Mod_MPU6050_RuntimeStatistic_t,
                      struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration,
                      Olimex_Mod_MPU6050_SessionManager_t,
                      Olimex_Mod_MPU6050_ControlMessage_t,
                      Olimex_Mod_MPU6050_Message,
                      Olimex_Mod_MPU6050_SessionMessage,
                      struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_StreamBase_t;
struct Olimex_Mod_MPU6050_SignalHandlerConfiguration
 : Common_SignalHandlerConfiguration
{
  Olimex_Mod_MPU6050_SignalHandlerConfiguration ()
   : Common_SignalHandlerConfiguration ()
   , actionTimerID (-1)
   , consoleMode (false)
   , interfaceHandle (NULL)
   , peerAddress ()
   , stream (NULL)
   , useReactor (false)
  {}

  long                             actionTimerID;
  bool                             consoleMode;
  Olimex_Mod_MPU6050_IConnector_t* interfaceHandle;
  ACE_INET_Addr                    peerAddress;
  Olimex_Mod_MPU6050_StreamBase_t* stream;
  bool                             useReactor;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Stream_Configuration_T<//stream_name_string_,
                               struct Olimex_Mod_MPU6050_NetlinkStreamConfiguration,
                               struct Olimex_Mod_MPU6050_NetlinkModuleHandlerConfiguration> Olimex_Mod_MPU6050_NetlinkStreamConfiguration_t;
class Olimex_Mod_MPU6050_NetlinkConnectionConfiguration
 : public Net_StreamConnectionConfiguration_T<Olimex_Mod_MPU6050_NetlinkStreamConfiguration_t,
                                              NET_TRANSPORTLAYER_NETLINK>
{
 public:
  Olimex_Mod_MPU6050_NetlinkConnectionConfiguration ()
   : Net_StreamConnectionConfiguration_T<Olimex_Mod_MPU6050_NetlinkStreamConfiguration_t,
                                         NET_TRANSPORTLAYER_NETLINK> ()
   , userData (NULL)
  {}

  Olimex_Mod_MPU6050_NetlinkUserData* userData;
};
#endif // ACE_WIN32 || ACE_WIN64

typedef Net_IConnectionManager_T<ACE_INET_Addr,
                                 Olimex_Mod_MPU6050_ConnectionConfiguration,
                                 struct Olimex_Mod_MPU6050_ConnectionState,
                                 Net_StreamStatistic_t,
                                 //////
                                 struct Olimex_Mod_MPU6050_UserData> Olimex_Mod_MPU6050_IConnectionManager_t;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
typedef Net_IConnectionManager_T<Net_Netlink_Addr,
                                 Olimex_Mod_MPU6050_NetlinkConnectionConfiguration,
                                 struct Olimex_Mod_MPU6050_NetlinkConnectionState,
                                 Net_StreamStatistic_t,
                                 //////
                                 struct Olimex_Mod_MPU6050_NetlinkUserData> Olimex_Mod_MPU6050_INetlinkConnectionManager_t;
#endif

class Olimex_Mod_MPU6050_EventHandler;
struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration
 : Stream_ModuleHandlerConfiguration
{
  Olimex_Mod_MPU6050_ModuleHandlerConfiguration ()
   : Stream_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfigurations (NULL)
   , connectionManager (NULL)
   , consoleMode (false)
   , inbound (true)
   , passive (false)
   , printFinalReport (true)
   , pushStatisticMessages (true)
   , stream (NULL)
   , subscriber (NULL)
  {}

  Olimex_Mod_MPU6050_IConnection_t*               connection; // UDP source/IO module
  Stream_Net_StreamConnectionConfigurations_t*    connectionConfigurations;
  Olimex_Mod_MPU6050_IConnectionManager_t*        connectionManager; // UDP IO module

  bool                                            consoleMode;
  bool                                            inbound;
  bool                                            passive;
  bool                                            printFinalReport;
  bool                                            pushStatisticMessages;
  Olimex_Mod_MPU6050_StreamBase_t*                stream;
  Olimex_Mod_MPU6050_Notification_t*              subscriber;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Olimex_Mod_MPU6050_NetlinkModuleHandlerConfiguration
 : Olimex_Mod_MPU6050_ModuleHandlerConfiguration
{
  Olimex_Mod_MPU6050_NetlinkModuleHandlerConfiguration ()
   : Olimex_Mod_MPU6050_ModuleHandlerConfiguration ()
   , connection (NULL)
   , connectionConfiguration (NULL)
   , connectionManager (NULL)
  {}

  Olimex_Mod_MPU6050_INetlinkConnection_t*           connection; // Netlink source/IO module
  Olimex_Mod_MPU6050_NetlinkConnectionConfiguration* connectionConfiguration;
  Olimex_Mod_MPU6050_INetlinkConnectionManager_t*    connectionManager; // Netlink IO module

};
#endif // ACE_WIN32 || ACE_WIN64

struct Olimex_Mod_MPU6050_StreamConfiguration
 : Stream_Configuration
{
  Olimex_Mod_MPU6050_StreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {}

  struct Olimex_Mod_MPU6050_UserData* userData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Olimex_Mod_MPU6050_NetlinkStreamConfiguration
 : Stream_Configuration
{
  Olimex_Mod_MPU6050_NetlinkStreamConfiguration ()
   : Stream_Configuration ()
   , userData (NULL)
  {}

  struct Olimex_Mod_MPU6050_NetlinkUserData* userData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Olimex_Mod_MPU6050_Configuration
{
  Olimex_Mod_MPU6050_Configuration ()
   : connectionConfiguration ()
   , moduleConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData (NULL)
  {}

  Olimex_Mod_MPU6050_ConnectionConfiguration               connectionConfiguration;

  struct Stream_ModuleConfiguration                        moduleConfiguration;
  struct Olimex_Mod_MPU6050_ModuleHandlerConfiguration     moduleHandlerConfiguration;
  Olimex_Mod_MPU6050_StreamConfiguration_t                 streamConfiguration;

  struct Olimex_Mod_MPU6050_UserData*                      userData;
};

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
struct Olimex_Mod_MPU6050_NetlinkConfiguration
 : Olimex_Mod_MPU6050_Configuration
{
  Olimex_Mod_MPU6050_NetlinkConfiguration ()
   : Olimex_Mod_MPU6050_Configuration ()
   , connectionConfiguration ()
   , moduleHandlerConfiguration ()
   , streamConfiguration ()
   , userData (NULL)
  {}

  Olimex_Mod_MPU6050_NetlinkConnectionConfiguration           connectionConfiguration;

  struct Olimex_Mod_MPU6050_NetlinkModuleHandlerConfiguration moduleHandlerConfiguration;
  Olimex_Mod_MPU6050_NetlinkStreamConfiguration_t             streamConfiguration;

  struct Olimex_Mod_MPU6050_NetlinkUserData*                  userData;
};
#endif // ACE_WIN32 || ACE_WIN64

struct Olimex_Mod_MPU6050_GTK_CBData
 : Common_UI_GTK_CBData
{
  Olimex_Mod_MPU6050_GTK_CBData ()
   : Common_UI_GTK_CBData ()
   , clientMode (false)
   , contextIdData (0)
   , contextIdInformation (0)
   , eventQueue ()
   , frameCounter (0)
   , messageQueue ()
#if defined (GTKGL_SUPPORT)
   , OpenGLCamera ()
   , OpenGLContext (NULL)
#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
   , openGLAxesListId (0)
#endif // GTK_CHECK_VERSION (x,0,0)
   , OpenGLTextureId (0)
   , VAO (0)
   , VBO (0)
   , EBO (0)
   , shader ()
   , OpenGLRefreshId (0)
   , OpenGLDoubleBuffered (OLIMEX_MOD_MPU6050_OPENGL_DOUBLE_BUFFERED)
#endif // GTKGL_SUPPORT
   , temperature ()
   , temperatureIndex (-1)
   , timestamp (ACE_Time_Value::zero)
  {
    resetCamera ();
  }

#if defined (GTKGL_SUPPORT)
  void resetCamera ()
  {
    ACE_OS::memset (&OpenGLCamera, 0, sizeof (struct Camera));
    OpenGLCamera.zoom = OLIMEX_MOD_MPU6050_OPENGL_CAMERA_DEFAULT_ZOOM;
  }
#endif /* GTKGL_SUPPORT */

  bool                          clientMode;
  // *NOTE*: on the host ("server"), use the device bias registers instead !
  // *TODO*: implement a client->server protocol to do this
  struct SensorBias             clientSensorBias; // client side ONLY (!)
  guint                         contextIdData; // status bar context
  guint                         contextIdInformation; // status bar context
  Olimex_Mod_MPU6050_Events_t   eventQueue;
  unsigned int                  frameCounter;
  Olimex_Mod_MPU6050_Messages_t messageQueue;
#if defined (GTKGL_SUPPORT)
  struct Camera                 OpenGLCamera;
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,24,1)
  GdkGLContext*                 OpenGLContext;
#elif GTK_CHECK_VERSION (3,16,0)
  GdkGLContext*                 OpenGLContext;
#else
#if defined (GTKGLAREA_SUPPORT)
  GglaContext*                  OpenGLContext;
#else
  gpointer                      OpenGLContext;
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,24,1) */
#elif GTK_CHECK_VERSION (2,0,0)
  GLuint                        OpenGLAxesListId;
#if defined (GTKGLAREA_SUPPORT)
  GdkGLContext*                 OpenGLContext;
#else
  gpointer                      OpenGLContext;
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
  GLuint                        OpenGLTextureId;
  GLuint                        VAO;
  GLuint                        VBO;
  GLuint                        EBO;
  Common_GL_Shader              shader;
  guint                         OpenGLRefreshId;
  bool                          OpenGLDoubleBuffered;
#endif /* GTKGL_SUPPORT */
  gfloat                        temperature[OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE * 2];
  int                           temperatureIndex;
  ACE_Time_Value                timestamp;
};

#endif // #ifndef OLIMEX_MOD_MPU6050_TYPES_H
