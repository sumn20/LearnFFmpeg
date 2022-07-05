//
// Created by Sumn on 2021/12/6.
//

#include "packet_queue.h"

PacketQueue::PacketQueue() {
    init();

}

PacketQueue::PacketQueue(const char *queueNameParam) {
    init();
    queueName=queueNameParam;

}

PacketQueue::~PacketQueue() {
    LOGCATI("%s PacketQueue",queueName);
    flush();
    pthread_mutex_destroy(&mLock);
    pthread_cond_destroy(&mCondition);
}

void PacketQueue::init() {
    int initLockCode=pthread_mutex_init(&mLock, nullptr);
    LOGCATD("initLockCode %d",initLockCode);
    int initConditionCode=pthread_cond_init(&mCondition,nullptr);
    LOGCATD("initConditionCode %d",initConditionCode);
    mNbPackets=0;
    mFirst=nullptr;
    mLast=nullptr;
    mAbortRequest= false;
}

void PacketQueue::flush() {
    AudioPacketList *pkt,*pkt1;
    AudioPacket *audioPacket;
    pthread_mutex_lock(&mLock);
    for (pkt=mFirst;pkt!=nullptr;pkt=pkt1){
        pkt1=pkt->next;
        audioPacket=pkt->pkt;
        if (nullptr!=audioPacket){
            delete audioPacket;
        }
        delete pkt;
        pkt= nullptr;
    }
    mLast=nullptr;
    mFirst=nullptr;
    mNbPackets=0;
    pthread_mutex_unlock(&mLock);
}

int PacketQueue::put(AudioPacket *pkt) {
    if (mAbortRequest) {
        delete pkt;
        return -1;
    }
    AudioPacketList *pkt1 = new AudioPacketList();
    if (!pkt1)
        return -1;
    pkt1->pkt = pkt;
    pkt1->next = nullptr;
    int getLockCode = pthread_mutex_lock(&mLock);
    if (mLast == nullptr) {
        mFirst = pkt1;
    } else {
        mLast->next = pkt1;
    }
    mLast = pkt1;
    mNbPackets++;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
    return 0;

}

int PacketQueue::get(AudioPacket **pkt, bool block) {
    AudioPacketList *pkt1;
    int ret;
    int getLockCode = pthread_mutex_lock(&mLock);
    for (;;) {
        if (mAbortRequest) {

            ret = -1;
            break;
        }

        pkt1 = mFirst;

        if (pkt1) {

            mFirst = pkt1->next;
            if (!mFirst)
                mLast = nullptr;
            mNbPackets--;

            *pkt = pkt1->pkt;
            delete pkt1;
            pkt1 = nullptr;
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            pthread_cond_wait(&mCondition, &mLock);
        }
    }
    pthread_mutex_unlock(&mLock);
    return ret;
}

int PacketQueue::size() {
    pthread_mutex_lock(&mLock);
    int  size=mNbPackets;
    pthread_mutex_unlock(&mLock);
    return size;
}

void PacketQueue::abort() {
    pthread_mutex_lock(&mLock);
    mAbortRequest = true;
    pthread_cond_signal(&mCondition);
    pthread_mutex_unlock(&mLock);
}
