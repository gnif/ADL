#include "xcb.h"
#include "image.h"

#include <xcb/dri3.h>
#include <xcb/present.h>

#include <stdlib.h>

ADL_STATUS xcbImageGetSupported(const ADLImageBackend ** result)
{
  static const ADLImageBackend supported[] =
  {
    ADL_IMAGE_BACKEND_DMABUF,
    ADL_IMAGE_BACKEND_BUFFER,
    0
  };
  *result = supported;
  return ADL_OK;
}

ADL_STATUS xcbImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage * result)
{
  WindowData * wdata = ADL_GET_WINDOW_DATA(window);
  ImageData  * idata = ADL_GET_IMAGE_DATA (result);

  idata->window = window;
  idata->def    = def;

  switch(def.format)
  {
    case ADL_IMAGE_FORMAT_RGBA:
      switch(def.bpp)
      {
        case 24: idata->format = &this.formatRGB ; break;
        case 32: idata->format = &this.formatRGBA; break;
      }
      break;

    case ADL_IMAGE_FORMAT_BGRA:
      switch(def.bpp)
      {
        case 24: idata->format = &this.formatBGR;  break;
        case 32: idata->format = &this.formatBGRA; break;
      }
      break;
  }

  if (!idata->format || idata->format->id == 0)
    return ADL_ERR_UNSUPPORTED;

  switch(def.backend)
  {
    case ADL_IMAGE_BACKEND_DMABUF:
    {
      idata->pixmap = xcb_generate_id(this.xcb);

      xcb_void_cookie_t c =
        xcb_dri3_pixmap_from_buffer_checked(
          this.xcb,
          idata->pixmap,
          wdata->window,
          def.h * def.pitch,
          def.w,
          def.h,
          def.pitch,
          def.bpp,
          def.depth,
          def.u.dmabuf.fd
        );

      xcb_generic_error_t *error;
      if ((error = xcb_request_check(this.xcb, c)))
      {
        ADL_ERROR(ADL_ERR_PLATFORM, "dri3_pixmap_from_buffer failure: code %d",
          error->error_code);
        free(error);
        return ADL_ERR_PLATFORM;
      }
      break;
    }

    case ADL_IMAGE_BACKEND_BUFFER:
    {
      idata->pixmap = xcb_generate_id(this.xcb);
      xcb_create_pixmap(
        this.xcb,
        def.depth,
        idata->pixmap,
        wdata->window,
        def.w,
        def.h
      );

      xcb_gcontext_t gc = xcb_generate_id(this.xcb);
      xcb_create_gc(this.xcb, gc, idata->pixmap, 0, 0);
      xcb_put_image(
        this.xcb,
        XCB_IMAGE_FORMAT_Z_PIXMAP,
        idata->pixmap,
        gc,
        def.w,
        def.h,
        0, 0,
        0,
        def.depth,
        def.h * def.pitch,
        def.u.buffer
      );
      xcb_free_gc(this.xcb, gc);
      break;
    }

    default:
      return ADL_ERR_UNSUPPORTED_BACKEND;
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
  ImageData  * idata = ADL_GET_IMAGE_DATA(image);
  WindowData * wdata = ADL_GET_WINDOW_DATA(idata->window);

  if (idata->def.bpp != wdata->bpp)
    return ADL_ERR_UNSUPPORTED_FORMAT;

  xcb_present_pixmap(
    this.xcb,
    wdata->window,
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
