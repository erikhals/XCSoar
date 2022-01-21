/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2021 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "ui/canvas/custom/TopCanvas.hpp"
#include "ConfigChooser.hpp"
#include "GBM.hpp"
#include "ui/canvas/opengl/Init.hpp"
#include "ui/canvas/opengl/Globals.hpp"
#include "ui/opengl/Features.hpp"
#include "system/Error.hxx"
#include "util/RuntimeError.hxx"

#include <stdio.h>

#ifdef MESA_KMS
#include "Hardware/DisplayDPI.hpp"

#include <span>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef RASPBERRY_PI
/* on the Raspberry Pi 4, /dev/dri/card1 is the VideoCore IV (the
   "legacy mode") which we want to use for now), and /dev/dri/card0 is
   V3D which I havn't figured out yet */
static constexpr const char *DEFAULT_DRI_DEVICE = "/dev/dri/card1";
#else
constexpr const char * DEFAULT_DRI_DEVICE = "/dev/dri/card0";
#endif

struct drm_fb {
  struct gbm_bo *bo;
  uint32_t fb_id;
  int dri_fd;
};
#endif

/**
 * Returns the EGL API to bind to using eglBindAPI().
 */
static constexpr EGLenum
GetBindAPI()
{
  return HaveGLES()
    ? EGL_OPENGL_ES_API
    : EGL_OPENGL_API;
}

#ifdef MESA_KMS

static int
OpenDriDevice()
{
  const char* dri_device = getenv("DRI_DEVICE");
  if (nullptr == dri_device)
    dri_device = DEFAULT_DRI_DEVICE;
  printf("Using DRI device %s (use environment variable "
           "DRI_DEVICE to override)\n",
         dri_device);
  int dri_fd = open(dri_device, O_RDWR);
  if (dri_fd == -1)
    throw FormatErrno("Could not open DRI device %s", dri_device);

  return dri_fd;
}

static drmModeConnector *
ChooseConnector(int dri_fd, const std::span<const uint32_t> connectors)
{
  for (const auto id : connectors) {
    auto *connector = drmModeGetConnector(dri_fd, id);
    if (connector != nullptr && connector->connection == DRM_MODE_CONNECTED &&
        connector->count_modes > 0)
      return connector;

    drmModeFreeConnector(connector);
  }

  throw std::runtime_error("No usable DRM connector found");
}

static drmModeConnector *
ChooseConnector(int dri_fd, const drmModeRes &resources)
{
  const std::span connectors{
    resources.connectors, std::size_t(resources.count_connectors),
  };

  return ChooseConnector(dri_fd, connectors);
}

#endif

#if !defined(USE_X11) && !defined(USE_WAYLAND)

