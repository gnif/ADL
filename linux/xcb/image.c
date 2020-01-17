#include "xcb.h"
#include "image.h"

#include <xcb/dri3.h>
#include <xcb/present.h>

#include <stdlib.h>

ADL_STATUS xcbImageGetSupported(ADLImageBackend * result)
{
  *result = ADL_IMAGE_BACKEND_DMABUF;
  return ADL_OK;
}

ADL_STATUS xcbImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage * result)
{
  WindowData * wdata = ADL_GET_WINDOW_DATA(window);
  ImageData  * idata = ADL_GET_IMAGE_DATA (result);

  idata->window = wdata->window;
  idata->def    = def;

  /* for now we only support dmabuf */
  if (def.backend != ADL_IMAGE_BACKEND_DMABUF)
    return ADL_ERR_UNSUPPORTED_BACKEND;

  /* check the bpp matches the window, this is required for xcb_present_pixmap */
  {
    xcb_get_geometry_cookie_t c =
      xcb_get_geometry(this.xcb, idata->window);
    xcb_get_geometry_reply_t * r =
      xcb_get_geometry_reply(this.xcb, c, NULL);
    if (!r)
      return ADL_ERR_PLATFORM;
    if (r->depth != def.bpp)
      return ADL_ERR_UNSUPPORTED_FORMAT;
    free(r);
  }

  idata->pixmap = xcb_generate_id(this.xcb);

  {
    xcb_void_cookie_t c =
      xcb_dri3_pixmap_from_buffer_checked(
        this.xcb,
        idata->pixmap,
        idata->window,
        def.h * def.stride,
        def.w,
        def.h,
        def.stride,
        def.bpp,
        def.depth,
        def.u.dmabuf.fd
      );

    xcb_generic_error_t *error;
    if ((error = xcb_request_check(this.xcb, c)))
    {
      DEBUG_ERROR(ADL_ERR_PLATFORM, "dri3_pixmap_from_buffer failure: code %d",
        error->error_code);
      free(error);
      return ADL_ERR_PLATFORM;
    }
  }

  ADL_SET_IMAGE_ID(result, idata->pixmap);
  return ADL_OK;
}

ADL_STATUS xcbImageDestroy(ADLImage * image)
{
  ImageData * idata = ADL_GET_IMAGE_DATA(image);

  xcb_free_pixmap(this.xcb, idata->pixmap);
  return ADL_OK;
}

ADL_STATUS xcbImageUpdate(ADLImage * image)
{
  ImageData * idata = ADL_GET_IMAGE_DATA(image);

  xcb_present_pixmap(
    this.xcb,
    idata->window,
    idata->pixmap,
    idata->serial++,
    0, // valid
    0, // dirty
    0, // x
    0, // y
    0,
    0,
    0,
    XCB_PRESENT_OPTION_COPY,
    0, 0, 0, 0,
    NULL
  );

  return ADL_OK;
}
