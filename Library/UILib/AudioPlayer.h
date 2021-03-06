﻿#pragma once
#pragma comment(lib, "vfw32.lib")

#define MAX_AUDIO_FRAME_SIZE 192000

#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

#define SAMPLE_CORRECTION_PERCENT_MAX 10
#define AUDIO_DIFF_AVG_NB 20

#include "BaseControl.h"
#include "BaseLib/MemoryStream.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>

#include <SDL/include/SDL.h>
#include <SDL/include/SDL_thread.h>
}

#include <Windows.h>

#include <string>
#include <sstream>
#include <mutex>
#include <chrono>
#include <thread>
#include <queue>

namespace jojogame
{
struct AudioPacketQueue
{
    AVPacketList *firstPacket, *lastPacket;
    int size;
    std::mutex mutex;
    std::condition_variable cond;
};

struct AudioState
{
    AVFormatContext *formatContext;

    AVCodecContext *audioCodecContext;
    int audioStreamIndex;
    AVStream *audioStream;
    AudioPacketQueue audioQueue;
    uint8_t audioBuffer[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    unsigned int audioBufferSize;
    unsigned int audioBufferIndex;
    AVFrame audioFrame;
    AVPacket audioPacket;
    uint8_t *audioPacketData;
    int audioPacketSize;

    bool playing;
    bool finishQueue;

    int audioClock;

    int maxPts;
};

class CAudioPlayerControl
{
public:
    static void RegisterFunctions(lua_State *L);

    CAudioPlayerControl();
    virtual ~CAudioPlayerControl();

    bool IsPlaying();

    bool LoadFromMe5File(std::wstring filePath, int groupIndex, int subIndex);
    bool LoadFromFile(std::wstring fileName);
    void Destroy();

    void Play(int playCount);
    void Stop();

    static int Read(void *opaque, unsigned char *buf, int buf_size);
    static int64_t Seek(void *opaque, int64_t offset, int whence);

private:
    mutable AudioState _state{};
    CMemoryStream *_inputStream = nullptr;
    std::thread *_audioThread = nullptr;
    int _playCount = 1;
    mutable bool _stop = true;
};
} // namespace jojogame
