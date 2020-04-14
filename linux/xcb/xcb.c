/*
  MIT License

  Copyright (c) 2020 Geoffrey McRae <geoff@hostfission.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#include "src/adl.h"
#include "src/window.h"
#include "interface/adl.h"

#include "atoms.h"
#include "image.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <signal.h>

#include "xcb.h"

struct State this;

static ADL_STATUS xcbWindowSetTitle(ADLWindow * window, const char * title);
static ADL_STATUS xcbWindowSetClassName(ADLWindow * window,
    const char * className);
static ADL_STATUS xcbPointerWarp(ADLWindow * window, int x, int y);

static const char * xcbErrString(int error)
{
  switch(error)
  {
    case XCB_CONN_ERROR:
      return "XCB_CONN_ERROR";
    case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
      return "XCB_CONN_CLOSED_EXT_NOTSUPPORTED";
    case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
      return "XCB_CONN_CLOSED_MEM_INSUFFICIENT";
    case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
      return "XCB_CONN_CLOSED_REQ_LEN_EXCEED";
    case XCB_CONN_CLOSED_PARSE_ERR:
      return "XCB_CONN_CLOSED_PARSE_ERR";
    case XCB_CONN_CLOSED_INVALID_SCREEN:
      return "XCB_CONN_CLOSED_INVALID_SCREEN";
    default:
      return "Unknown XCB Error";
   }
}

static void setWindowProperties(WindowData * data, const ADLWindowDef props)
{
  struct WMSizeHints hints =
  {
    .flags       = WM_SIZE_HINT_P_WIN_GRAVITY,
    .win_gravity = XCB_GRAVITY_STATIC
  };

  if (props.flags & ADL_WINDOW_FLAG_CENTER)
    hints.win_gravity = XCB_GRAVITY_CENTER;
  else
  {
    hints.flags |= WM_SIZE_HINT_P_POSITION;
    hints.x = props.x;
    hints.y = props.y;
  }

  changeProperty(XCB_PROP_MODE_REPLACE, data->window,
    IA_XCB_ATOM_WM_NORMAL_HINTS, IA_XCB_ATOM_WM_SIZE_HINTS, 32,
    sizeof(struct WMSizeHints) >> 2, &hints);


  {
#define SET_FLAG(x, y) \
    if ((props.flags & ADL_WINDOW_FLAG_ ##x) && haveAtom(IA_NET_WM_STATE_ ##y)) \
      state[stateCount++] = getAtom(IA_NET_WM_STATE_ ##y);

    xcb_atom_t state[13];
    int stateCount = 0;

    SET_FLAG(MODAL       , MODAL            )
    SET_FLAG(STICKY      , STICKY           )
    SET_FLAG(MAXV        , MAXIMIZED_VERT   )
    SET_FLAG(MAXH        , MAXIMIZED_HORZ   )
    SET_FLAG(SHADED      , SHADED           )
    SET_FLAG(SKIP_TASKBAR, SKIP_TASKBAR     )
    SET_FLAG(SKIP_PAGER  , SKIP_PAGER       )
    SET_FLAG(HIDDEN      , HIDDEN           )
    SET_FLAG(FULLSCREEN  , FULLSCREEN       )
    SET_FLAG(ABOVE       , ABOVE            )
    SET_FLAG(BELOW       , BELOW            )
    SET_FLAG(ATTENTION   , DEMANDS_ATTENTION)
    SET_FLAG(FOCUSED     , FOCUSED          )

    if (stateCount)
      changeProperty(XCB_PROP_MODE_REPLACE, data->window, IA_NET_WM_STATE,
        IA_XCB_ATOM_ATOM, 32, stateCount, state);

#undef SET_FLAG
  }

  {
#define SET_TYPE(x, y) \
    case ADL_WINDOW_TYPE_ ##x: \
      type[typeCount++] = getAtom(IA_NET_WM_WINDOW_TYPE_ ##y); \
      break;

    xcb_atom_t type[3];
    int typeCount = 0;

    switch(props.type)
    {
      SET_TYPE(DESKTOP     , DESKTOP      );
      SET_TYPE(DOCK        , DOCK         );
      SET_TYPE(TOOLBAR     , TOOLBAR      );
      SET_TYPE(MENU        , MENU         );
      SET_TYPE(UTILITY     , UTILITY      );
      SET_TYPE(SPLASH      , SPLASH       );
      SET_TYPE(DIALOG      , DIALOG       );
      SET_TYPE(DROPDOWN    , DROPDOWN_MENU);
      SET_TYPE(POPUP       , POPUP_MENU   );
      SET_TYPE(TOOLTIP     , TOOLTIP      );
      SET_TYPE(NOTIFICATION, NOTIFICATION );
      SET_TYPE(COMBO       , COMBO        );
      SET_TYPE(DND         , DND          );

      case ADL_WINDOW_TYPE_NORMAL:
        break;
    }

    if (props.borderless)
    {
      if (haveAtom(IA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE))
        type[typeCount++] = getAtom(IA_KDE_NET_WM_WINDOW_TYPE_OVERRIDE);
    }

    // normal is always appended
    type[typeCount++] = getAtom(IA_NET_WM_WINDOW_TYPE_NORMAL);

    changeProperty(XCB_PROP_MODE_REPLACE, data->window, IA_NET_WM_WINDOW_TYPE,
        IA_XCB_ATOM_ATOM, 32, typeCount, type);

#undef SET_TYPE
  }

  if (haveAtom(IA_MOTIF_WM_HINTS))
  {
    struct MotifHints hints = {.flags = props.borderless ? 2 : 0};
    changeProperty(XCB_PROP_MODE_REPLACE, data->window, IA_MOTIF_WM_HINTS,
        IA_XCB_ATOM_INTEGER, 32, 5, &hints);
  }
}

static ADL_STATUS getParentWindowOffset(xcb_window_t window, WindowData * data)
{
  xcb_generic_error_t * error;
  if (!data->parent)
  {
    xcb_query_tree_cookie_t qc = xcb_query_tree(this.xcb, window);
    xcb_query_tree_reply_t *qr = xcb_query_tree_reply(this.xcb, qc, &error);

    if (error)
    {
      ADL_ERROR(ADL_ERR_PLATFORM,
        "xcb_query_tree failed: code=%d, res=%d",
        error->error_code, error->resource_id);
      free(error);
      return ADL_ERR_PLATFORM;
    }

    data->parent = qr->parent;
    free(qr);
  }

  xcb_translate_coordinates_cookie_t tc =
    xcb_translate_coordinates(this.xcb, data->parent,
        this.screen->root, 0, 0);

  xcb_translate_coordinates_reply_t *tr =
    xcb_translate_coordinates_reply(this.xcb, tc, &error);

  if (error)
  {
    ADL_ERROR(ADL_ERR_PLATFORM,
      "xcb_translate_coordinates failed: code=%d, res=%d",
      error->error_code, error->resource_id);
    free(error);
    return ADL_ERR_PLATFORM;
  }

  data->transX = tr->dst_x;
  data->transY = tr->dst_y;
  free(tr);

  return ADL_OK;
}

static ADL_STATUS xcbTest()
{
  ADL_STATUS status = ADL_ERR_UNSUPPORTED;

  /* try and connect to the display */
  this.display = XOpenDisplay(NULL);
  if (!this.display)
    goto out_no_x;

  this.xcb = XGetXCBConnection(this.display);
  if (xcb_connection_has_error(this.xcb) == 0)
    status = ADL_OK;
  else
    xcb_disconnect(this.xcb);

  XCloseDisplay(this.display);
