#include "audioProcess.h"

#ifdef WIN32
#include <windows.h>
#endif

/* #define SAMPLE_RATE  (17932) // Test failure to open with this value. */
#define SAMPLE_RATE  (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_SECONDS     (5)
/* #define DITHER_FLAG     (paDitherOff) */
#define DITHER_FLAG     (0) /**/
/** Set to 1 if you want to capture the recording to a file. */
#define WRITE_TO_FILE   (0)

/* Select sample format. */
#if 1
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

lyAudioProcess::lyAudioProcess()
{
	err = Pa_Initialize();
	if(err != paNoError)
	{
		printf("ERROR: Pa_Initialize returned 0x%x\n", err);
		Pa_Terminate();
		fprintf(stderr, "Error number: %d\n", err);
		fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
		return;
	}

	printf("PortAudio version: 0x%08X\n", Pa_GetVersion());
}


lyAudioProcess::~lyAudioProcess()
{
	Pa_Terminate();
}

std::vector<std::string> lyAudioProcess::getAudioInputDevs()
{
	//TODO 判断初始化是否成功
	std::vector<std::string> inputdevs;
	/*numDevices = Pa_GetDeviceCount();
     if (numDevices < 0)
     {
         printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
         err = numDevices;
		 return inputdevs;
     }

     printf("Number of devices = %d\n", numDevices);
     for (i = 0; i < numDevices; i++)
     {
         deviceInfo = Pa_GetDeviceInfo(i);
         printf("--------------------------------------- device #%d\n", i);
		 if (deviceInfo->maxInputChannels > 0)
		 {
			 inputdevs.push_back(deviceInfo->name);
		 }
     }*/

	return inputdevs;
}

std::vector<std::string> lyAudioProcess::getAudioOutputDevs()
{
	std::vector<std::string> outputdevs;
	/*numDevices = Pa_GetDeviceCount();
	if (numDevices < 0)
	{
		printf("ERROR: Pa_GetDeviceCount returned 0x%x\n", numDevices);
		err = numDevices;
		return outputdevs;
	}

	printf("Number of devices = %d\n", numDevices);
	for (i = 0; i < numDevices; i++)
	{
		deviceInfo = Pa_GetDeviceInfo(i);
		printf("--------------------------------------- device #%d\n", i);

		if (deviceInfo->maxOutputChannels > 0)
		{
			outputdevs.push_back(deviceInfo->name);
		}
	}*/
	return outputdevs;
}

static int recordCallback(const void *inputBuffer, void *outputBuffer,
	unsigned long framesPerBuffer,
	const PaStreamCallbackTimeInfo* timeInfo,
	PaStreamCallbackFlags statusFlags,
	void *userData)
{
	lyAudioProcess *data = (lyAudioProcess*)userData;
	{
		data->pushAudioInput(const_cast<void*>(inputBuffer), framesPerBuffer * sizeof(SAMPLE) * data->NUM_CHANNELS);
		//static FILE * f = fopen("D:\\portaudio.pcm", "w+b");
		//fwrite(inputBuffer, 1, framesPerBuffer * sizeof(SAMPLE) * data->NUM_CHANNELS, f);
		return paContinue;
	}
}

void lyAudioProcess::pushAudioInput(void* inputBuffer,int buffersize)
{
	std::lock_guard<std::mutex> lck(m_audioOperater);
	void* newinput = malloc(buffersize);
	memcpy(newinput,inputBuffer, buffersize);
	lyAudioItem item;
	item.audiobuffer = newinput;
	item.buffersize = buffersize;
	m_inputBufferQu.push_back(item);
}

int lyAudioProcess::popAudioInput(lyAudioItem& item)
{
	std::lock_guard<std::mutex> lck(m_audioOperater);
	if (m_inputBufferQu.size() > 0)
	{
		item = m_inputBufferQu.at(0);
		m_inputBufferQu.erase(m_inputBufferQu.begin());
		return 0;
	}
	return -1;
}

int lyAudioProcess::startAudioCaputre()
{
	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default input device.\n");
		return -1;
	}
	inputParameters.channelCount = Pa_GetDeviceInfo(inputParameters.device)->maxInputChannels;                    /* stereo input */
	NUM_CHANNELS = inputParameters.channelCount;
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
	inputParameters.hostApiSpecificStreamInfo = NULL;

	/* Record some audio. -------------------------------------------- */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,                  /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,      /* we won't output out of range samples so don't bother clipping them */
		recordCallback,
		this);
	if (err != paNoError) return -2;

	err = Pa_StartStream(stream);
	if (err != paNoError) return -3;
	printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);
	return 0;
}