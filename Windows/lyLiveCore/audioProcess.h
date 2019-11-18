#pragma once

#include <stdio.h>
#include <math.h>
#include "portaudio.h"
#include <vector>
#include <string>
#include <mutex>


#if PA_USE_ASIO
 #include "pa_asio.h"
#endif



struct lyAudioItem
{
	void* audiobuffer;
	int buffersize;
};

class lyAudioProcess
{
public:
	lyAudioProcess();
	~lyAudioProcess();
	int NUM_CHANNELS = 2;

	std::vector<std::string> getAudioInputDevs();
	std::vector<std::string> getAudioOutputDevs();

	int startAudioCaputre();

	void pushAudioInput(void* inputBuffer, int buffersize);
	int popAudioInput(lyAudioItem& item);

private:
	PaStreamParameters  inputParameters,outputParameters;
	PaStream* stream;
	PaError err = paNoError;
	std::vector<lyAudioItem> m_inputBufferQu;
	int m_audioframesize;
	std::mutex m_audioOperater;
};