out_no_x:
  this.xcb = NULL;
  return status;
}

static void xcbSignalHandler(int sig)
{
  switch(sig)
  {
    case SIGINT:
    case SIGTERM:
    {
      adlQuit();
      break;
    }
  }
}

static ADL_STATUS xcbInitialize()
{
  ADL_STATUS status = ADL_OK;
  int err;

  this.display = XOpenDisplay(NULL);
  if (!this.display)
  {
    status = ADL_ERR_PLATFORM;
    ADL_ERROR(status, "XCB failed to connect to the X server");
    goto err_out;
  }

  this.xcb = XGetXCBConnection(this.display);
  if ((err = xcb_connection_has_error(this.xcb)) != 0)
  {
    status = ADL_ERR_PLATFORM;
    ADL_ERROR(status, "XCB get connection failed with: %s", xcbErrString(err));
    goto err_x;
  }

  this.screen = xcb_setup_roots_iterator(xcb_get_setup(this.xcb)).data;

  /* initialize the atom lookup table */
  for(int i = 0; i < IA_COUNT; ++i)
  {
    /* skip over values that are known, ie XCB_* */
    if (internAtom[i].atom)
      continue;

    xcb_generic_error_t * error;
    xcb_intern_atom_cookie_t c = xcb_intern_atom(this.xcb, 1,
        strlen(internAtom[i].name), internAtom[i].name);
    xcb_intern_atom_reply_t * r = xcb_intern_atom_reply(this.xcb, c, &error);

    if (error)
    {
      ADL_ERROR(ADL_ERR_PLATFORM, "xcb_intern_atom failed: code=%d, res=%d",
        error->error_code, error->resource_id);
      free(error);
      status = ADL_ERR_PLATFORM;
      goto err_disconnect;
    }

    internAtom[i].atom = r->atom;
    free(r);
  }

  /* we need xkb */
  {
    xcb_xkb_use_extension_cookie_t c =
      xcb_xkb_use_extension(this.xcb,
          XCB_XKB_MAJOR_VERSION, XCB_XKB_MINOR_VERSION);

    xcb_xkb_use_extension_reply_t * r =
      xcb_xkb_use_extension_reply(this.xcb, c, NULL);

    if (!r)
    {
      ADL_INFO(ADL_ERR_PLATFORM, "xcb_xkb_use_extension failed");
      status = ADL_ERR_PLATFORM;
      goto err_disconnect;
    }

    free(r);
  }

  /* prevent auto-repeat of key up events */
  xcb_xkb_per_client_flags(this.xcb, XCB_XKB_ID_USE_CORE_KBD,
    XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
    0, 0, 0);

  /* lookup the keyboard mapping */
  {
    xcb_xkb_get_names_cookie_t c =
      xcb_xkb_get_names(this.xcb, XCB_XKB_ID_USE_CORE_KBD,
          XCB_XKB_NAME_DETAIL_KEY_NAMES);

    xcb_xkb_get_names_reply_t * r =
      xcb_xkb_get_names_reply(this.xcb, c, NULL);

    if (!r)
    {
      ADL_INFO(ADL_ERR_PLATFORM, "xcb_xkb_get_names");
      status = ADL_ERR_PLATFORM;
      goto err_disconnect;
    }

    xcb_xkb_get_names_value_list_t list;
    {
      void * buffer;
      buffer = xcb_xkb_get_names_value_list(r);
      xcb_xkb_get_names_value_list_unpack(
        buffer,
        r->nTypes,
        r->indicators,
        r->virtualMods,
        r->groupNames,
        r->nKeys,
        r->nKeyAliases,
        r->nRadioGroups,
        r->which,
        &list);
    }

    const int length =
      xcb_xkb_get_names_value_list_key_names_length(r, &list);
    xcb_xkb_key_name_iterator_t iter =
      xcb_xkb_get_names_value_list_key_names_iterator(r, &list);

    for (int i = 0; i < length; i++)
    {
      xcb_xkb_key_name_t *key_name = iter.data;
      strncpy(this.keyMap[i], key_name->name, 4);
      xcb_xkb_key_name_next(&iter);
    }

    free(r);
  }

  /* create a blank cursor for when we need to hide it */
  this.blankPointer = xcb_generate_id(this.xcb);
  this.blankPixmap  = xcb_generate_id(this.xcb);

  {
    xcb_void_cookie_t c;
    xcb_generic_error_t * error;

    c = xcb_create_pixmap_checked(
          this.xcb,
          1,
          this.blankPixmap,
          this.screen->root,
          1,
          1);

    if ((error = xcb_request_check(this.xcb, c)))
    {
      ADL_INFO(ADL_ERR_PLATFORM, "failed to create the blank pixmap");
      status = ADL_ERR_PLATFORM;
      free(error);
      goto err_disconnect;
    }

    c = xcb_create_cursor_checked(
          this.xcb,
          this.blankPointer,
          this.blankPixmap,
          this.blankPixmap,
          0, 0, 0, 0, 0, 0, 0, 0);

    if ((error = xcb_request_check(this.xcb, c)))
    {
      ADL_INFO(ADL_ERR_PLATFORM, "failed to create the blank pointer");
      status = ADL_ERR_PLATFORM;
      free(error);
      goto err_free_cursor_pixmap;
    }
  }

  /* load the default pointer for when we need to restore it */
  if (xcb_cursor_context_new(this.xcb, this.screen, &this.cursorContext) < 0)
  {
    ADL_INFO(ADL_ERR_PLATFORM, "failed to initialize xcb-cursor");
    status = ADL_ERR_PLATFORM;
    goto err_free_cursor;
  }

  this.defaultPointer = xcb_cursor_load_cursor(this.cursorContext, "left_ptr");

  /* lookup the render formats for later use */
  {
    xcb_render_query_pict_formats_cookie_t c =
      xcb_render_query_pict_formats(this.xcb);

    xcb_render_query_pict_formats_reply_t * r =
      xcb_render_query_pict_formats_reply(this.xcb, c, 0);

    xcb_render_pictforminfo_t * format =
      xcb_render_query_pict_formats_formats(r);

    for(int i = 0; i < r->num_formats; ++i, ++format)
    {
      if (format->type != XCB_RENDER_PICT_TYPE_DIRECT)
        continue;

      // must contain the red channel
      if (format->direct.red_mask != 0xff)
        continue;

      switch(format->direct.red_shift)
      {
        //RGBA, ARGB
#ifdef ENDIAN_LITTLE
        case 0:
        case 8:
#else
        case 16:
        case 24:
#endif
        {
          if (format->depth == 24)
          {
            this.formatRGB = *format;
            break;
          }

          // if has the alpha channel
          if (format->depth == 32 && format->direct.alpha_mask == 0xff)
          {
            switch(format->direct.alpha_shift)
            {
#ifdef ENDIAN_LITTLE
              case 24: this.formatRGBA = *format; break;
              case 0 : this.formatARGB = *format; break;
#else
              case 0 : this.formatRGBA = *format; break;
              case 24: this.formatARGB = *format; break;
#endif
            }
          }
        }
        break;

        //BGRA, ABGR
#ifdef ENDIAN_LITTLE
        case 16:
        case 24:
#else
        case 0:
        case 8:
#endif
        {
          if (format->depth == 24)
          {
            this.formatBGR = *format;
            break;
          }

          // if has the alpha channel
          if (format->depth == 32 && format->direct.alpha_mask == 0xff)
          {
            switch(format->direct.alpha_shift)
            {
#ifdef ENDIAN_LITTLE
              case 24: this.formatBGRA = *format; break;
              case 0 : this.formatABGR = *format; break;
#else
              case 0 : this.formatBGRA = *format; break;
              case 24: this.formatABGR = *format; break;
#endif
            }
          }
        }
        break;
      }
    }

    free(r);
  }

  this.fd = xcb_get_file_descriptor(this.xcb);

  /* install the signal handler so we can catch SIGINT/SIGTERM */
  signal(SIGINT , xcbSignalHandler);
  signal(SIGTERM, xcbSignalHandler);

  return status;

err_free_cursor:
  xcb_free_cursor(this.xcb, this.blankPointer);
err_free_cursor_pixmap:
  xcb_free_pixmap(this.xcb, this.blankPixmap);
err_disconnect:
  xcb_disconnect(this.xcb);
  this.xcb = NULL;
err_x:
  XCloseDisplay(this.display);
  this.display = NULL;
err_out:
  return status;
}

