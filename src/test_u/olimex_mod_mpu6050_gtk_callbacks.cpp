/***************************************************************************
 *   Copyright (C) 2010 by Erik Sohns   *
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

#include "olimex_mod_mpu6050_gtk_callbacks.h"

#include <cmath>
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "math.h"
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Log_Msg.h"

#if defined (GLEW_SUPPORT)
#include "GL/glew.h"
#endif // GLEW_SUPPORT
#include "GL/gl.h"
#include "glm/glm.hpp"
#include "GL/glu.h"
#include "GL/glut.h"

#include "gmodule.h"

#include "gdk/gdkkeysyms.h"

#include "common_timer_manager.h"

#include "common_ui_defines.h"

#include "common_ui_gtk_manager_common.h"

#include "olimex_mod_mpu6050_common.h"
#include "olimex_mod_mpu6050_defines.h"
#include "olimex_mod_mpu6050_gtk_gl_callbacks.h"
#include "olimex_mod_mpu6050_macros.h"
#include "olimex_mod_mpu6050_message.h"
#include "olimex_mod_mpu6050_opengl.h"
#include "olimex_mod_mpu6050_types.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */
G_MODULE_EXPORT gboolean
idle_initialize_ui_cb (gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::idle_initialize_ui_cb"));

  // sanity check(s)
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
    static_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->UIState);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  GtkWindow* main_window_p =
    GTK_WINDOW (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_WINDOW_MAIN)));
  ACE_ASSERT (main_window_p);
  gtk_window_set_default_size (main_window_p,
                               OLIMEX_MOD_MPU6050_DEFAULT_UI_WIDGET_WINDOW_MAIN_SIZE_WIDTH,
                               OLIMEX_MOD_MPU6050_DEFAULT_UI_WIDGET_WINDOW_MAIN_SIZE_HEIGHT);

  GtkDialog* about_dialog_p =
    GTK_DIALOG (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DIALOG_ABOUT)));
  ACE_ASSERT (about_dialog_p);

  //if (cb_data_p->clientMode)
  //{
  //  GtkImageMenuItem* menu_item_p =
  //    GTK_IMAGE_MENU_ITEM (gtk_builder_get_object ((*iterator).second.second,
  //                                               ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_MENU_VIEW_CALIBRATE)));
  //  ACE_ASSERT (menu_item_p);
  //  gtk_widget_set_sensitive (GTK_WIDGET (menu_item_p), FALSE);
  //} // end IF

  GtkWidget* drawing_area_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DRAWING_AREA)));
  ACE_ASSERT (drawing_area_p);

#if GTK_CHECK_VERSION (3,0,0)
#else
   GtkCurve* curve_p =
     GTK_CURVE (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_CURVE)));
   ACE_ASSERT (curve_p);
   // *WARNING*: glade/gtk crashes when declaring the curve type to be anything
   //            other than GTK_CURVE_TYPE_SPLINE (default)
   //gtk_curve_set_curve_type (curve_p, GTK_CURVE_TYPE_FREE);
   gtk_curve_set_range (curve_p,
                        0.0F,
                        static_cast <gfloat> (OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE - 1),
                        0.0F,
                        100.0F);
   gtk_curve_set_vector (curve_p,
                         OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE,
                         cb_data_p->temperature);
#endif // GTK_CHECK_VERSION (3,0,0)

  GtkStatusbar* status_bar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_STATUS_BAR)));
  ACE_ASSERT (status_bar_p);
  cb_data_p->contextIdData =
      gtk_statusbar_get_context_id (status_bar_p,
                                    ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_STATUS_BAR_CONTEXT_DATA));
  cb_data_p->contextIdInformation =
    gtk_statusbar_get_context_id (status_bar_p,
                                  ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_STATUS_BAR_CONTEXT_INFORMATION));

#if defined (GTKGL_SUPPORT)
  GtkBox* box_p = NULL;
  Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator;
  Common_UI_GTK_State_t& state_r =
    const_cast<Common_UI_GTK_State_t&> (COMMON_UI_GTK_MANAGER_SINGLETON::instance ()->getR ());
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea* gl_area_p = NULL;
  GdkGLContext* gl_context_p = NULL;
  GError* error_p = NULL;
#endif // GTK_CHECK_VERSION (3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GDK_GL_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int gl_attributes_a[] = {GDK_GL_USE_GL,
                           // GDK_GL_BUFFER_SIZE
                           // GDK_GL_LEVEL
                           GDK_GL_RGBA, GDK_GL_DOUBLEBUFFER,
                           //    GDK_GL_STEREO
                           //    GDK_GL_AUX_BUFFERS
                           GDK_GL_RED_SIZE, 1, GDK_GL_GREEN_SIZE, 1,
                           GDK_GL_BLUE_SIZE, 1, GDK_GL_ALPHA_SIZE, 1,
                           //    GDK_GL_DEPTH_SIZE
                           //    GDK_GL_STENCIL_SIZE
                           //    GDK_GL_ACCUM_RED_SIZE
                           //    GDK_GL_ACCUM_GREEN_SIZE
                           //    GDK_GL_ACCUM_BLUE_SIZE
                           //    GDK_GL_ACCUM_ALPHA_SIZE
                           //
                           //    GDK_GL_X_VISUAL_TYPE_EXT
                           //    GDK_GL_TRANSPARENT_TYPE_EXT
                           //    GDK_GL_TRANSPARENT_INDEX_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_RED_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_GREEN_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_BLUE_VALUE_EXT
                           //    GDK_GL_TRANSPARENT_ALPHA_VALUE_EXT
                           GDK_GL_NONE};
  GtkGLArea* gl_area_p = NULL;
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,0,0)
#endif // GTKGL_SUPPORT

#if defined (GTKGL_SUPPORT)
  box_p =
    GTK_BOX (gtk_builder_get_object ((*iterator).second.second,
                                     ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_OPENGL_PARENT)));
  ACE_ASSERT (box_p);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  gl_area_p = GTK_GL_AREA (gtk_gl_area_new ());
  ACE_ASSERT (gl_area_p);
  // gtk_widget_realize (GTK_WIDGET (gl_area_p));
  gl_context_p = gtk_gl_area_get_context (gl_area_p);
  //ACE_ASSERT (gl_context_p);
  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p, gl_context_p));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);

  gint major_version, minor_version;
  gtk_gl_area_get_required_version (gl_area_p,
                                    &major_version,
                                    &minor_version);
#else
#if defined (GTKGLAREA_SUPPORT)
  /* Attribute list for gtkglarea widget. Specifies a
     list of Boolean attributes and enum/integer
     attribute/value pairs. The last attribute must be
     GGLA_NONE. See glXChooseVisual manpage for further
     explanation.
  */
  int attribute_list[] = {
    GGLA_RGBA,
    GGLA_RED_SIZE,   1,
    GGLA_GREEN_SIZE, 1,
    GGLA_BLUE_SIZE,  1,
    GGLA_DOUBLEBUFFER,
    GGLA_NONE
  };

  GglaArea* gl_area_p = GGLA_AREA (ggla_area_new (attribute_list));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to ggla_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                 gl_area_p->glcontext));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);
