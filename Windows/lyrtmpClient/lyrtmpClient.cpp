#include "lyrtmpClient.h"

lyrtmpClient::lyrtmpClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	this->setWindowTitle(tr("鲤鱼Rtmp客户端"));

	GetLiveCoreImp()->setVideoRenderCallBack(this);
}

void lyrtmpClient::videoRenderCallBack(int width, int height, unsigned char* datas)
{
	ui.widget_panel->videoRenderCallBack(width,height,datas);
}

void lyrtmpClient::on_pushButton_rtmp_clicked()
{
	QString rtmpurl = ui.lineEdit_url->text();
	GetLiveCoreImp()->startCapture(0, "");
	GetLiveCoreImp()->startLive(rtmpurl.toStdString().c_str(), 0);
}