static ADL_STATUS xcbDeinitialize()
{
  xcb_free_cursor(this.xcb, this.defaultPointer);
  xcb_free_cursor(this.xcb, this.blankPointer  );
  xcb_free_pixmap(this.xcb, this.blankPixmap   );
  xcb_cursor_context_free(this.cursorContext);
  xcb_disconnect(this.xcb);
  this.xcb = NULL;
  return ADL_OK;
}

static ADL_STATUS xcbWindowCreate(const ADLWindowDef def, ADLWindow * result)
{
  const uint32_t eventMask =
    XCB_EVENT_MASK_EXPOSURE         |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE |
    XCB_EVENT_MASK_KEY_PRESS        | XCB_EVENT_MASK_KEY_RELEASE       |
    XCB_EVENT_MASK_BUTTON_PRESS     | XCB_EVENT_MASK_BUTTON_RELEASE    |
    XCB_EVENT_MASK_POINTER_MOTION   | XCB_EVENT_MASK_ENTER_WINDOW      |
    XCB_EVENT_MASK_LEAVE_WINDOW     | XCB_EVENT_MASK_EXPOSURE;

  uint32_t values[3] =
  {
    this.screen->white_pixel,
    XCB_GRAVITY_STATIC,
    eventMask
  };

  if (def.flags & ADL_WINDOW_FLAG_CENTER)
    values[1] = XCB_GRAVITY_CENTER;

  xcb_window_t parent = this.screen->root;
  if (def.parent)
  {
    WindowData * pdata = ADL_GET_WINDOW_DATA(def.parent);
    parent = pdata->window;
  }

  xcb_window_t window = xcb_generate_id(this.xcb);
  xcb_void_cookie_t c =
    xcb_create_window_checked(
      this.xcb,
      XCB_COPY_FROM_PARENT,
      window,
      parent,
      def.x, def.y,
      def.w, def.h,
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      XCB_COPY_FROM_PARENT,
      XCB_CW_BACK_PIXEL | XCB_CW_WIN_GRAVITY | XCB_CW_EVENT_MASK,
      values
    );

  xcb_generic_error_t *error;
  if ((error = xcb_request_check(this.xcb, c)))
  {
    ADL_ERROR(ADL_ERR_PLATFORM, "xcb_create_window failed: code=%d, res=%d",
      error->error_code, error->resource_id);
    free(error);
    return ADL_ERR_PLATFORM;
  }

  /* set the window's ID and get the window data */
  ADL_SET_WINDOW_ID(result, window);
  WindowData * data = ADL_GET_WINDOW_DATA(result);
  memset(data, 0, sizeof(WindowData));

  /* setup the local window data */
  data->eventMask      = eventMask;
  data->window         = window;
  data->currentPointer = this.defaultPointer;

  /* get the window bit depth */
  {
    xcb_get_geometry_cookie_t  c = xcb_get_geometry(this.xcb, window);
    xcb_get_geometry_reply_t * r = xcb_get_geometry_reply(this.xcb, c, NULL);
    if (!r)
    {
      xcb_destroy_window(this.xcb, window);
      return ADL_ERR_PLATFORM;
    }
    data->bpp = r->depth;
    free(r);
  }

  /* register for close events */
  changeProperty(XCB_PROP_MODE_REPLACE, window, IA_WM_PROTOCOLS,
    IA_XCB_ATOM_ATOM, 32, 1, &internAtom[IA_WM_DELETE_WINDOW].atom);

  /* set the window properties */
  setWindowProperties(data, def);
  xcbWindowSetClassName(result, def.className);
  xcbWindowSetTitle    (result, def.title    );
  return ADL_OK;
}