#else
  ACE_ASSERT (false);
  ACE_NOTSUP_RETURN (G_SOURCE_REMOVE);
  ACE_NOTREACHED (return G_SOURCE_REMOVE;)
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  gl_area_p =
    GTK_GL_AREA (gtk_gl_area_new (gl_attributes_a));
  if (!gl_area_p)
  {
    ACE_DEBUG ((LM_CRITICAL,
                ACE_TEXT ("failed to gtk_gl_area_new(), aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  state_r.OpenGLContexts.insert (std::make_pair (gl_area_p,
                                                 gl_area_p->glcontext));
  opengl_contexts_iterator = state_r.OpenGLContexts.find (gl_area_p);
#else
  GdkGLConfigMode features = static_cast<GdkGLConfigMode> (GDK_GL_MODE_DOUBLE  |
                                                           GDK_GL_MODE_ALPHA   |
                                                           GDK_GL_MODE_DEPTH   |
                                                           GDK_GL_MODE_STENCIL |
                                                           GDK_GL_MODE_ACCUM);
  GdkGLConfigMode configuration_mode =
    static_cast<GdkGLConfigMode> (GDK_GL_MODE_RGBA | features);
  GdkGLConfig* gl_config_p = gdk_gl_config_new_by_mode (configuration_mode);
  if (!gl_config_p)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gdk_gl_config_new_by_mode(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF

  if (!gtk_widget_set_gl_capability (GTK_WIDGET (drawing_area_2), // widget
                                     gl_config_p,                 // configuration
                                     NULL,                        // share list
                                     true,                        // direct
                                     GDK_GL_RGBA_TYPE))           // render type
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_widget_set_gl_capability(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  state_r.OpenGLContexts.insert (std::make_pair (gtk_widget_get_window (GTK_WIDGET (drawing_area_2)),
                                                 gl_config_p));
  opengl_contexts_iterator = ui_cb_data_base_p->UIState.OpenGLContexts.find (gl_area_p);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
  ACE_ASSERT (opengl_contexts_iterator != state_r.OpenGLContexts.end ());

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  drawing_area_p = GTK_WIDGET (gl_area_p);
  gtk_widget_set_events (GTK_WIDGET (gl_area_p),
                         GDK_EXPOSURE_MASK       |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON1_MOTION_MASK |
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_KEY_PRESS_MASK      |
                         GDK_STRUCTURE_MASK);
#else
#if defined (GTKGLAREA_SUPPORT)
  drawing_area_p = GTK_WIDGET (gl_area_p);
  gtk_widget_set_events (drawing_area_p,
                         GDK_EXPOSURE_MASK       |
                         GDK_POINTER_MOTION_MASK |
                         GDK_BUTTON1_MOTION_MASK |
                         GDK_BUTTON_PRESS_MASK   |
                         GDK_KEY_PRESS_MASK      |
                         GDK_STRUCTURE_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else
#if defined (GTKGLAREA_SUPPORT)
  gtk_widget_set_events (GTK_WIDGET ((*opengl_contexts_iterator).first),
                         GDK_EXPOSURE_MASK |
                         GDK_BUTTON_PRESS_MASK);
#endif // GTKGLAREA_SUPPORT
#endif /* GTK_CHECK_VERSION (3,0,0) */

#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  // *NOTE*: (try to) enable legacy mode on Win32
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  gtk_gl_area_set_required_version ((*opengl_contexts_iterator).first, 2, 1);
#endif // ACE_WIN32 || ACE_WIN64
  gtk_gl_area_set_use_es ((*opengl_contexts_iterator).first, FALSE);
  // *WARNING*: the 'renderbuffer' (in place of 'texture') image attachment
  //            concept appears to be broken; setting this to 'false' gives
  //            "fb setup not supported" (see: gtkglarea.c:734)
  // *TODO*: more specifically, glCheckFramebufferStatusEXT() returns
  //         GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT; find out what is
  //         going on
  // *TODO*: the depth buffer feature is broken on Win32
  gtk_gl_area_set_has_alpha ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_depth_buffer ((*opengl_contexts_iterator).first, TRUE);
  gtk_gl_area_set_has_stencil_buffer ((*opengl_contexts_iterator).first, FALSE);
  gtk_gl_area_set_auto_render ((*opengl_contexts_iterator).first, TRUE);
  gtk_widget_set_can_focus (GTK_WIDGET ((*opengl_contexts_iterator).first), FALSE);
  gtk_widget_set_hexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
  gtk_widget_set_visible (GTK_WIDGET ((*opengl_contexts_iterator).first), TRUE);
#endif /* GTK_CHECK_VERSION (3,16,0) */
#endif /* GTK_CHECK_VERSION (3,0,0) */

  GtkDrawingArea* drawing_area_2 =
    GTK_DRAWING_AREA (gtk_builder_get_object ((*iterator).second.second,
                                              ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DRAWING_AREA)));
  ACE_ASSERT (drawing_area_2);
  //GList* children_p, *iterator_p;
  //children_p = gtk_container_get_children (GTK_CONTAINER (box_p));
  //for (iterator_p = children_p;
  //     iterator_p != NULL;
  //     iterator_p = g_list_next (iterator_p))
  //  if (GTK_WIDGET (iterator_p->data) == GTK_WIDGET (drawing_area_2))
  //  {
  //    gtk_widget_destroy (GTK_WIDGET (iterator_p->data));
  //    break;
  //  } // end IF
  gtk_widget_destroy (GTK_WIDGET (drawing_area_2));
  gtk_box_pack_start (box_p,
                      GTK_WIDGET ((*opengl_contexts_iterator).first),
                      TRUE, // expand
                      TRUE, // fill
                      0);   // padding
#if GTK_CHECK_VERSION (3,8,0)
//  gtk_builder_expose_object ((*iterator).second.second,
//                             ACE_TEXT_ALWAYS_CHAR (TEST_U_STREAM_UI_GTK_GLAREA_3D_NAME),
//                             G_OBJECT ((*opengl_contexts_iterator).first));
#endif /* GTK_CHECK_VERSION (3,8,0) */
#endif /* GTKGL_SUPPORT */

  // step2: (auto-)connect signals/slots
  // *NOTE*: glade_xml_signal_autoconnect doesn't work reliably
  //glade_xml_signal_autoconnect(xml);
  gtk_builder_connect_signals ((*iterator).second.second,
                               userData_in);

  // step2a: connect default signals
  // *NOTE*: glade_xml_signal_connect_data doesn't work reliably
//  g_signal_connect_swapped (G_OBJECT (main_window_p),
//                            ACE_TEXT_ALWAYS_CHAR ("destroy"),
//                            G_CALLBACK (gtk_main_quit),
//                            NULL);
  g_signal_connect (G_OBJECT (main_window_p),
                    ACE_TEXT_ALWAYS_CHAR ("destroy"),
                    G_CALLBACK (quit_clicked_gtk_cb),
                    cb_data_p);
//                   G_CALLBACK(gtk_widget_destroyed),
//                   &main_dialog_p,
//  g_signal_connect (G_OBJECT (main_window_p),
//                    ACE_TEXT_ALWAYS_CHAR ("delete-event"),
//                    G_CALLBACK (delete_event_cb),
//                    NULL);
  GtkWidget* widget_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_MENU_FILE_QUIT)));
  ACE_ASSERT (widget_p);
  g_signal_connect (G_OBJECT (widget_p),
                    ACE_TEXT_ALWAYS_CHAR ("activate"),
                    G_CALLBACK (quit_clicked_gtk_cb),
                    cb_data_p);

  widget_p =
    GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                      ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_MENU_VIEW_CALIBRATE)));
  ACE_ASSERT (widget_p);
  g_signal_connect (G_OBJECT (widget_p),
                    ACE_TEXT_ALWAYS_CHAR ("activate"),
                    G_CALLBACK (calibrate_clicked_gtk_cb),
                    cb_data_p);

  widget_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_MENU_HELP_ABOUT)));
  ACE_ASSERT (widget_p);
  g_signal_connect (G_OBJECT (widget_p),
                    ACE_TEXT_ALWAYS_CHAR ("activate"),
                    G_CALLBACK (about_clicked_gtk_cb),
                    cb_data_p);
  g_signal_connect_swapped (G_OBJECT (about_dialog_p),
                            ACE_TEXT_ALWAYS_CHAR ("response"),
                            G_CALLBACK (gtk_widget_hide),
                            about_dialog_p);

  // step3a: initialize OpenGL
  // if (!gtk_gl_init_check (&cb_data_p->argc, &cb_data_p->argv))
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to gtk_gl_init_check(): \"%m\", aborting\n")));
  //   return G_SOURCE_REMOVE;
  // } // end IF

  // int mode = (GDK_GL_MODE_RGBA  |
  //             GDK_GL_MODE_DEPTH);
  // if (cb_data_p->openGLDoubleBuffered)
  //   mode |= GDK_GL_MODE_DOUBLE;
  // GdkGLConfig* configuration_p =
  //     gdk_gl_config_new_by_mode (static_cast<GdkGLConfigMode> (mode));
  // if (!configuration_p)
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to gdk_gl_config_new_by_mode(): \"%m\", aborting\n")));
  //   return G_SOURCE_REMOVE;
  // } // end IF

  // if (gtk_widget_get_realized (drawing_area_p))
  //   gtk_widget_unrealize (drawing_area_p);
  // if (!gtk_widget_set_gl_capability (drawing_area_p,    // (container) widget
  //                                    configuration_p,   // GdkGLConfig: configuration
  //                                    NULL,              // GdkGLContext: share list
  //                                    TRUE,              // direct rendering ?
  //                                    GDK_GL_RGBA_TYPE)) // render_type
  // {
  //   ACE_DEBUG ((LM_ERROR,
  //               ACE_TEXT ("failed to gtk_widget_set_gl_capability(): \"%m\", aborting\n")));

  //   // clean up
  //   g_free (configuration_p);

  //   return G_SOURCE_REMOVE;
  // } // end IF
  // // *TODO*: free configuration_p ?

  // step4: connect custom signals
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("button-press-event"),
                    G_CALLBACK (button_cb),
                    cb_data_p);
