//
// Created by Sumn on 2021/12/7.
//

#include "accompany_decoder_controller.h"

AccompanyDecoderController::AccompanyDecoderController() {
    accompanyDecoder= nullptr;
    playPosition=0.0f;

}

AccompanyDecoderController::~AccompanyDecoderController() {

}

int AccompanyDecoderController::getMusicMeta(const char *accompanyPath, int *accompanyMetaData) {
    //获取伴奏的meta
    AccompanyDecoder*pAccompanyDecoder=new AccompanyDecoder();
    pAccompanyDecoder->getMusicMeta(accompanyPath,accompanyMetaData);
    delete pAccompanyDecoder;
    accompanySampleRate=accompanyMetaData[0];
    return 0;
}

float AccompanyDecoderController::getPlayPosition() {
    return playPosition;
}

void* AccompanyDecoderController::startDecoderThread(void *ptr) {
    LOGCATI("enterAccompanyDecoderController::startDecoderThread");
    AccompanyDecoderController* decoderController=(AccompanyDecoderController*)ptr;
    int getLockCode=pthread_mutex_lock(&decoderController->mLock);
    while (decoderController->isRunning){
        decoderController->decodeSongPacket();
        if (decoderController->packetPool->geDecoderAccompanyPacketQueueSize()>QUEUE_SIZE_MAX_THRESHOLD){
            pthread_cond_wait(&decoderController->mCondition,&decoderController->mLock);
        }
    }
    pthread_mutex_unlock(&decoderController->mLock);

}

void AccompanyDecoderController::initAccompanyDecoder(const char *accompanyPath) {
    //初始化两个decoder
    accompanyDecoder=new AccompanyDecoder();
    accompanyDecoder->init(accompanyPath,accompanyPacketBufferSize);

}

void AccompanyDecoderController::init(const char *accompanyPath, float packetBufferTimePercent) {

    //初始化全局变量
    volume =1.0f;
    accompanyMax=1.0f;

    //计算出伴奏和原唱的bufferSize
    int  accompanyByteCountPerSec=accompanySampleRate*CHANNEL_PER_FRAME*BITS_PER_CHANNEL/BITS_PER_BYTE;
    accompanyPacketBufferSize= (int) ((accompanyByteCountPerSec / 2)
                                      * packetBufferTimePercent);
    //初始化两个decoder
    initAccompanyDecoder(accompanyPath);
    //初始化队列以及开启线程
    packetPool=PacketPool::GetInstance();
    packetPool->initAccompanyPacketQueue();
    initDecoderThread();
}

void AccompanyDecoderController::initDecoderThread() {
    isRunning= true;
    pthread_mutex_init(&mLock, nullptr);
    pthread_cond_init(&mCondition, nullptr);
    pthread_create(&songDecoderThread, nullptr,startDecoderThread,this);

}
//需要子类完成自己的操作
void AccompanyDecoderController::decodeSongPacket() {
    AudioPacket * accompanyPacket=accompanyDecoder->decodePacket();
    accompanyPacket->action=AudioPacket::AUDIO_PACKER_ACTION_PLAY;
    packetPool->pushDecoderAccompanyPacketToQueue(accompanyPacket);

}

void AccompanyDecoderController::destroyDecoderThread() {
    isRunning= false;
    void* status;
    int getLockCode=pthread_mutex_lock(&mLock);
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
    pthread_join(songDecoderThread, &status);
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);

}

int AccompanyDecoderController::readSamples(short *samples, int size, int *slientSizeArr) {
    int result=-1;
    AudioPacket* accompanyPacket= nullptr;
    packetPool->getDecoderAccompanyPacket(&accompanyPacket, true);
    if (nullptr!=accompanyPacket){
        int samplePacketSize=accompanyPacket->size;
        if (samplePacketSize!=-1&&samplePacketSize<=size){
            memcpy(samples,accompanyPacket->buffer,samplePacketSize*2);
            adjustSamplesVolume(samples,samplePacketSize,volume/accompanyMax);
            delete accompanyPacket;
            result=samplePacketSize;
        }
    } else{
        return -2;
    }
    if (packetPool->geDecoderAccompanyPacketQueueSize()<QUEUE_SIZE_MIN_THRESHOLD){
        int  getLockCode=pthread_mutex_lock(&mLock);
        if (result!=-1){
            pthread_cond_signal(&mCondition);
        }
        pthread_mutex_unlock(&mLock);
    }
    return result;
}

void AccompanyDecoderController::destroy() {
    destroyDecoderThread();
    packetPool->abortDecoderAccompanyPacketQueue();
    packetPool->destroyDecoderAccompanyPacketQueue();
    if (nullptr != accompanyDecoder) {
        accompanyDecoder->destroy();
        delete accompanyDecoder;
        accompanyDecoder = nullptr;
    }
}