static ADL_STATUS xcbWindowDestroy(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);

  if (data->currentPointer != this.defaultPointer)
    xcb_free_cursor(this.xcb, data->currentPointer);

  xcb_destroy_window(this.xcb, data->window);
  return ADL_OK;
}

static ADL_STATUS xcbWindowShow(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_map_window(this.xcb, data->window);
  return ADL_OK;
}

static ADL_STATUS xcbWindowHide(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_unmap_window(this.xcb, data->window);
  return ADL_OK;
}

static ADL_STATUS xcbWindowSetTitle(ADLWindow * window, const char * title)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_window_t win  = data->window;

  const int len = strlen(title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_XCB_ATOM_WM_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_XCB_ATOM_WM_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_VISIBLE_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_VISIBLE_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, len, title);

  return ADL_OK;
}

static ADL_STATUS xcbWindowSetClassName(ADLWindow * window,
    const char * className)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_window_t win  = data->window;

  /* https://tronche.com/gui/x/icccm/sec-4.html#WM_CLASS
   * The WM_CLASS property (of type STRING without control characters) contains
   * two consecutive null-terminated strings */

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_XCB_ATOM_WM_CLASS,
    IA_XCB_ATOM_STRING, 8, strlen(className) + 1, className);

  changeProperty(XCB_PROP_MODE_APPEND, win, IA_XCB_ATOM_WM_CLASS,
    IA_XCB_ATOM_STRING, 8, strlen(className) + 1, className);

  return ADL_OK;
}

