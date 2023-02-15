#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QUdpSocket>
#include <QThread>
#include <QTimer>
#include <QDateTime>
#include <QRegExpValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	ui->lineEdit_sn->setAttribute(Qt::WA_InputMethodEnabled, false);
	QRegExp regx("^[A-Za-z0-9_-]+$");
	QValidator *validator = new QRegExpValidator(regx, ui->lineEdit_sn);
	ui->lineEdit_sn->setValidator(validator);

    mUdpSocket = NULL;
    mUdpPort = 60008;

    mTimer = new QTimer(this);
    connect(mTimer, SIGNAL(timeout()), SLOT(onTestTimeout()));
    mTimer->setSingleShot(true);

    initTestItems();

    connect(ui->pushButton_test, SIGNAL(clicked()),
                    this, SLOT(onBtnTestClicked()));

	connect(ui->lineEdit_sn, SIGNAL(returnPressed()),
			this, SLOT(onSnInputFinished()));
}

MainWindow::~MainWindow()
{
    delete mTimer;
    mTimer = NULL;
    stopUdpComm();
    delete ui;
}

void MainWindow::initTestItems(void)
{
    mTestItems[TEST_ITEM_GET_MODE].index = TEST_ITEM_GET_MODE;
    mTestItems[TEST_ITEM_GET_MODE].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_MODE].cmd = "Mode";
    mTestItems[TEST_ITEM_GET_MODE].item = -1;
    mTestItems[TEST_ITEM_GET_MODE].param = "TimeOut:3";
    mTestItems[TEST_ITEM_GET_MODE].cbox = NULL;
    mTestItems[TEST_ITEM_GET_MODE].label = NULL;
    mTestItems[TEST_ITEM_GET_MODE].msg = "查询板卡状态";

    mTestItems[TEST_ITEM_SET_MODE].index = TEST_ITEM_SET_MODE;
    mTestItems[TEST_ITEM_SET_MODE].type = CMD_TYPE_SET;
    mTestItems[TEST_ITEM_SET_MODE].cmd = "Mode";
    mTestItems[TEST_ITEM_SET_MODE].item = -1;
    mTestItems[TEST_ITEM_SET_MODE].param = "Test;TimeOut:3";
    mTestItems[TEST_ITEM_SET_MODE].cbox = NULL;
    mTestItems[TEST_ITEM_SET_MODE].label = NULL;
    mTestItems[TEST_ITEM_SET_MODE].msg = "设置测试模式";

    mTestItems[TEST_ITEM_SET_BSN].index = TEST_ITEM_SET_BSN;
    mTestItems[TEST_ITEM_SET_BSN].type = CMD_TYPE_SET;
    mTestItems[TEST_ITEM_SET_BSN].cmd = "BSN";
    mTestItems[TEST_ITEM_SET_BSN].item = 1;
    mTestItems[TEST_ITEM_SET_BSN].param = "TimeOut:5";
    mTestItems[TEST_ITEM_SET_BSN].cbox = ui->checkBox_setBsn;
    mTestItems[TEST_ITEM_SET_BSN].label = ui->label_resultSetBsn;
    mTestItems[TEST_ITEM_SET_BSN].msg = "写入板卡SN";

    mTestItems[TEST_ITEM_GET_BSN].index = TEST_ITEM_GET_BSN;
    mTestItems[TEST_ITEM_GET_BSN].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_BSN].cmd = "BSN";
    mTestItems[TEST_ITEM_GET_BSN].item = 1;
    mTestItems[TEST_ITEM_GET_BSN].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_BSN].cbox = ui->checkBox_getBsn;
    mTestItems[TEST_ITEM_GET_BSN].label = ui->label_resultGetBsn;
    mTestItems[TEST_ITEM_GET_BSN].msg = "读取板卡SN";

    mTestItems[TEST_ITEM_GET_FWVER].index = TEST_ITEM_GET_FWVER;
    mTestItems[TEST_ITEM_GET_FWVER].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_FWVER].cmd = "FsVer";
    mTestItems[TEST_ITEM_GET_FWVER].item = -1;
    mTestItems[TEST_ITEM_GET_FWVER].param = "TimeOut:3";
    mTestItems[TEST_ITEM_GET_FWVER].cbox = ui->checkBox_itemFwVer;
    mTestItems[TEST_ITEM_GET_FWVER].label = ui->label_resultFwVer;
    mTestItems[TEST_ITEM_GET_FWVER].msg = "查询系统版本";

    mTestItems[TEST_ITEM_GET_RS232_1].index = TEST_ITEM_GET_RS232_1;
    mTestItems[TEST_ITEM_GET_RS232_1].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS232_1].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS232_1].item = 1;
    mTestItems[TEST_ITEM_GET_RS232_1].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS232_1].cbox = ui->checkBox_itemRs232_1;
    mTestItems[TEST_ITEM_GET_RS232_1].label = ui->label_resultRs232_1;
    mTestItems[TEST_ITEM_GET_RS232_1].msg = "测试RS232-1";


    mTestItems[TEST_ITEM_GET_RS485_1].index = TEST_ITEM_GET_RS485_1;
    mTestItems[TEST_ITEM_GET_RS485_1].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS485_1].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS485_1].item = 2;
    mTestItems[TEST_ITEM_GET_RS485_1].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS485_1].cbox = ui->checkBox_itemRs485_1;
    mTestItems[TEST_ITEM_GET_RS485_1].label = ui->label_resultRs485_5;
    mTestItems[TEST_ITEM_GET_RS485_1].msg = "测试RS485-1";

    mTestItems[TEST_ITEM_GET_RS485_2].index = TEST_ITEM_GET_RS485_2;
    mTestItems[TEST_ITEM_GET_RS485_2].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS485_2].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS485_2].item = 3;
    mTestItems[TEST_ITEM_GET_RS485_2].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS485_2].cbox = ui->checkBox_itemRs485_2;
    mTestItems[TEST_ITEM_GET_RS485_2].label = ui->label_resultRs485_6;
    mTestItems[TEST_ITEM_GET_RS485_2].msg = "测试RS485-2";

    mTestItems[TEST_ITEM_GET_RS485_3].index = TEST_ITEM_GET_RS485_3;
    mTestItems[TEST_ITEM_GET_RS485_3].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS485_3].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS485_3].item = 4;
    mTestItems[TEST_ITEM_GET_RS485_3].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS485_3].cbox = ui->checkBox_itemRs485_7;
    mTestItems[TEST_ITEM_GET_RS485_3].label = ui->label_resultRs485_7;
    mTestItems[TEST_ITEM_GET_RS485_3].msg = "测试RS485-3";

    mTestItems[TEST_ITEM_GET_RS485_4].index = TEST_ITEM_GET_RS485_4;
    mTestItems[TEST_ITEM_GET_RS485_4].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS485_4].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS485_4].item = 5;
    mTestItems[TEST_ITEM_GET_RS485_4].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS485_4].cbox = ui->checkBox_itemRs485_5;
    mTestItems[TEST_ITEM_GET_RS485_4].label = ui->label_resultRs485_8;
    mTestItems[TEST_ITEM_GET_RS485_4].msg = "测试RS485-4";

    mTestItems[TEST_ITEM_GET_RS485_5].index = TEST_ITEM_GET_RS485_5;
    mTestItems[TEST_ITEM_GET_RS485_5].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_RS485_5].cmd = "COM";
    mTestItems[TEST_ITEM_GET_RS485_5].item = 6;
    mTestItems[TEST_ITEM_GET_RS485_5].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_RS485_5].cbox = ui->checkBox_itemRs485_6;
    mTestItems[TEST_ITEM_GET_RS485_5].label = ui->label_resultRs485_9;
    mTestItems[TEST_ITEM_GET_RS485_5].msg = "测试RS485-5";

    mTestItems[TEST_ITEM_GET_ETH0].index = TEST_ITEM_GET_ETH0;
    mTestItems[TEST_ITEM_GET_ETH0].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_ETH0].cmd = "NET";
    mTestItems[TEST_ITEM_GET_ETH0].item = 0;
    mTestItems[TEST_ITEM_GET_ETH0].param = "type:ping;TimeOut:5";
    mTestItems[TEST_ITEM_GET_ETH0].cbox = ui->checkBox_itemEth0;
    mTestItems[TEST_ITEM_GET_ETH0].label = ui->label_resultEth0;
    mTestItems[TEST_ITEM_GET_ETH0].msg = "测试以太网-1";

    mTestItems[TEST_ITEM_GET_ETH1].index = TEST_ITEM_GET_ETH1;
    mTestItems[TEST_ITEM_GET_ETH1].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_ETH1].cmd = "NET";
    mTestItems[TEST_ITEM_GET_ETH1].item = 1;
    mTestItems[TEST_ITEM_GET_ETH1].param = "type:ping;TimeOut:12";
    mTestItems[TEST_ITEM_GET_ETH1].cbox = ui->checkBox_itemEth1;
    mTestItems[TEST_ITEM_GET_ETH1].label = ui->label_resultEth1;
    mTestItems[TEST_ITEM_GET_ETH1].msg = "测试以太网-2";

    mTestItems[TEST_ITEM_GET_ETH2].index = TEST_ITEM_GET_ETH2;
    mTestItems[TEST_ITEM_GET_ETH2].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_ETH2].cmd = "NET";
    mTestItems[TEST_ITEM_GET_ETH2].item = 2;
    mTestItems[TEST_ITEM_GET_ETH2].param = "type:ping;TimeOut:12";
    mTestItems[TEST_ITEM_GET_ETH2].cbox = ui->checkBox_itemEth2;
    mTestItems[TEST_ITEM_GET_ETH2].label = ui->label_resultEth2;
    mTestItems[TEST_ITEM_GET_ETH2].msg = "测试以太网-3";


    mTestItems[TEST_ITEM_GET_CAN0].index = TEST_ITEM_GET_CAN0;
    mTestItems[TEST_ITEM_GET_CAN0].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_CAN0].cmd = "CAN";
    mTestItems[TEST_ITEM_GET_CAN0].item = 0;
    mTestItems[TEST_ITEM_GET_CAN0].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_CAN0].cbox = ui->checkBox_itemCan0;
    mTestItems[TEST_ITEM_GET_CAN0].label = ui->label_resultCan0;
    mTestItems[TEST_ITEM_GET_CAN0].msg = "测试CAN-0";


    mTestItems[TEST_ITEM_GET_CAN1].index = TEST_ITEM_GET_CAN1;
    mTestItems[TEST_ITEM_GET_CAN1].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_CAN1].cmd = "CAN";
    mTestItems[TEST_ITEM_GET_CAN1].item = 1;
    mTestItems[TEST_ITEM_GET_CAN1].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_CAN1].cbox = ui->checkBox_itemCan1;
    mTestItems[TEST_ITEM_GET_CAN1].label = ui->label_resultCan1;
    mTestItems[TEST_ITEM_GET_CAN1].msg = "测试CAN-1";

    mTestItems[TEST_ITEM_GET_CAN2].index = TEST_ITEM_GET_CAN2;
    mTestItems[TEST_ITEM_GET_CAN2].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_CAN2].cmd = "CAN";
    mTestItems[TEST_ITEM_GET_CAN2].item = 2;
    mTestItems[TEST_ITEM_GET_CAN2].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_CAN2].cbox = ui->checkBox_itemCan2;
    mTestItems[TEST_ITEM_GET_CAN2].label = ui->label_resultCan2;
    mTestItems[TEST_ITEM_GET_CAN2].msg = "测试CAN-2";


    mTestItems[TEST_ITEM_GET_SDCARD].index = TEST_ITEM_GET_SDCARD;
    mTestItems[TEST_ITEM_GET_SDCARD].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_SDCARD].cmd = "FLASH";
    mTestItems[TEST_ITEM_GET_SDCARD].item = 1;
    mTestItems[TEST_ITEM_GET_SDCARD].param = "status:rw;TimeOut:5";
    mTestItems[TEST_ITEM_GET_SDCARD].cbox = ui->checkBox_itemSdcard;
    mTestItems[TEST_ITEM_GET_SDCARD].label = ui->label_resultSdcard;
    mTestItems[TEST_ITEM_GET_SDCARD].msg = "测试SD卡";

