//
// Created by Sumn on 2021/12/14.
//

#include "egl_core.h"

EGLCore::EGLCore() {
    pfneglPresentationTimeANDROID= nullptr;
    display=EGL_NO_DISPLAY;
    context=EGL_NO_CONTEXT;

}

EGLCore::~EGLCore() {

}
//egl对象释放
void EGLCore::release() {
    if (EGL_NO_DISPLAY!=display&&EGL_NO_CONTEXT!=context){
        eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
        LOGCATI("after eglMakeCurrent...");
        eglDestroyContext(display,context);
        LOGCATI("after eglDestroyContext...");
    }
    display=EGL_NO_DISPLAY;
    context=EGL_NO_CONTEXT;

}
//suface对象释放
void EGLCore::releaseSurface(EGLSurface eglSurface) {
    eglDestroySurface(display,eglSurface);
    eglSurface=EGL_NO_SURFACE;
}

EGLContext EGLCore::getContext() {
    return context;
}

EGLDisplay EGLCore::getDisplay() {
    return display;
}

EGLConfig EGLCore::getConfig() {
    return config;
}

EGLSurface EGLCore::createWindowSurface(ANativeWindow *_window) {
    EGLSurface surface= nullptr;
    EGLint format;
    if (_window== nullptr){
        LOGCATE("EGLCore::createWindowSurface  _window is NULL");
        return nullptr;
    }
    if (!eglGetConfigAttrib(display,config,EGL_NATIVE_VISUAL_ID,&format)){
        LOGCATE("eglGetConfigAttrib() returned error %d", eglGetError());
        release();
        return surface;
    }
    ANativeWindow_setBuffersGeometry(_window,0,0,format);
    if (!(surface=eglCreateWindowSurface(display,config,_window, nullptr))){
        LOGCATE("eglCreateWindowSurface() returned error %d", eglGetError());
    }
    return surface;
}

EGLSurface EGLCore::createOffscreenSurface(int width, int height) {
    EGLSurface surface;
    EGLint PbufferAttributes[]={EGL_WIDTH,width,EGL_HEIGHT,height,EGL_NONE,EGL_NONE};
    if (!(surface = eglCreatePbufferSurface(display, config, PbufferAttributes))) {
        LOGCATE("eglCreatePbufferSurface() returned error %d", eglGetError());
    }
    return surface;
}

int EGLCore::setPresentationTime(EGLSurface surface, khronos_stime_nanoseconds_t nsecs) {
    pfneglPresentationTimeANDROID(display,surface,nsecs);
    return 1;
}

int EGLCore::querySurface(EGLSurface surface, int what) {
    int  value=-1;
    eglQuerySurface(display,surface,what,&value);
    return value;
}

bool EGLCore::swapBuffers(EGLSurface eglSurface) {
    return eglSwapBuffers(display,eglSurface);
}

bool EGLCore::makeCurrent(EGLSurface eglSurface) {
    return eglMakeCurrent(display,eglSurface,eglSurface,context);
}

void EGLCore::doneCurrent() {
eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
}

bool EGLCore::init() {
    return this->init(nullptr);
}

bool EGLCore::initWithSharedContext() {
    EGLContext  context1=EglShareContext::getSharedContext();
    if (context1==EGL_NO_CONTEXT){
        return false;
    }
    return init(context1);
}

bool EGLCore::init(EGLContext sharedContext) {
    EGLint numConfigs;
    EGLint width;
    EGLint height;

    const EGLint attribs[] = { EGL_BUFFER_SIZE, 32, EGL_ALPHA_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                               EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };

    if ((display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGCATE("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(display, 0, 0)) {
        LOGCATE("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(display, attribs, &config, 1, &numConfigs)) {
        LOGCATE("eglChooseConfig() returned error %d", eglGetError());
        release();
        return false;
    }

    EGLint eglContextAttributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    if (!(context = eglCreateContext(display, config, NULL == sharedContext ? EGL_NO_CONTEXT : sharedContext, eglContextAttributes))) {
        LOGCATE("eglCreateContext() returned error %d", eglGetError());
        release();
        return false;
    }

    pfneglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC)eglGetProcAddress("eglPresentationTimeANDROID");
    if (!pfneglPresentationTimeANDROID) {
        LOGCATE("eglPresentationTimeANDROID is not available!");
    }

    return true;
}
