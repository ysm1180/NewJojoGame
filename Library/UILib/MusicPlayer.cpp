﻿#include "MusicPlayer.h"
#include "BaseLib/ConsoleOutput.h"
#include "CommonLib/FileManager.h"
#include "CommonLib/ME5File.h"

namespace jojogame {
void InitPacketQueue(AudioPacketQueue* queue)
{
    queue->size = 0;
    queue->firstPacket = nullptr;
    queue->lastPacket = nullptr;
}

static int GetPacketQueue(AudioState* audioState, AudioPacketQueue* queue, AVPacket* packet, int block)
{
    AVPacketList* packetList;
    int result;

    std::unique_lock<std::mutex> lock(queue->mutex);
    for (;;)
    {
        if (!audioState->playing)
        {
            result = -1;
            break;
        }

        packetList = queue->firstPacket;
        if (packetList)
        {
            queue->firstPacket = packetList->next;
            if (!queue->firstPacket)
            {
                queue->lastPacket = nullptr;
            }
            queue->size -= packetList->pkt.size;
            *packet = packetList->pkt;
            av_free(packetList);
            result = 1;
            break;
        }
        if (!block)
        {
            result = 0;
            break;
        }
        if (audioState->finishQueue || !audioState->playing)
        {
            audioState->playing = false;
            result = -1;
            break;
        }
        queue->cond.wait(lock);
    }
    lock.unlock();

    return result;
}

int PutPacketQueue(AudioPacketQueue* queue, AVPacket* packet)
{
    AVPacketList* packetList = (AVPacketList *)av_malloc(sizeof(AVPacketList));
    if (!packetList)
    {
        return AVERROR(ENOMEM);
    }

    av_init_packet(&packetList->pkt);
    if (av_packet_ref(&packetList->pkt, packet) < 0)
    {
        return AVERROR(ENOMEM);
    }
    packetList->next = nullptr;

    std::unique_lock<std::mutex> lock(queue->mutex);
    if (!queue->lastPacket)
    {
        queue->firstPacket = packetList;
    }
    else
    {
        queue->lastPacket->next = packetList;
    }
    queue->lastPacket = packetList;
    queue->size += packetList->pkt.size;
    queue->cond.notify_one();
    lock.unlock();

    return 0;
}

static int ResamplingAudio(AVCodecContext* audio_decode_ctx,
                            AVFrame* audio_decode_frame,
                            enum AVSampleFormat out_sample_fmt,
                            int out_channels,
                            int out_sample_rate,
                            uint8_t* out_buf)
{
    SwrContext* swr_ctx = nullptr;
    int ret = 0;
    int64_t in_channel_layout = audio_decode_ctx->channel_layout;
    int64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_nb_channels = 0;
    int out_linesize = 0;
    int in_nb_samples = 0;
    int out_nb_samples = 0;
    int max_out_nb_samples = 0;
    uint8_t** resampled_data = nullptr;
    int resampled_data_size = 0;

    swr_ctx = swr_alloc();
    if (!swr_ctx)
    {
        printf("swr_alloc error\n");
        return -1;
    }

    in_channel_layout = (audio_decode_ctx->channels ==
                            av_get_channel_layout_nb_channels(audio_decode_ctx->channel_layout))
                            ? audio_decode_ctx->channel_layout
                            : av_get_default_channel_layout(audio_decode_ctx->channels);
    if (in_channel_layout <= 0)
    {
        CConsoleOutput::OutputConsoles(L"in_channel_layout error\n");
        return -1;
    }

    if (out_channels == 1)
    {
        out_channel_layout = AV_CH_LAYOUT_MONO;
    }
    else if (out_channels == 2)
    {
        out_channel_layout = AV_CH_LAYOUT_STEREO;
    }
    else
    {
        out_channel_layout = AV_CH_LAYOUT_SURROUND;
    }

    in_nb_samples = audio_decode_frame->nb_samples;
    if (in_nb_samples <= 0)
    {
        CConsoleOutput::OutputConsoles(L"in_nb_samples error\n");
        return -1;
    }

    av_opt_set_int(swr_ctx, "in_channel_layout", in_channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", audio_decode_ctx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_decode_ctx->sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", out_channel_layout, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", out_sample_fmt, 0);

    if ((ret = swr_init(swr_ctx)) < 0)
    {
        CConsoleOutput::OutputConsoles(L"Failed to initialize the resampling context\n");
        return -1;
    }

    max_out_nb_samples = out_nb_samples = av_rescale_rnd(in_nb_samples,
                                                         out_sample_rate,
                                                         audio_decode_ctx->sample_rate,
                                                         AV_ROUND_UP);

    if (max_out_nb_samples <= 0)
    {
        CConsoleOutput::OutputConsoles(L"av_rescale_rnd error\n");
        return -1;
    }

    out_nb_channels = av_get_channel_layout_nb_channels(out_channel_layout);

    ret = av_samples_alloc_array_and_samples(&resampled_data, &out_linesize, out_nb_channels, out_nb_samples,
                                             out_sample_fmt, 0);
    if (ret < 0)
    {
        CConsoleOutput::OutputConsoles(L"av_samples_alloc_array_and_samples error\n");
        return -1;
    }

    out_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx, audio_decode_ctx->sample_rate) + in_nb_samples,
                                    out_sample_rate, audio_decode_ctx->sample_rate, AV_ROUND_UP);
    if (out_nb_samples <= 0)
    {
        CConsoleOutput::OutputConsoles(L"av_rescale_rnd error\n");
        return -1;
    }