static ADL_STATUS xcbWindowSetGrab(ADLWindow * window, bool enable)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_window_t win  = data->window;

  if (data->grabbed == enable)
    return ADL_OK;

  if (enable)
  {
    xcb_grab_pointer_cookie_t c =
      xcb_grab_pointer(this.xcb, 1, win,
        data->eventMask, XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, win, XCB_NONE,
        XCB_TIME_CURRENT_TIME);

    xcb_grab_pointer_reply_t *reply;
    if ((reply = xcb_grab_pointer_reply(this.xcb, c, NULL)))
    {
      if (reply->status != XCB_GRAB_STATUS_SUCCESS)
      {
        ADL_ERROR(ADL_ERR_PLATFORM, "failed to grab the pointer");
        free(reply);
        return ADL_ERR_PLATFORM;
      }
      free(reply);
    }
  }
  else
  {
    xcb_void_cookie_t c =
      xcb_ungrab_pointer_checked(this.xcb, XCB_TIME_CURRENT_TIME);

    xcb_generic_error_t *error;
    if ((error = xcb_request_check(this.xcb, c)))
    {
      ADL_ERROR(ADL_ERR_PLATFORM, "failed to un-grab the pointer");
      free(error);
      return ADL_ERR_PLATFORM;
    }
  }

  data->grabbed = enable;
  return ADL_OK;
}

static ADL_STATUS xcbWindowSetRelative(ADLWindow * window, bool enable)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  if (data->relative == enable)
    return ADL_OK;

  data->relative = enable;
  return ADL_OK;
}

static ADL_STATUS xcbWindowSetFocus(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_set_input_focus(this.xcb, XCB_INPUT_FOCUS_PARENT, data->window,
      XCB_CURRENT_TIME);
  xcb_flush(this.xcb);
  return ADL_OK;
}

static ADL_STATUS xcbWindowEvent(ADLWindow * window, ADLEvent * event)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);

  const xcb_client_message_event_t xe =
  {
    .response_type = XCB_CLIENT_MESSAGE,
    .format        = 32,
    .sequence      = 0,
    .window        = data->window,
    .type          = getAtom(IA_ADL_EVENT),
    .data          = {
      {event->type}
    }
  };

  xcb_send_event(this.xcb, false, data->window, XCB_EVENT_MASK_NO_EVENT,
      (const char *)&xe);

  xcb_flush(this.xcb);

  return ADL_OK;
}

