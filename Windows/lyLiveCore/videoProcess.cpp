//////////////////////////////////////////////////////////////////////////
//
// preview.cpp : Preview helper class.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "videoProcess.h"


std::vector<std::string> lyVideoProcess::getCaptureList()
{
	std::vector<std::string> DevList;
	int DevicesNum = m_camera.listDevices();
	for (int i = 0; i < DevicesNum; i++)
	{
		DevList.push_back(m_camera.getDeviceName(i));
	}
	return DevList;
}

int lyVideoProcess::startCapture(int index, std::string capturename)
{
	//TODO 根据index与capturename重新遍历，以判断是否设备发生变化
	m_devID = index; // 范围：0 ~ numDevices - 1 选择设备
	m_camera.setupDevice(m_devID, 1280, 720);// 开启相关设备
	if (!m_camera.isDeviceSetup(m_devID))
		return -1;
	int width = m_camera.getWidth(m_devID);
	int height = m_camera.getHeight(m_devID);
	int imgSize = m_camera.getSize(m_devID);
	return 0;
}

int lyVideoProcess::stopCapture()
{
	m_camera.stopDevice(m_devID);
	m_devID = -1;
	return 0;
}

int lyVideoProcess::getImageSize(int& width, int& height)
{
	int localwidth = m_camera.getWidth(m_devID);
	int localheight = m_camera.getHeight(m_devID);
	width = localwidth;
	height = localheight;

	int localimgSize = m_camera.getSize(m_devID);
	return localimgSize;
	return 0;
}

int lyVideoProcess::getRGBPixels(unsigned char* _dataBuffer)
{
	//TODO 记得加互斥锁
	if(m_camera.isFrameNew(m_devID))
	{
		m_camera.getPixels(m_devID, _dataBuffer, true, true);
		return 0;
	}
	return -1;
}