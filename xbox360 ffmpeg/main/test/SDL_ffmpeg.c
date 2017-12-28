/*******************************************************************************
*                                                                              *
*   SDL_ffmpeg is a library for basic multimedia functionality.                *
*   SDL_ffmpeg is based on ffmpeg.                                             *
*                                                                              *
*   Copyright (C) 2007  Arjan Houben                                           *
*                                                                              *
*   SDL_ffmpeg is free software: you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published   *
*	by the Free Software Foundation, either version 3 of the License, or any   *
*   later version.                                                             *
*                                                                              *
*   This program is distributed in the hope that it will be useful,            *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
*   GNU Lesser General Public License for more details.                        *
*                                                                              *
*   You should have received a copy of the GNU Lesser General Public License   *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
*                                                                              *
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <SDL.h>
#include <SDL_thread.h>
#endif

#ifdef __unix__
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#endif

#include "SDL/SDL_ffmpeg.h"

int FFMPEG_init_was_called = 0;


int getAudioFrame( SDL_ffmpegFile*, AVPacket*, int16_t*, SDL_ffmpegAudioFrame* );

int getVideoFrame( SDL_ffmpegFile*, AVPacket*, AVFrame*, SDL_ffmpegVideoFrame* );

SDL_ffmpegFile* SDL_ffmpegCreateFile() {

    /* create SDL_ffmpegFile pointer */
    SDL_ffmpegFile *file = (SDL_ffmpegFile*)malloc( sizeof(SDL_ffmpegFile) );
    if(!file) return 0;

    /* create a semaphore for every file */
    file->decode = LIB_CreateSemaphore(1);

    /* allocate room for VStreams */
    file->vs = (SDL_ffmpegStream**)malloc( sizeof(SDL_ffmpegStream*) * MAX_STREAMS );
    if(!file->vs) {
        free( file );
        return 0;
    }

    /* allocate room for AStreams */
    file->as = (SDL_ffmpegStream**)malloc( sizeof(SDL_ffmpegStream*) * MAX_STREAMS );
    if(!file->as) {
        free( file );
        return 0;
    }

    /* initialize variables with standard values */
    file->audioStream = -1;
    file->videoStream = -1;

    file->offset = 0;
    file->videoOffset = 0;
    file->startTime = 0;

    file->threadID = 0;

    return file;
}

void SDL_ffmpegFree(SDL_ffmpegFile* file) {

    SDL_ffmpegStopDecoding(file);

    SDL_ffmpegFlush(file);

    free(file);
}