    if (out_nb_samples > max_out_nb_samples)
    {
        av_free(resampled_data[0]);
        ret = av_samples_alloc(resampled_data, &out_linesize, out_nb_channels, out_nb_samples, out_sample_fmt, 1);
        max_out_nb_samples = out_nb_samples;
    }

    if (swr_ctx)
    {
        ret = swr_convert(swr_ctx, resampled_data, out_nb_samples,
                          (const uint8_t **)audio_decode_frame->data, audio_decode_frame->nb_samples);
        if (ret < 0)
        {
            CConsoleOutput::OutputConsoles(L"swr_convert_error\n");
            return -1;
        }

        resampled_data_size = av_samples_get_buffer_size(&out_linesize, out_nb_channels, ret, out_sample_fmt, 1);
        if (resampled_data_size < 0)
        {
            CConsoleOutput::OutputConsoles(L"av_samples_get_buffer_size error\n");
            return -1;
        }
    }
    else
    {
        CConsoleOutput::OutputConsoles(L"swr_ctx null error\n");
        return -1;
    }

    memcpy(out_buf, resampled_data[0], resampled_data_size);

    if (resampled_data)
    {
        av_freep(&resampled_data[0]);
    }
    av_freep(&resampled_data);
    resampled_data = nullptr;

    if (swr_ctx)
    {
        swr_free(&swr_ctx);
    }
    return resampled_data_size;
}

int DecodeAudioFrame(AudioState* audioState, uint8_t* audioBuffer, int bufferSize)
{
    int dataSize = 0;
    AVPacket* packet = &audioState->audioPacket;

    for (;;)
    {
        while (audioState->audioPacketSize > 0)
        {
            int packetError = avcodec_send_packet(audioState->audioCodecContext, packet);
            if (packet->size < 0 || packetError < 0)
            {
                // If error, skip frame
                audioState->audioPacketSize = 0;
                break;
            }
            dataSize = 0;
            if (avcodec_receive_frame(audioState->audioCodecContext, &audioState->audioFrame) == 0)
            {
                dataSize = ResamplingAudio(audioState->audioCodecContext, &audioState->audioFrame, AV_SAMPLE_FMT_S16, audioState->audioFrame.channels, audioState->audioFrame.sample_rate, audioBuffer);
            }

            audioState->audioPacketData += packet->size;
            audioState->audioPacketSize -= packet->size;

            if (dataSize <= 0)
            {
                // No data yet, get more frames
                continue;
            }

            return dataSize;
        }

        if (packet->data)
        {
            av_packet_unref(packet);
        }

        if (!audioState->playing || GetPacketQueue(audioState, &audioState->audioQueue, packet, 1) < 0)
        {
            return -1;
        }

        audioState->audioPacketData = packet->data;
        audioState->audioPacketSize = packet->size;
    }
}

