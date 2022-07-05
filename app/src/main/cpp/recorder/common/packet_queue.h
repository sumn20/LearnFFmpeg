//
// Created by Sumn on 2021/12/6.
//

#ifndef LEARNFFMPEG_PACKET_QUEUE_H
#define LEARNFFMPEG_PACKET_QUEUE_H

#include <pthread.h>
#include <cstring>
#include "common_tools.h"
#include "../../util/log_util.h"
// 音频包队列
typedef struct AudioPacket {
    static const int AUDIO_PACKER_ACTION_PLAY = 0;
    static const int AUDIO_PACKER_ACTION_PAUSE = 100;
    static const int AUDIO_PACKER_ACTION_SEEK = 101;

    short *buffer;
    int size;
    float position;
    int action;
    float extra_param1;
    float extra_param2;

    AudioPacket() {
        buffer = NULL;
        size = 0;
        position = -1;
        action = 0;
        extra_param1 = 0;
        extra_param2 = 0;
    }

    ~AudioPacket() {
        if (NULL != buffer) {
            delete[]buffer;
            buffer = NULL;
        }
    }


} AudioPacket;
typedef struct AudioPacketList {
    AudioPacket *pkt;
    struct AudioPacketList *next;

    AudioPacketList() {
        pkt = NULL;
        next = NULL;
    }


} AudioPacketList;

inline void buildPackFromBuffer(AudioPacket *audioPacket, short *samples, int samplesSize) {
    short *packetBuffer = new short[samplesSize];
    if (NULL != packetBuffer) {
        memcpy(packetBuffer, samples, samplesSize);
        audioPacket->buffer = packetBuffer;
        audioPacket->size = samplesSize;
    } else {
        audioPacket->size = -1;
    }

}


class PacketQueue {
public:
    PacketQueue();

    PacketQueue(const char *queueNameParam);

    ~PacketQueue();

    void init();

    void flush();

    int put(AudioPacket *audioPacket);

    /*如果 <0 表示中止 如果没有数据包则为 0，如果有数据包则 > 0 */
    int get(AudioPacket **pAudioPacket, bool block);

    int size();

    void abort();

private:
    AudioPacketList *mFirst;
    AudioPacketList *mLast;
    int mNbPackets;
    bool mAbortRequest;
    pthread_mutex_t mLock;
    pthread_cond_t mCondition;
    const char *queueName;
};


#endif //LEARNFFMPEG_PACKET_QUEUE_H