#if defined (GTKGL_SUPPORT)
#else
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                    G_CALLBACK (configure_cb),
                    cb_data_p);
#if GTK_CHECK_VERSION (3,0,0)
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("draw"),
                    G_CALLBACK (expose_cb),
                    cb_data_p);
#else
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                    G_CALLBACK (expose_cb),
                    cb_data_p);
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // GTKGL_SUPPORT
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("key-press-event"),
                    G_CALLBACK (key_cb),
                    cb_data_p);
  g_signal_connect (drawing_area_p,
                    ACE_TEXT_ALWAYS_CHAR ("motion-notify-event"),
                    G_CALLBACK (motion_cb),
                    cb_data_p);

#if defined (GTKGL_SUPPORT)
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("realize"),
                    G_CALLBACK (glarea_realize_cb),
                    userData_in);
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("unrealize"),
                    G_CALLBACK (glarea_unrealize_cb),
                    userData_in);
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
 g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                   ACE_TEXT_ALWAYS_CHAR ("create-context"),
                   G_CALLBACK (glarea_create_context_cb),
                   userData_in);
 g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                   ACE_TEXT_ALWAYS_CHAR ("render"),
                   G_CALLBACK (glarea_render_cb),
                   userData_in);
 g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                   ACE_TEXT_ALWAYS_CHAR ("resize"),
                   G_CALLBACK (glarea_resize_cb),
                   userData_in);
#else
#if defined (GTKGLAREA_SUPPORT)
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                    G_CALLBACK (glarea_configure_event_cb),
                    userData_in);
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("draw"),
                    G_CALLBACK (glarea_expose_event_cb),
                    userData_in);
#else
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("size-allocate"),
                    G_CALLBACK (glarea_size_allocate_event_cb),
                    userData_in);
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("draw"),
                    G_CALLBACK (glarea_draw_cb),
                    userData_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGLAREA_SUPPORT)
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                    G_CALLBACK (glarea_configure_event_cb),
                    userData_in);
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                    G_CALLBACK (glarea_expose_event_cb),
                    userData_in);
#else
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("configure-event"),
                    G_CALLBACK (glarea_configure_event_cb),
                    userData_in);
  g_signal_connect (G_OBJECT ((*opengl_contexts_iterator).first),
                    ACE_TEXT_ALWAYS_CHAR ("expose-event"),
                    G_CALLBACK (glarea_expose_event_cb),
                    userData_in);
#endif // GTKGLAREA_SUPPORT
#endif // GTK_CHECK_VERSION (3,0,0)
#endif // GTKGL_SUPPORT

//  // step5: use correct screen
//  if (parentWidget_in)
//    gtk_window_set_screen (GTK_WINDOW (dialog),
//                           gtk_widget_get_screen (const_cast<GtkWidget*> (//parentWidget_in)));

  // step6: draw main window
  gtk_widget_show_all (GTK_WIDGET (main_window_p));

  // step7: initialize fps, schedule refresh
  cb_data_p->timestamp = COMMON_TIME_POLICY ();
#if defined (GTKGL_SUPPORT)
  cb_data_p->OpenGLRefreshId =
    g_timeout_add (static_cast<guint> (OLIMEX_MOD_MPU6050_UI_WIDGET_GL_REFRESH_INTERVAL),
                   process_cb,
                   cb_data_p);
  if (!cb_data_p->OpenGLRefreshId)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to g_timeout_add(): \"%m\", aborting\n")));
    return G_SOURCE_REMOVE;
  } // end IF
  //cb_data_p->OpenGLRefreshId = g_idle_add_full (10000,
  //                                              process_cb,
  //                                              cb_data_p,
  //                                              NULL);
  //if (cb_data_p->OpenGLRefreshId == 0)
  //{
  //  ACE_DEBUG ((LM_ERROR,
  //              ACE_TEXT ("failed to g_idle_add_full(): \"%m\", aborting\n")));
  //  return G_SOURCE_REMOVE;
  //} // end IF
  else
    cb_data_p->UIState->eventSourceIds.insert (cb_data_p->OpenGLRefreshId);
#endif /* GTKGL_SUPPORT */

  // one-shot action
  return G_SOURCE_REMOVE;
}

