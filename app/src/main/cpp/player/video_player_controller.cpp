//
// Created by Sumn on 2021/12/15.
//

#include "video_player_controller.h"

#define LOG_TAG "video_player_controller"
/*
 * class VideoPlayerController
 *
 */
VideoPlayerController::VideoPlayerController() {
    userCancelled = false;

    videoOutput = NULL;
    audioOutput = NULL;
    synchronizer = NULL;

    screenWidth = 0;
    screenHeight = 0;
}

VideoPlayerController::~VideoPlayerController() {
    LOGCATI("~VideoPlayerController");

    videoOutput = NULL;
    audioOutput = NULL;
    synchronizer = NULL;
}

void VideoPlayerController::signalOutputFrameAvailable() {
//  LOGCATI("signalOutputFrameAvailable");
    if (NULL != videoOutput){
        videoOutput->signalFrameAvailable();
    }
}

bool VideoPlayerController::initAVSynchronizer() {
    synchronizer = new AVSynchronizer();
    return synchronizer->init(requestHeader, g_jvm, obj, minBufferedDuration, maxBufferedDuration);
}

void  VideoPlayerController::initVideoOutput(ANativeWindow* window){
    LOGCATI("VideoPlayerController::initVideoOutput beigin width:%d, height:%d", screenWidth, screenHeight);
    if (window == NULL || userCancelled){
        return;
    }
    videoOutput = new VideoOutput();
    videoOutput->initOutPut(window, screenWidth, screenHeight,videoCallbackGetTex, this);
}

bool VideoPlayerController::startAVSynchronizer() {
    LOGCATI("enter VideoPlayerController::startAVSynchronizer...");
    bool ret = false;

    if (userCancelled) {
        return ret;
    }

    if (this->initAVSynchronizer()) {
        if (synchronizer->validAudio()) {
            ret = this->initAudioOutput();
        }
    }
    if(ret){
        if (NULL != synchronizer && !synchronizer->isValid()) {
            ret = false;
        } else{
            isPlaying = true;
            synchronizer->start();
            LOGCATI("call audioOutput start...");
            if (NULL != audioOutput) {
                SLresult sLresult=   audioOutput->start();
                if (sLresult!=SL_RESULT_SUCCESS){
                    LOGCATE("audio error");
                }
            }
            LOGCATI("After call audioOutput start...");
        }
    }

    LOGCATI("VideoPlayerController::startAVSynchronizer() init result:%s", (ret? "success" : "fail"));
    this->setInitializedStatus(ret);

    return ret;
}

int VideoPlayerController::videoCallbackGetTex(FrameTexture** frameTex, void* ctx, bool forceGetFrame){
    VideoPlayerController* playerController = (VideoPlayerController*) ctx;
    return playerController->getCorrectRenderTexture(frameTex, forceGetFrame);
}

int VideoPlayerController::getCorrectRenderTexture(FrameTexture** frameTex, bool forceGetFrame){
    int ret = -1;

    if (!synchronizer->isDestroyed) {
        if(synchronizer->isPlayCompleted()) {
            LOGCATI("Video Render Thread render Completed We will Render First Frame...");
            (*frameTex) = synchronizer->getFirstRenderTexture();
        } else {
            (*frameTex) = synchronizer->getCorrectRenderTexture(forceGetFrame);
        }
        ret = 0;
    }
    return ret;
}

void VideoPlayerController::onSurfaceCreated(ANativeWindow* window, int width, int height) {
    LOGCATI("enter VideoPlayerController::onSurfaceCreated...");

    if (window != NULL){
        this->window = window;
    }

    if (userCancelled){
        return;
    }

    if (width > 0 && height > 0){
        this->screenHeight = height;
        this->screenWidth = width;
    }
    if (!videoOutput) {
        initVideoOutput(window);
    }else{
        videoOutput->onSurfaceCreate(window);
    }
    LOGCATI("Leave VideoPlayerController::onSurfaceCreated...");
}

void VideoPlayerController::onSurfaceDestroyed() {
    LOGCATI("enter VideoPlayerController::onSurfaceDestroyed...");
    if (videoOutput) {
        videoOutput->onSurfaceDestroyed();
    }
}

int VideoPlayerController::audioCallbackFillData(byte* outData, size_t bufferSize, void* ctx) {
    VideoPlayerController* playerController = (VideoPlayerController*) ctx;
    return playerController->consumeAudioFrames(outData, bufferSize);
}

void VideoPlayerController::setInitializedStatus(bool initCode) {
    LOGCATI("enter VideoPlayerController::setInitializedStatus...");
    JNIEnv *env = 0;
    int status = 0;
    bool needAttach = false;
    status = g_jvm->GetEnv((void **) (&env), JNI_VERSION_1_4);

    // don't know why, if detach directly, will crash
    if (status < 0) {
        if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
            LOGCATE("%s: AttachCurrentThread() failed", __FUNCTION__);
            return;
        }

        needAttach = true;
    }

    jclass jcls = env->GetObjectClass(obj);

    jmethodID onInitializedFunc = env->GetMethodID(jcls, "onInitializedFromNative", "(Z)V");
    env->CallVoidMethod(obj, onInitializedFunc, initCode);

    if (needAttach) {
        if (g_jvm->DetachCurrentThread() != JNI_OK) {
            LOGCATE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
    }
    LOGCATI("leave VideoPlayerController::setInitializedStatus...");
}

