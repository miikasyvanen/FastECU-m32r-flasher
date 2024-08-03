#include "serialport.h"

serialport::serialport() {

}

serialport::~serialport() {

}

QStringList serialport::check_serialports()
{
    const auto serialPortsInfo = QSerialPortInfo::availablePorts();
    serial_ports.clear();

    is_serialport_available = false;

    for (const QSerialPortInfo &serialPortInfo : serialPortsInfo){
        serial_ports.append(serialPortInfo.portName() + " - " + serialPortInfo.description());
    }

    sort(serial_ports.begin(), serial_ports.end(), less<QString>());
    //qDebug() << "Serial ports: " << serial_ports;

    return serial_ports;
}

QString serialport::open_serialport(QString serial_port, QString baudrate, QString parity)
{
#ifdef Q_OS_LINUX
    serial_port.insert(0, serialport_prefix_linux);
#endif
#if defined(_WIN32) || defined(WIN32) || defined (_WIN64) || defined (WIN64)
    serial_port = serialport_prefix_win + serial_port;
#endif

    qDebug() << "Opening serial port '" + serial_port + "' with baudrate '" + baudrate + "', please wait...";
    if (!(serial->isOpen() && serial->isWritable())){
        serial->setPortName(serial_port);
        serial->setBaudRate(baudrate.toDouble());
        serial->setDataBits(QSerialPort::Data8);
        serial->setStopBits(QSerialPort::OneStop);
        if (parity == "No parity")
            serial->setParity(QSerialPort::NoParity);
        if (parity == "Even parity")
            serial->setParity(QSerialPort::EvenParity);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (serial->open(QIODevice::ReadWrite)){
            serial->clearError();
            serial->clear();
            serial->flush();
            opened_serialport = serial_port;
            //connect(serial, SIGNAL(readyRead()), this, SLOT(read_data()), Qt::DirectConnection);
            qRegisterMetaType<QSerialPort::SerialPortError>();
            connect(serial, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(handle_error(QSerialPort::SerialPortError)));

            //send_log_window_message("Serial port '" + serialPort + "' is open at baudrate " + serialPortBaudRate, true, true);
            qDebug() << "Serial port '" + serial_port + "' is open at baudrate " + serialport_baudrate;
            return opened_serialport;
        }
        else
        {
            //SendLogWindowMessage("Couldn't open serial port '" + serialPort + "'", true, true);
            qDebug() << "Couldn't open serial port '" + serial_port + "'";
            opened_serialport.clear();
            return opened_serialport;
        }

    }
    else{
        //SendLogWindowMessage("Serial port '" + serialPort + "' is already opened", true, true);
        qDebug() << "Serial port '" + serial_port + "' is already opened";
        return opened_serialport;
    }

    //return openedSerialPort;
    return opened_serialport;
}

QString serialport::close_serialport()
{
    if (serial->isOpen())
    {
        qDebug() << "Closing serial port '" + opened_serialport + "'";
        serial->close();
        qDebug() << "Serial port '" + opened_serialport + "' closed!";
        opened_serialport.clear();
    }
    return opened_serialport;
}

void serialport::reset_connection()
{
    close_serialport();
}

bool serialport::is_serialport_open()
{
    return serial->isOpen();
}

bool serialport::write_data(QByteArray output, bool echo_check)
{
    QByteArray msg;
    QByteArray received;

    received.clear();
    if (is_serialport_open())
    {
        for (int i = 0; i < output.length(); i++)
        {
            msg[0] = output.at(i);
            serial->write(msg, 1);
            // Add serial echo read during transmit to speed up a little
            if (serial->bytesAvailable() && echo_check)
                received.append(serial->read(1));
        }
        QTime dieTime = QTime::currentTime().addMSecs(200);
        while (received.length() < output.length() && (QTime::currentTime() < dieTime) && echo_check)
        {
            if (serial->bytesAvailable())
            {
                dieTime = QTime::currentTime().addMSecs(200);
                received.append(serial->read(1));
            }
            QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
        }

        return STATUS_SUCCESS;
    }

    return STATUS_ERROR;
}

QByteArray serialport::read_data()
{
    QByteArray received;

    while (serial->bytesAvailable())
    {
        //qDebug() << "Byte received...";
        received.append(serial->readAll());
        delay(1);
    }
    return received;
}

void serialport::delay(int timeout)
{
    QTime dieTime = QTime::currentTime().addMSecs(timeout);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

void serialport::handle_error(QSerialPort::SerialPortError error)
{
    //qDebug() << "Error:" << error;

    if (error == QSerialPort::NoError)
    {
    }
    else if (error == QSerialPort::DeviceNotFoundError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::PermissionError)
    {
    }
    else if (error == QSerialPort::OpenError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::NotOpenError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::WriteError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::ReadError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::ResourceError)
    {
        reset_connection();
    }
    else if (error == QSerialPort::UnsupportedOperationError)
    {
    }
    else if (error == QSerialPort::TimeoutError)
    {
        reset_connection();
        //qDebug() << "Timeout error";
        /*
        if (serial->isOpen())
            serial->flush();
        else
            serial->close();
        */
    }
    else if (error == QSerialPort::UnknownError)
    {
        reset_connection();
    }
}

