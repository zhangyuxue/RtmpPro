#include "QVideoRender.h"
#include <QPainter>

QVideoRender::QVideoRender(QWidget *parent)
	: QWidget(parent)
{
}


QVideoRender::~QVideoRender()
{
}

void QVideoRender::videoRenderCallBack(int width, int height, unsigned char* datas)
{
	m_rendermutex.lock();
	if (m_picdatas == nullptr)
	{
		m_picdatas =(unsigned char*)malloc(width * height * 3);
	}
	m_imgw = width;
	m_imgh = height;
	memcpy(m_picdatas, datas, width * height * 3);
	m_rendermutex.unlock();
	this->update();
}

void QVideoRender::paintEvent(QPaintEvent *event)
{
	m_rendermutex.lock();
	QImage img(m_picdatas,m_imgw, m_imgh, QImage::Format_RGB888);
	
	QPainter p(this);
	p.drawImage(0, 0, img);
	m_rendermutex.unlock();
}