//    mTestItems[TEST_ITEM_GET_USBHOST].index = TEST_ITEM_GET_USBHOST;
//    mTestItems[TEST_ITEM_GET_USBHOST].type = CMD_TYPE_GET;
//    mTestItems[TEST_ITEM_GET_USBHOST].cmd = "USB";
//    mTestItems[TEST_ITEM_GET_USBHOST].item = 1;
//    mTestItems[TEST_ITEM_GET_USBHOST].param = "status:rw;TimeOut:5";
//    mTestItems[TEST_ITEM_GET_USBHOST].cbox = ui->checkBox_itemUsbHost;
//    mTestItems[TEST_ITEM_GET_USBHOST].label = ui->label_resultUsbHost;
//    mTestItems[TEST_ITEM_GET_USBHOST].msg = "测试USB口";

    mTestItems[TEST_ITEM_SET_LCD].index = TEST_ITEM_SET_LCD;
    mTestItems[TEST_ITEM_SET_LCD].type = CMD_TYPE_SET;
    mTestItems[TEST_ITEM_SET_LCD].cmd = "LCD";
    mTestItems[TEST_ITEM_SET_LCD].item = 0;
    mTestItems[TEST_ITEM_SET_LCD].param = "TimeOut:5";
    mTestItems[TEST_ITEM_SET_LCD].cbox = ui->checkBox_itemLcd;
    mTestItems[TEST_ITEM_SET_LCD].label = ui->label_resultLcd;
    mTestItems[TEST_ITEM_SET_LCD].msg = "测试LCD";


    mTestItems[TEST_ITEM_SET_RTC].index = TEST_ITEM_SET_RTC;
    mTestItems[TEST_ITEM_SET_RTC].type = CMD_TYPE_SET;
    mTestItems[TEST_ITEM_SET_RTC].cmd = "RTC";
    mTestItems[TEST_ITEM_SET_RTC].item = 0;
    mTestItems[TEST_ITEM_SET_RTC].param = "TimeOut:5";
    mTestItems[TEST_ITEM_SET_RTC].cbox = ui->checkBox_itemRtc;
    mTestItems[TEST_ITEM_SET_RTC].label = ui->label_resultRtc;
    mTestItems[TEST_ITEM_SET_RTC].msg = "测试RTC";

    mTestItems[TEST_ITEM_GET_GPS].index = TEST_ITEM_GET_GPS;
    mTestItems[TEST_ITEM_GET_GPS].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_GPS].cmd = "GPS";
    mTestItems[TEST_ITEM_GET_GPS].item = 0;
    mTestItems[TEST_ITEM_GET_GPS].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_GPS].cbox = ui->checkBox_itemGps;
    mTestItems[TEST_ITEM_GET_GPS].label = ui->label_resultGps;
    mTestItems[TEST_ITEM_GET_GPS].msg = "测试GPS";

    mTestItems[TEST_ITEM_GET_4G].index = TEST_ITEM_GET_4G;
    mTestItems[TEST_ITEM_GET_4G].type = CMD_TYPE_GET;
    mTestItems[TEST_ITEM_GET_4G].cmd = "4G";
    mTestItems[TEST_ITEM_GET_4G].item = 0;
    mTestItems[TEST_ITEM_GET_4G].param = "TimeOut:5";
    mTestItems[TEST_ITEM_GET_4G].cbox = ui->checkBox_item4G;
    mTestItems[TEST_ITEM_GET_4G].label = ui->label_result4G;
    mTestItems[TEST_ITEM_GET_4G].msg = "测试4G";
}