bool VideoPlayerController::init(char *srcFilenameParam, JavaVM *g_jvm, jobject obj, int* max_analyze_duration, int analyzeCnt, int probesize, bool fpsProbeSizeConfigured,
                                 float minBufferedDuration, float maxBufferedDuration){
    isPlaying = false;
    synchronizer = NULL;
    audioOutput = NULL;
    videoOutput = NULL;

    requestHeader = new DecoderRequestHeader();
    requestHeader->init(srcFilenameParam, max_analyze_duration, analyzeCnt, probesize, fpsProbeSizeConfigured);
    this->g_jvm = g_jvm;
    this->obj = obj;
    this->minBufferedDuration = minBufferedDuration;
    this->maxBufferedDuration = maxBufferedDuration;

    pthread_create(&initThreadThreadId, 0, initThreadCallback, this);

    userCancelled = false;
    return true;
}

int VideoPlayerController::getAudioChannels() {
    int channels = -1;
    if (NULL != synchronizer) {
        channels = synchronizer->getAudioChannels();
    }
    return channels;
}

bool VideoPlayerController::initAudioOutput() {
    LOGCATI("VideoPlayerController::initAudioOutput");

    int channels = this->getAudioChannels();
    if (channels < 0) {
        LOGCATI("VideoDecoder get channels failed ...");
        return false;
    }
    int sampleRate = synchronizer->getAudioSampleRate();
    if (sampleRate < 0) {
        LOGCATI("VideoDecoder get sampleRate failed ...");
        return false;
    }
    audioOutput = new AudioOutput();
    SLresult result = audioOutput->initSoundTrack(channels, sampleRate, audioCallbackFillData, this);
    if (SL_RESULT_SUCCESS != result) {
        LOGCATI("audio manager failed on initialized...");
        delete audioOutput;
        audioOutput = NULL;
        return false;
    }
    return true;
}

void VideoPlayerController::play() {
    LOGCATI("VideoPlayerController::play %d ", (int)isPlaying);
    if (this->isPlaying)
        return;
    this->isPlaying = true;
    if (NULL != audioOutput) {
        audioOutput->play();
    }
}

void VideoPlayerController::pause() {
    LOGCATI("VideoPlayerController::pause");
    if (!this->isPlaying)
        return;
    this->isPlaying = false;
    if (NULL != audioOutput) {
        audioOutput->pause();
    }
}

void VideoPlayerController::resetRenderSize(int left, int top, int width, int height) {
    LOGCATI("VideoPlayerController::resetRenderSize");
    if (NULL != videoOutput) {
        LOGCATI("VideoPlayerController::resetRenderSize NULL != videoOutput width:%d, height:%d", width, height);
        videoOutput->resetRenderSize(left, top, width, height);
    } else {
        LOGCATI("VideoPlayerController::resetRenderSize NULL == videoOutput width:%d, height:%d", width, height);
        screenWidth = width;
        screenHeight = height;
    }
}

int VideoPlayerController::consumeAudioFrames(byte* outData, size_t bufferSize) {
    int ret = bufferSize;
    if(this->isPlaying &&
       synchronizer && !synchronizer->isDestroyed && !synchronizer->isPlayCompleted()) {
//      LOGCATI("Before synchronizer fillAudioData...");
        ret = synchronizer->fillAudioData(outData, bufferSize);
//      LOGCATI("After synchronizer fillAudioData... ");
        signalOutputFrameAvailable();
    } else {
        LOGCATI("VideoPlayerController::consumeAudioFrames set 0");
        memset(outData, 0, bufferSize);
    }
    return ret;
}

float VideoPlayerController::getDuration() {
    if (NULL != synchronizer) {
        return synchronizer->getDuration();
    }
    return 0.0f;
}

int VideoPlayerController::getVideoFrameWidth() {
    if (NULL != synchronizer) {
        return synchronizer->getVideoFrameWidth();
    }
    return 0;
}

int VideoPlayerController::getVideoFrameHeight() {
    if (NULL != synchronizer) {
        return synchronizer->getVideoFrameHeight();
    }
    return 0;
}

float VideoPlayerController::getBufferedProgress() {
    if (NULL != synchronizer) {
        return synchronizer->getBufferedProgress();
    }
    return 0.0f;
}

float VideoPlayerController::getPlayProgress() {
    if (NULL != synchronizer) {
        return synchronizer->getPlayProgress();
    }
    return 0.0f;
}

void VideoPlayerController::seekToPosition(float position) {
    LOGCATI("enter VideoPlayerController::seekToPosition...");
    if (NULL != synchronizer) {
        return synchronizer->seekToPosition(position);
    }
}


void VideoPlayerController::destroy() {
    LOGCATI("enter VideoPlayerController::destroy...");

    userCancelled = true;

    if (synchronizer){
        //中断request
        synchronizer->interruptRequest();
    }

    pthread_join(initThreadThreadId, 0);

    if (NULL != videoOutput) {
        videoOutput->stopOutPut();
        delete videoOutput;
        videoOutput = NULL;
    }

    if (NULL != synchronizer) {
        synchronizer->isDestroyed = true;
        this->pause();
        LOGCATI("stop synchronizer ...");
        synchronizer->destroy();

        LOGCATI("stop audioOutput ...");
        if (NULL != audioOutput) {
            audioOutput->stop();
            delete audioOutput;
            audioOutput = NULL;
        }
        synchronizer->clearFrameMeta();
        delete synchronizer;
        synchronizer = NULL;
    }
    if(NULL != requestHeader){
        requestHeader->destroy();
        delete requestHeader;
        requestHeader = NULL;
    }

    LOGCATI("leave VideoPlayerController::destroy...");
}

void* VideoPlayerController::initThreadCallback(void *myself){
    VideoPlayerController *controller = (VideoPlayerController*) myself;
    controller->startAVSynchronizer();
    pthread_exit(0);
    return 0;
}

EGLContext VideoPlayerController::getUploaderEGLContext() {
    if (NULL != synchronizer) {
        return synchronizer->getUploaderEGLContext();
    }
    return NULL;
}