static ADL_STATUS xcbProcessEvent(int timeout, ADLEvent * event)
{
  xcb_generic_event_t * xevent;

  if (timeout < 0)
    xevent = xcb_wait_for_event(this.xcb);
  else
  {
    xevent = xcb_poll_for_queued_event(this.xcb);
    if (!xevent && timeout > 0)
    {
      fd_set fds;
      FD_ZERO(&fds);
      FD_SET (this.fd, &fds);
      struct timeval tv =
      {
        .tv_sec  = timeout / 1000,
        .tv_usec = (timeout % 1000) * 1000
      };

      if (select(this.fd + 1, &fds, NULL, NULL, &tv) == 0)
        return ADL_OK;

      xevent = xcb_poll_for_event(this.xcb);
    }
  }

  if (!xevent)
    return ADL_OK;


  ADL_STATUS status;
  const bool generated = (xevent->response_type & 0x80) == 0x80;

  switch(xevent->response_type & ~0x80)
  {
    case XCB_CLIENT_MESSAGE:
    {
      xcb_client_message_event_t * e = (xcb_client_message_event_t *)xevent;
      if (e->type == getAtom(IA_WM_PROTOCOLS))
      {
        if (e->data.data32[0] != getAtom(IA_WM_DELETE_WINDOW))
          break;

        event->type   = ADL_EVENT_CLOSE;
        event->window = windowFindById(e->window);
        break;
      }

      if (e->type == getAtom(IA_ADL_EVENT))
      {
        event->type   = ADL_EVENT_QUIT;
        event->window = windowFindById(e->window);
        break;
      }

      break;
    }

    case XCB_EXPOSE:
    {
      xcb_expose_event_t * e = (xcb_expose_event_t *)xevent;
      event->type   = ADL_EVENT_PAINT;
      event->window = windowFindById(e->window);
      event->u.paint.x    = e->x;
      event->u.paint.y    = e->y;
      event->u.paint.w    = e->width;
      event->u.paint.h    = e->height;
      event->u.paint.more = e->count > 0;
      break;
    }

    case XCB_MAP_NOTIFY:
    {
      xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)xevent;
      event->type   = ADL_EVENT_SHOW;
      event->window = windowFindById(e->event);
      break;
    }

    case XCB_UNMAP_NOTIFY:
    {
      xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)xevent;
      event->type   = ADL_EVENT_HIDE;
      event->window = windowFindById(e->event);
      break;
    }

    case XCB_VISIBILITY_NOTIFY:
    {
      xcb_visibility_notify_event_t * e =
        (xcb_visibility_notify_event_t *)xevent;

      if (e->state == XCB_VISIBILITY_FULLY_OBSCURED)
        event->type = ADL_EVENT_HIDE;
      else
        event->type = ADL_EVENT_SHOW;

      event->window = windowFindById(e->window);
      break;
    }

    case XCB_CONFIGURE_NOTIFY:
    {
      xcb_configure_notify_event_t * e =
        (xcb_configure_notify_event_t *)xevent;

      event->type   = ADL_EVENT_WINDOW_CHANGE;
      event->window = windowFindById(e->window);

      /* non-generated events need translating */
      if (!generated && event->window)
      {
        WindowData * data = ADL_GET_WINDOW_DATA(event->window);
        if ((status = getParentWindowOffset(e->window, data)) != ADL_OK)
          return status;

        e->x += data->transX;
        e->y += data->transY;
      }

      event->u.win.x = e->x;
      event->u.win.y = e->y;
      event->u.win.w = e->width;
      event->u.win.h = e->height;

      WindowData *data = ADL_GET_WINDOW_DATA(event->window);
      data->x = e->x;
      data->y = e->y;
      data->w = e->width;
      data->h = e->height;
      break;
    }

    case XCB_KEY_PRESS:
    {
      xcb_key_press_event_t * e = (xcb_key_press_event_t *)xevent;
      event->type            = ADL_EVENT_KEY_DOWN;
      event->window          = windowFindById(e->child ? e->child : e->event);
      event->u.key.keyname   = this.keyMap[e->detail - 8];
      event->u.key.scancode  = e->detail - 8;
      break;
    }

    case XCB_KEY_RELEASE:
    {
      xcb_key_release_event_t * e = (xcb_key_release_event_t *)xevent;
      event->type           = ADL_EVENT_KEY_UP;
      event->window         = windowFindById(e->child ? e->child : e->event);
      event->u.key.keyname  = this.keyMap[e->detail - 8];
      event->u.key.scancode = e->detail - 8;
      break;
    }

    case XCB_BUTTON_PRESS:
    {
      xcb_button_press_event_t * e = (xcb_button_press_event_t *)xevent;
      event->type      = ADL_EVENT_MOUSE_DOWN;

      event->window    = windowFindById(e->child ? e->child : e->event);
      WindowData *data = ADL_GET_WINDOW_DATA(event->window);
      event->u.mouse.x = e->event_x;
      event->u.mouse.y = e->event_y;

      switch(e->detail)
      {
        case 1: data->mouseButtonState |= ADL_MOUSE_BUTTON_LEFT   ; break;
        case 2: data->mouseButtonState |= ADL_MOUSE_BUTTON_MIDDLE ; break;
        case 3: data->mouseButtonState |= ADL_MOUSE_BUTTON_RIGHT  ; break;
        case 4: break;
        case 5: break;
        case 6: break;
        case 7: break;
        case 8: data->mouseButtonState |= ADL_MOUSE_BUTTON_BACK   ; break;
        case 9: data->mouseButtonState |= ADL_MOUSE_BUTTON_FORWARD; break;
        default:
          // custom buttons
          data->mouseButtonState |= ADL_MOUSE_BUTTON_CUSTOM | (1 << e->detail);
          break;
      }

      event->u.mouse.buttons = data->mouseButtonState;
      switch(e->detail)
      {
        case 4: event->u.mouse.buttons |= ADL_MOUSE_BUTTON_WUP   ; break;
        case 5: event->u.mouse.buttons |= ADL_MOUSE_BUTTON_WDOWN ; break;
        case 6: event->u.mouse.buttons |= ADL_MOUSE_BUTTON_WLEFT ; break;
        case 7: event->u.mouse.buttons |= ADL_MOUSE_BUTTON_WRIGHT; break;
      }
      break;
    }

    case XCB_BUTTON_RELEASE:
    {
      xcb_button_release_event_t * e = (xcb_button_release_event_t *)xevent;
      event->type      = ADL_EVENT_MOUSE_UP;
      event->window    = windowFindById(e->child ? e->child : e->event);

      WindowData *data = ADL_GET_WINDOW_DATA(event->window);
      event->u.mouse.x = e->event_x;
      event->u.mouse.y = e->event_y;

      switch(e->detail)
      {
        case 1: data->mouseButtonState &= ~ADL_MOUSE_BUTTON_LEFT   ; break;
        case 2: data->mouseButtonState &= ~ADL_MOUSE_BUTTON_MIDDLE ; break;
        case 3: data->mouseButtonState &= ~ADL_MOUSE_BUTTON_RIGHT  ; break;
        case 8: data->mouseButtonState &= ~ADL_MOUSE_BUTTON_BACK   ; break;
        case 9: data->mouseButtonState &= ~ADL_MOUSE_BUTTON_FORWARD; break;
        default:
          // custom buttons
          data->mouseButtonState &= ~ADL_MOUSE_BUTTON_CUSTOM | (1 << e->detail);
          break;
      }

      event->u.mouse.buttons = data->mouseButtonState;
      break;
    }

    case XCB_MOTION_NOTIFY:
    {
      xcb_motion_notify_event_t * e = (xcb_motion_notify_event_t *)xevent;
      event->type            = ADL_EVENT_MOUSE_MOVE;
      event->window          = windowFindById(e->child ? e->child : e->event);

      WindowData *data = ADL_GET_WINDOW_DATA(event->window);
      event->u.mouse.x       = e->event_x;
      event->u.mouse.y       = e->event_y;
      event->u.mouse.buttons = data->mouseButtonState;
      event->u.mouse.warping = data->warping;

      data->pointerX = e->event_x;
      data->pointerY = e->event_y;

      /* check for warp completion */
      if (data->warping)
      {
        /* if the warp completed */
        if (xevent->full_sequence >= data->warpCookie.sequence)
        {
          /* pass back the warp details for ADL to figure out */
          event->u.mouse.warp    = true;
          event->u.mouse.warpX   = e->event_x - data->warpX;
          event->u.mouse.warpY   = e->event_y - data->warpY;
          event->u.mouse.warping = false;
          data->warping          = false;
        }
        else
        {
          data->warpX = e->event_x;
          data->warpY = e->event_y;
        }
      }

      if (data->relative && !data->warping)
      {
        /* check if we hit the warp threshold */
        if (abs((data->w >> 1) - e->event_x) > (data->w >> 2) ||
            abs((data->h >> 1) - e->event_y) > (data->h >> 2))
        {
          xcbPointerWarp(event->window, data->w >> 1, data->h >> 1);
          xcb_flush(this.xcb);
        }
      }
      break;
    }

    case XCB_ENTER_NOTIFY:
    {
      xcb_enter_notify_event_t * e = (xcb_enter_notify_event_t *)xevent;

      event->type   = ADL_EVENT_MOUSE_ENTER;
      event->window = windowFindById(e->child ? e->child : e->event);
      WindowData *data = ADL_GET_WINDOW_DATA(event->window);

      event->u.mouse.x       = e->event_x;
      event->u.mouse.y       = e->event_y;
      event->u.mouse.buttons = data->mouseButtonState;
      event->u.mouse.warping = data->warping;

      data->pointerX = e->event_x;
      data->pointerY = e->event_y;
      break;
    }

    case XCB_LEAVE_NOTIFY:
    {
      xcb_leave_notify_event_t * e = (xcb_leave_notify_event_t *)xevent;

      event->type   = ADL_EVENT_MOUSE_LEAVE;
      event->window = windowFindById(e->child ? e->child : e->event);
      WindowData *data = ADL_GET_WINDOW_DATA(event->window);

      event->u.mouse.x       = e->event_x;
      event->u.mouse.y       = e->event_y;
      event->u.mouse.buttons = data->mouseButtonState;
      event->u.mouse.warping = data->warping;

      data->pointerX = e->event_x;
      data->pointerY = e->event_y;
      break;
    }
  }

  free(xevent);
  return ADL_OK;
}