G_MODULE_EXPORT gboolean
idle_finalize_ui_cb (gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::idle_finalize_ui_cb"));

  // sanity check(s)
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
      static_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->UIState);

#if defined (GTKGL_SUPPORT)
  // *NOTE*: somehow the 'unrealize' callback was not being called on the
  //         GtkGLArea --> invoke manually
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (cb_data_p->UIState->lock);
    // *NOTE*: (on win32,) finalize seems to be called several times... :-(
    if (!cb_data_p->UIState->OpenGLContexts.empty ())
    {
      Common_UI_GTK_GLContextsIterator_t opengl_contexts_iterator =
        cb_data_p->UIState->OpenGLContexts.begin ();
      GtkWidget* widget_p = GTK_WIDGET ((*opengl_contexts_iterator).first);
      gtk_widget_unrealize (widget_p);
      cb_data_p->UIState->OpenGLContexts.clear ();
    } // end IF
  } // end lock scope
#endif // GTKGL_SUPPORT

  // synch access
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (cb_data_p->UIState->lock);
    size_t num_messages = cb_data_p->messageQueue.size ();
    while (!cb_data_p->messageQueue.empty ())
    {
      cb_data_p->messageQueue.front ()->release ();
      cb_data_p->messageQueue.pop_front ();
    } // end WHILE
    if (num_messages)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("flushed %Q messages\n"),
                  num_messages));

    cb_data_p->UIState->eventSourceIds.clear ();
  } // end lock scope

  gtk_main_quit ();

  // one-shot action
  return G_SOURCE_REMOVE;
}

/////////////////////////////////////////

G_MODULE_EXPORT gboolean
button_cb (GtkWidget* widget_in,
           GdkEventButton* event_in,
           gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::button_cb"));

  ACE_UNUSED_ARG (event_in);
  ACE_UNUSED_ARG (userData_in);
//  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
//      reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);

//  // sanity check(s)
//  ACE_ASSERT (cb_data_p);

  gtk_widget_grab_focus (widget_in);

  return TRUE; // done (do not propagate further)
}

G_MODULE_EXPORT gboolean
configure_cb (GtkWidget* widget_in,
              GdkEventConfigure* event_in,
              gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::configure_cb"));

  static bool is_first_invokation = true;

  // sanity check(s)
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
      reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (widget_in);
  ACE_ASSERT (cb_data_p);
  ACE_ASSERT (cb_data_p->UIState);

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
#else
   cb_data_p->OpenGLContext = gtk_widget_get_gl_context (widget_in);
   ACE_ASSERT (cb_data_p->OpenGLContext);
   cb_data_p->OpenGLDrawable = gtk_widget_get_gl_drawable (widget_in);
   ACE_ASSERT (cb_data_p->OpenGLDrawable);

   if (!gdk_gl_drawable_gl_begin (cb_data_p->OpenGLDrawable,
                                  cb_data_p->OpenGLContext))
   {
     ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to gdk_gl_drawable_gl_begin(), aborting\n")));
     return FALSE; // propagate
   } // end IF
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */

#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
  if (!cb_data_p->OpenGLAxesListId)
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (cb_data_p->UIState->lock);
    cb_data_p->OpenGLAxesListId = ::axes ();
    if (!glIsList (cb_data_p->OpenGLAxesListId))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ::axes (): \"%m\", aborting\n")));
      return G_SOURCE_REMOVE;
    } // end IF
  } // end IF
#endif /* GTK_CHECK_VERSION (3,0,0) */

  glMatrixMode (GL_PROJECTION);
  // specify the lower left corner, as well as width/height of the viewport
  GtkAllocation allocation_s;
  gtk_widget_get_allocation (widget_in,
                             &allocation_s);
  glViewport (0, 0,
              allocation_s.width,
              allocation_s.height);

  // setup a perspective projection
  // *NOTE*: initially, the "camera" points down the negative z-axis (y is up)
  glLoadIdentity ();
  //gluPerspective (OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_FOVY,
  //                (widget_in->allocation.width / widget_in->allocation.height),
  //                OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_ZNEAR,
  //                OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_ZFAR); // setup a perspective projection
  GLdouble fW, fH;
  fH =
   ::tan (OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_FOVY / 360.0 * M_PI) *
   OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_ZNEAR;
  fW = fH * (allocation_s.width / allocation_s.height);
  glFrustum (-fW, fW,
             -fH, fH,
             OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_ZNEAR,
             OLIMEX_MOD_MPU6050_OPENGL_PERSPECTIVE_ZFAR);

  glMatrixMode (GL_MODELVIEW);

  // set up the camera ?
  if (is_first_invokation)
    cb_data_p->resetCamera ();

  // *NOTE*: standard "right-hand" coordinate system (RHCS) is:
  //         x (thumb): right, y (index): up, z (middle): out of the screen
//  // top-down (RHCS: x --> up [y --> towards you, z --> right]
  //cb_data_p->OpenGLCamera.position[0] = 0.0F;  // eye position (*NOTE*: relative to standard
  //cb_data_p->OpenGLCamera.position[1] = 10.0F; //                       "right-hand" coordinate
  //cb_data_p->OpenGLCamera.position[2] = 0.0F;  //                       system [RHCS])
  //cb_data_p->OpenGLCamera.looking_at[0] = 0.0F; // looking-at position (RHCS notation)
  //cb_data_p->OpenGLCamera.looking_at[1] = 0.0F;
  //cb_data_p->OpenGLCamera.looking_at[2] = 0.0F;
  //cb_data_p->OpenGLCamera.up[0] = 1.0F; // up direction (RHCS notation, relative to "eye"
  //cb_data_p->OpenGLCamera.up[1] = 0.0F; // position and looking-at direction)
  //cb_data_p->OpenGLCamera.up[2] = 0.0F;

  // behind (RHCS: -z --> up [y --> right,
//  gluLookAt (-10.0, 0.0, 0.0, // eye position (*NOTE*: relative to standard
//                              //                       "right-hand" coordinate
//                              //                       system [RHCS])
//             0.0, 0.0, 0.0,   // looking-at position (RHCS notation)
//             0.0, 0.0, -1.0); // up direction (RHCS notation, relative to eye
//                              // position and looking-at direction)
  // *NOTE*: in order for this to work with the device, the modelview CS needs
  //         to be switched to "left-hand" coordinate system as well
