//
// Created by Sumn on 2021/12/6.
//

#include "accompany_decoder.h"

#define LOG_TAG "AccompanyDecoder"

AccompanyDecoder::AccompanyDecoder() {
    this->seek_seconds = 0.0f;
    this->seek_req = false;
    this->seek_resp = false;
    accompanyFilePath = nullptr;
}

AccompanyDecoder::~AccompanyDecoder() {
    if (nullptr != accompanyFilePath) {
        delete[] accompanyFilePath;
        accompanyFilePath = nullptr;
    }
}

int AccompanyDecoder::getMusicMeta(const char *fileString, int *metaData) {
    init(fileString);
    int sampleRate = avCodecContext->sample_rate;
    LOGCATI("sampleRte is %d", sampleRate);
    int bitRate = avCodecContext->bit_rate;
    LOGCATI("bitRate is %d", bitRate);
    destroy();
    metaData[0] = sampleRate;
    metaData[1] = bitRate;
    return 0;
}

void AccompanyDecoder::init(const char *fileString, int packetBufferSizeParam) {
    init(fileString);
    packetBufferSize = packetBufferSizeParam;
}

int AccompanyDecoder::init(const char *audioFile) {
    LOGCATI("enter AccompanyDecoder::init");
    audioBuffer = nullptr;
    position = -1.0f;
    audioBufferCursor = 0;
    audioBufferSize = 0;
    swrContext = nullptr;
    swrBuffer = nullptr;
    swrBufferSize = 0;
    seek_success_read_frame_success = true;
    isNeedFirstFrameCorrectFlag = true;
    firstFrameCorrectionInSecs = 0.0f;
    //网络初始化
    avformat_network_init();
    //对FFmpeg log初始化
    av_log_set_callback(ffp_log_callback_report);

    avFormatContext = avformat_alloc_context();
    //开始打开文件
    LOGCATI("开始打开文件: %s", audioFile);
    if (nullptr == accompanyFilePath) {
        int length = strlen(audioFile);
        accompanyFilePath = new char[length + 1];
        //由于最后一个是'\0' 所以memset的长度要设置为length+1
        memset(accompanyFilePath, 0, length + 1);
        memcpy(accompanyFilePath, audioFile, length + 1);
    }
    int result = avformat_open_input(&avFormatContext, audioFile, nullptr, nullptr);
    if (result != 0) {
        LOGCATE("文件打开失败%s,错误码为:%d", audioFile, result);
        return -1;
    } else {
        LOGCATI("文件打开成功%s,", audioFile);
    }
    avFormatContext->max_analyze_duration = 50000;
    //检查文件中的流信息
    result = avformat_find_stream_info(avFormatContext, nullptr);
    if (result < 0) {
        LOGCATE("文件中无流信息 错误码为:%d", result);
        return -1;
    } else {
        LOGCATI("文件中流信息查询成功");
    }
    stream_index = av_find_best_stream(avFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    LOGCATI("stream_index is %d", stream_index);
    if (stream_index == -1) {
        LOGE("无音频流");
        return -1;
    }
    //音频流
    AVStream *audioStream = avFormatContext->streams[stream_index];
    //4.0获取解码器上下文
    avCodecContext = avcodec_alloc_context3(nullptr);
    avcodec_parameters_to_context(avCodecContext, audioStream->codecpar);
    LOGCATI("avCodecContext->codec_id is %d AV_CODEC_ID_AAC is %d", avCodecContext->codec_id,
            AV_CODEC_ID_AAC);
    if (audioStream->time_base.den && audioStream->time_base.num) {
        timeBase = av_q2d(audioStream->time_base);
    } else if (avCodecContext->time_base.den && avCodecContext->time_base.num) {
        timeBase = av_q2d(avCodecContext->time_base);
    }
    //根据解码器上下文找到对应的解码器
    AVCodec *avCodec = avcodec_find_decoder(avCodecContext->codec_id);
    if (avCodec == nullptr) {
        LOGCATE("无法找到对应的解码器");
        return -1;
    }
    //打开解码器
    result = avcodec_open2(avCodecContext, avCodec, nullptr);
    if (result < 0) {
        LOGCATE("解码器打开失败 错误码:%d", result);
        return -1;
    } else {
        LOGCATI("解码器打开成功");
    }
    //判断是否需要re sampler
    if (!audioCodecIsSupported()) {
        LOGCATI("因为不支持音频编解码器所以我们将初始化 swresampler ...");
        /**
		 * 初始化resampler
		 * @param s               Swr context, can be NULL
		 * @param out_ch_layout   output channel layout (AV_CH_LAYOUT_*)
		 * @param out_sample_fmt  output sample format (AV_SAMPLE_FMT_*).
		 * @param out_sample_rate output sample rate (frequency in Hz)
		 * @param in_ch_layout    input channel layout (AV_CH_LAYOUT_*)
		 * @param in_sample_fmt   input sample format (AV_SAMPLE_FMT_*).
		 * @param in_sample_rate  input sample rate (frequency in Hz)
		 * @param log_offset      logging level offset
		 * @param log_ctx         parent logging context, can be NULL
		 */
        swrContext = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(OUT_PUT_CHANNELS),
                                        AV_SAMPLE_FMT_S16, avCodecContext->sample_rate,
                                        av_get_default_channel_layout(avCodecContext->channels),
                                        avCodecContext->sample_fmt, avCodecContext->sample_rate, 0,
                                        NULL);
        if (!swrContext || swr_init(swrContext)) {
            if (swrContext)
                swr_free(&swrContext);
            avcodec_close(avCodecContext);
            LOGCATI("init resampler failed...");
            return -1;
        }
    }
    LOGCATI("channels is %d sampleRate is %d", avCodecContext->channels,
            avCodecContext->sample_rate);
    pAudioFrame = av_frame_alloc();
    return 1;
}

