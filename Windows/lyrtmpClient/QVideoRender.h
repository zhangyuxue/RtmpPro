#pragma once

#include <QWidget>
#include <QMutex>

class QVideoRender :
	public QWidget
{
public:
	QVideoRender(QWidget *parent = Q_NULLPTR);
	~QVideoRender();

public:
	virtual void videoRenderCallBack(int width, int height, unsigned char* datas);

protected:
	virtual void paintEvent(QPaintEvent *event);
private:
	unsigned char* m_picdatas=nullptr;
	QMutex m_rendermutex;
	int m_imgw=0;
	int m_imgh=0;
};