void MusicAudioCallback(void* userdata, Uint8* stream, int len)
{
    auto audioState = (AudioState *)userdata;

    while (audioState->playing && len > 0)
    {
        if (audioState->audioBufferIndex >= audioState->audioBufferSize)
        {
            // We have already sent all our data; get more
            int audioSize = DecodeAudioFrame(audioState, audioState->audioBuffer, sizeof(audioState->audioBuffer));
            if (audioSize < 0)
            {
                // If error, output silence
                audioState->audioBufferSize = 1024; // arbitrary?
                memset(audioState->audioBuffer, 0, audioState->audioBufferSize);
            }
            else
            {
                audioState->audioBufferSize = static_cast<unsigned int>(audioSize);
            }
            audioState->audioBufferIndex = 0;
        }

        int computedLen = audioState->audioBufferSize - audioState->audioBufferIndex;
        if (computedLen > len)
        {
            computedLen = len;
        }
        memcpy(stream, (uint8_t *)audioState->audioBuffer + audioState->audioBufferIndex, computedLen);
        len -= computedLen;
        stream += computedLen;
        audioState->audioBufferIndex += computedLen;
    }
}

void CMusicPlayerControl::RegisterFunctions(lua_State* L)
{
    LUA_BEGIN(CMusicPlayerControl, "_MusicPlayer");

    LUA_METHOD(IsPlaying);

    LUA_METHOD(Play);
    LUA_METHOD(Stop);
    LUA_METHOD(Create);
    LUA_METHOD(Destroy);
}

CMusicPlayerControl::CMusicPlayerControl(std::wstring fileName)
{
    _state.fileName = CFileManager::GetInstance().GetFilePath(fileName);
}

CMusicPlayerControl::~CMusicPlayerControl()
{
    Destroy();
}

bool CMusicPlayerControl::IsPlaying()
{
    return _state.playing;
}