static ADL_STATUS xcbFlush()
{
  xcb_flush(this.xcb);
  return ADL_OK;
}

static ADL_STATUS xcbPointerWarp(ADLWindow * window, int x, int y)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);

  data->warping    = true;
  data->warpX      = data->pointerX;
  data->warpY      = data->pointerY;
  data->warpCookie =
    xcb_warp_pointer(
      this.xcb,
      XCB_NONE,
      data->window,
      0, 0, 0, 0,
      x, y
    );

  return ADL_OK;
}

static ADL_STATUS xcbPointerVisible(ADLWindow * window, bool visible)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);

  if (visible)
  {
    xcb_change_window_attributes(this.xcb, data->window, XCB_CW_CURSOR,
        &(data->currentPointer));
  }
  else
  {
    xcb_change_window_attributes(this.xcb, data->window, XCB_CW_CURSOR,
        &this.blankPointer);
  }

  xcb_flush(this.xcb);
  return ADL_OK;
}

static ADL_STATUS xcbPointerSetCursor(ADLWindow * window, ADLImage * source,
    ADLImage * mask, int x, int y)
{
  WindowData * wData = ADL_GET_WINDOW_DATA(window);

  if (!source)
  {
    if (wData->currentPointer != this.defaultPointer)
      xcb_free_cursor(this.xcb, wData->currentPointer);

    wData->currentPointer = this.defaultPointer;
    xcb_change_window_attributes(this.xcb, wData->window, XCB_CW_CURSOR,
        &(wData->currentPointer));

    xcb_flush(this.xcb);
    return ADL_OK;
  }

  ImageData *  sData = ADL_GET_IMAGE_DATA(source);
  xcb_cursor_t cid   = xcb_generate_id(this.xcb);

  if (mask)
  {
    ImageData * mData = ADL_GET_IMAGE_DATA(mask);
    xcb_create_cursor(
      this.xcb,
      cid,
      sData->pixmap,
      mData->pixmap,
      0, 0, 0,
      0, 0, 0,
      x, y);
  }
  else
  {
    xcb_render_picture_t pic = xcb_generate_id(this.xcb);
    xcb_render_create_picture(this.xcb, pic, sData->pixmap, sData->format->id,
        0, 0);
    xcb_render_create_cursor(this.xcb, cid, pic, x, y);
    xcb_render_free_picture(this.xcb, pic);
  }

  if (wData->currentPointer != this.defaultPointer)
    xcb_free_cursor(this.xcb, wData->currentPointer);

  wData->currentPointer = cid;
  xcb_change_window_attributes(this.xcb, wData->window, XCB_CW_CURSOR, &cid);
  xcb_flush(this.xcb);
  return ADL_OK;
}

