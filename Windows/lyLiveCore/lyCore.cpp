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
 * ��ʼ��winsock
 *
 * @�ɹ��򷵻�1 , ʧ���򷵻���Ӧ�������
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
 * �ͷ�winsock
 *
 * @�ɹ��򷵻�0 , ʧ���򷵻���Ӧ�������
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
		unsigned long sampleRate = 44100;  //���������
		unsigned int numChannels = 1;         //����������
		unsigned long inputSample = 0;        //����������С���ڴ򿪱�����ʱ��õ���ֵ
		unsigned long maxOutputBytes = 0;  //��������������������ݴ�С����������ֵ��Ҳ�Ǵ򿪱�����ʱ���
		unsigned int    mPCMBitSize = 16;    //pcmλ����ڼ���һ֡pcm��С

		int mPCMBufferSize = 0;    //һ֡PCM�����С
		int mCountSize = 0;           //���㻺���С
		char* mPCMBuffer;           //PCM����

		faacEncHandle encoder;    //faac���������
		faacEncConfigurationPtr config;   //faac������
		encoder = faacEncOpen(sampleRate, numChannels, &inputSample, &maxOutputBytes);

		config = faacEncGetCurrentConfiguration(encoder);   //��ȡ��ǰ�����������þ��  
		config->aacObjectType = LOW;    //����AAC����
		config->useLfe = 0;    //�Ƿ�����һ������Ϊ��Ƶͨ��
		config->useTns = 1;  //�Ƿ�ʹ��˲ʱ���������˲���(�������ò��Ǻ����)
		config->allowMidside = 0;  //�Ƿ�����midSide coding (��MPEG-2 AAC ϵͳ�У�M/S(Mid/Side) Stereo coding���ṩ�ڶ������ź��У�ÿ��������(channel pair)����ϣ�Ҳ����ÿ��ͨ���ԣ��ǶԳƵ��������˶��������������ߣ��䷽ʽ�򵥣��Ҷ�λ����������������ĸ����� һ���������������������ƶȴ�ʱ�����õ����������ÿһƵ�������������ٽ���ϣ��ֱ�Ϊ���ҡ�����������Ƶ�ϲ�(L+R)�����(L-R)�������µ�������һ�㣬����ת����Sid������������Сʱ��M/S Stereo coding ���Խ�ʡ��ͨ����λ�������������λӦ������һ����ת������������Mid ��������������ߴ˱���Ч�ʡ�)
		config->outputFormat = 1;  // RAW_STREAM = 0, ADTS_STREAM=1  (ADTS����ʵ�ֵ�֡�������룬raw����ȱ��ͷ�޷���֡���룬����޷���ʵʱ����)
		config->bitRate = 48000;  //���ñ�����
		config->inputFormat = FAAC_INPUT_16BIT;  //��������PCM��ʽ
		faacEncSetConfiguration(encoder, config);   //Ӧ������
		//FILE* AACFile = NULL;
		//AACFile = fopen("D:/ttttt.aac", "wb+");
		int timestamp = 0;
		while (1)
		{
			lyAudioItem audioitem;
			if (m_audioProcess->popAudioInput(audioitem)
				== 0)
			{
				unsigned char* aacData = new unsigned char[maxOutputBytes];   //�����������ݣ�Ҳ����AAC���ݣ����λ��

			   //��ʼ���룬encoderΪ�����������mPCMBufferΪPCM���ݣ�inputSampleΪ�򿪱�����ʱ�õ���������������
			   //aacDataΪ��������ݴ��λ�ã�maxOutputBytesΪ������������ֽ�����retΪ��������ݳ���
				int ret = faacEncEncode(encoder, (int32_t *)audioitem.audiobuffer, inputSample,
					aacData, maxOutputBytes);
				//retΪ0ʱ���������ʧ�ܣ����Ǳ����ٶȽ��������»��滹δ��ȫflush������һ��ѭ���������ñ���ӿڣ��� ret>0 ʱ��ʾ����ɹ����ҷ���ֵΪ��������ݳ���
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
						/*�ͷ�ϵͳ�ڴ�*/
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

					/*����sps pps*/
					rtmpusher->sendSpsAndPps(sps,sps_len,pps,pps_len);

				}
				else {

					/*������ͨ֡*/
					rtmpusher->sendVideoData(pNals[i].p_payload, pNals[i].i_payload, GetCurrentTimeMsec());
				}
				last += pNals[i].i_payload;
				//timestamp += 1;
			}
			//����������ô��
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