void MainWindow::clearMessage(void)
{
    ui->label_msgOut->clear();
}

void MainWindow::setupNetwork(bool connect)
{
    if (connect) {
        mRemoteAddr = QHostAddress(ui->lineEdit_mtuIp->text().toLocal8Bit().data());
        ui->lineEdit_mtuIp->setEnabled(false);
        startUdpComm();
        clearMessage();
    } else {
        stopUdpComm();
        ui->lineEdit_mtuIp->setEnabled(true);
    }
}

void MainWindow::onBtnTestClicked(void)
{
    qInfo() << "button test clicked";

    clearMessage();
    if (!mIsOnTest) {
        //start test
        setupNetwork(true);
        startTest();
    } else {
        //stop test
        stopTest();
        setupNetwork(false);
    }
}

void MainWindow::onSnInputFinished()
{
        QString text = ui->lineEdit_sn->text();
        qInfo() << "SN input: " << text;

        if (text.count() < ui->lineEdit_sn->maxLength()) {
                ui->label_msgOut->setStyleSheet("color:red;");
                ui->label_msgOut->setText("SN长度不符: " + text);
                ui->lineEdit_sn->clear();
                return;
        }

	ui->lineEdit_sn->setEnabled(false);
	mBsnReady = true;
	QTimer::singleShot(20, this, SLOT(doNextTest()));
}