#if defined(ADL_HAS_EGL)
static ADL_STATUS xcbEGLGetDisplay(EGLDisplay ** display)
{
  *display = eglGetDisplay(this.display);
  if (*display == EGL_NO_DISPLAY)
  {
    *display = NULL;
    return ADL_ERR_PLATFORM;
  }

  return ADL_OK;
}

static ADL_STATUS xcbEGLCreateWindowSurface(EGLDisplay * display,
  EGLint * config, ADLWindow * window, const EGLint * attribs,
  EGLSurface * surface)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  *surface = eglCreateWindowSurface(display, config, data->window, attribs);
  if (!*surface)
    return ADL_ERR_PLATFORM;
  return ADL_OK;
}
#endif

static struct ADLPlatform xcb =
{
  .name               = "XCB",
  .test               = xcbTest,
  .init               = xcbInitialize,
  .deinit             = xcbDeinitialize,
  .processEvent       = xcbProcessEvent,
  .flush              = xcbFlush,

  .windowDataSize     = sizeof(WindowData),
  .windowCreate       = xcbWindowCreate,
  .windowDestroy      = xcbWindowDestroy,
  .windowShow         = xcbWindowShow,
  .windowHide         = xcbWindowHide,
  .windowSetTitle     = xcbWindowSetTitle,
  .windowSetClassName = xcbWindowSetClassName,
  .windowSetGrab      = xcbWindowSetGrab,
  .windowSetRelative  = xcbWindowSetRelative,
  .windowSetFocus     = xcbWindowSetFocus,
  .windowEvent        = xcbWindowEvent,

  .imageDataSize      = sizeof(ImageData),
  .imageGetSupported  = xcbImageGetSupported,
  .imageCreate        = xcbImageCreate,
  .imageDestroy       = xcbImageDestroy,
  .imageUpdate        = xcbImageUpdate,

  .pointerWarp        = xcbPointerWarp,
  .pointerVisible     = xcbPointerVisible,
  .pointerSetCursor   = xcbPointerSetCursor,

#if defined(ADL_HAS_EGL)
  .eglGetDisplay          = xcbEGLGetDisplay,
  .eglCreateWindowSurface = xcbEGLCreateWindowSurface
#endif
};

adl_platform(xcb);
