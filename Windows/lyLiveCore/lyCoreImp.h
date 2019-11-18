#pragma once

#include <Windows.h>
#include <string>
#include <vector>

#ifdef LY_LIVECORE_EXPORTS
#define LY_LIVECORE_API __declspec(dllexport)
#else
#define LY_LIVECORE_API __declspec(dllimport)
#endif


class lyLiveCoreEvent
{
public:
	virtual void CoreEventBack(int code,std::wstring msg) = 0;
};

class lyLiveCoreVideoRenderImp
{
public:
	virtual void videoRenderCallBack(int width,int height, unsigned char* datas)=0;
};

class lyLiveCoreImp
{
public:
	/*
	*	初始化参数，主要是com接口调用mediaofoundation
	*/
	virtual BOOL InitializeApp(HWND videohand) = 0;

	/*
	*
	*/
	virtual std::vector<std::string> GetCaptureList()=0;
	virtual int startCapture(int index, std::string capturename) = 0;
	virtual int stopCapture() = 0;
	virtual int getImageSize(int& width, int& height)=0;
	virtual int getRGBPixels(unsigned char* _dataBuffer)=0;
	virtual int setVideoRenderCallBack(lyLiveCoreVideoRenderImp* rendcall)=0;
	/*
	
	*/
	virtual std::vector<std::string> getAudioInputDevs()=0;
	virtual std::vector<std::string> getAudioOutputDevs()=0;

	/*
	*/
	virtual int startLive(std::string url,int mode)=0;
	virtual int stopLive(std::string url)=0;
	/************************************************************************/
	/* 函数功能：                  
	/* 返 回 值：                  0 : 成功
	/************************************************************************/
};

LY_LIVECORE_API lyLiveCoreImp* GetLiveCoreImp();