void MainWindow::startTest(void)
{
        qInfo() << "start test";

       mIsOnTest = true;
       ui->pushButton_test->setText("停止测试");

       for (int8_t idx = TEST_ITEM_FIRST; idx < TEST_ITEM_MAX; idx++) {
           if (mTestItems[idx].cbox)
                mTestItems[idx].cbox->setEnabled(false);
       }

        mTestIndex = TEST_ITEM_FIRST;
        QTimer::singleShot(20, this, SLOT(doNextTest()));
}

void MainWindow::stopTest(void)
{
	qInfo() << "stop test";

	mTimer->stop();
	mIsOnTest = false;
	mBsnReady = false;
	ui->pushButton_test->setText("开始测试");

	for (int8_t idx = TEST_ITEM_FIRST; idx < TEST_ITEM_MAX; idx++) {
		if (mTestItems[idx].cbox)
			mTestItems[idx].cbox->setEnabled(true);
	}

	ui->lineEdit_sn->setEnabled(false);
}

void MainWindow::doNextTest(void)
{
    if (!mIsOnTest)
        return;

    if (mTestIndex == TEST_ITEM_MAX) {
        //rotate to the first
        mTestIndex = TEST_ITEM_FIRST;
        mBsnReady = false;
        QTimer::singleShot(3000, this, SLOT(doNextTest()));
        return;
    }

    if (mTestIndex == TEST_ITEM_FIRST) {
                //wait for SN input if set SN is selected
                if (mTestItems[TEST_ITEM_SET_BSN].cbox->isChecked() && !mBsnReady) {
                        ui->lineEdit_sn->clear();
                        ui->lineEdit_sn->setEnabled(true);
                        ui->lineEdit_sn->setFocus();
                        return;
                }

		//set all result texts to initial state
		for (int8_t idx = TEST_ITEM_FIRST; idx < TEST_ITEM_MAX; idx++) {
			if (mTestItems[idx].label) {
				mTestItems[idx].label->setStyleSheet("color:gray;");
				mTestItems[idx].label->setText("未测试");
			}
		}
    }

    while (mTestItems[mTestIndex].cbox && !mTestItems[mTestIndex].cbox->isChecked()) {
        //find test item selected
        mTestIndex++;
        if (mTestIndex == TEST_ITEM_MAX) {
            doNextTest();
            return;
        }
    }

    qInfo() << "start test " << QString::number(mTestIndex);

    ui->label_msgOut->setStyleSheet("color:black;");
    ui->label_msgOut->setText(mTestItems[mTestIndex].msg);

    if (mTestItems[mTestIndex].label) {
        mTestItems[mTestIndex].label->setStyleSheet("color:black;");
        mTestItems[mTestIndex].label->setText("测试中...");
    }

    sendPacket((TestIndex_t)mTestIndex);

	//check timeout
	if (mTestIndex <= TEST_ITEM_SET_MODE)
		mTimer->start(3000);
	else
		mTimer->start(15000);
}

