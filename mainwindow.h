#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QScreen>
#include <QTimer>

#include <serialport.h>

#include <modules/uj20_unbrick.h>
#include <modules/uj30_unbrick.h>

#define STATUS_SUCCESS	0x00
#define STATUS_ERROR	0x01

#define SLEEP_BYTE_INTERVAL 0
#define SLEEP_CMD_INTERVAL 50

#define M16C_BL_CMD_CODE_9600                  0xB0
#define M16C_BL_CMD_CODE_19200                 0xB1
#define M16C_BL_CMD_CODE_38400                 0xB2
#define M16C_BL_CMD_CODE_57600                 0xB3
#define M16C_BL_CMD_CODE_115200                0xB4
#define M16C_BL_CMD_CODE_PAGE_READ             0xFF
#define M16C_BL_CMD_CODE_PAGE_PROGRAM          0x41
#define M16C_BL_CMD_CODE_BLOCK_ERASE           0x20
#define M16C_BL_CMD_CODE_ERASE_ALL_UNLOCKED    0xA7
#define M16C_BL_CMD_CODE_READ_STATUS_REG       0x70
#define M16C_BL_CMD_CODE_CLEAR_STATUS_REG      0x50
#define M16C_BL_CMD_CODE_READ_LOCK_BIT_STATUS  0x71
#define M16C_BL_CMD_CODE_LOCK_BIT_PROGRAM      0x77
#define M16C_BL_CMD_CODE_LOCK_BIT_ENABLE       0x7A
#define M16C_BL_CMD_CODE_LOCK_BIT_DISABLE      0x75
#define M16C_BL_CMD_CODE_ID_CHECK              0xF5
#define M16C_BL_CMD_CODE_DOWNLOAD              0xFA
#define M16C_BL_CMD_CODE_VERSION               0xFB
#define M16C_BL_CMD_CODE_BOOT_AREA_OUTPUT      0xFC

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QString title = "FastECU M32R Flasher";
    QString version = "0.0-dev0";

    QString get_selected_baudrate();

private:
    uint16_t serialport_check_timer_timeout = 1000;
    uint16_t serialport_open_timer_timeout = 1000;

    QString bl_version;
    QString bl_status;
    QString bl_id;

    QString prev_serialport;
    QStringList serial_ports;

    QTimer *serialport_check_timer;
    QTimer *serialport_open_timer;

    serialport *serial;

    Ui::MainWindow *ui;

    //int send_logwindow_message(QString message, bool timestamp, bool linefeed);
    QString parse_message_to_hex(QByteArray received);
    void delay(int timeout);

public slots:
    QString get_filename_to_write();
    void set_filename_to_write(QString filename);

    //QStringList get_serial_port_list();
    //void set_serial_port_list(QStringList serial_ports);

    QString get_serialport_combobox_text();

    QString get_baudrate_combobox_text();
    int get_baudrate_combobox_index();
    void set_baudrate_combobox_index(int index);

    QString get_parity_combobox_text();
    int get_parity_combobox_index();
    void set_parity_combobox_index(int index);

    void send_logwindow_message(QString message, bool timestamp, bool linefeed);
    void set_progressbar_value(int value);

private slots:
    int add_file_to_write();
    void check_serialports();

    bool open_serialport();
    bool writeFileToFlash();

    uint8_t calculate_checksum(QByteArray output, bool dec_0x100);

    QString requestSsmInit();
    QString requestVersion();
    QString requestVersionWithSync();
    QString requestStatus();
    QString requestId();

};
#endif // MAINWINDOW_H
