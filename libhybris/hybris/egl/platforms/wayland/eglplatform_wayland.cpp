/****************************************************************************************
 **
 ** Copyright (C) 2013 Jolla Ltd.
 ** Contact: Carsten Munk <carsten.munk@jollamobile.com>
 ** All rights reserved.
 **
 ** This file is part of Wayland enablement for libhybris
 **
 ** You may use this file under the terms of the GNU Lesser General
 ** Public License version 2.1 as published by the Free Software Foundation
 ** and appearing in the file license.lgpl included in the packaging
 ** of this file.
 **
 ** This library is free software; you can redistribute it and/or
 ** modify it under the terms of the GNU Lesser General Public
 ** License version 2.1 as published by the Free Software Foundation
 ** and appearing in the file license.lgpl included in the packaging
 ** of this file.
 ** 
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 ** Lesser General Public License for more details.
 ** 
 ****************************************************************************************/

#include <android-config.h>
#include <ws.h>
#include <malloc.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
extern "C" {
#include <eglplatformcommon.h>
};

extern "C" {
#include <wayland-client.h>
#include <wayland-egl.h>
}

#include "wayland_window.h"
#include "logging.h"
#include "wayland-egl-priv.h"

static gralloc_module_t *gralloc = 0;
static alloc_device_t *alloc = 0;

extern "C" void waylandws_init_module(struct ws_egl_interface *egl_iface)
{
	int err;
	hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const hw_module_t **) &gralloc);
	err = gralloc_open((const hw_module_t *) gralloc, &alloc);
	TRACE("++ %lu wayland: got gralloc %p err:%s", pthread_self(), gralloc, strerror(-err));
	eglplatformcommon_init(egl_iface, gralloc, alloc);
}

extern "C" int waylandws_IsValidDisplay(EGLNativeDisplayType display)
{
	return 1;
}

extern "C" EGLNativeWindowType waylandws_CreateWindow(EGLNativeWindowType win, EGLNativeDisplayType display)
{
	WaylandNativeWindow *window = new WaylandNativeWindow((struct wl_egl_window *) win, (struct wl_display *) display, alloc);
	window->common.incRef(&window->common);
	return (EGLNativeWindowType) static_cast<struct ANativeWindow *>(window);
}

extern "C" void waylandws_DestroyWindow(EGLNativeWindowType win)
{
	WaylandNativeWindow *window = static_cast<WaylandNativeWindow *>((struct ANativeWindow *)win);
	window->common.decRef(&window->common);
}

extern "C" int waylandws_post(EGLNativeWindowType win, void *buffer)
{
	struct wl_egl_window *eglwin = (struct wl_egl_window *) win;

	return ((WaylandNativeWindow *) eglwin->nativewindow)->postBuffer((ANativeWindowBuffer *) buffer);
}

extern "C" __eglMustCastToProperFunctionPointerType waylandws_eglGetProcAddress(const char *procname) 
{
	if (strcmp(procname, "eglHybrisWaylandPostBuffer") == 0)
	{
		return (__eglMustCastToProperFunctionPointerType) waylandws_post;
	}
	else
	return eglplatformcommon_eglGetProcAddress(procname);
}

extern "C" void waylandws_passthroughImageKHR(EGLContext *ctx, EGLenum *target, EGLClientBuffer *buffer, const EGLint **attrib_list)
{
	eglplatformcommon_passthroughImageKHR(ctx, target, buffer, attrib_list);
}

struct ws_module ws_module_info = {
	waylandws_init_module,
	waylandws_IsValidDisplay,
	waylandws_CreateWindow,
	waylandws_DestroyWindow,
	waylandws_eglGetProcAddress,
	waylandws_passthroughImageKHR,
	eglplatformcommon_eglQueryString
};







