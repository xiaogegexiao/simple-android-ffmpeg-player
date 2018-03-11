//
// Created by Xiao Mei on 8/3/18.
//

#include <jni.h>
#include <map>

extern "C" {
    #include "stdlib.h"
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/pixfmt.h>
    #include <libavutil/imgutils.h>
    #include <android/bitmap.h>

    /*for android logs*/
    #include <android/log.h>
    #define LOG_TAG "niroffmpeg_native"
    #define LOGI(...) __android_log_print(4, LOG_TAG, __VA_ARGS__);
    #define LOGE(...) __android_log_print(6, LOG_TAG, __VA_ARGS__);

    int Java_com_nirovision_nostalgia_JniBridge_decodeStream(JNIEnv *env, jobject instance, jstring streamUrl_, jobject callback);
    void sendBackBitmap(JNIEnv *pEnv, uint8_t * buffer, int bufferSize, int height, int width, jobject callback);
    bool keepStreaming(JNIEnv *pEnv, jobject callback);
}

JNIEXPORT int JNICALL
Java_com_nirovision_nostalgia_JniBridge_decodeStream(JNIEnv *env, jobject instance,
                                                              jstring streamUrl_, jobject callback) {
    const char *streamUrl = env->GetStringUTFChars(streamUrl_, 0);

    AVFormatContext *pFormatCtx = NULL;
    int             i, videoStream;
    AVCodecContext  *pCodecCtx = NULL;
    AVCodec         *pCodec = NULL;
    AVFrame         *pFrame = NULL;
    AVFrame         *pFrameRGBA = NULL;
    AVDictionary    *opts = NULL;
    AVPacket        packet;
    int             frameFinished;
    void* 			buffer;
    int             bufferSize;
    void*           bitmapBuf;

    AVDictionary    *optionsDict = NULL;
    struct SwsContext      *sws_ctx = NULL;

    // Register all formats and codecs
    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    // Open video stream
    LOGI("video stream: %s\n", streamUrl);

    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    int open_result = avformat_open_input(&pFormatCtx, streamUrl, NULL, &opts);
    if(open_result!=0) {
        LOGE("cannot open stream %d", open_result);
        return -1; // Couldn't open video stream
    }

    // Retrieve stream information
    if(avformat_find_stream_info(pFormatCtx, NULL)<0)
        return -1; // Couldn't find stream information

    LOGI("dump info");
    // Dump information about stream onto standard error
    av_dump_format(pFormatCtx, 0, streamUrl, 0);

    LOGI("try to find first video");
    // Find the first video stream
    videoStream=-1;
    for(i=0; i<pFormatCtx->nb_streams; i++) {
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            videoStream=i;
            break;
        }
    }
    if(videoStream==-1) {
        LOGI("didn't find a video stream");
        return -1; // Didn't find a video stream
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtx=pFormatCtx->streams[videoStream]->codec;

    LOGI("find decoder");
    // Find the decoder for the video stream
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL) {
        LOGI("Unsupported codec!\n");
        return -1; // Codec not found
    }

    LOGI("open codec");
    // Open codec
    if(avcodec_open2(pCodecCtx, pCodec, &optionsDict)<0)
        return -1; // Could not open codec

    // Allocate video frame
    pFrame=av_frame_alloc();

    // Allocate an AVFrame structure
    pFrameRGBA=av_frame_alloc();
    if(pFrameRGBA==NULL)
        return -1;

    LOGI("create bitmap");
    //create a bitmap as the buffer for pFrameRGBA
    bufferSize = avpicture_get_size(AV_PIX_FMT_RGBA, pCodecCtx->width, pCodecCtx->height);
    buffer = new uint8_t[bufferSize];
    //get the scaling context
    sws_ctx = sws_getContext
            (
                    pCodecCtx->width,
                    pCodecCtx->height,
                    pCodecCtx->pix_fmt,
                    pCodecCtx->width,
                    pCodecCtx->height,
                    AV_PIX_FMT_RGBA,
                    SWS_BILINEAR,
                    NULL,
                    NULL,
                    NULL
            );

    // Assign appropriate parts of bitmap to image planes in pFrameRGBA
    // Note that pFrameRGBA is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture *)pFrameRGBA, (uint8_t*)buffer, AV_PIX_FMT_RGBA
            ,
                   pCodecCtx->width, pCodecCtx->height);

    LOGI("start reading frame");
    // Read frames and save first five frames to disk
    while(keepStreaming(env, callback) && av_read_frame(pFormatCtx, &packet)>=0) {
        // Is this a packet from the video stream?
        if(packet.stream_index==videoStream) {
            // Decode video frame
            avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
                                  &packet);
            // Did we get a video frame?
            if(frameFinished) {
                // Convert the image from its native format to RGBA
                sws_scale(sws_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
                          pCodecCtx->height, pFrameRGBA->data, pFrameRGBA->linesize);
                sendBackBitmap(env, (uint8_t*)buffer, bufferSize, pCodecCtx->height, pCodecCtx->width, callback);
            }
        }
        // Free th
        // e packet that was allocated by av_read_frame
        av_free_packet(&packet);
    }
    LOGI("finish reading frame");

    // Free the RGB image
    av_free(pFrameRGBA);

    // Free the YUV frame
    av_free(pFrame);

    // Close the codec
    avcodec_close(pCodecCtx);

    // Close the video file
    avformat_close_input(&pFormatCtx);
    avformat_network_deinit();
    env->ReleaseStringUTFChars(streamUrl_, streamUrl);
    return 0;
}

bool keepStreaming(JNIEnv *pEnv, jobject callback) {
    jclass callbackCls = pEnv->GetObjectClass(callback);
    jmethodID keepStreamingMethod = pEnv->GetMethodID(callbackCls, "keepStreaming", "()Z");
    jboolean keepStreaming = pEnv->CallBooleanMethod(callback, keepStreamingMethod);
    bool res = (bool)(keepStreaming == JNI_TRUE);
    pEnv->DeleteLocalRef(callbackCls);
    return res;
}

void sendBackBitmap(JNIEnv *pEnv, uint8_t * buffer, int bufferSize, int height, int width, jobject callback) {
    jclass callbackCls = pEnv->GetObjectClass(callback);
    jmethodID onVideo = pEnv->GetMethodID(callbackCls, "onVideo", "(II[B)V");
//    pEnv->CallVoidMethod(callback, onVideo, bitmap);

    jbyteArray retArray = pEnv->NewByteArray(bufferSize);

    void *temp = pEnv->GetPrimitiveArrayCritical((jarray)retArray, 0);
    memcpy(temp, buffer, bufferSize);
    pEnv->ReleasePrimitiveArrayCritical(retArray, temp, 0);

    pEnv->CallVoidMethod(callback, onVideo, height, width, retArray);
    pEnv->DeleteLocalRef(retArray);
    pEnv->DeleteLocalRef(callbackCls);
}