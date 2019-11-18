//////////////////////////////////////////////////////////////////////////
//
// preview.h : Preview helper class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "videoInput.h"
#include <vector>
#include <string>

class lyVideoProcess
{
public:
	lyVideoProcess() {}

	std::vector<std::string> getCaptureList();

	int startCapture(int index,std::string capturename);
	int stopCapture();
	int getImageSize(int& width, int& height);
	int getRGBPixels(unsigned char* _dataBuffer);
	
private:
	videoInput m_camera;
	int m_devID = -1;
};