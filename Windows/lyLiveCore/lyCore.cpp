#include "lyCore.h"
#include <thread>

#include "audioProcess.h"
#include "videoProcess.h"


lyLiveCoreImp* GetLiveCoreImp()
{
	return lyLiveCore::getInstance();
}

lyLiveCore* lyLiveCore::m_selfPtr=nullptr;
lyLiveCore::lyLiveCore()
{
	m_videoProcess = new lyVideoProcess;
	m_audioProcess = new lyAudioProcess;
}


lyLiveCore::~lyLiveCore()
{
}

BOOL lyLiveCore::InitializeApp(HWND videohand)
{
	return true;
}

std::vector<std::string> lyLiveCore::GetCaptureList()
{
	return m_videoProcess->getCaptureList();
	std::vector<std::string> tt;
	return tt;
}

int lyLiveCore::startCapture(int index, std::string capturename)
{
	return m_videoProcess->startCapture(index,capturename);
	return 0;
}

int lyLiveCore::stopCapture()
{
	return m_videoProcess->stopCapture();
	return 0;
}

int lyLiveCore::getImageSize(int& width, int& height)
{
	return m_videoProcess->getImageSize(width,height);
	return 0;
}

int lyLiveCore::getRGBPixels(unsigned char* _dataBuffer)
{
	return m_videoProcess->getRGBPixels(_dataBuffer);
	return 0;
}


extern "C"
{
#include "../ThirdPart/x264/x264.h"
}
#include "lyCoreUtils.h"
#include "faac.h"
#include "faaccfg.h"
#include "libyuv.h"


#ifdef _WIN32
//#include <winsock2.h>
//#include <time.h>
#else
#include <sys/time.h>
#endif

unsigned long long GetCurrentTimeMsec()
{
#ifdef _WIN32
	struct timeval tv;
	time_t clock;
	struct tm tm;
	SYSTEMTIME wtm;

	GetLocalTime(&wtm);
	tm.tm_year = wtm.wYear - 1900;
	tm.tm_mon = wtm.wMonth - 1;
	tm.tm_mday = wtm.wDay;
	tm.tm_hour = wtm.wHour;
	tm.tm_min = wtm.wMinute;
	tm.tm_sec = wtm.wSecond;
	tm.tm_isdst = -1;
	clock = mktime(&tm);
	tv.tv_sec = clock;
	tv.tv_usec = wtm.wMilliseconds * 1000;
	return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000);
#endif
}
#include "RtmpPusher.h"

/**
 * 初始化winsock
 *
 * @成功则返回1 , 失败则返回相应错误代码
 */
int InitSockets()
{
#ifdef WIN32     
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else     
	return TRUE;
#endif     
}

/**
 * 释放winsock
 *
 * @成功则返回0 , 失败则返回相应错误代码
 */
inline void CleanupSockets()
{
#ifdef WIN32     
	WSACleanup();
#endif     
}


