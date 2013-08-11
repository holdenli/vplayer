#include "common.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include "lodepng.h"
#include <iostream>
#include <sstream>

const enum AVPixelFormat DST_FORMAT = PIX_FMT_BGR32;

void encodeOneStep(const char* filename, const unsigned char* image, unsigned width, unsigned height)
{
    /*Encode the image*/
    unsigned error = lodepng_encode32_file(filename, image, width, height);

    /*if there's an error, display it*/
    if(error) std::cerr << "ERROR: " << error << ": "<< lodepng_error_text(error) << std::endl;
}

int main(int argc, char* args[])
{
    (void)argc;
    (void)args;

    av_register_all();

    AVFormatContext *format_context = NULL;

    if (avformat_open_input(&format_context, args[1], NULL, NULL) != 0)
    {
        std::cerr << "ERROR: Could not open file" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(format_context, NULL))
    {
        std::cerr << "ERROR: Could not find streams" << std::endl;
        return -1;
    }

    av_dump_format(format_context, 0, args[1], 0);

    
    // Find the video stream
    int video_stream = -1;
    for (unsigned int i = 0; i < format_context->nb_streams; i++)
    {
        if (format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            video_stream = i;
            break;
        }
    }
    if (video_stream < 0)
    {
        std::cerr << "ERROR: Could not find video stream" << std::endl;
        return -1;
    }
    AVCodecContext *codec_context = format_context->streams[video_stream]->codec;

    // Decode
    AVCodec *codec = avcodec_find_decoder(codec_context->codec_id);
    if (codec == NULL)
    {
        std::cerr << "ERROR: Unsupported codec" << std::endl;
        return -1;
    }
    if (avcodec_open2(codec_context, codec, NULL) < 0)
    {
        std::cerr << "ERROR: Could not open codec" << std::endl;
        return -1;
    }

    AVFrame *frame = avcodec_alloc_frame();
    AVFrame *frameRGB = avcodec_alloc_frame();
    if (frame == NULL || frameRGB == NULL)
    {
        return -1;
    }

    // Create 24 bit RGB image
    int num_bytes = avpicture_get_size(DST_FORMAT,
        codec_context->width, codec_context->height);
    uint8_t *buffer = (uint8_t*)av_malloc(num_bytes * sizeof(uint8_t));
    avpicture_fill((AVPicture *)frameRGB, buffer, DST_FORMAT,
        codec_context->width, codec_context->height);
    SwsContext *sws_context = sws_getCachedContext(NULL,
        codec_context->width, codec_context->height, codec_context->pix_fmt,
        codec_context->width, codec_context->height, DST_FORMAT,
        SWS_BICUBIC, 0, 0, 0
        );

    // Read data
    int frame_finished;
    AVPacket packet;

    for (int i = 0; av_read_frame(format_context, &packet) >= 0;)
    {
        // is packet from video?
        if (packet.stream_index == video_stream)
        {
            // decode video frame
            avcodec_decode_video2(codec_context, frame, &frame_finished, &packet);

            if (frame_finished)
            {
                sws_scale(
                    sws_context, frame->data, frame->linesize,
                    0, codec_context->height,
                    frameRGB->data, frameRGB->linesize
                    );

                // Save frame
                if (++i > 0)
                {
                    std::cerr << "Saving frame " << i << std::endl;
                    std::stringstream ss;
                    ss << "test_";
                    ss << i;;
                    ss << ".png";
                    std::string str = ss.str();
                    encodeOneStep(str.c_str(), buffer, codec_context->width, codec_context->height);
                }
            }
        }

        av_free_packet(&packet);
    }

    // Done
    av_free(frame);
    av_free(frameRGB);
    av_free(buffer);
    sws_freeContext(sws_context); 
    avcodec_close(codec_context);
    avformat_close_input(&format_context);
    return 0;
}