bool AccompanyDecoder::audioCodecIsSupported() {
    if (avCodecContext->sample_fmt == AV_SAMPLE_FMT_S16) {
        return true;
    }
    return false;
}

AudioPacket *AccompanyDecoder::decodePacket() {
    short *samples = new short[packetBufferSize];
    int stereoSampleSize = readSamples(samples, packetBufferSize);
    AudioPacket *samplePacket = new AudioPacket();
    if (stereoSampleSize > 0) {
        //构造一个packet
        samplePacket->buffer = samples;
        samplePacket->size = stereoSampleSize;
        /** 这里由于每一个packet的大小不一样有可能是200ms 但是这样子position就有可能不准确了 **/
        samplePacket->position = position;

    } else {
        samplePacket->size = -1;
    }

    return samplePacket;
}

int AccompanyDecoder::readSamples(short *samples, int size) {
    if (seek_req) {
        audioBufferCursor = audioBufferSize;
    }
    int sampleSize = size;
    while (size > 0) {
        if (audioBufferCursor < audioBufferSize) {
            int audioBufferDataSize = audioBufferSize - audioBufferCursor;
            int copySize = MIN(size, audioBufferDataSize);
            memcpy(samples + (sampleSize - size), audioBuffer + audioBufferCursor, copySize * 2);
            size -= copySize;
            audioBufferCursor += copySize;
        } else {
            if (readFrame() < 0) {
                break;
            }
        }
    }
    int fillSize = sampleSize - size;
    if (fillSize == 0) {
        return -1;
    }
    return fillSize;
}

void AccompanyDecoder::seek_frame() {
    LOGCATI("\n seek frame firstFrameCorrectionInSecs is %.6f, seek_seconds=%f, position=%f \n",
            firstFrameCorrectionInSecs, seek_seconds, position);
    float targetPosition = seek_seconds;
    float currentPosition = position;
    float frameDuration = duration;
    if (targetPosition < currentPosition) {
        this->destroy();
        this->init(accompanyFilePath);
        currentPosition = 0.0;
    }
    int readFrameCode = -1;
    while (true) {

        av_init_packet(&packet);
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if (readFrameCode >= 0) {
            currentPosition += frameDuration;
            if (currentPosition >= targetPosition) {
                break;
            }
        }
        av_free_packet(&packet);
    }
    seek_resp = true;
    seek_req = false;
    seek_success_read_frame_success = false;

}