//  glScalef (1.0F, 1.0F, -1.0F);
  // *TODO*: do this without glu, i.e. render the complete scene using only
  //         translations, rotations and scaling
  //gluLookAt (cb_data_p->OpenGLCamera.position[0],
  //           cb_data_p->OpenGLCamera.position[1],
  //           cb_data_p->OpenGLCamera.position[2],
  //           cb_data_p->OpenGLCamera.looking_at[0],
  //           cb_data_p->OpenGLCamera.looking_at[1],
  //           cb_data_p->OpenGLCamera.looking_at[2],
  //           cb_data_p->OpenGLCamera.up[0],
  //           cb_data_p->OpenGLCamera.up[1],
  //           cb_data_p->OpenGLCamera.up[2]);

  // set up features ?
  if (is_first_invokation)
  {
    glShadeModel (GL_SMOOTH); // shading mathod: GL_SMOOTH or GL_FLAT
    //glPixelStorei (GL_UNPACK_ALIGNMENT, 4); // 4-byte pixel alignment

    // enable/disable features
    glEnable (GL_DEPTH_TEST);
    glEnable (GL_LIGHTING);
    //glEnable (GL_TEXTURE_2D);
    //glEnable (GL_CULL_FACE);

    glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glEnable (GL_BLEND);

    //glEnable (GL_SCISSOR_TEST);

    // track material ambient and diffuse from surface color, call it before
    // glEnable(GL_COLOR_MATERIAL)
    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable (GL_COLOR_MATERIAL);

    //glLineWidth (1.0F);
    //glEnable (GL_LINE_SMOOTH);

    //glClearColor (0.0F, 0.0F, 0.0F, 0.0F); // background color
    //glClearStencil (0); // clear stencil buffer
    glClearDepth (1.0F); // 0 is near, 1 is far
    //glDepthFunc (GL_LEQUAL);
    //glColor3f (1.0F, 1.0F, 1.0F); // white
  } // end IF

  // set up lighting ?
  if (is_first_invokation)
  {
    // set up light colors (ambient, diffuse, specular)
    GLfloat light_ambient[] = {0.0F, 0.0F, 0.0F, 1.0F};
    glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
    GLfloat light_diffuse[] = {0.9F, 0.9F, 0.9F, 1.0F};
    glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    GLfloat light_specular[] = {1.0F, 1.0F, 1.0F, 1.0F};
    glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);

    // position the light in eye space
    GLfloat light0_position[] = {0.0F,
                                 OLIMEX_MOD_MPU6050_OPENGL_CAMERA_DEFAULT_ZOOM * 2,
                                 OLIMEX_MOD_MPU6050_OPENGL_CAMERA_DEFAULT_ZOOM * 2,
                                 0.0F}; // --> directional light
    glLightfv (GL_LIGHT0, GL_POSITION, light0_position);

    glEnable (GL_LIGHT0);
  } // end IF

#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
#else
  gdk_gl_drawable_gl_end (cb_data_p->OpenGLDrawable);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

  is_first_invokation = false;

  return TRUE; // done (do not propagate further)
}

//G_MODULE_EXPORT gboolean
//delete_event_cb (GtkWidget* widget_in,
//                 GdkEvent* event_in,
//                 gpointer userData_in)
//{
//  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::delete_event_cb"));

//  ACE_UNUSED_ARG (widget_in);
//  ACE_UNUSED_ARG (event_in);
//  ACE_UNUSED_ARG (userData_in);

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("received delete event...\n")));

//  return FALSE; // propagate (*NOTE*: emits "destroy" signal)
//}

G_MODULE_EXPORT gboolean
#if GTK_CHECK_VERSION (3,0,0)
expose_cb (GtkWidget* widget_in,
           cairo_t* context_in,
           gpointer userData_in)
#else
expose_cb (GtkWidget* widget_in,
           GdkEventExpose* event_in,
           gpointer userData_in)
#endif // GTK_CHECK_VERSION (3,0,0)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::expose_cb"));

#if GTK_CHECK_VERSION (3,0,0)
  ACE_UNUSED_ARG (context_in);
#else
  ACE_UNUSED_ARG (event_in);
#endif // GTK_CHECK_VERSION (3,0,0)

  // sanity check(s)
  ACE_ASSERT (widget_in);
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
      reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

//  ACE_Guard<ACE_SYNCH_RECURSIVE_MUTEX> aGuard (cb_data_p->lock);

#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
#else
  if (!gdk_gl_drawable_gl_begin (cb_data_p->OpenGLDrawable,
                                 cb_data_p->OpenGLContext))
   {
     ACE_DEBUG ((LM_ERROR,
                 ACE_TEXT ("failed to gdk_gl_drawable_gl_begin(), aborting\n")));
     return FALSE; // propagate
   } // end IF
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */

  // step1: clear screen
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  // step2: set camera
  glPushMatrix ();

  // step2a: apply model transformation(s)
  //glTranslatef (cb_data_p->OpenGLCamera.translation[0],
  //              cb_data_p->OpenGLCamera.translation[1],
  //              cb_data_p->OpenGLCamera.translation[2]);
  glTranslatef (0.0F, 0.0F, -cb_data_p->OpenGLCamera.zoom);
  glRotatef (cb_data_p->OpenGLCamera.rotation[0], 1.0F, 0.0F, 0.0F);
  glRotatef (cb_data_p->OpenGLCamera.rotation[1], 0.0F, 1.0F, 0.0F);
  glRotatef (cb_data_p->OpenGLCamera.rotation[2], 0.0F, 0.0F, 1.0F);

  // step2b: apply view transformation(s)
  // *NOTE*: corrections so the teapot orients the same as the chip
  // *NOTE*: OpenGL uses RHCS, positive rotation is counter-clockwise (CCW)
  //         (that is, CCW looking TOWARDS the origin)
  glRotatef (90.0F, 0.0F, 0.0F, 1.0F);
  glRotatef (90.0F, 1.0F, 0.0F, 0.0F);

  // step3: draw object(s)
  // step3a: draw model
  const static bool solid = TRUE; // wireframe
  const static float scale = 1.0;
  draw_teapot (solid, scale);

  // step3b: draw axes
  // restrict viewport to upper left corner
  static GLsizei axes_height;
  GtkAllocation allocation_s;
  gtk_widget_get_allocation (widget_in,
                             &allocation_s);

  axes_height =
    static_cast<GLsizei> (allocation_s.height * OLIMEX_MOD_MPU6050_OPENGL_AXES_SIZE);
  glViewport (0, allocation_s.height - axes_height,
              static_cast<GLsizei> (allocation_s.width * OLIMEX_MOD_MPU6050_OPENGL_AXES_SIZE),
              axes_height);
  //glScissor (0, allocation.height - axes_height,
  //           static_cast<GLsizei> (allocation.width * OLIMEX_MOD_MPU6050_OPENGL_AXES_SIZE),
  //           axes_height);
  //glEnable (GL_SCISSOR_TEST);
  //static GLfloat clear_color[4];
  //glGetFloatv (GL_COLOR_CLEAR_VALUE, clear_color);
  //glClearColor (1.0F, 1.0F, 1.0F, 1.0F);
  //glClear (GL_COLOR_BUFFER_BIT |
  //         GL_DEPTH_BUFFER_BIT);

  // set specific features
  static GLfloat color[4];
  glGetFloatv (GL_CURRENT_COLOR, color);
  //glEnable (GL_COLOR_MATERIAL);
  glDisable (GL_LIGHTING);

  // step1: axes don't translate/zoom
  static GLfloat modelview_matrix[16];
  glGetFloatv (GL_MODELVIEW_MATRIX, modelview_matrix);
  glPushMatrix ();
  modelview_matrix[12] = 0.0F;
  modelview_matrix[13] = 0.0F;
  modelview_matrix[14] = -OLIMEX_MOD_MPU6050_OPENGL_CAMERA_DEFAULT_ZOOM;
  glLoadMatrixf (modelview_matrix);

  // draw
#if GTK_CHECK_VERSION (3,0,0)
#elif GTK_CHECK_VERSION (2,0,0)
  glCallList (cb_data_p->OpenGLAxesListId);
