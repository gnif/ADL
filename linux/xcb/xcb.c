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

#include "adl.h"
#include "window.h"

#include "interface/adl.h"
#include "logging.h"
#include "atoms.h"

#include <xcb/xcb.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct State
{
  xcb_connection_t * xcb;
  int                fd;
  xcb_screen_t     * screen;

  ADLMouseButton mouseButtonState;
};

typedef struct
{
  xcb_window_t window;
}
WindowData;

static struct State this;

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

static void updateWindowProperties(xcb_window_t window, const ADLWindowDef props)
{
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
      changeProperty(XCB_PROP_MODE_REPLACE, window, IA_NET_WM_STATE,
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

    changeProperty(XCB_PROP_MODE_REPLACE, window, IA_NET_WM_WINDOW_TYPE,
        IA_XCB_ATOM_ATOM, 32, typeCount, type);

#undef SET_TYPE
  }

  if (haveAtom(IA_MOTIF_WM_HINTS))
  {
    struct MotifHints hints = {.flags = props.borderless ? 2 : 0};
    changeProperty(XCB_PROP_MODE_REPLACE, window, IA_MOTIF_WM_HINTS,
        IA_XCB_ATOM_INTEGER, 32, 5, &hints);
  }
}

static ADL_STATUS xcbTest()
{
  ADL_STATUS status = ADL_ERR_UNSUPPORTED;
  /* try and connect to the display */
  this.xcb = xcb_connect(NULL, NULL);
  if (xcb_connection_has_error(this.xcb) == 0)
  {
    xcb_disconnect(this.xcb);
    status = ADL_OK;
  }

    this.xcb = NULL;
    return status;
}

static ADL_STATUS xcbInitialize()
{
  ADL_STATUS status = ADL_OK;
  int err;
  this.xcb = xcb_connect(NULL, NULL);
  if ((err = xcb_connection_has_error(this.xcb)) != 0)
  {
    status = ADL_ERR_PLATFORM;
    DEBUG_ERROR(status, "XCB connection failed with: %s", xcbErrString(err));
    goto err_out;
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
      DEBUG_ERROR(ADL_ERR_PLATFORM, "xcb_intern_atom failed: code=%d, res=%d",
        error->error_code, error->resource_id);
      free(error);
      status = ADL_ERR_PLATFORM;
      goto err_disconnect;
    }

    internAtom[i].atom = r->atom;
    free(r);
  }

  this.fd = xcb_get_file_descriptor(this.xcb);
  return status;

err_disconnect:
  xcb_disconnect(this.xcb);
  this.xcb = NULL;
err_out:
  return status;
}

static ADL_STATUS xcbDeinitialize()
{
  xcb_disconnect(this.xcb);
  this.xcb = NULL;
  return ADL_OK;
}

ADL_STATUS xcbWindowCreate(const ADLWindowDef def, ADLWindow * result)
{
  const uint32_t values[2] =
  {
    this.screen->white_pixel,
    XCB_EVENT_MASK_EXPOSURE         |
    XCB_EVENT_MASK_STRUCTURE_NOTIFY | XCB_EVENT_MASK_VISIBILITY_CHANGE |
    XCB_EVENT_MASK_KEY_PRESS        | XCB_EVENT_MASK_KEY_RELEASE       |
    XCB_EVENT_MASK_BUTTON_PRESS     | XCB_EVENT_MASK_BUTTON_RELEASE    |
    XCB_EVENT_MASK_POINTER_MOTION
  };

  xcb_window_t window = xcb_generate_id(this.xcb);
  xcb_void_cookie_t c =
    xcb_create_window_checked(
      this.xcb,
      XCB_COPY_FROM_PARENT,
      window,
      this.screen->root,
      def.x, def.y,
      def.w, def.h,
      0,
      XCB_WINDOW_CLASS_INPUT_OUTPUT,
      this.screen->root_visual,
      XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
      values
    );

  xcb_generic_error_t *error;
  if ((error = xcb_request_check(this.xcb, c)))
  {
    DEBUG_ERROR(ADL_ERR_PLATFORM, "xcb_create_window failed: code=%d, res=%d",
      error->error_code, error->resource_id);
    free(error);
    return ADL_ERR_PLATFORM;
  }

  changeProperty(XCB_PROP_MODE_REPLACE, window, IA_WM_PROTOCOLS,
    IA_XCB_ATOM_ATOM, 32, 1, &internAtom[IA_WM_DELETE_WINDOW].atom);

  updateWindowProperties(window, def);

  WindowData * win = ADL_GET_WINDOW_DATA(result);
  win->window = window;
  return ADL_OK;
}

