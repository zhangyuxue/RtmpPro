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
	//TODO ����index��capturename���±��������ж��Ƿ��豸�����仯
	m_devID = index; // ��Χ��0 ~ numDevices - 1 ѡ���豸
	m_camera.setupDevice(m_devID, 1280, 720);// ��������豸
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
	//TODO �ǵüӻ�����
	if(m_camera.isFrameNew(m_devID))
	{
		m_camera.getPixels(m_devID, _dataBuffer, true, true);
		return 0;
	}
	return -1;
}