//
// Created by Sumn on 2021/12/14.
//

#ifndef LEARNFFMPEG_EGL_CORE_H
#define LEARNFFMPEG_EGL_CORE_H

#include <pthread.h>
#include <android/native_window.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <KHR/khrplatform.h>
#include "common_tools.h"
#include "egl_share_context.h"

typedef EGLBoolean (EGLAPIENTRYP PFNEGLPRESENTATIONTIMEANDROIDPROC)(EGLDisplay display,
                                                                    EGLSurface surface,
                                                                    khronos_stime_nanoseconds_t time);

class EGLCore {
public:
    EGLCore();

    virtual ~EGLCore();

    bool init();

    bool init(EGLContext sharedContext);

    bool initWithSharedContext();

    EGLSurface createWindowSurface(ANativeWindow *_window);

    EGLSurface createOffscreenSurface(int width, int height);

    bool makeCurrent(EGLSurface eglSurface);

    void doneCurrent();

    bool swapBuffers(EGLSurface eglSurface);

    int querySurface(EGLSurface surface, int what);

    int setPresentationTime(EGLSurface surface, khronos_stime_nanoseconds_t nsecs);

    void releaseSurface(EGLSurface eglSurface);

    void release();

    EGLContext getContext();

    EGLDisplay getDisplay();

    EGLConfig getConfig();

private:
    EGLDisplay display;
    EGLConfig config;
    EGLContext context;

    PFNEGLPRESENTATIONTIMEANDROIDPROC pfneglPresentationTimeANDROID;


};


#endif //LEARNFFMPEG_EGL_CORE_H
