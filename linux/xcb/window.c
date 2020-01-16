#include "window.h"
#include "src/image.h"

ADL_STATUS xcbImageGetSupported(ADLImageBackend * result)
{
  return ADL_OK;
}

ADL_STATUS xcbImageCreate(ADLWindow * window, const ADLImageDef def,
    ADLImage * result)
{
  ADL_SET_IMAGE_ID(result, 10);
  return ADL_OK;
}

ADL_STATUS xcbImageDestroy(ADLImage * image)
{
  return ADL_OK;
}

ADL_STATUS xcbImageUpdate(ADLImage * image)
{
  return ADL_OK;
}