#endif /* GTK_CHECK_VERSION (3,0,0) */

  // restore standard camera perspective
  glPopMatrix ();

  // reset specific features
  glColor3f (color[0], color[1], color[2]);
  //glDisable (GL_COLOR_MATERIAL);
  glEnable (GL_LIGHTING);

  // restore viewport
  //glClearColor (clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
  //glDisable (GL_SCISSOR_TEST);
  glViewport (0, 0,
              allocation_s.width,
              allocation_s.height);

  // return to 3D-projection
  glPopMatrix ();

  // step3c: draw fps
  // switch to 2D-projection (i.e. "HUD"-mode)
  glMatrixMode (GL_PROJECTION);
  glPushMatrix ();
  glLoadIdentity ();
  glOrtho (0.0, allocation_s.width,
           0.0, allocation_s.height,
           OLIMEX_MOD_MPU6050_OPENGL_ORTHO_ZNEAR,
           OLIMEX_MOD_MPU6050_OPENGL_ORTHO_ZFAR);
  glMatrixMode (GL_MODELVIEW);
  glPushMatrix ();
  glLoadIdentity ();

  cb_data_p->frameCounter++;
  const static ACE_Time_Value one_second (1, 0);
  ACE_Time_Value elapsed = COMMON_TIME_POLICY () - cb_data_p->timestamp;
  ACE_UINT64 elapsed_usec;
  elapsed.to_usec (elapsed_usec);
  if (elapsed > one_second)
    elapsed_usec -= 1000000;
  ::frames_per_second (static_cast<float> (cb_data_p->frameCounter) *
                       (1000000.0F / static_cast<float> (elapsed_usec)));

  // return to 3D-projection
  glPopMatrix ();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix ();
  glMatrixMode (GL_MODELVIEW);

  if (elapsed >= one_second)
  {
    cb_data_p->frameCounter = 0;
    cb_data_p->timestamp += one_second;
  } // end IF

  // step4: swap buffers / flush
#if GTK_CHECK_VERSION (3,0,0)
  glFlush ();
#elif GTK_CHECK_VERSION (2,0,0)
#if defined (GTKGLAREA_SUPPORT)
#else
  if (cb_data_p->OpenGLDoubleBuffered)
     gdk_gl_drawable_swap_buffers (cb_data_p->OpenGLDrawable);
   else
    glFlush ();

  gdk_gl_drawable_gl_end (cb_data_p->OpenGLDrawable);
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */

  return TRUE; // done (do not propagate further)
}

G_MODULE_EXPORT gboolean
process_cb (gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::process_cb"));

  // sanity check(s)
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
      reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // step0: process event queue
  GtkStatusbar* status_bar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_STATUS_BAR)));
  ACE_ASSERT (status_bar_p);
  Olimex_Mod_MPU6050_Message* message_p = NULL;
  { ACE_Guard<ACE_SYNCH_MUTEX> aGuard (cb_data_p->UIState->lock);
    if (cb_data_p->eventQueue.empty ())
    {
      return TRUE; // --> continue processing
      //goto expose;
    } // end IF
    switch (cb_data_p->eventQueue.front ())
    {
      case OLIMEX_MOD_MPU6050_EVENT_CONNECT:
      {
        cb_data_p->eventQueue.pop_front ();
        gtk_statusbar_push (status_bar_p,
                            cb_data_p->contextIdInformation,
                            ACE_TEXT_ALWAYS_CHAR ("connected"));
        return TRUE; // continue processing
      }
      case OLIMEX_MOD_MPU6050_EVENT_DISCONNECT:
      {
        cb_data_p->eventQueue.pop_front ();
        gtk_statusbar_push (status_bar_p,
                            cb_data_p->contextIdInformation,
                            ACE_TEXT_ALWAYS_CHAR ("disconnected"));
        return TRUE; // continue processing
      }
      case OLIMEX_MOD_MPU6050_EVENT_MESSAGE:
      {
        cb_data_p->eventQueue.pop_front ();
        message_p = cb_data_p->messageQueue.front ();
        cb_data_p->messageQueue.pop_front ();
        break;
      }
      case OLIMEX_MOD_MPU6050_EVENT_SESSION_MESSAGE:
      {
        cb_data_p->eventQueue.pop_front ();
        return TRUE; // continue processing
      }
      default:
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("invalid event (was: %d), aborting\n"),
                    cb_data_p->eventQueue.front ()));
        cb_data_p->eventQueue.pop_front ();
        return FALSE; // --> stop processing
      }
    } // end SWITCH
  } // end lock scope
  ACE_ASSERT (message_p);
  ACE_ASSERT (message_p->size () == OLIMEX_MOD_MPU6050_STREAM_BUFFER_SIZE);

  // step1a: extract sensor data
  float ax, ay, az, t, gx, gy, gz;
  extract_data (message_p->rd_ptr (),
                ax, ay, az,
                t,
                gx, gy, gz);

  // step1b: correct data with bias information
  if (cb_data_p->clientMode)
  {
    ax -= cb_data_p->clientSensorBias.ax_bias;
    ay -= cb_data_p->clientSensorBias.ay_bias;
    az -= cb_data_p->clientSensorBias.az_bias;
    gx -= cb_data_p->clientSensorBias.gx_bias;
    gy -= cb_data_p->clientSensorBias.gy_bias;
    gz -= cb_data_p->clientSensorBias.gz_bias;
  } // end IF

  // step1c: update status bar
  gchar buffer[BUFSIZ];
  ACE_OS::memset (buffer, 0, sizeof (buffer));
  int result =
    ACE_OS::sprintf (buffer,
                     ACE_TEXT_ALWAYS_CHAR ("[accelerometer (g/s): %6.3f,%6.3f,%6.3f], [gyrometer (°/s): %8.3f,%8.3f,%8.3f], [temperature (°C): %6.2f]"),
                     ax, ay, az,
                     gx, gy, gz,
                     t);
  if (result < 0)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to sprintf(): \"%m\", aborting\n")));

    // clean up
    message_p->release ();

    return FALSE; // --> stop processing
  } // end IF
  gtk_statusbar_pop (status_bar_p,
                     cb_data_p->contextIdData);
  gtk_statusbar_push (status_bar_p,
                      cb_data_p->contextIdData,
                      buffer);

  // step2: process sensor data
  static float angle_x, angle_y, angle_z;
  // step2a: acceleration / rotation
  //cb_data_p->OpenGLCamera.translation[0] +=
  //  OLIMEX_MOD_MPU6050_OPENGL_CAMERA_TRANSLATION_FACTOR * diff_x;
  //cb_data_p->OpenGLCamera.translation[1] -=
  //  OLIMEX_MOD_MPU6050_OPENGL_CAMERA_TRANSLATION_FACTOR * diff_y;
  //cb_data_p->OpenGLCamera.translation[2] -=
  //  OLIMEX_MOD_MPU6050_OPENGL_CAMERA_TRANSLATION_FACTOR * diff_y;
  // *TODO*: x,y rotation directions seem inverted for some reason...
  angle_x = -gx * (1.0F / static_cast<float> (OLIMEX_MOD_MPU6050_DATA_RATE));
  angle_y = -gy * (1.0F / static_cast<float> (OLIMEX_MOD_MPU6050_DATA_RATE));
  angle_z =  gz * (1.0F / static_cast<float> (OLIMEX_MOD_MPU6050_DATA_RATE));
  cb_data_p->OpenGLCamera.rotation[0] += angle_x;
  cb_data_p->OpenGLCamera.rotation[1] += angle_y;
  cb_data_p->OpenGLCamera.rotation[2] += angle_z;

  // step2b: temperature
  // GtkCurve* curve_p =
  //   GTK_CURVE (gtk_builder_get_object ((*iterator).second.second,
  //                                    ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_CURVE)));
  // ACE_ASSERT (curve_p);
  // cb_data_p->temperatureIndex++;
  // if (cb_data_p->temperatureIndex == OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE)
  // {
  //   cb_data_p->temperatureIndex = 0;
  //   //ACE_OS::memset (cb_data_p->temperature,
  //   //                0,
  //   //                sizeof (cb_data_p->temperature));
  //   gtk_curve_reset (curve_p);
  // } // end IF
  cb_data_p->temperature[cb_data_p->temperatureIndex * 2] =
    static_cast<gfloat> (cb_data_p->temperatureIndex);
  cb_data_p->temperature[(cb_data_p->temperatureIndex * 2) + 1] =
    ((t + OLIMEX_MOD_MPU6050_THERMOMETER_OFFSET) /
     OLIMEX_MOD_MPU6050_THERMOMETER_RANGE) *
    OLIMEX_MOD_MPU6050_UI_WIDGET_CURVE_MAXIMUM_Y;
  //gtk_curve_set_range (curve_p,
  //                     0.0F,
  //                     static_cast<gfloat> (cb_data_p->temperatureIndex),
  //                     0.0F,
  //                     100.0F);
  // gtk_curve_set_vector (curve_p,
  //                       OLIMEX_MOD_MPU6050_TEMPERATURE_BUFFER_SIZE,
  //                       //cb_data_p->temperatureIndex + 1,
  //                       cb_data_p->temperature);

