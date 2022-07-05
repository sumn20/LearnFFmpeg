//
// Created by Sumn on 2021/12/15.
//
#include <jni.h>
#include "common_tools.h"
#include <android/native_window_jni.h>
#include <android/native_window.h>
#include "video_player_controller.h"

VideoPlayerController *videoPlayerController = nullptr;

static ANativeWindow *window = nullptr;


extern "C" {
#include "libavutil/ffversion.h"
JNIEXPORT jstring JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_getFFmpegVersion(JNIEnv *env, jobject thiz) {

    return env->NewStringUTF(FFMPEG_VERSION);
}
JNIEXPORT jboolean JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_prepare(JNIEnv *env, jobject obj,
                                                             jstring videoMergeFilePathParam,
                                                             jintArray max_analyze_duration,
                                                             jint size, jint probesize,
                                                             jboolean fpsProbeSizeConfigured,
                                                             jfloat minBufferedDuration,
                                                             jfloat maxBufferedDuration,
                                                             jint width, jint height,
                                                             jobject surface) {
    LOGCATI("Enter Java_com_project_learnffmpeg_videoplayer_VideoPlayer_prepare");
    JavaVM *g_jvm= nullptr;
    env->GetJavaVM(&g_jvm);
    jobject g_object=env->NewGlobalRef(obj);
    char * videoMergeFilePath=(char *)env->GetStringUTFChars(videoMergeFilePathParam, nullptr);
    if (nullptr== videoPlayerController){
        videoPlayerController=new VideoPlayerController();
    }
    window=ANativeWindow_fromSurface(env,surface);
    jint* max_analyze_duration_params=env->GetIntArrayElements(max_analyze_duration, nullptr);
    jboolean initCode=videoPlayerController->init(videoMergeFilePath,g_jvm,g_object,max_analyze_duration_params,size,probesize,fpsProbeSizeConfigured,minBufferedDuration,maxBufferedDuration);
    videoPlayerController->onSurfaceCreated(window,width,height);
    env->ReleaseIntArrayElements(max_analyze_duration,max_analyze_duration_params,0);
    env->ReleaseStringUTFChars(videoMergeFilePathParam,videoMergeFilePath);
    return initCode;
}
JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_onSurfaceCreated(JNIEnv *env, jobject thiz,
                                                                      jobject surface) {
    if (nullptr!=videoPlayerController){
        window=ANativeWindow_fromSurface(env,surface);
        videoPlayerController->onSurfaceCreated(window,0,0);
    }
}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_onSurfaceDestroyed(JNIEnv *env, jobject thiz,
                                                                        jobject surface) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->onSurfaceDestroyed();

    }
}


JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_pause(JNIEnv *env, jobject thiz) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->pause();
    }
}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_play(JNIEnv *env, jobject thiz) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->play();
    }
}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_stop(JNIEnv *env, jobject thiz) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->destroy();
        delete videoPlayerController;
        videoPlayerController = nullptr;
    }
}

JNIEXPORT jfloat JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_getBufferedProgress(JNIEnv *env,
                                                                         jobject thiz) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->getBufferedProgress();
    }
}


JNIEXPORT jfloat JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_getPlayProgress(JNIEnv *env, jobject thiz) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->getPlayProgress();
    }
}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_seekToPosition(JNIEnv *env, jobject thiz,
                                                                    jfloat position) {
    if (nullptr!=videoPlayerController){
        videoPlayerController->seekToPosition(position);
    }
}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_seekCurrent(JNIEnv *env, jobject thiz,
                                                                 jfloat position) {
    if(NULL != videoPlayerController) {
		videoPlayerController->seekToPosition(position);
    }
}


JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_beforeSeekCurrent(JNIEnv *env, jobject thiz) {


}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_afterSeekCurrent(JNIEnv *env, jobject thiz) {


}

JNIEXPORT void JNICALL
Java_com_project_learnffmpeg_videoplayer_VideoPlayer_resetRenderSize(JNIEnv *env, jobject thiz,
                                                                     jint left, jint top,
                                                                     jint width, jint height) {
    if(nullptr != videoPlayerController) {
        videoPlayerController->resetRenderSize(left, top, width, height);
    }

}

}
