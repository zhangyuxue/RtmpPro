//#ifndef X264ENCODER_VIDEOENCODER_H
//#define X264ENCODER_VIDEOENCODER_H

#pragma once

#include <cstdint>
#include <memory>


class VideoEncoder
{
    public:
    VideoEncoder(uint16_t outWidth, uint16_t outHeight, uint32_t fps) :
    _outWidth(outWidth),_outHeight(outHeight), _fps(fps) {}
    virtual ~VideoEncoder() = default;
    //virtual uint32_t encode(const std::unique_ptr<uint8_t[]>& imgData,
    //                        uint16_t width,
    //                        uint16_t height,
    //                        std::unique_ptr<uint8_t[]>& encodedData) = 0;
    protected:
    uint16_t _outWidth;
    uint16_t _outHeight;
    uint32_t _fps;
    bool _isAlive;
    int64_t _frameCounter = 0;

};



//#endif //X264ENCODER_VIDEOENCODER_H