//expose:
  // invalidate drawing area
  // *NOTE*: the drawing area is not refreshed automatically unless the window
  //         is resized
  GtkWidget* drawing_area_p =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                          ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DRAWING_AREA)));
  ACE_ASSERT (drawing_area_p);
  gtk_widget_queue_draw (drawing_area_p);
  //gtk_widget_queue_draw (GTK_WIDGET (curve_p));
  //gdk_window_invalidate_rect (//gtk_widget_get_root_window (drawing_area),
  //                            drawing_area_p->window,
  //                            &drawing_area_p->allocation,
  //                            FALSE);
  //gdk_window_process_updates (drawing_area_p->window, FALSE);

  // clean up
  message_p->release ();
  //ACE_DEBUG ((LM_DEBUG,
  //            ACE_TEXT ("buffer pool: %d/%d [%.0f%%] --> %u byte(s)\n"),
  //            cb_data_p->allocator->cache_size (),
  //            OLIMEX_MOD_MPU6050_MAXIMUM_NUMBER_OF_INFLIGHT_MESSAGES,
  //            ((static_cast<float> (cb_data_p->allocator->cache_size ()) /
  //              static_cast<float> (OLIMEX_MOD_MPU6050_MAXIMUM_NUMBER_OF_INFLIGHT_MESSAGES)) * 100.0F),
  //            cb_data_p->allocator->cache_depth ()));

  return TRUE; // --> continue processing
}

G_MODULE_EXPORT gboolean
key_cb (GtkWidget* widget_in,
        GdkEventKey* event_in,
        gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::key_cb"));

  // sanity check(s)
  ACE_ASSERT (event_in);
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
    reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

  switch (event_in->keyval)
  {
    case GDK_KEY_Left:
    {
      // *NOTE*: the camera moves on a circle in a plane perpendicular to the
      //         plane between the "view" and "up" directions. Therefore, the
      //         distance to the "target" remains constant
      // *NOTE*: camera positions are (discrete) positions on the circle
      //         perimeter

      //// compute movement plane
      //glm::vec3 unit_vector = glm::cross (cb_data_p->OpenGLCamera.looking_at,
      //                                    cb_data_p->OpenGLCamera.up);
      ////unit_vector = glm::normalize (unit_vector);

      //// compute distance factor
      //glm::vec3 direction =
      // cb_data_p->OpenGLCamera.looking_at - cb_data_p->OpenGLCamera.position;
      //GLfloat factor = glm::length (direction);

      //// compute position on circle perimeter
      //cb_data_p->OpenGLCamera.angle -= OLIMEX_MOD_MPU6050_OPENGL_CAMERA_ROTATION_STEP;
      //GLfloat angle = cb_data_p->OpenGLCamera.angle * 360.0F;

      //// compute camera position
      //glm::vec3 p, q, r;
      //p = cb_data_p->OpenGLCamera.position;
      //r = unit_vector;
      //r[0] = ::cos (angle);
      //r[1] = ::sin (angle);
      //r[2] = 0.0F;
      //r = glm::normalize (cb_data_p->OpenGLCamera.looking_at + r);
      //q = (r - p) * factor;
      //cb_data_p->OpenGLCamera.position = q;

      //gluLookAt (cb_data_p->OpenGLCamera.position[0],
      //           cb_data_p->OpenGLCamera.position[1],
      //           cb_data_p->OpenGLCamera.position[2],
      //           cb_data_p->OpenGLCamera.looking_at[0],
      //           cb_data_p->OpenGLCamera.looking_at[1],
      //           cb_data_p->OpenGLCamera.looking_at[2],
      //           cb_data_p->OpenGLCamera.up[0],
      //           cb_data_p->OpenGLCamera.up[1],
      //           cb_data_p->OpenGLCamera.up[2]);

      break;
    }
    case GDK_KEY_Right:
      break;
    //case GDK_KEY_Up:
    //  break;
    //case GDK_KEY_Down:
    //  break;
    //case GDK_KEY_c:
    //case GDK_KEY_C:
    //  break;
    //case GDK_KEY_d:
    //case GDK_KEY_D:
    //  break;
    case GDK_KEY_r:
    case GDK_KEY_R:
    {
      cb_data_p->resetCamera ();

      gtk_widget_queue_draw (widget_in);

      break;
    }
    default:
      return FALSE; // propagate
  } // end SWITCH

  return TRUE; // done (do not propagate further)
}

