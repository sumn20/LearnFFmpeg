//
// Created by Sumn on 2021/12/6.
//

#ifndef LEARNFFMPEG_PACKET_POOL_H
#define LEARNFFMPEG_PACKET_POOL_H
#include "packet_queue.h"
#include "common_tools.h"

class PacketPool {
private:
    //构造方法私有化
    PacketPool();
    //单例
    static PacketPool* instance;
    /** 边录边合---伴奏的packet queue **/
    PacketQueue* accompanyPacketQueue;
    /** 边录边合---人声的packet queue **/
    PacketQueue* audioPacketQueue;
    /** 直播---发送的packet queue **/
    PacketQueue* livePacketQueue;
    /** 直播---接收的packet queue **/
    PacketQueue* liveSubscriberPacketQueue;
    /** 试音---的packet queue **/
    PacketQueue* tuningPacketQueue;
    /** 解码线程---解码出来的伴奏的queue **/
    PacketQueue* decoderAccompanyPacketQueue;
    /** 解码线程---解码出来的原唱的queue **/
    PacketQueue* decoderOriginalSongPacketQueue;

public:
    static PacketPool* GetInstance(); //工厂方法(用来获得实例)
    virtual ~PacketPool();

    /** 解码出来的伴奏的queue的所有操作 **/
    virtual void initDecoderAccompanyPacketQueue();
    virtual void abortDecoderAccompanyPacketQueue();
    virtual void destroyDecoderAccompanyPacketQueue();
    virtual int getDecoderAccompanyPacket(AudioPacket **audioPacket, bool block);
    virtual void pushDecoderAccompanyPacketToQueue(AudioPacket* audioPacket);
    virtual void clearDecoderAccompanyPacketToQueue();
    virtual int geDecoderAccompanyPacketQueueSize();

    /** 解码出来的原唱的queue的所有操作 **/
    virtual void initDecoderOriginalSongPacketQueue();
    virtual void abortDecoderOriginalSongPacketQueue();
    virtual void destroyDecoderOriginalSongPacketQueue();
    virtual int getDecoderOriginalSongPacket(AudioPacket **audioPacket, bool block);
    virtual void pushDecoderOriginalSongPacketToQueue(AudioPacket* audioPacket);
    virtual void clearDecoderOriginalSongPacketToQueue();
    virtual int getDecoderOriginalSongPacketQueueSize();

    /** 人声的packet queue的所有操作 **/
    virtual void initAudioPacketQueue();
    virtual void abortAudioPacketQueue();
    virtual void destroyAudioPacketQueue();
    virtual int getAudioPacket(AudioPacket **audioPacket, bool block);
    virtual void pushAudioPacketToQueue(AudioPacket* audioPacket);
    virtual void clearAudioPacketToQueue();
    virtual int getAudioPacketQueueSize();

    /** 伴奏的packet queue的所有操作 **/
    virtual void initAccompanyPacketQueue();
    virtual void abortAccompanyPacketQueue();
    virtual void destroyAccompanyPacketQueue();
    virtual int getAccompanyPacket(AudioPacket **accompanyPacket, bool block);
    virtual void pushAccompanyPacketToQueue(AudioPacket* accompanyPacket);
    virtual int getAccompanyPacketQueueSize();
    virtual void clearAccompanyPacketQueue();

    /** 直播发送的packet queue的所有操作 **/
    virtual void initLivePacketQueue();
    virtual void abortLivePacketQueue();
    virtual void destroyLivePacketQueue();
    virtual int getLivePacket(AudioPacket **livePacket, bool block);
    virtual void pushLivePacketToQueue(AudioPacket* livePacket);
    virtual int getLivePacketQueueSize();

    /** 直播接收的packet queue的所有操作 **/
    virtual void initLiveSubscriberPacketQueue();
    virtual void abortLiveSubscriberPacketQueue();
    virtual void destroyLiveSubscriberPacketQueue();
    virtual int getLiveSubscriberPacket(AudioPacket **livePacket, bool block);
    virtual void pushLiveSubscriberPacketToQueue(AudioPacket* livePacket);
    virtual int getLiveSubscriberPacketQueueSize();

    /** 试音的packet queue的所有操作 **/
    virtual void initTuningPacketQueue();
    virtual void abortTuningPacketQueue();
    virtual void destroyTuningPacketQueue();
    virtual int getTuningPacket(AudioPacket **livePacket, bool block);
    virtual void pushTuningPacketToQueue(AudioPacket* livePacket);
    virtual int getTuningPacketQueueSize();

};


#endif //LEARNFFMPEG_PACKET_POOL_H
