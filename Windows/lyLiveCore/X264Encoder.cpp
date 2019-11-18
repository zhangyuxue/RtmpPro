#include "X264Encoder.h"
#include <string>
#include <cstring>


X264Encoder::~X264Encoder()
{
    if (_encoder)
    {
        x264_picture_clean(&_picIn);
        memset((char*)&_picIn, 0, sizeof(_picIn));
        memset((char*)&_picOut, 0, sizeof(_picOut));

        x264_encoder_close(_encoder);
        _encoder = nullptr;
    }
}


X264Encoder::X264Encoder(uint16_t outWidth, uint16_t outHeight, uint32_t fps) : VideoEncoder(outWidth, outHeight, fps)
{
    x264_param_t param;
    x264_param_default_preset(
            &param, "ultrafast", "zerolatency");
    param.i_width = _outWidth;
    param.i_height = _outHeight;
    param.i_fps_num = _fps;
    param.i_fps_den = 1;
    param.rc.i_qp_constant = 20;

    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.f_rf_constant = 20;
    param.rc.f_rf_constant_max = 25;

    param.i_csp = X264_CSP_I420;

    x264_param_apply_profile(&param, "baseline");
    _encoder = x264_encoder_open(&param);
    x264_encoder_headers(_encoder, &_nals, &_nheader);

    // Initialize X264 Picture
    x264_picture_alloc(&_picIn, X264_CSP_I420, _outWidth, _outHeight);

    _isAlive = true;
}


uint32_t X264Encoder::encodeAndSend(unsigned char *pic_yuv420, int y_size)
{
    // Encode
    _picIn.i_pts = _frameCounter++;
    uint32_t frameSize = x264_encoder_encode(_encoder, &_nals, &_numNals, &_picIn, &_picOut);

    // Copy output data
	////rgb888_to_yuv420p(pic_rgb24,pic_yuv420,imgw,imgh);
	memcpy(_picIn.img.plane[0], pic_yuv420, y_size);         //Y
	memcpy(_picIn.img.plane[1], pic_yuv420 + y_size, y_size / 4);     //U
	memcpy(_picIn.img.plane[2], pic_yuv420 + y_size + y_size / 4, y_size / 4);     //V

	for (int j = 0; j < _numNals; ++j) {
		//rtpPusher->sendMediaData(KN_MEDIA_VIDEO, _nals[j].p_payload, _nals[j].i_payload, _frameCounter + j);
	}
    return frameSize;
}