ADL_STATUS xcbWindowDestroy(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_destroy_window(this.xcb, data->window);
  xcb_flush(this.xcb);
  return ADL_OK;
}

ADL_STATUS xcbWindowShow(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_map_window(this.xcb, data->window);
  xcb_flush(this.xcb);
  return ADL_OK;
}

ADL_STATUS xcbWindowHide(ADLWindow * window)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_unmap_window(this.xcb, data->window);
  xcb_flush(this.xcb);
  return ADL_OK;
}

ADL_STATUS xcbWindowSetTitle(ADLWindow * window, const char * title)
{
  WindowData * data = ADL_GET_WINDOW_DATA(window);
  xcb_window_t win  = data->window;

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_XCB_ATOM_WM_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_XCB_ATOM_WM_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_VISIBLE_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  changeProperty(XCB_PROP_MODE_REPLACE, win, IA_NET_WM_VISIBLE_ICON_NAME,
    IA_XCB_ATOM_STRING, 8, strlen(title), title);

  return ADL_OK;
}

static ADL_STATUS xcbProcessEvent(int timeout, ADLEvent * event, void ** window)
{
  xcb_generic_event_t * xevent;

  if (timeout < 0)
    xevent = xcb_wait_for_event(this.xcb);
  else
  {
    if (timeout > 0)
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
    }

    xevent = xcb_poll_for_event(this.xcb);
  }

  if (!xevent)
    return ADL_OK;

  const bool generated = (xevent->response_type & 0x80) == 0x80;
  switch(xevent->response_type & ~0x80)
  {
    case XCB_CLIENT_MESSAGE:
    {
      xcb_client_message_event_t * e = (xcb_client_message_event_t *)xevent;
      if (e->type != internAtom[IA_WM_PROTOCOLS].atom)
        break;

      if (e->data.data32[0] != internAtom[IA_WM_DELETE_WINDOW].atom)
        break;

      event->type = ADL_EVENT_CLOSE;
      *window        = (void *)(uintptr_t)e->window;
      break;
    }

    case XCB_MAP_NOTIFY:
    {
      xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)xevent;
      event->type = ADL_EVENT_SHOW;
      *window        = (void *)(uintptr_t)e->window;
      break;
    }

    case XCB_UNMAP_NOTIFY:
    {
      xcb_map_notify_event_t * e = (xcb_map_notify_event_t *)xevent;
      event->type = ADL_EVENT_HIDE;
      *window        = (void *)(uintptr_t)e->window;
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

      *window = (void *)(uintptr_t)e->window;
      break;
    }

    case XCB_CONFIGURE_NOTIFY:
    {
      xcb_configure_notify_event_t * e =
        (xcb_configure_notify_event_t *)xevent;

      /* non-generated events need translating */
      if (!generated)
      {
        xcb_window_t parent;
        {
          xcb_query_tree_cookie_t c =
            xcb_query_tree(this.xcb, e->window);

          xcb_generic_error_t *error;
          xcb_query_tree_reply_t *r =
            xcb_query_tree_reply(this.xcb, c, &error);

          if (error)
          {
            DEBUG_ERROR(ADL_ERR_PLATFORM,
              "xcb_query_tree failed: code=%d, res=%d",
              error->error_code, error->resource_id);
            free(error);
            return ADL_ERR_PLATFORM;
          }

          parent = r->parent;
          free(r);
        }

        xcb_translate_coordinates_cookie_t c =
          xcb_translate_coordinates(this.xcb, parent, this.screen->root,
              e->x, e->y);

        xcb_generic_error_t *error;
        xcb_translate_coordinates_reply_t *r =
          xcb_translate_coordinates_reply(this.xcb, c, &error);

        if (error)
        {
          DEBUG_ERROR(ADL_ERR_PLATFORM,
            "xcb_translate_coordinates failed: code=%d, res=%d",
            error->error_code, error->resource_id);
          free(error);
          return ADL_ERR_PLATFORM;
        }

        e->x = r->dst_x;
        e->y = r->dst_y;
        free(r);
      }

      event->type    = ADL_EVENT_WINDOW_CHANGE;
      event->u.win.x = e->x;
      event->u.win.y = e->y;
      event->u.win.w = e->width;
      event->u.win.h = e->height;

      *window = (void *)(uintptr_t)e->window;
      break;
    }

    case XCB_KEY_PRESS:
    {
      xcb_key_press_event_t * e = (xcb_key_press_event_t *)xevent;
      event->type           = ADL_EVENT_KEY_DOWN;
      event->u.key.scancode = e->detail;
      *window = (void *)(uintptr_t)e->event;
      break;
    }

    case XCB_KEY_RELEASE:
    {
      xcb_key_release_event_t * e = (xcb_key_release_event_t *)xevent;
      event->type           = ADL_EVENT_KEY_UP;
      event->u.key.scancode = e->detail;
      *window = (void *)(uintptr_t)e->event;
      break;
    }

    case XCB_BUTTON_PRESS:
    {
      xcb_button_press_event_t * e = (xcb_button_press_event_t *)xevent;
      event->type      = ADL_EVENT_MOUSE_DOWN;
      event->u.mouse.x = e->event_x;
      event->u.mouse.y = e->event_y;
      *window = (void *)(uintptr_t)e->event;

      switch(e->detail)
      {
        case 1: this.mouseButtonState |= ADL_MOUSE_BUTTON_LEFT   ; break;
        case 2: this.mouseButtonState |= ADL_MOUSE_BUTTON_MIDDLE ; break;
        case 3: this.mouseButtonState |= ADL_MOUSE_BUTTON_RIGHT  ; break;
        case 4: break;
        case 5: break;
        case 6: break;
        case 7: break;
        case 8: this.mouseButtonState |= ADL_MOUSE_BUTTON_BACK   ; break;
        case 9: this.mouseButtonState |= ADL_MOUSE_BUTTON_FORWARD; break;
        default:
          // custom buttons
          this.mouseButtonState |= ADL_MOUSE_BUTTON_CUSTOM | (1 << e->detail);
          break;
      }

      event->u.mouse.buttons = this.mouseButtonState;
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
      event->u.mouse.x = e->event_x;
      event->u.mouse.y = e->event_y;
      *window = (void *)(uintptr_t)e->event;

      switch(e->detail)
      {
        case 1: this.mouseButtonState &= ~ADL_MOUSE_BUTTON_LEFT   ; break;
        case 2: this.mouseButtonState &= ~ADL_MOUSE_BUTTON_MIDDLE ; break;
        case 3: this.mouseButtonState &= ~ADL_MOUSE_BUTTON_RIGHT  ; break;
        case 8: this.mouseButtonState &= ~ADL_MOUSE_BUTTON_BACK   ; break;
        case 9: this.mouseButtonState &= ~ADL_MOUSE_BUTTON_FORWARD; break;
        default:
          // custom buttons
          this.mouseButtonState &= ~ADL_MOUSE_BUTTON_CUSTOM | (1 << e->detail);
          break;
      }

      event->u.mouse.buttons = this.mouseButtonState;
      break;
    }

    case XCB_MOTION_NOTIFY:
    {
      xcb_motion_notify_event_t * e = (xcb_motion_notify_event_t *)xevent;
      event->type            = ADL_EVENT_MOUSE_MOVE;
      event->u.mouse.x       = e->event_x;
      event->u.mouse.y       = e->event_y;
      event->u.mouse.buttons = this.mouseButtonState;
      *window = (void *)(uintptr_t)e->event;
      break;
    }
  }

  free(xevent);
  return ADL_OK;
}

static struct ADLPlatform xcb =
{
  .name           = "XCB",
  .test           = xcbTest,
  .init           = xcbInitialize,
  .deinit         = xcbDeinitialize,
  .processEvent   = xcbProcessEvent,
  .windowDataSize = sizeof(WindowData),
  .windowCreate   = xcbWindowCreate,
  .windowDestroy  = xcbWindowDestroy,
  .windowShow     = xcbWindowShow,
  .windowHide     = xcbWindowHide,
  .windowSetTitle = xcbWindowSetTitle
};

adl_platform(xcb);