SDL_ffmpegFile* SDL_ffmpegOpen(const char* filename) {

    /* register all codecs */
    if(!FFMPEG_init_was_called) {
        FFMPEG_init_was_called = 1;
        av_register_all();
    }

    /* open new ffmpegFile */
    SDL_ffmpegFile *file = SDL_ffmpegCreateFile();
    if(!file) return 0;

    /* information about format is stored in file->_ffmpeg */

    /* open the file */
    if(av_open_input_file( (AVFormatContext**)(&file->_ffmpeg), filename, 0, 0, 0) != 0) {
        fprintf(stderr, "could not open \"%s\"\n", filename);
        free(file);
        return 0;
    }

    /* retrieve format information */
    if(av_find_stream_info(file->_ffmpeg) < 0) {
        fprintf(stderr, "could not retrieve video stream info");
        free(file);
        return 0;
    }

    /* find the streams in the file */
    file->VStreams = 0;
    file->AStreams = 0;
    file->threadActive = 0;

    /* iterate through all the streams and store audio/video streams */
    size_t i;
    for(i=0; i<((AVFormatContext*)file->_ffmpeg)->nb_streams; i++) {

        if(((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {

            /* if this is a packet of the correct type we create a new stream */
            SDL_ffmpegStream* stream = (SDL_ffmpegStream*)malloc( sizeof(SDL_ffmpegStream) );

            if(stream) {
                /* we set our stream to zero */
                memset(stream, 0, sizeof(SDL_ffmpegStream));

                /* save unique streamid */
                stream->id = i;

                /* the timeBase is what we use to calculate from/to pts */
                stream->timeBase = av_q2d(((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base) * 1000;

                /* save width, height and pixFmt of our outputframes */
                stream->width = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->width;
                stream->height = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->height;
                stream->pixFmt = PIX_FMT_RGB24;

                /* _ffmpeg holds data about streamcodec */
                stream->_ffmpeg = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec;

                /* get the correct decoder for this stream */
                AVCodec *codec = avcodec_find_decoder(((AVCodecContext*)stream->_ffmpeg)->codec_id);

                /* initialize empty audiobuffer */
                memset(stream->audioBuffer, 0, sizeof(SDL_ffmpegAudioFrame)*SDL_FFMPEG_MAX_BUFFERED_AUDIOFRAMES);
                memset(stream->videoBuffer, 0, sizeof(SDL_ffmpegVideoFrame)*SDL_FFMPEG_MAX_BUFFERED_VIDEOFRAMES);

                if(!codec) {
                    free(stream);
                    fprintf(stderr, "could not find codec\n");
                } else if(avcodec_open(((AVFormatContext*)file->_ffmpeg)->streams[i]->codec, codec) < 0) {
                    free(stream);
                    fprintf(stderr, "could not open decoder\n");
                } else {

                    /* copy metadata from AVStream into our stream */
                    stream->frameRate[0] = ((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base.num;
                    stream->frameRate[1] = ((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base.den;
                    memcpy(stream->language, ((AVFormatContext*)file->_ffmpeg)->streams[i]->language, 4);
                    stream->sampleRate = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->sample_rate;
                    stream->channels = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->channels;
                    memcpy(stream->codecName, ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->codec_name, 32);

                    file->vs[file->VStreams] = stream;
                    file->VStreams++;

                    /* create semaphore for thread-safe use */
                    stream->sem = LIB_CreateSemaphore(1);
                }
            }
        } else if(((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO) {

            /* if this is a packet of the correct type we create a new stream */
            SDL_ffmpegStream* stream = (SDL_ffmpegStream*)malloc( sizeof(SDL_ffmpegStream) );

            if(stream) {
                /* we set our stream to zero */
                memset(stream, 0, sizeof(SDL_ffmpegStream));

                /* save unique streamid */
                stream->id = i;

                /* the timeBase is what we use to calculate from/to pts */
                stream->timeBase = av_q2d(((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base) * 1000;

                /* _ffmpeg holds data about streamcodec */
                stream->_ffmpeg = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec;

                stream->width = 0;
                stream->height = 0;
                stream->pixFmt = PIX_FMT_RGB24;

                /* get the correct decoder for this stream */
                AVCodec *codec = avcodec_find_decoder(((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->codec_id);

                if(!codec) {
                    free( stream );
                    fprintf(stderr, "could not find codec\n");
                } else if(avcodec_open(((AVFormatContext*)file->_ffmpeg)->streams[i]->codec, codec) < 0) {
                    free( stream );
                    fprintf(stderr, "could not open decoder\n");
                } else {

                    /* copy metadata from AVStream into our stream */
                    stream->frameRate[0] = ((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base.num;
                    stream->frameRate[1] = ((AVFormatContext*)file->_ffmpeg)->streams[i]->time_base.den;
                    memcpy(stream->language, ((AVFormatContext*)file->_ffmpeg)->streams[i]->language, 4);
                    stream->sampleRate = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->sample_rate;
                    stream->channels = ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->channels;
                    memcpy(stream->codecName, ((AVFormatContext*)file->_ffmpeg)->streams[i]->codec->codec_name, 32);

                    file->as[file->AStreams] = stream;
                    file->AStreams++;

                    /* create semaphore for thread-safe use */
                    stream->sem = LIB_CreateSemaphore(1);
                }
            }
        }
    }

    return file;
}

SDL_ffmpegVideoFrame* SDL_ffmpegGetVideoFrame(SDL_ffmpegFile* file) {

    if( !SDL_ffmpegValidVideo(file) || file->pause ) return 0;

    /* working on videobuffer should always be done from inside semaphore */
    SDL_SemWait(file->vs[file->videoStream]->sem);

    SDL_ffmpegVideoFrame *f = 0;

    int i;
    for(i=0; i<SDL_FFMPEG_MAX_BUFFERED_VIDEOFRAMES; i++) {

        if(file->vs[file->videoStream]->videoBuffer[i].filled) {
            if(!f || f->timestamp > file->vs[file->videoStream]->videoBuffer[i].timestamp) {
                if(file->vs[file->videoStream]->videoBuffer[i].timestamp <= SDL_ffmpegGetPosition(file)) {
                    f = &file->vs[file->videoStream]->videoBuffer[i];
                }
            }
        }
    }

    /* no frame = no lock */
    if(!f) LIB_SemPost(file->vs[file->videoStream]->sem);

    return f;
}

int SDL_ffmpegReleaseVideo(SDL_ffmpegFile *file, SDL_ffmpegVideoFrame *frame) {

    /* no video, means no releasing */
    if( !SDL_ffmpegValidVideo(file) ) return -1;

    SDL_FreeSurface( frame->buffer );

    frame->filled = 0;

    LIB_SemPost(file->vs[file->videoStream]->sem);

    return 0;
}

SDL_ffmpegStream* SDL_ffmpegGetAudioStream(SDL_ffmpegFile *file, int audioID) {

    /* check if we have any audiostreams */
    if(!file->AStreams) return 0;

    /* check if the requested id is possible */
    if(audioID >= file->AStreams) return 0;

    /* return ausiostream linked to audioID */
    return file->as[audioID];
}

int SDL_ffmpegSelectAudioStream(SDL_ffmpegFile* file, int audioID) {

    /* check if we have any audiostreams */
    if(!file->AStreams) return -1;

    /* check if the requested id is possible */
    if(audioID >= file->AStreams) return -1;

    /* set current audiostream to stream linked to audioID */
    file->audioStream = audioID;

    return 0;
}

SDL_ffmpegStream* SDL_ffmpegGetVideoStream(SDL_ffmpegFile *file, int videoID) {

    /* check if we have any videostreams */
    if(!file->VStreams) return 0;

    /* check if the requested id is possible */
    if(videoID >= file->VStreams) return 0;

    /* return ausiostream linked to videoID */
    return file->vs[videoID];
}

int SDL_ffmpegSelectVideoStream(SDL_ffmpegFile* file, int videoID) {

    /* check if we have any videostreams */
    if(!file->VStreams) return -1;

    /* check if the requested id is possible */
    if(videoID >= file->VStreams) return -1;

    /* set current videostream to stream linked to videoID */
    file->videoStream = videoID;

    return 0;
}

int SDL_ffmpegStartDecoding(SDL_ffmpegFile* file) {

    /* start a thread that continues to fill audio/video buffers */
    if(!file->threadID) file->threadID = LIB_CreateThread(SDL_ffmpegDecodeThread, file);

    return 0;
}

int SDL_ffmpegStopDecoding(SDL_ffmpegFile* file) {

    /* stop decode thread */
    file->threadActive = 0;
    if(file->threadID) LIB_WaitThread(file->threadID, 0);

    /* set threadID to zero, so we can check for concurrent threads */
    file->threadID = 0;

    return -1;
}

int SDL_ffmpegDecodeThread(void* data) {

    /* unpack the void pointer */
    SDL_ffmpegFile* file = (SDL_ffmpegFile*)data;

    /* flag this thread as active, used for stopping */
    file->threadActive = 1;

    /* create a packet for our data */
    AVPacket pack;

    /* allocate another frame for unknown->RGB conversion */
    AVFrame *inFrameRGB = avcodec_alloc_frame();

    if(SDL_ffmpegValidVideo(file)) {
        /* allocate buffer */
        uint8_t *inVideoBuffer = (uint8_t*)malloc(  avpicture_get_size(PIX_FMT_RGB24,
                                                        file->vs[file->videoStream]->width,
                                                        file->vs[file->videoStream]->height) );

        /* put buffer into our reserved frame */
        avpicture_fill( (AVPicture*)inFrameRGB,
                        inVideoBuffer,
                        PIX_FMT_RGB24,
                        file->vs[file->videoStream]->width,
                        file->vs[file->videoStream]->height);
    }

    /* allocate temporary audiobuffer */
    int16_t *samples = (int16_t*)malloc( AVCODEC_MAX_AUDIO_FRAME_SIZE );

    while(file->threadActive) {

        /* let's start by entering the video semaphore */
        SDL_SemWait(file->decode);

        /* read a packet from the file */
        int decode = av_read_frame(file->_ffmpeg, &pack);

        /* we leave the decode semaphore */
        LIB_SemPost(file->decode);

        /* if we did not get a packet, we wait a bit and try again */
        if(decode < 0) {
            /* thread is idle */
            SDL_Delay(10);
            continue;
        }

        /* we got a packet, lets handle it */

        /* If it's a audio packet from our stream... */
        if( SDL_ffmpegValidAudio(file) && pack.stream_index == file->as[file->audioStream]->id ) {

            int a = 0;
            while(file->threadActive && file->as[file->audioStream]->audioBuffer[a].size) {
                a++;
                if(a >= SDL_FFMPEG_MAX_BUFFERED_AUDIOFRAMES) {
                    a = 0;
                    /* we tried all buffer options, wait a bit and try them again */
                    SDL_Delay(10);
                }
            }

            /* we got out of the loop, meaning we found an empty space in the audio buffer */

            /* we enter the audio semaphore */
            SDL_SemWait(file->as[file->audioStream]->sem);

                /* write found audioFrame into empty audiobuffer place */
                getAudioFrame( file, &pack, samples, &file->as[file->audioStream]->audioBuffer[a] );

            /* we leave the audio semaphore */
            LIB_SemPost(file->as[file->audioStream]->sem);

        }

        /* If it's a video packet from our video stream... */
        if( SDL_ffmpegValidVideo(file) && pack.stream_index == file->vs[file->videoStream]->id ) {

            int a = 0;
            while(file->threadActive && file->vs[file->videoStream]->videoBuffer[a].filled) {
                a++;
                if(a >= SDL_FFMPEG_MAX_BUFFERED_VIDEOFRAMES) {
                    a = 0;
                    /* we tried all buffer options, wait a bit and try them again */
                    SDL_Delay(10);
                }
            }

            /* we got out of the loop, meaning we found an empty space in the video buffer */

            /* we enter the video semaphore */
            SDL_SemWait(file->vs[file->videoStream]->sem);

                /* write found audioFrame into empty audiobuffer place */
                getVideoFrame( file, &pack, inFrameRGB, &file->vs[file->videoStream]->videoBuffer[a] );

            /* we leave the audio semaphore */
            LIB_SemPost(file->vs[file->videoStream]->sem);

        }

        /* if we stop this thread, we can release the packet we reserved */
        av_free_packet(&pack);
    }

    return 0;
}

int SDL_ffmpegSeek(SDL_ffmpegFile* file, int64_t timestamp) {

    /* if the seekposition is out of bounds, return */
    if(timestamp >= SDL_ffmpegGetDuration(file)) return -1;

    /* we enter the decode semaphore so the decode thread cannot be working on
       data we are trying to flush */
    SDL_SemWait(file->decode);

    /* if the stream has an offset, add it to the start time */
    int64_t startOffset = 0;
    if(((AVFormatContext*)file->_ffmpeg)->start_time != AV_NOPTS_VALUE) {
        /* inFormatCtx->start_time is in AV_TIME_BASE fractional seconds */
        startOffset = ((AVFormatContext*)file->_ffmpeg)->start_time;
    }

    /* calculate the final timestamp for the seek action this is in AV_TIME_BASE fractional seconds */
    startOffset += (timestamp * AV_TIME_BASE) / 1000;

    /* do the actual seeking, AVSEEK_FLAG_BACKWARD means we jump to the first
       keyframe before the one we want */
    if(av_seek_frame(file->_ffmpeg, -1, startOffset, AVSEEK_FLAG_BACKWARD) >= 0) {

        /* set some values in our file so we now were to start playing */
        file->offset = timestamp;
        file->startTime = SDL_GetTicks();

        /* then there is our flush call */
        SDL_ffmpegFlush(file);

        /* and we are done, lets release the decode semaphore so the decode
           thread can move on, filling buffer from our new position */
        LIB_SemPost(file->decode);

        return 0;
    }

    /* and release our lock on the decodethread */
    LIB_SemPost(file->decode);

    return -1;
}

int SDL_ffmpegSeekRelative(SDL_ffmpegFile *file, int64_t timestamp) {

    /* same thing as normal seek, just take into account the current position */
    return SDL_ffmpegSeek(file, SDL_ffmpegGetPosition(file) + timestamp);
}

int SDL_ffmpegFlush(SDL_ffmpegFile *file) {

    /* if we have a valid audio stream, we flush it */
    if( SDL_ffmpegValidAudio(file) ) {

        /* flush audiobuffer from semaphore, be thread-safe! */
        SDL_SemWait(file->as[file->audioStream]->sem);

        int i;
        for(i=0; i<SDL_FFMPEG_MAX_BUFFERED_AUDIOFRAMES; i++) {

            if(file->as[file->audioStream]->audioBuffer[i].size) {
                free( file->as[file->audioStream]->audioBuffer[i].buffer );
                file->as[file->audioStream]->audioBuffer[i].size = 0;
            }
        }

        avcodec_flush_buffers( (AVCodecContext*)file->as[file->audioStream]->_ffmpeg );

        LIB_SemPost(file->as[file->audioStream]->sem);
    }

    /* if we have a valid video stream, we flush some more */
    if( SDL_ffmpegValidVideo(file) ) {

        /* again, be thread safe! */
        SDL_SemWait(file->vs[file->videoStream]->sem);

        int i;
        for(i=0; i<SDL_FFMPEG_MAX_BUFFERED_VIDEOFRAMES; i++) {

            if(file->vs[file->videoStream]->videoBuffer[i].filled) {
                SDL_FreeSurface( file->vs[file->videoStream]->videoBuffer[i].buffer );
                file->vs[file->videoStream]->videoBuffer[i].filled = 0;
            }
        }

        avcodec_flush_buffers( (AVCodecContext*)file->vs[file->videoStream]->_ffmpeg );

        LIB_SemPost(file->vs[file->videoStream]->sem);
    }

    return 0;
}

SDL_ffmpegAudioFrame* SDL_ffmpegGetAudioFrame(SDL_ffmpegFile *file) {

    if( !SDL_ffmpegValidAudio(file) || file->pause ) return 0;

    /* working on audiobuffer should always be done from inside semaphore */
    SDL_SemWait(file->as[file->audioStream]->sem);

    SDL_ffmpegAudioFrame *f = 0;

    int i;
    for(i=0; i<SDL_FFMPEG_MAX_BUFFERED_AUDIOFRAMES; i++) {

        if(file->as[file->audioStream]->audioBuffer[i].size) {
            if(!f || f->timestamp > file->as[file->audioStream]->audioBuffer[i].timestamp) {
                if(file->as[file->audioStream]->audioBuffer[i].timestamp <= SDL_ffmpegGetPosition(file)) {
                    f = &file->as[file->audioStream]->audioBuffer[i];
                }
            }
        }
    }

    /* if there was no frame found, release semaphore */
    if(!f) LIB_SemPost(file->as[file->audioStream]->sem);

    return f;
}

int SDL_ffmpegReleaseAudio(SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame, int len) {

    /* no audio, means no releasing */
    if( !SDL_ffmpegValidAudio(file) ) return -1;

    /* we move the correct amount of data or invalidate the frame entirely */
    frame->size -= len;

    if(frame->size > 0) {
        /* move memory so we can use this frame again */
        memmove( frame->buffer, frame->buffer+len, frame->size );
    } else {
        /* free allocated buffer */
        free( frame->buffer );
        /* frame should be 0, but better safe than sorry.. */
        frame->size = 0;
    }

    /* work on audiodata is done, so we release the semaphore */
    if(frame) LIB_SemPost(file->as[file->audioStream]->sem);

    return 0;
}

int64_t SDL_ffmpegGetPosition(SDL_ffmpegFile *file) {

    /* return the current playposition of our file */
    return (SDL_GetTicks() + file->offset) - file->startTime;
}

SDL_AudioSpec* SDL_ffmpegGetAudioSpec(SDL_ffmpegFile *file, int samples, void *callback) {

    /* create audio spec */
    SDL_AudioSpec *spec = (SDL_AudioSpec*)malloc( sizeof(SDL_AudioSpec) );

    if(spec) {
        spec->format = AUDIO_S16SYS;
        spec->samples = samples;
        spec->userdata = file;
        spec->callback = callback;
        spec->freq = 48000;
        spec->channels = 2;

        /* if we have a valid audiofile, we can use its data to create a
           more appropriate audio spec */
        if( SDL_ffmpegValidAudio(file) ) {
            spec->freq = file->as[file->audioStream]->sampleRate;
            spec->channels = file->as[file->audioStream]->channels;
        }
    }

    return spec;
}

int64_t SDL_ffmpegGetDuration(SDL_ffmpegFile *file) {

    /* returns the duration of the entire file, please note that ffmpeg doesn't
       always get this value right! so don't bet your life on it... */
    return ((AVFormatContext*)file->_ffmpeg)->duration / (AV_TIME_BASE / 1000);
}

int SDL_ffmpegGetVideoSize(SDL_ffmpegFile *file, int *w, int *h) {

    if(!w || !h) return -1;

    /* if we have a valid video file selected, we use it
       if not, we send default values and return.
       by checking the return value you can check if you got a valid size */
    if( SDL_ffmpegValidVideo(file) ) {
        *w = file->vs[file->videoStream]->width;
        *h = file->vs[file->videoStream]->height;
        return 0;
    }

    *w = 320;
    *h = 240;
    return -1;
}

int SDL_ffmpegValidAudio(SDL_ffmpegFile* file) {

    /* this function is used to check if we selected a valid audio stream */
    if(file->audioStream < 0 || file->audioStream >= file->AStreams) return 0;

    return 1;
}

int SDL_ffmpegValidVideo(SDL_ffmpegFile* file) {

    /* this function is used to check if we selected a valid video stream */
    if(file->videoStream < 0 || file->videoStream >= file->VStreams) return 0;

    return 1;
}

int SDL_ffmpegPause(SDL_ffmpegFile *file, int state) {

    /* by putting 0 into state, we play the file
       this behaviour is analogue to SDL audio */
    file->pause = state;

    if(!file->pause) {
        file->startTime = SDL_GetTicks();
    }

    return 0;
}

int SDL_ffmpegGetState(SDL_ffmpegFile *file) {
    return file->pause;
}

int getAudioFrame( SDL_ffmpegFile *file, AVPacket *pack, int16_t *samples, SDL_ffmpegAudioFrame *frame ) {

    uint8_t *data = pack->data;
    int size = pack->size;
    int len;
    int audioSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;


    frame->timestamp = pack->pts * file->as[file->audioStream]->timeBase;

    if(SDL_ffmpegGetPosition(file) > frame->timestamp) {
        ((AVCodecContext*)file->as[file->audioStream]->_ffmpeg)->hurry_up = 1;
    } else {
        ((AVCodecContext*)file->as[file->audioStream]->_ffmpeg)->hurry_up = 0;
    }

    while(size > 0 && file->threadActive) {

        /* Decode the packet */
        len = avcodec_decode_audio2(file->as[file->audioStream]->_ffmpeg, samples, &audioSize, data, size);

        /* if error, or hurry state, we skip the frame */
        if(len <= 0 || !audioSize || ((AVCodecContext*)file->as[file->audioStream]->_ffmpeg)->hurry_up) return -1;

        /* change pointers */
        data += len;
        size -= len;
    }

    frame->size = audioSize;
    frame->buffer = (uint8_t*)malloc( audioSize );

    memcpy(frame->buffer, samples, audioSize);

    return 0;
}

int getVideoFrame( SDL_ffmpegFile* file, AVPacket *pack, AVFrame *inFrameRGB, SDL_ffmpegVideoFrame *frame ) {

    /* usefull when dealing with B frames */
    if(pack->dts == AV_NOPTS_VALUE) {
        /* if we did not get a valid timestamp, we make one up based on the last
           valid timestamp + the duration of a frame */
        frame->timestamp = file->vs[file->videoStream]->lastTimeStamp + file->vs[file->videoStream]->timeBase;
    } else {
        /* write timestamp into the buffer */
        frame->timestamp = file->vs[file->videoStream]->timeBase * pack->dts;
    }

    if(SDL_ffmpegGetPosition(file) > frame->timestamp) {
        ((AVCodecContext*)file->vs[file->videoStream]->_ffmpeg)->hurry_up = 1;
    } else {
        ((AVCodecContext*)file->vs[file->videoStream]->_ffmpeg)->hurry_up = 0;
    }

    int got_frame = 0;

    /* allocate a frame */
    AVFrame *inFrame = avcodec_alloc_frame();

    /* Decode the packet */
    avcodec_decode_video(file->vs[file->videoStream]->_ffmpeg, inFrame, &got_frame, pack->data, pack->size);

    if( got_frame && !((AVCodecContext*)file->vs[file->videoStream]->_ffmpeg)->hurry_up ) {

        /* we convert whatever type of data we got to RGB24 */
        img_convert((AVPicture*)inFrameRGB, PIX_FMT_RGB24, (AVPicture*)inFrame,
                    ((AVCodecContext*)file->vs[file->videoStream]->_ffmpeg)->pix_fmt,
                    file->vs[file->videoStream]->width, file->vs[file->videoStream]->height);

        /* we create a SDL_Surface to store the frame data */
        frame->buffer = SDL_CreateRGBSurface( 0, file->vs[file->videoStream]->width,
                                              file->vs[file->videoStream]->height, 24,
                                              0x0000FF, 0x00FF00, 0xFF0000, 0 );

        /* copy image data to SDL_Surface */
        memcpy( frame->buffer->pixels, inFrameRGB->data[0],
                file->vs[file->videoStream]->width * file->vs[file->videoStream]->height * 3);

        /* flag this frame as not-empty */
        frame->filled = 1;

        /* we write the lastTimestamp we got */
        file->vs[file->videoStream]->lastTimeStamp = frame->timestamp;
    } else {
        return -1;
    }

    return 0;
}