//bool CMusicPlayerControl::LoadMusicFromMe5File(std::wstring filePath, int groupIndex, int subIndex)
//{
//    AVCodecParameters* audioCodecParameters = nullptr;
//    int error = 0;
//
//    _state.formatContext = nullptr;
//
//    CME5File me5File;
//    me5File.Open(filePath);
//
//    int size = me5File.GetItemByteSize(groupIndex, subIndex);
//    auto* by = new BYTE[size];
//
//    me5File.GetItemByteArr(by, groupIndex, subIndex);
//
//    auto buffer = ::av_malloc(size);
//    avio_alloc_context(buffer, size, 0, )
//
//    int length = WideCharToMultiByte(CP_UTF8, 0, _state.fileName.c_str(), -1, nullptr, 0, nullptr, nullptr);
//    char* buffer = new char[length + 1];
//    WideCharToMultiByte(CP_UTF8, 0, _state.fileName.c_str(), -1, buffer, length, nullptr, nullptr);
//    error = avformat_open_input(&_state.formatContext, buffer, nullptr, nullptr);
//    delete[] buffer;
//    if (error < 0)
//    {
//        CConsoleOutput::OutputConsoles(L"File open error");
//        return false;
//    }
//
//    if (avformat_find_stream_info(_state.formatContext, nullptr) < 0)
//    {
//        CConsoleOutput::OutputConsoles(L"Cannot find stream information");
//        return false;
//    }
//
//    // Find video stream
//    for (unsigned int i = 0; i < _state.formatContext->nb_streams; i++)
//    {
//        if (_state.formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
//            audioCodecParameters == nullptr)
//        {
//            audioCodecParameters = _state.formatContext->streams[i]->codecpar;
//            _state.audioStream = _state.formatContext->streams[i];
//            _state.audioStreamIndex = i;
//        }
//    }
//    if (audioCodecParameters == nullptr)
//    {
//        CConsoleOutput::OutputConsoles(L"Cannot find audio stream");
//        return false;
//    }
//
//    AVCodec* audioCodec = avcodec_find_decoder(audioCodecParameters->codec_id);
//    if (audioCodec == nullptr)
//    {
//        CConsoleOutput::OutputConsoles(L"Cannot find audio decoder");
//        return false;
//    }
//
//    _state.audioCodecContext = avcodec_alloc_context3(audioCodec);
//    if (avcodec_parameters_to_context(_state.audioCodecContext, audioCodecParameters) < 0)
//    {
//        CConsoleOutput::OutputConsoles(L"Failed to convert to context");
//        return false;
//    }
//
//    if (avcodec_open2(_state.audioCodecContext, audioCodec, nullptr) < 0)
//    {
//        CConsoleOutput::OutputConsoles(L"Cannot open codec");
//        return false;
//    }
//
//    _state.syncType = SyncType::AudioMaster;
//
//    _state.audioBufferSize = 0;
//    _state.audioBufferIndex = 0;
//    memset(&_state.audioPacket, 0, sizeof(_state.audioPacket));
//    InitPacketQueue(&_state.audioQueue);
//
//    _state.frameQueueRearIndex = 0;
//    _state.frameQueueWIndex = 0;
//    _state.frameQueueSize = 0;
//    _state.audioBufferSize = 0;
//    _state.audioBufferIndex = 0;
//    _state.audioPacketData = nullptr;
//    _state.audioPacketSize = 0;
//    memset(&_state.audioFrame, 0, sizeof(_state.audioFrame));
//
//    return true;
//}

bool CMusicPlayerControl::Create()
{
    AVCodecParameters* audioCodecParameters = nullptr;
    int error = 0;

    _state.formatContext = nullptr;

    int length = WideCharToMultiByte(CP_UTF8, 0, _state.fileName.c_str(), -1, nullptr, 0, nullptr, nullptr);
    char* buffer = new char[length + 1];
    WideCharToMultiByte(CP_UTF8, 0, _state.fileName.c_str(), -1, buffer, length, nullptr, nullptr);
    error = avformat_open_input(&_state.formatContext, buffer, nullptr, nullptr);
    delete[] buffer;
    if (error < 0)
    {
        CConsoleOutput::OutputConsoles(L"File open error");
        return false;
    }

    if (avformat_find_stream_info(_state.formatContext, nullptr) < 0)
    {
        CConsoleOutput::OutputConsoles(L"Cannot find stream information");
        return false;
    }

    // Find video stream
    for (unsigned int i = 0; i < _state.formatContext->nb_streams; i++)
    {
        if (_state.formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO &&
            audioCodecParameters == nullptr)
        {
            audioCodecParameters = _state.formatContext->streams[i]->codecpar;
            _state.audioStream = _state.formatContext->streams[i];
            _state.audioStreamIndex = i;
        }
    }
    if (audioCodecParameters == nullptr)
    {
        CConsoleOutput::OutputConsoles(L"Cannot find audio stream");
        return false;
    }

    AVCodec* audioCodec = avcodec_find_decoder(audioCodecParameters->codec_id);
    if (audioCodec == nullptr)
    {
        CConsoleOutput::OutputConsoles(L"Cannot find audio decoder");
        return false;
    }

    _state.audioCodecContext = avcodec_alloc_context3(audioCodec);
    if (avcodec_parameters_to_context(_state.audioCodecContext, audioCodecParameters) < 0)
    {
        CConsoleOutput::OutputConsoles(L"Failed to convert to context");
        return false;
    }

    if (avcodec_open2(_state.audioCodecContext, audioCodec, nullptr) < 0)
    {
        CConsoleOutput::OutputConsoles(L"Cannot open codec");
        return false;
    }

    _state.audioBufferSize = 0;
    _state.audioBufferIndex = 0;
    memset(&_state.audioPacket, 0, sizeof(_state.audioPacket));
    InitPacketQueue(&_state.audioQueue);

    _state.frameQueueRearIndex = 0;
    _state.frameQueueWIndex = 0;
    _state.frameQueueSize = 0;
    _state.audioBufferSize = 0;
    _state.audioBufferIndex = 0;
    _state.audioPacketData = nullptr;
    _state.audioPacketSize = 0;
    memset(&_state.audioFrame, 0, sizeof(_state.audioFrame));

    return true;
}