int lyLiveCore::startLive(std::string rtmp_url, int mode)
{
	InitSockets();
	vvav::RtmpPusher* rtmpusher = new vvav::RtmpPusher;
	if (rtmpusher->init(rtmp_url.c_str(), 5000) != 0)
		return -1;
	m_audioProcess->startAudioCaputre();

	std::thread* AACthread = new std::thread([=]() {
		unsigned long sampleRate = 44100;  //编码采样率
		unsigned int numChannels = 1;         //编码声道数
		unsigned long inputSample = 0;        //输入样本大小，在打开编码器时会得到此值
		unsigned long maxOutputBytes = 0;  //最大输出，编码后的输出数据大小不会高于这个值，也是打开编码器时获得
		unsigned int    mPCMBitSize = 16;    //pcm位深，用于计算一帧pcm大小

		int mPCMBufferSize = 0;    //一帧PCM缓存大小
		int mCountSize = 0;           //计算缓存大小
		char* mPCMBuffer;           //PCM缓存

		faacEncHandle encoder;    //faac编码器句柄
		faacEncConfigurationPtr config;   //faac设置类
		encoder = faacEncOpen(sampleRate, numChannels, &inputSample, &maxOutputBytes);

		config = faacEncGetCurrentConfiguration(encoder);   //获取当前编码器的设置句柄  
		config->aacObjectType = LOW;    //设置AAC类型
		config->useLfe = 0;    //是否允许一个声道为低频通道
		config->useTns = 1;  //是否使用瞬时噪声定形滤波器(具体作用不是很清楚)
		config->allowMidside = 0;  //是否允许midSide coding (在MPEG-2 AAC 系统中，M/S(Mid/Side) Stereo coding被提供在多声道信号中，每个声道对(channel pair)的组合，也就是每个通道对，是对称地排列在人耳听觉的左右两边，其方式简单，且对位串不会引起较显著的负担。 一般其在左右声道数据相似度大时常被用到，并需记载每一频带的四种能量临界组合，分别为左、右、左右声道音频合并(L+R)及相减(L-R)的两种新的能量。一般，若所转换的Sid声道的能量较小时，M/S Stereo coding 可以节省此通道的位数，而将多余的位应用于另一个所转换的声道，即Mid 声道，进而可提高此编码效率。)
		config->outputFormat = 1;  // RAW_STREAM = 0, ADTS_STREAM=1  (ADTS可以实现单帧单独解码，raw由于缺少头无法单帧解码，因此无法做实时传输)
		config->bitRate = 48000;  //设置比特率
		config->inputFormat = FAAC_INPUT_16BIT;  //设置输入PCM格式
		faacEncSetConfiguration(encoder, config);   //应用设置
		//FILE* AACFile = NULL;
		//AACFile = fopen("D:/ttttt.aac", "wb+");
		int timestamp = 0;
		while (1)
		{
			lyAudioItem audioitem;
			if (m_audioProcess->popAudioInput(audioitem)
				== 0)
			{
				unsigned char* aacData = new unsigned char[maxOutputBytes];   //编码后输出数据（也就是AAC数据）存放位置

			   //开始编码，encoder为编码器句柄，mPCMBuffer为PCM数据，inputSample为打开编码器时得到的输入样本数据
			   //aacData为编码后数据存放位置，maxOutputBytes为编码后最大输出字节数，ret为编码后数据长度
				int ret = faacEncEncode(encoder, (int32_t *)audioitem.audiobuffer, inputSample,
					aacData, maxOutputBytes);
				//ret为0时不代表编码失败，而是编码速度较慢，导致缓存还未完全flush，可用一个循环继续调用编码接口，当 ret>0 时表示编码成功，且返回值为编码后数据长度
				while (ret == 0)
				{
					ret = faacEncEncode(encoder, (int32_t *)audioitem.audiobuffer, inputSample, aacData, maxOutputBytes);
				}

				if (ret > 0)
				{
				 	unsigned char *buf;
					unsigned char spec_buf[2048] = { 0 };
					unsigned long len;
					int rethead = faacEncGetDecoderSpecificInfo(encoder, &buf, &len);
					if (rethead == 0)
						memcpy(spec_buf, buf, len);
						int spec_len = len;
						rtmpusher->sendAacSequenceHeader(spec_buf,len);
						/*释放系统内存*/
						free(buf);					{

					}
						rtmpusher->sendAudioData(aacData, maxOutputBytes, GetCurrentTimeMsec());
					//fwrite(aacData, 1, maxOutputBytes, AACFile);
					//m_rtpPusher->sendMediaData(KN_MEDIA_AUDIO, aacData, maxOutputBytes, timestamp++);
				}
				free(audioitem.audiobuffer);
			}

			Sleep(10);
		}




	});
	AACthread->detach();

	std::thread* x264thread = new std::thread([=]() {
		int ret;
		int y_size;
		int i, j;

		//Encode 50 frame
		//if set 0, encode all frame
		//int frame_num = 25;
		int csp = X264_CSP_I420;
		int width = 1280, height = 720;

		int iNal = 0;
		x264_nal_t* pNals = NULL;
		x264_t* pHandle = NULL;
		x264_picture_t* pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
		x264_picture_t* pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
		x264_param_t* pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

		x264_param_default(pParam);
		pParam->i_width = width;
		pParam->i_height = height;

		pParam->i_csp = csp;
		x264_param_apply_profile(pParam, x264_profile_names[5]);

		pHandle = x264_encoder_open(pParam);

		x264_picture_init(pPic_out);
		x264_picture_alloc(pPic_in, csp, pParam->i_width, pParam->i_height);

		//ret = x264_encoder_headers(pHandle, &pNals, &iNal);

		y_size = pParam->i_width * pParam->i_height;
		int imgw = 0;
		int imgh = 0;
		int buffersize = getImageSize(imgw, imgh);
		i = 0;
		//FILE* h264File = NULL;
		//h264File = fopen("D:/ttttt.h264", "wb+");
		unsigned char *pic_rgb24 = (unsigned char *)malloc(buffersize);
		unsigned char *pic_yuv420 = (unsigned char *)malloc(buffersize / 2);
		int dts = 0;
		int pts = 0;
		double fps = 25;
		uint32_t timestamp = 0;

		while (1)
		{
			if (getRGBPixels(pic_rgb24) != 0)
			{
				Sleep(5);
				continue;
			}
			if(m_videoRender != nullptr)
			{
				m_videoRender->videoRenderCallBack(width, height,pic_rgb24);
			}


			if (pic_rgb24)
			{
				//memcpy(pPic_in->img.plane[0], pic_rgb24, width*height*3);
				libyuv::RGB24ToI420(pic_rgb24, width * 3,
					pic_yuv420, width,
					pic_yuv420 + width * height, (width + 1) / 2,
					pic_yuv420 + width * height + ((width + 1) / 2)*((height + 1) / 2), (width + 1) / 2,
					width, height);
				//rgb888_to_yuv420p(pic_rgb24,pic_yuv420,imgw,imgh);
				//RGB24_TO_YUV420(pic_rgb24, imgw, imgh, pic_yuv420);
				memcpy(pPic_in->img.plane[0], pic_yuv420, y_size );         //Y
				memcpy(pPic_in->img.plane[1], pic_yuv420+y_size, y_size/4);     //U
				memcpy(pPic_in->img.plane[2], pic_yuv420 + y_size+y_size/4,y_size/4);     //V
			}	
			
			pPic_in->i_pts = i;
			pPic_in->i_type = X264_TYPE_IDR;
			ret = x264_encoder_encode(pHandle, &pNals, &iNal, pPic_in, pPic_out);
			if (ret < 0) {
				printf("Error.\n");
				return -1;
			}
			uint8_t sps[2048] = { 0 };
			uint8_t pps[2048] = { 0 };
			int sps_len = 0;
			int pps_len = 0;
			int i, last;
			//timestamp = GetCurrentTimeMsec()+60;
			for (i = 0, last = 0; i < iNal; i++) {
				if (pNals[i].i_type == NAL_SPS) {
					sps_len = pNals[i].i_payload - 4;
					memcpy(sps, pNals[i].p_payload + 4, sps_len);
				}
				else if (pNals[i].i_type == NAL_PPS) {
					pps_len = pNals[i].i_payload - 4;
					memcpy(pps, pNals[i].p_payload + 4, pps_len);

					/*发送sps pps*/
					rtmpusher->sendSpsAndPps(sps,sps_len,pps,pps_len);

				}
				else {

					/*发送普通帧*/
					rtmpusher->sendVideoData(pNals[i].p_payload, pNals[i].i_payload, GetCurrentTimeMsec());
				}
				last += pNals[i].i_payload;
				//timestamp += 1;
			}
			//超出上限怎么办
			//timestamp += 60;
			//i+=30;
			i++;
			Sleep(10);
		}
		free(pic_rgb24);
		free(pic_yuv420);
		i = 0;

		x264_picture_clean(pPic_in);
		x264_encoder_close(pHandle);
		pHandle = NULL;

		free(pPic_in);
		free(pPic_out);
		free(pParam);

	});
	x264thread->detach();
	return 0;
}

int lyLiveCore::stopLive(std::string url)
{

	return 0;
}

std::vector<std::string> lyLiveCore::getAudioInputDevs()
{
	std::vector<std::string> tt;
	return tt;
//	return m_audioProcess->getAudioInputDevs();
}

std::vector<std::string> lyLiveCore::getAudioOutputDevs()
{
	std::vector<std::string> tt;
	return tt;
//	return m_audioProcess->getAudioOutputDevs();
}

int lyLiveCore::setVideoRenderCallBack(lyLiveCoreVideoRenderImp* rendcall)
{
	std::lock_guard<std::mutex> lck(m_videoMutex);
	m_videoRender = rendcall;
	return 0;
}