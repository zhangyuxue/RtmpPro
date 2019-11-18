//#ifndef CODEC_SAMPLES_X264ENCODER1_H
//#define CODEC_SAMPLES_X264ENCODER1_H
#pragma once

#include "VideoEncoder.h"

extern "C"
{
#include "../ThirdPart/x264/x264.h"
}


class X264Encoder : public VideoEncoder{
public:
    X264Encoder(uint16_t outWidth, uint16_t outHeight, uint32_t fps);

    ~X264Encoder() override;

    uint32_t encodeAndSend(unsigned char *pic_yuv420,int y_size);

private:
    x264_t *_encoder;
    x264_picture_t _picIn;
    x264_picture_t _picOut;
    int _nheader;
    x264_nal_t *_nals;
    int _numNals;
};
//#endif //CODEC_SAMPLES_X264ENCODER1_H
