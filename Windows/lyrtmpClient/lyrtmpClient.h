#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_lyrtmpClient.h"
#include "../lyLiveCore/lyCoreImp.h"

class lyrtmpClient : public QMainWindow, public lyLiveCoreVideoRenderImp
{
	Q_OBJECT

public:
	lyrtmpClient(QWidget *parent = Q_NULLPTR);

public slots:
	void on_pushButton_rtmp_clicked();
private:
	virtual void videoRenderCallBack(int width, int height, unsigned char* datas);
private:
	Ui::lyrtmpClientClass ui;
};
