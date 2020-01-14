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

#include "interface/adl.h"
#include "logging.h"

#include <xcb/xcb.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

struct State
{
  xcb_connection_t * xcb;
  xcb_screen_t     * screen;

  xcb_atom_t wmProtocols;
  xcb_atom_t wmDeleteWindow;
};

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

static ADL_STATUS getAtom(const char * name, xcb_atom_t * atom)
{
  assert(atom);

  xcb_generic_error_t *     error;
  xcb_intern_atom_cookie_t  c = xcb_intern_atom(this.xcb, 0, strlen(name), name);
  xcb_intern_atom_reply_t * r = xcb_intern_atom_reply(this.xcb, c, &error);

  if (error)
  {
    DEBUG_ERROR(ADL_ERR_PLATFORM, "xcb_intern_atom failed: code=%d, res=%d",
      error->error_code, error->resource_id);
    free(error);
    return ADL_ERR_PLATFORM;
  }

  *atom = r->atom;
  free(r);

  return ADL_OK;
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

  if ((status = getAtom("WM_PROTOCOLS", &this.wmProtocols)) != ADL_OK)
    goto err_disconnect;

  if ((status = getAtom("WM_DELETE_WINDOW", &this.wmDeleteWindow)) != ADL_OK)
    goto err_disconnect;

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
    XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS
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

  xcb_change_property(this.xcb, XCB_PROP_MODE_REPLACE, window, this.wmProtocols,
      XCB_ATOM_ATOM, 32, 1, &this.wmDeleteWindow);

  *result = (ADLWindow)((uintptr_t)window);
  return ADL_OK;
}

ADL_STATUS xcbWindowDestroy(ADLWindow window)
{
  xcb_window_t win = (xcb_window_t)((uintptr_t)window);
  xcb_destroy_window(this.xcb, win);
  xcb_flush(this.xcb);
  return ADL_OK;
}

ADL_STATUS xcbWindowShow(ADLWindow window)
{
  xcb_window_t win = (xcb_window_t)((uintptr_t)window);
  xcb_map_window(this.xcb, win);
  xcb_flush(this.xcb);
  return ADL_OK;
}

ADL_STATUS xcbWindowHide(ADLWindow window)
{
  xcb_window_t win = (xcb_window_t)((uintptr_t)window);
  xcb_unmap_window(this.xcb, win);
  xcb_flush(this.xcb);
  return ADL_OK;
}

static ADL_STATUS xcbProcessEvents(ADLEvent * event)
{
  xcb_generic_event_t * xevent;
  xevent = xcb_poll_for_event(this.xcb);
  if (!xevent)
  {
    event->type = ADL_EVENT_NONE;
    return ADL_OK;
  }

  //DEBUG_INFO(ADL_OK, "Event: %u", xevent->response_type);
  switch(xevent->response_type & ~0x80)
  {
    case XCB_CLIENT_MESSAGE:
    {
      xcb_client_message_event_t * e = (xcb_client_message_event_t *)xevent;
      if (e->type != this.wmProtocols)
        break;

      if (e->data.data32[0] != this.wmDeleteWindow)
        break;

      event->type           = ADL_EVENT_CLOSE;
      event->u.close.window = (ADLWindow)(uintptr_t)e->window;
      break;
    }
  }

  free(xevent);
  return ADL_OK;
}

static struct ADLPlatform xcb =
{
  .name          = "XCB",
  .test          = xcbTest,
  .init          = xcbInitialize,
  .deinit        = xcbDeinitialize,
  .processEvents = xcbProcessEvents,
  .windowCreate  = xcbWindowCreate,
  .windowDestroy = xcbWindowDestroy,
  .windowShow    = xcbWindowShow,
  .windowHide    = xcbWindowHide
};

adl_platform(xcb);