void CMusicPlayerControl::Destroy()
{
    Stop();

    if (_state.audioCodecContext)
    {
        avcodec_free_context(&_state.audioCodecContext);
        _state.audioCodecContext = nullptr;
    }
    if (_state.formatContext)
    {
        avformat_close_input(&_state.formatContext);
        _state.formatContext = nullptr;
    }
}

bool PlaySound(AudioState* audioState)
{
    SDL_AudioSpec audioSpec{};
    auto audioContext = audioState->audioCodecContext;
    audioSpec.freq = audioContext->sample_rate;
    audioSpec.format = AUDIO_S16SYS;
    audioSpec.channels = static_cast<Uint8>(audioContext->channels);
    audioSpec.silence = 0;
    audioSpec.samples = 4096;
    audioSpec.callback = MusicAudioCallback;
    audioSpec.userdata = audioState;

    /*int length = WideCharToMultiByte(CP_UTF8, 0, audioState->fileName.c_str(), -1, nullptr, 0, nullptr, nullptr);
    char* buffer = new char[length + 1];
    WideCharToMultiByte(CP_UTF8, 0, audioState->fileName.c_str(), -1, buffer, length, nullptr, nullptr);

    Uint32 wav_length;
    Uint8 *wav_buffer;
    if (SDL_LoadWAV(buffer, &audioSpec, &wav_buffer, &wav_length) == NULL)
    {
        return false;
    }
    delete[] buffer;

    audioSpec.callback = MusicAudioCallback;
    audioSpec.userdata = audioState;*/

    if (SDL_OpenAudio(&audioSpec, nullptr))
    {
        return false;
    }

    audioState->audioHwBufferSize = audioSpec.size;
    SDL_PauseAudio(0);

    AVPacket packet;
    for (;;)
    {
        if (!audioState->playing)
        {
            audioState->audioQueue.cond.notify_all();
            break;
        }

        // seek stuff goes here
        if (audioState->audioQueue.size > MAX_AUDIOQ_SIZE)
        {
            SDL_Delay(10);
            continue;
        }

        if (av_read_frame(audioState->formatContext, &packet) < 0)
        {
            if (audioState->formatContext->pb->error == 0)
            {
                audioState->finishQueue = true;
                audioState->audioQueue.cond.notify_all();
                continue;
            }
            audioState->playing = false;
            break;
        }

        if (packet.stream_index == audioState->audioStreamIndex)
        {
            PutPacketQueue(&audioState->audioQueue, &packet);
        }
        else
        {
            av_packet_unref(&packet);
        }
    }

    return true;
}

void CMusicPlayerControl::Play()
{
    std::queue<int> queue;
    _state.eventQueue.swap(queue);
    _state.playing = true;
    _state.finishQueue = false;

    std::thread([&]()
    {
        PlaySound(&_state);
    }).detach();
}

void CMusicPlayerControl::Stop()
{
    if (_state.playing)
    {
        _state.playing = false;
        SDL_CloseAudio();
    }
}
}