TopCanvas::TopCanvas()
{
#if defined(MESA_KMS)
  dri_fd = OpenDriDevice();

  native_display = gbm_create_device(dri_fd);
  if (native_display == nullptr)
    throw std::runtime_error("Could not create GBM device");

  evctx = { 0 };
  evctx.version = DRM_EVENT_CONTEXT_VERSION;
  evctx.page_flip_handler = [](int fd, unsigned int frame, unsigned int sec,
                               unsigned int usec, void *flip_finishedPtr) {
    *reinterpret_cast<bool*>(flip_finishedPtr) = true;
  };

  drmModeRes *resources = drmModeGetResources(dri_fd);
  if (resources == nullptr)
    throw std::runtime_error("drmModeGetResources() failed");

  connector = ChooseConnector(dri_fd, *resources);

  if (auto *encoder = drmModeGetEncoder(dri_fd, connector->encoder_id)) {
    crtc_id = encoder->crtc_id;
    drmModeFreeEncoder(encoder);
  } else
    throw std::runtime_error("No usable DRM encoder found");

  mode = connector->modes[0];

  native_window = gbm_surface_create(native_display, mode.hdisplay,
                                     mode.vdisplay,
                                     XCSOAR_GBM_FORMAT,
                                     GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
  if (native_window == nullptr)
    throw std::runtime_error("Could not create GBM surface");

  if (connector->mmWidth > 0 && connector->mmHeight > 0)
    Display::ProvideSizeMM(mode.hdisplay, mode.vdisplay,
                           connector->mmWidth,
                           connector->mmHeight);
#endif

  CreateEGL(native_display, native_window);
}

#endif

void
TopCanvas::CreateEGL(EGLNativeDisplayType native_display,
                     EGLNativeWindowType native_window)
{
  display = eglGetDisplay(native_display);
  if (display == EGL_NO_DISPLAY)
    throw std::runtime_error("eglGetDisplay(EGL_DEFAULT_DISPLAY) failed");

  if (!eglInitialize(display, nullptr, nullptr))
    throw std::runtime_error("eglInitialize() failed");

  if (!eglBindAPI(GetBindAPI()))
    throw std::runtime_error("eglBindAPI() failed");

  const EGLConfig chosen_config = EGL::ChooseConfig(display);

  surface = eglCreateWindowSurface(display, chosen_config,
                                   native_window, nullptr);
  if (surface == nullptr)
    throw FormatRuntimeError("eglCreateWindowSurface() failed: %#x", eglGetError());

  const PixelSize effective_size = GetNativeSize();
  if (effective_size.width == 0 || effective_size.height == 0)
    throw std::runtime_error("eglQuerySurface() failed");

#ifdef HAVE_GLES2
  static constexpr EGLint context_attributes[] = {
    EGL_CONTEXT_CLIENT_VERSION, 2,
    EGL_NONE
  };
#else
  const EGLint *context_attributes = nullptr;
#endif

  context = eglCreateContext(display, chosen_config,
                             EGL_NO_CONTEXT, context_attributes);

  if (!eglMakeCurrent(display, surface, surface, context))
    throw std::runtime_error("eglMakeCurrent() failed");

  OpenGL::SetupContext();
  SetupViewport(effective_size);
}

TopCanvas::~TopCanvas() noexcept
{
  eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
  eglDestroySurface(display, surface);
  eglDestroyContext(display, context);
  eglTerminate(display);

#ifdef MESA_KMS
  if (nullptr != saved_crtc)
    drmModeSetCrtc(dri_fd, saved_crtc->crtc_id, saved_crtc->buffer_id,
                   saved_crtc->x, saved_crtc->y, &connector->connector_id, 1,
                   &saved_crtc->mode);
  gbm_surface_destroy(native_window);
  gbm_device_destroy(native_display);
  close(dri_fd);
#endif
}

PixelSize
TopCanvas::GetNativeSize() const
{
  GLint w, h;
  if (!eglQuerySurface(display, surface, EGL_WIDTH, &w) ||
      !eglQuerySurface(display, surface, EGL_HEIGHT, &h) ||
      w <= 0 || h <= 0)
    return PixelSize(0, 0);

  return PixelSize(w, h);
}

void
TopCanvas::Flip()
{
  if (!eglSwapBuffers(display, surface)) {
    fprintf(stderr, "eglSwapBuffers() failed: 0x%x\n", eglGetError());
    exit(EXIT_FAILURE);
  }

#ifdef MESA_KMS
  gbm_bo *new_bo = gbm_surface_lock_front_buffer(native_window);

  drm_fb *fb = (drm_fb*) gbm_bo_get_user_data(new_bo);
  if (!fb) {
    fb = new drm_fb;
    fb->bo = new_bo;
    fb->dri_fd = dri_fd;

    int ret = drmModeAddFB(dri_fd, gbm_bo_get_width(new_bo),
                           gbm_bo_get_height(new_bo), 24, 32,
                           gbm_bo_get_stride(new_bo),
                           gbm_bo_get_handle(new_bo).u32, &fb->fb_id);
    if (ret != 0) {
      fprintf(stderr, "drmModeAddFB() failed: %d\n", ret);
      exit(EXIT_FAILURE);
    }
  }

  gbm_bo_set_user_data(new_bo,
                       fb,
                       [](struct gbm_bo *bo, void *data) {
    struct drm_fb *fb = (struct drm_fb*) data;
    if (fb->fb_id)
      drmModeRmFB(fb->dri_fd, fb->fb_id);

    delete fb;
  });

  if (nullptr == current_bo) {
    saved_crtc = drmModeGetCrtc(dri_fd, crtc_id);
    drmModeSetCrtc(dri_fd, crtc_id, fb->fb_id, 0, 0,
                   &connector->connector_id, 1, &mode);
  } else {

    bool flip_finished = false;
    int page_flip_ret = drmModePageFlip(dri_fd, crtc_id, fb->fb_id,
                                        DRM_MODE_PAGE_FLIP_EVENT,
                                        &flip_finished);
    if (0 != page_flip_ret) {
      fprintf(stderr, "drmModePageFlip() failed: %d\n", page_flip_ret);
      exit(EXIT_FAILURE);
    }
    while (!flip_finished) {
      int handle_event_ret = drmHandleEvent(dri_fd, &evctx);
      if (0 != handle_event_ret) {
        fprintf(stderr, "drmHandleEvent() failed: %d\n", handle_event_ret);
        exit(EXIT_FAILURE);
      }
    }

    gbm_surface_release_buffer(native_window, current_bo);
  }

  current_bo = new_bo;
#endif
}
