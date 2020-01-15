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

#ifndef _H_LINUX_XCB_ATOMS
#define _H_LINUX_XCB_ATOMS

#include <xcb/xcb.h>

struct InternAtomDef
{
  const char * name;
  xcb_atom_t   atom;
};

#define INTERN_ATOMS \
  INTERN_ATOM1(WM_PROTOCOLS) \
  INTERN_ATOM1(WM_DELETE_WINDOW) \
  INTERN_ATOM2(_NET_WM_NAME) \
  INTERN_ATOM2(_NET_WM_VISIBLE_NAME) \
  INTERN_ATOM2(_NET_WM_ICON_NAME) \
  INTERN_ATOM2(_NET_WM_VISIBLE_ICON_NAME) \
  INTERN_ATOM2(_NET_WM_DESKTOP) \
  INTERN_ATOM2(_NET_WM_WINDOW_TYPE) \
  INTERN_ATOM2(_NET_WM_STATE) \
  INTERN_ATOM2(_NET_WM_ALLOWED_ACTIONS) \
  INTERN_ATOM2(_NET_WM_STRUT) \
  INTERN_ATOM2(_NET_WM_STRUT_PARTIAL) \
  INTERN_ATOM2(_NET_WM_ICON_GEOMETRY) \
  INTERN_ATOM2(_NET_WM_ICON) \
  INTERN_ATOM2(_NET_WM_PID) \
  INTERN_ATOM2(_NET_WM_HANDLED_ICONS) \
  INTERN_ATOM2(_NET_WM_USER_TIME) \
  INTERN_ATOM2(_NET_WM_USER_TIME_WINDOW) \
  INTERN_ATOM2(_NET_FRAME_EXTENTS) \
  INTERN_ATOM2(_NET_WM_OPAQUE_REGION) \
  INTERN_ATOM2(_NET_WM_BYPASS_COMPOSITOR) \
  \
  INTERN_ATOM3(XCB_ATOM_ATOM) \
  INTERN_ATOM3(XCB_ATOM_STRING) \
  INTERN_ATOM3(XCB_ATOM_WM_NAME) \
  INTERN_ATOM3(XCB_ATOM_WM_ICON_NAME)

#define INTERN_ATOM1(x) IA_ ##x,
#define INTERN_ATOM2(x) IA ##x,
#define INTERN_ATOM3(x) IA_ ##x,
typedef enum
{
  IA_INVALID = -1,
  INTERN_ATOMS
  IA_COUNT
}
InternAtom;
#undef INTERN_ATOM1
#undef INTERN_ATOM2
#undef INTERN_ATOM3

#define INTERN_ATOM1(x) {.name = #x, 0 },
#define INTERN_ATOM2(x) {.name = #x, 0 },
#define INTERN_ATOM3(x) {.name = #x, x },
static struct InternAtomDef internAtom[] = {INTERN_ATOMS};
#undef INTERN_ATOM1
#undef INTERN_ATOM2
#undef INTERN_ATOM3

inline static void _changeProperty(
  xcb_connection_t * xcb,
  const uint8_t      mode,
  const xcb_window_t window,
  const xcb_atom_t   property,
  const xcb_atom_t   type,
  const uint8_t      format,
  const uint32_t     data_len,
  const void   *     data)
{
  if (property == 0 || type == 0)
    return;

  xcb_change_property(
    xcb,
    mode,
    window,
    property,
    type,
    format,
    data_len,
    data
  );
}

#define changeProperty(mode, window, property,type, format, data_len, data) \
  _changeProperty(this.xcb, mode, window, internAtom[property].atom, \
    internAtom[type].atom, format, data_len, data)

#endif
