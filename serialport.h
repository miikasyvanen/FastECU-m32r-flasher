#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QCoreApplication>

#include <QDebug>
#include <QStringList>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTime>
#include <QWidget>

#define STATUS_SUCCESS	0x00
#define STATUS_ERROR	0x01

using namespace std;

class serialport : public QWidget
{
    Q_OBJECT

public:
    explicit serialport();
    ~serialport();

    QStringList check_serialports();
    bool is_serialport_open();
    bool write_data(QByteArray output, bool echo_check);
    QByteArray read_data();

private:
    bool is_serialport_available = false;
    bool port_ok = true;
    bool port_nok = false;

    QString serialport_prefix_linux = "/dev/";
    QString serialport_prefix_win = "";

    QString selected_serialport;
    QString opened_serialport;

    QString serialport_baudrate = "4800";

    QStringList serial_ports;

    QSerialPort *serial = new QSerialPort();

    void reset_connection();
    void delay(int timeout);

public slots:
    QString open_serialport(QString serial_port, QString baudrate, QString parity);
    QString close_serialport();

private slots:
    void handle_error(QSerialPort::SerialPortError error);

};

#endif // SERIALPORT_H