int AccompanyDecoder::readFrame() {
    if (seek_req) {
        this->seek_frame();
    }
    int ret = 1;
    av_init_packet(&packet);
    int gotFrame = 0;
    int readFrameCode = -1;
    while (true) {
        readFrameCode = av_read_frame(avFormatContext, &packet);
        if (readFrameCode >= 0) {
            if (packet.stream_index == stream_index) {
                int len = avcodec_decode_audio4(avCodecContext, pAudioFrame,
                                                &gotFrame, &packet);
                if (len < 0) {
                    LOGCATI("decode recorder.audio error, skip packet");
                }
                if (gotFrame) {
                    int numChannels = OUT_PUT_CHANNELS;
                    int numFrames = 0;
                    void *audioData;
                    if (swrContext) {
                        const int ratio = 2;
                        const int bufSize = av_samples_get_buffer_size(nullptr, numChannels,
                                                                       pAudioFrame->nb_samples *
                                                                       ratio, AV_SAMPLE_FMT_S16, 1);
                        if (!swrBuffer || swrBufferSize < bufSize) {
                            swrBufferSize = bufSize;
                            swrBuffer = realloc(swrBuffer, swrBufferSize);
                        }
                        byte *outbuf[2] = {(byte *) swrBuffer, NULL};
                        numFrames = swr_convert(swrContext, outbuf, pAudioFrame->nb_samples * ratio,
                                                (const uint8_t **) pAudioFrame->data,
                                                pAudioFrame->nb_samples);
                        if (numFrames < 0) {
                            LOGCATI("fail resample recorder.audio");
                            ret = -1;
                            break;
                        }
                        audioData = swrBuffer;
                    } else {
                        if (avCodecContext->sample_fmt != AV_SAMPLE_FMT_S16) {
                            LOGCATI("bucheck, recorder.audio format is invalid");
                            ret = -1;
                            break;
                        }
                        audioData = pAudioFrame->data[0];
                        numFrames = pAudioFrame->nb_samples;
                    }
                    if (isNeedFirstFrameCorrectFlag && position >= 0) {
                        float expectedPosition = position + duration;
                        float actualPosition =
                                av_frame_get_best_effort_timestamp(pAudioFrame) * timeBase;
                        firstFrameCorrectionInSecs = actualPosition - expectedPosition;
                        isNeedFirstFrameCorrectFlag = false;
                    }
                    duration = av_frame_get_pkt_duration(pAudioFrame) * timeBase;
                    position = av_frame_get_best_effort_timestamp(pAudioFrame) * timeBase -
                               firstFrameCorrectionInSecs;
                    if (!seek_success_read_frame_success) {
                        LOGCATI("position is %.6f", position);
                        actualSeekPosition = position;
                        seek_success_read_frame_success = true;
                    }
                    audioBufferSize = numFrames * numChannels;
//					LOGCATI(" \n duration is %.6f position is %.6f audioBufferSize is %d\n", duration, position, audioBufferSize);
                    audioBuffer = (short *) audioData;
                    audioBufferCursor = 0;
                    break;
                }

            }
        } else {
            ret = -1;
            break;
        }
    }
    av_free_packet(&packet);
    return ret;
}



void AccompanyDecoder::destroy() {
    if (nullptr != swrBuffer) {
        free(swrBuffer);
        swrBuffer = nullptr;
        swrBufferSize = 0;
    }
    if (nullptr != swrContext) {
        swr_free(&swrContext);
        swrContext = nullptr;
    }
    if (nullptr != pAudioFrame) {
        av_free(pAudioFrame);
        pAudioFrame = nullptr;
    }
    if (nullptr != avCodecContext) {
        avcodec_close(avCodecContext);
        avCodecContext = nullptr;
    }
    if (nullptr != avFormatContext) {
        LOGCATI("leave LiveReceiver::destory");
        avformat_close_input(&avFormatContext);
        avFormatContext = nullptr;
    }
}





