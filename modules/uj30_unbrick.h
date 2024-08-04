#ifndef UJ30_UNBRICK_H
#define UJ30_UNBRICK_H

#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>
#include <QWidget>

#include <serialport.h>

#define STATUS_SUCCESS	0x00
#define STATUS_ERROR	0x01

class UJ30_Unbrick : public QWidget
{
    Q_OBJECT

public:
    explicit UJ30_Unbrick(QWidget *parent = nullptr);
    ~UJ30_Unbrick();

    bool sendKernelToRam();
    bool sendFileToFlash();
    bool unbrick_uj30();

    uint8_t calculate_checksum(QByteArray output, bool dec_0x100);
    QString parse_message_to_hex(QByteArray received);
    void delay(int timeout);

private:
    QString prev_serialport;
    QStringList serial_ports;

    serialport *serial;

    bool open_serialport();

signals:
    QString get_filename_to_write();
    void set_filename_to_write(QString filename);

    QString get_serialport_combobox_text();
    void set_serialport_combobox(QStringList);

    QString get_baudrate_combobox_text();
    int get_baudrate_combobox_index();
    void set_baudrate_combobox_index(int);
    QString get_parity_combobox_text();
    int get_parity_combobox_index();
    void set_parity_combobox_index(int);
    void send_logwindow_message(QString, bool, bool);
    void set_progressbar_value(int);

public slots:

};

#endif // UJ30_UNBRICK_H
