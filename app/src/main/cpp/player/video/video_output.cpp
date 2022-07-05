//
// Created by Sumn on 2021/12/14.
//

#include "video_output.h"
#define LOG_TAG "video_output"

VideoOutput::VideoOutput() {
    renderer = nullptr;
    handler = nullptr;
    queue = nullptr;
    surfaceWindow = nullptr;
    forceGetFrame = false;
    surfaceExists = false;
    eglCore = nullptr;
    isANativeWindowValid = false;
    renderTexSurface = EGL_NO_SURFACE;
    eglHasDestroyed = false;
}

VideoOutput::~VideoOutput() {

}

/** 初始化Output **/
bool VideoOutput::initOutPut(ANativeWindow *window, int screenWidth, int screenHeight,
                             getTextureCallback produceDataCallback, void *ctx) {
    LOGCATI("VideoOutput::initOutput");
    this->ctx = ctx;
    this->produceDataCallback = produceDataCallback;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    if (nullptr != window) {
        isANativeWindowValid = true;
    }
    queue = new MessageQueue("video output message queue");
    handler = new VideoOuPutHandler(this, queue);
    handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_CREATE_EGL_CONTEXT, window));
    pthread_create(&_threadId, nullptr, threadStartCallback, this);
    return true;
}

bool VideoOutput::createEglContext(ANativeWindow *window) {
    LOGCATI("enter VideoOutput::createEGLContext");
    eglCore = new EGLCore();
    LOGCATI("enter VideoOutput use sharecontext");
    bool ret = eglCore->initWithSharedContext();
    if (!ret) {
        LOGCATE("create EGL Context failed...");
        return false;
    }
    this->createWindowSurface(window);
    // must do this before share context in Huawei p6, or will crash
    eglCore->doneCurrent();
    return ret;
}

void *VideoOutput::threadStartCallback(void *myself) {
    VideoOutput *output = (VideoOutput *) myself;
    output->processMessage();
    pthread_exit(nullptr);
    return nullptr;
}

void VideoOutput::processMessage() {
    bool renderingEnabled = true;
    while (renderingEnabled) {
        Message *msg = nullptr;
        if (queue->dequeueMessage(&msg, true) > 0) {
//			LOGCATI("msg what is %d", msg->getWhat());
            if (MESSAGE_QUEUE_LOOP_QUIT_FLAG == msg->execute()) {
                renderingEnabled = false;
            }
            delete msg;
        }
    }
}

/** 当surface创建的时候的调用 **/
void VideoOutput::onSurfaceCreate(ANativeWindow *window) {
    LOGCATI("enter VideoOutput::onSurfaceCreated");
    if (handler) {
        isANativeWindowValid = true;
        handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_CREATE_WINDOW_SURFACE, window));
        handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_RENDER_FRAME));
    }
}

void VideoOutput::createWindowSurface(ANativeWindow *window) {
    LOGCATI("enter VideoOutput::createWindowSurface");
    this->surfaceWindow = window;
    renderTexSurface = eglCore->createWindowSurface(window);
    if (nullptr != renderTexSurface) {
        eglCore->makeCurrent(renderTexSurface);
        // must after makeCurrent
        renderer = new VideoGLSurfaceRender();
        // there must be right：1080, 810 for 4:3
        bool isGLViewInitialized = renderer->init(screenWidth, screenHeight);
        if (!isGLViewInitialized) {
            LOGCATI("GL View failed on initialized...");
        } else {
            surfaceExists = true;
            forceGetFrame = true;
        }
    }
    LOGCATI("Leave VideoOutput::createWindowSurface");
}

void VideoOutput::resetRenderSize(int left, int top, int width, int height) {
    if (width > 0 && height > 0) {
        screenWidth = width;
        screenHeight = height;
        if (nullptr != renderer) {
            renderer->resetRenderSize(left, top, width, height);
        }
    }
}

/** 绘制视频帧 **/
void VideoOutput::signalFrameAvailable() {
    if (surfaceExists) {
        if (handler) {
            handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_RENDER_FRAME));
        }
    }
}

bool VideoOutput::renderVideo() {
    FrameTexture *texture = nullptr;
    produceDataCallback(&texture, ctx, forceGetFrame);
    if (nullptr != texture && nullptr != renderer) {
        eglCore->makeCurrent(renderTexSurface);
        renderer->renderToViewWithAutoFill(texture->texId, screenWidth, screenHeight,
                                           texture->width, texture->height);
        if (!eglCore->swapBuffers(renderTexSurface)) {
            LOGCATE("eglSwapBuffers(renderTexSurface) returned error %d", eglGetError());
        }
    }
    if (forceGetFrame) {
        forceGetFrame = false;
    }
    return true;
}

void VideoOutput::onSurfaceDestroyed() {
    LOGCATI("enter VideoOutput::onSurfaceDestroyed");
    isANativeWindowValid = false;
    if (handler){
        handler->postMessage(new Message(VIDEO_OUTPUT_MESSAGE_DESTROY_WINDOW_SURFACE));
    }
}

void VideoOutput::destroyWindowSurface() {
    LOGCATI("enter VideoOutput::destroyWindowSurface");
    if (EGL_NO_SURFACE != renderTexSurface){
        if (renderer) {
            renderer->dealloc();
            delete renderer;
            renderer = NULL;
        }

        if (eglCore){
            eglCore->releaseSurface(renderTexSurface);
        }

        renderTexSurface = EGL_NO_SURFACE;
        surfaceExists = false;
        if(nullptr != surfaceWindow){
            LOGCATI("VideoOutput Releasing surfaceWindow");
            ANativeWindow_release(surfaceWindow);
            surfaceWindow = NULL;
        }
    }
}
/** 销毁Output **/
void VideoOutput::stopOutPut() {
    LOGCATI("enter VideoOutput::stopOutput");
    if (handler) {
        handler->postMessage(
                new Message(VIDEO_OUTPUT_MESSAGE_DESTROY_EGL_CONTEXT));
        handler->postMessage(new Message(MESSAGE_QUEUE_LOOP_QUIT_FLAG));
        pthread_join(_threadId, 0);
        if (queue) {
            queue->abort();
            delete queue;
            queue = NULL;
        }

        delete handler;
        handler = NULL;
    }
    LOGCATI("leave VideoOutput::stopOutput");
}

void VideoOutput::destroyEGLContext() {
    LOGCATI("enter VideoOutput::destroyEGLContext");
    if (EGL_NO_SURFACE != renderTexSurface){
        eglCore->makeCurrent(renderTexSurface);
    }
    this->destroyWindowSurface();

    if (NULL != eglCore){
        eglCore->release();
        delete eglCore;
        eglCore = NULL;
    }

    eglHasDestroyed = true;

    LOGCATI("leave VideoOutput::destroyEGLContext");
}