void MainWindow::onTestTimeout(void)
{
    QString msgOut = QString(mTestItems[mTestIndex].msg) + ": TimeOut";

    ui->label_msgOut->setStyleSheet("color:red;");
    ui->label_msgOut->setText(msgOut);

    if (mTestIndex <= TEST_ITEM_SET_MODE) {
        mTestIndex = TEST_ITEM_FIRST;
    } else {
        if (mTestItems[mTestIndex].label) {
            mTestItems[mTestIndex].label->setStyleSheet("color:red;");
            mTestItems[mTestIndex].label->setText("测试失败: TimeOut");
        }

        mTestIndex++;
    }

    doNextTest();
}

/* UDP communication */
void MainWindow::startUdpComm(void)
{
    stopUdpComm();

    mUdpSocket = new QUdpSocket();

    connect(mUdpSocket, SIGNAL(readyRead()),
            this, SLOT(ProcessPendingDatagrams()));
}

void MainWindow::stopUdpComm(void)
{
    if (mUdpSocket == NULL)
        return;

    disconnect(mUdpSocket, SIGNAL(readyRead()),
            this, SLOT(ProcessPendingDatagrams()));

    mUdpSocket->close();
    delete mUdpSocket;
    mUdpSocket = NULL;
}

void MainWindow::ProcessPendingDatagrams(void)
{
    while (mUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mUdpSocket->pendingDatagramSize());
        mUdpSocket->readDatagram(datagram.data(), datagram.size());
        processInboundPacket(datagram);
    }
}