G_MODULE_EXPORT gboolean
motion_cb (GtkWidget* widget_in,
           GdkEventMotion* event_in,
           gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::motion_cb"));

  // sanity check(s)
  ACE_ASSERT (event_in);
  if (event_in->is_hint)
    return FALSE; // propagate
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
    reinterpret_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT(cb_data_p);

  int diff_x =
   static_cast<int> (event_in->x) - cb_data_p->OpenGLCamera.last[0];
  int diff_y =
   static_cast<int> (event_in->y) - cb_data_p->OpenGLCamera.last[1];
  cb_data_p->OpenGLCamera.last[0] = static_cast<int> (event_in->x);
  cb_data_p->OpenGLCamera.last[1] = static_cast<int> (event_in->y);

  bool is_dirty = false;
  if (event_in->state & GDK_BUTTON1_MASK)
  {
    cb_data_p->OpenGLCamera.rotation[0] +=
      OLIMEX_MOD_MPU6050_OPENGL_CAMERA_ROTATION_FACTOR * diff_y;
    cb_data_p->OpenGLCamera.rotation[1] +=
      OLIMEX_MOD_MPU6050_OPENGL_CAMERA_ROTATION_FACTOR * diff_x;
    is_dirty = true;
  } // end IF
  if (event_in->state & GDK_BUTTON2_MASK)
  {
    cb_data_p->OpenGLCamera.translation[0] +=
      OLIMEX_MOD_MPU6050_OPENGL_CAMERA_TRANSLATION_FACTOR * diff_x;
    cb_data_p->OpenGLCamera.translation[1] -=
      OLIMEX_MOD_MPU6050_OPENGL_CAMERA_TRANSLATION_FACTOR * diff_y;
    is_dirty = true;
  } // end IF
  if (event_in->state & GDK_BUTTON3_MASK)
  {
    cb_data_p->OpenGLCamera.zoom -=
      OLIMEX_MOD_MPU6050_OPENGL_CAMERA_ZOOM_FACTOR * diff_x;
    is_dirty = true;
  } // end IF

  // invalidate drawing area
  // *NOTE*: the drawing area is not refreshed automatically unless the window
  //         is resized
  if (is_dirty)
    gtk_widget_queue_draw (widget_in);

  return TRUE; // done (do not propagate further)
}

/////////////////////////////////////////

G_MODULE_EXPORT gint
about_clicked_gtk_cb (GtkWidget* widget_in,
                      gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::about_clicked_gtk_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
      static_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);

  // sanity check(s)
  ACE_ASSERT (cb_data_p);

  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  // sanity check(s)
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  // retrieve about dialog handle
  GtkWidget* about_dialog =
      GTK_WIDGET (gtk_builder_get_object ((*iterator).second.second,
                                        ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DIALOG_ABOUT)));
  ACE_ASSERT (about_dialog);
  if (!about_dialog)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to gtk_builder_get_object(\"%s\"): \"%m\", aborting\n"),
                ACE_TEXT (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_DIALOG_ABOUT)));
    return FALSE; // propagate
  } // end IF

  // draw it
  if (!gtk_widget_get_visible (about_dialog))
    gtk_widget_show_all (about_dialog);

  return TRUE; // done (do not propagate further)
}

G_MODULE_EXPORT gint
calibrate_clicked_gtk_cb (GtkWidget* widget_in,
                          gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::calibrate_clicked_gtk_cb"));

  ACE_UNUSED_ARG (widget_in);

  // sanity check(s)
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
    static_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);
  Common_UI_GTK_BuildersIterator_t iterator =
    cb_data_p->UIState->builders.find (ACE_TEXT_ALWAYS_CHAR (COMMON_UI_DEFINITION_DESCRIPTOR_MAIN));
  ACE_ASSERT (iterator != cb_data_p->UIState->builders.end ());

  if (cb_data_p->clientMode)
  {
    ACE_OS::memset (&cb_data_p->clientSensorBias, 0, sizeof (cb_data_p->clientSensorBias));

    ACE_Guard<ACE_SYNCH_MUTEX> aGuard (cb_data_p->UIState->lock);
    if (cb_data_p->eventQueue.empty ())
    {
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("no data, returning\n")));
      return FALSE; // propagate
    } // end IF
    Olimex_Mod_MPU6050_Message* message_p = NULL;
    message_p = cb_data_p->messageQueue.front ();
    ACE_ASSERT (message_p);
    float dummy;
    extract_data (message_p->rd_ptr (),
                  cb_data_p->clientSensorBias.ax_bias,
                  cb_data_p->clientSensorBias.ay_bias,
                  cb_data_p->clientSensorBias.az_bias,
                  dummy,
                  cb_data_p->clientSensorBias.gx_bias,
                  cb_data_p->clientSensorBias.gy_bias,
                  cb_data_p->clientSensorBias.gz_bias);
    cb_data_p->clientSensorBias.az_bias += 1.0F;

    // update status bar
    gchar buffer[BUFSIZ];
    ACE_OS::memset (buffer, 0, sizeof (buffer));
    int result = ACE_OS::sprintf (buffer,
                                  ACE_TEXT_ALWAYS_CHAR ("updated bias data: [accelerometer (g/s): %6.3f,%6.3f,%6.3f], [gyrometer (°/s): %8.3f,%8.3f,%8.3f]"),
                                  cb_data_p->clientSensorBias.ax_bias,
                                  cb_data_p->clientSensorBias.ay_bias,
                                  cb_data_p->clientSensorBias.az_bias,
                                  cb_data_p->clientSensorBias.gx_bias,
                                  cb_data_p->clientSensorBias.gy_bias,
                                  cb_data_p->clientSensorBias.gz_bias);
    if (result < 0)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to sprintf(): \"%m\", continuing\n")));
      return FALSE; // propagate
    } // end IF
    GtkStatusbar* status_bar_p =
      GTK_STATUSBAR (gtk_builder_get_object ((*iterator).second.second,
                                           ACE_TEXT_ALWAYS_CHAR (OLIMEX_MOD_MPU6050_UI_WIDGET_NAME_STATUS_BAR)));
    ACE_ASSERT (status_bar_p);
    gtk_statusbar_push (status_bar_p,
                        cb_data_p->contextIdInformation,
                        buffer);
  } // end IF
  else
  {
    // *TODO*: implement a client->server protocol to do this
    ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (TRUE);
  } // end IF

  return TRUE; // done (do not propagate further)
}

G_MODULE_EXPORT gint
quit_clicked_gtk_cb (GtkWidget* widget_in,
                     gpointer userData_in)
{
  OLIMEX_MOD_MPU6050_TRACE (ACE_TEXT ("::quit_clicked_gtk_cb"));

  ACE_UNUSED_ARG (widget_in);
  struct Olimex_Mod_MPU6050_GTK_CBData* cb_data_p =
    static_cast<struct Olimex_Mod_MPU6050_GTK_CBData*> (userData_in);
  ACE_ASSERT (cb_data_p);

#if defined (GTKGL_SUPPORT)
  if (cb_data_p->OpenGLRefreshId)
  {
    g_source_remove (cb_data_p->OpenGLRefreshId);
    cb_data_p->OpenGLRefreshId = 0;
  } // end iF
#endif /* GTKGL_SUPPORT */

  // this is the "delete-event" / "destroy" handler
  // --> destroy the main dialog widget
  int result = -1;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  result = ACE_OS::raise (SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to raise(SIGINT): \"%m\", continuing\n")));
#else
  pid_t pid = ACE_OS::getpid ();
  result = ACE_OS::kill (pid, SIGINT);
  if (result == -1)
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to kill(%d, SIGINT): \"%m\", continuing\n"),
                pid));
#endif

//  ACE_DEBUG ((LM_DEBUG,
//              ACE_TEXT ("leaving GTK...\n")));

  return TRUE; // done (do not propagate further)
}
#ifdef __cplusplus
}
#endif /* __cplusplus */
