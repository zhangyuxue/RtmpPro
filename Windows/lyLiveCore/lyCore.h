#pragma once

#include "lyCoreImp.h"
#include <mutex>

class lyLiveCore : public lyLiveCoreImp
{
public:
	static lyLiveCore* getInstance() {
		if (m_selfPtr == nullptr)
		{
			m_selfPtr = new lyLiveCore;
		}
		return m_selfPtr;
	} ;

public:
	virtual BOOL InitializeApp(HWND videohand);
	//Video
	virtual std::vector<std::string> GetCaptureList();
	virtual int startCapture(int index, std::string capturename);
	virtual int stopCapture();
	virtual int getImageSize(int& width, int& height);
	virtual int getRGBPixels(unsigned char* _dataBuffer);
	virtual int setVideoRenderCallBack(lyLiveCoreVideoRenderImp* rendcall);

	//Audio
	virtual std::vector<std::string> getAudioInputDevs();
	virtual std::vector<std::string> getAudioOutputDevs();

	virtual int startLive(std::string url, int mode);
	virtual int stopLive(std::string url);


private:
	lyLiveCore();
	~lyLiveCore();

	static lyLiveCore* m_selfPtr;

	std::mutex m_videoMutex;
	lyLiveCoreVideoRenderImp* m_videoRender=nullptr;
	class lyVideoProcess* m_videoProcess;
	class lyAudioProcess* m_audioProcess;
};