void MainWindow::processInboundPacket(QByteArray& data)
{
    QString packetString = QString(data);
    QStringList list = packetString.split(" ");

    qInfo() << "UDP receive: " << packetString;

    //check msg
    if (list[0] != PACKET_HEAD) {
        qWarning() << "Invalid header";
        return;
    }

    if (list.last() != PACKET_TAIL) {
        qWarning() << "Invalid tail";
        return;
    }

    //check cmd
    if (list[2] != mTestItems[mTestIndex].cmd) {
        qWarning() << "Invalid cmd " << list[2] <<
                      ", expect " << mTestItems[mTestIndex].cmd;
        return;
    }

    if (mTestItems[mTestIndex].item >= 0 && mTestItems[mTestIndex].item != list[3].toInt()) {
        qWarning() << "Invalid item " << list[3] <<
                      ", expect " << mTestItems[mTestIndex].item;
        return;
    }

    mTimer->stop();
    QString result = QString(list[list.size() - 2]);
    qInfo() << "Get result: " << result;

    QString msgOut = QString(mTestItems[mTestIndex].msg) + ": " + result;
    ui->label_msgOut->setStyleSheet("color:black;");
    ui->label_msgOut->setText(msgOut);

    if (mTestItems[mTestIndex].label) {
        if (result.contains("OK")) {
            mTestItems[mTestIndex].label->setStyleSheet("color:green;");
            msgOut = QString("测试通过：") + result;
        } else {
            mTestItems[mTestIndex].label->setStyleSheet("color:red;");
            msgOut = QString("测试失败：") + result;
        }

        mTestItems[mTestIndex].label->setText(msgOut);
    }

    mTestIndex++;
    doNextTest();
}

void MainWindow::sendPacket(TestIndex_t index)
{
    if (mUdpSocket == NULL)
        return;

    QByteArray msg;
    QDataStream s(&msg, QIODevice::WriteOnly);

    //head
    const char *header = "*136";
    s.writeRawData(header, strlen(header));

    //direction
    s.writeRawData(" ", 1);
    const char *type = mTestItems[index].type;
    s.writeRawData(type, strlen(type));

    //cmd
    s.writeRawData(" ", 1);
    const char *cmd = mTestItems[index].cmd;
    s.writeRawData(cmd, strlen(cmd));

    //item
    const char item = mTestItems[index].item;
    if (item >= 0) {
        s.writeRawData(" ", 1);
        char itemString[8] = {0};
        itoa(item, itemString, 10);
        s.writeRawData(itemString, strlen(itemString));
    }

    //param
    s.writeRawData(" ", 1);

    if (index == TEST_ITEM_SET_RTC) {
        //get current time
        QDateTime time = QDateTime::currentDateTime().toUTC();
        QString timeStr = time.toString("yyyy.MM.dd-hh.mm.ss") + ";";
        const char *timeParam = timeStr.toLocal8Bit().data();
        s.writeRawData(timeParam, strlen(timeParam));
    } else if (index == TEST_ITEM_SET_BSN) {
        //append bsn
        QString bsn = ui->lineEdit_sn->text() + ";";
        const char *bsnParam = bsn.toLocal8Bit().data();
        s.writeRawData(bsnParam, strlen(bsnParam));
    }

    const char *param = mTestItems[index].param;
    s.writeRawData(param, strlen(param));

    //tail
    s.writeRawData(" #", 3);

    qInfo() << "UDP send " + msg;

    mUdpSocket->writeDatagram(msg.data(), msg.size(), mRemoteAddr, mUdpPort);
}





void MainWindow::on_allCheck_stateChanged(int arg1)
{

        bool status = ui->allCheck->isChecked();
        qDebug()<<"status"<<status;
        if(status == true)
        {
            //qDebug()<<"表示被全选";
            for (int8_t idx = TEST_ITEM_FIRST; idx < TEST_ITEM_MAX; idx++) {
                if (mTestItems[idx].cbox)
                     mTestItems[idx].cbox->setChecked(true);
            }

        }
        else if(status == false)
        {
            //qDebug()<<"表示全不选";
            for (int8_t idx = TEST_ITEM_FIRST; idx < TEST_ITEM_MAX; idx++) {
                if (mTestItems[idx].cbox)
                     mTestItems[idx].cbox->setChecked(false);
            }
        }

}
