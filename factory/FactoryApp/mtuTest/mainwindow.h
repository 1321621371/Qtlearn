#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QHostAddress>
#include <QCheckBox>
#include <QUdpSocket>
#include <QLabel>

//class QUdpSocket;
//class QCheckBox;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onBtnTestClicked(void);
    void onTestTimeout(void);
    void ProcessPendingDatagrams(void);
    void doNextTest(void);
    void onSnInputFinished();

    void on_allCheck_stateChanged(int arg1);

private:
    typedef enum {
        CMD_DIRECTION_GET = 0,
        CMD_DIRECTION_SET
    } CmdDirection_t;

    typedef struct {
        uint8_t index;
        const char *type;
        const char *cmd;
        int8_t item;
        const char *param;
        QCheckBox *cbox;
        QLabel *label;
        const char *msg;
    } UdpPacket_t;

    typedef enum {
        TEST_ITEM_FIRST = 0,
        TEST_ITEM_GET_MODE = 0,
        TEST_ITEM_SET_MODE,
        TEST_ITEM_SET_BSN,
        TEST_ITEM_GET_BSN,
        TEST_ITEM_GET_FWVER,
        TEST_ITEM_GET_RS232_1,
        TEST_ITEM_GET_RS485_1,
        TEST_ITEM_GET_RS485_2,
        TEST_ITEM_GET_RS485_3,
        TEST_ITEM_GET_RS485_4,
        TEST_ITEM_GET_RS485_5,
        TEST_ITEM_GET_ETH0,
        TEST_ITEM_GET_ETH1,
        TEST_ITEM_GET_ETH2,
        TEST_ITEM_GET_CAN0,
        TEST_ITEM_GET_CAN1,
        TEST_ITEM_GET_CAN2,
        TEST_ITEM_GET_SDCARD,
//        TEST_ITEM_GET_USBHOST,
        TEST_ITEM_SET_LCD,
        TEST_ITEM_SET_RTC,
        TEST_ITEM_GET_GPS,
        TEST_ITEM_GET_4G,
        TEST_ITEM_MAX,
    } TestIndex_t;

    const char *PACKET_HEAD = "*136";
    const char *PACKET_TAIL = "#";
    const char *PACKET_TYPE_GET = "GET";
    const char *PACKET_TYPE_SET = "SET";
    const char *PACKET_TYPE_RET = "RET";
    const char *DATA_SPLIT_TAG = " ";
    const char *PARAM_SPLIT_TAG = ";";
    const char *TIMEOUT_VAL_TAG = "TimeOut";

    Ui::MainWindow *ui;
    QUdpSocket *mUdpSocket;
    bool mIsOnTest = false;
    QHostAddress mRemoteAddr;
    quint16 mUdpPort;
    UdpPacket_t mTestItems[TEST_ITEM_MAX];
    const char* CMD_TYPE_GET = "GET";
    const char* CMD_TYPE_SET = "SET";
    int8_t mTestIndex = TEST_ITEM_FIRST;
    QTimer *mTimer;
    bool mBsnReady = false;

    void initTestItems(void);
    void startUdpComm(void);
    void stopUdpComm(void);
    void processInboundPacket(QByteArray& data);
    void clearMessage(void);
    void startTest(void);
    void stopTest(void);
    void sendPacket(TestIndex_t index);
    void setupNetwork(bool);
};
#endif // MAINWINDOW_H
