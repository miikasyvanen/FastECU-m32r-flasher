#include "uj20_unbrick.h"

UJ20_Unbrick::UJ20_Unbrick(QWidget *parent)
{
    serial = new serialport();

    connect(this, SIGNAL(get_filename_to_write()), parent, SLOT(get_filename_to_write()));
    connect(this, SIGNAL(set_filename_to_write(QString)), parent, SLOT(set_filename_to_write(QString)));
    connect(this, SIGNAL(get_serialport_combobox_text()), parent, SLOT(get_serialport_combobox_text()));
    connect(this, SIGNAL(get_baudrate_combobox_text()), parent, SLOT(get_baudrate_combobox_text()));
    connect(this, SIGNAL(get_baudrate_combobox_index()), parent, SLOT(get_baudrate_combobox_index()));
    connect(this, SIGNAL(set_baudrate_combobox_index(int)), parent, SLOT(set_baudrate_combobox_index(int)));
    connect(this, SIGNAL(get_parity_combobox_text()), parent, SLOT(get_parity_combobox_text()));
    connect(this, SIGNAL(get_parity_combobox_index()), parent, SLOT(get_parity_combobox_index()));
    connect(this, SIGNAL(set_parity_combobox_index(int)), parent, SLOT(set_parity_combobox_index(int)));
    connect(this, SIGNAL(send_logwindow_message(QString,bool,bool)), parent, SLOT(send_logwindow_message(QString,bool,bool)));
    connect(this, SIGNAL(set_progressbar_value(int)), parent, SLOT(set_progressbar_value(int)));
}

UJ20_Unbrick::~UJ20_Unbrick()
{

}

bool UJ20_Unbrick::sendKernelToRam()
{
    QByteArray output;
    QByteArray received;

    QString filename_kernel = "./kernels/uj20_kernel.bin";

    float progressBarValue = 0;

    int baudRateComboBoxIndex = emit get_baudrate_combobox_index();
    int parityComboBoxIndex = emit get_parity_combobox_index();

    emit set_baudrate_combobox_index(4);
    emit set_parity_combobox_index(1);
    serial->close_serialport();

    QFile file_kernel(filename_kernel);

    if (!file_kernel.open(QIODevice::ReadOnly ))
    {
        qDebug() << "Unable to open kernel file for reading";
        QMessageBox::warning(this, tr("Kernel file"), "Unable to open kernel file for reading");
        return STATUS_ERROR;
    }
    QByteArray kerneldata = file_kernel.readAll();
    file_kernel.close();

    int kernelfilesize = kerneldata.length();

    emit send_logwindow_message("Initialising serial port, please wait...", true, true);
    qDebug() << "Initialising serial port, please wait...";
    if(open_serialport())
    {
        emit send_logwindow_message("Serial port error!", true, true);
        qDebug() << "Serial port error!";
        return STATUS_ERROR;
    }
    else
    {
        emit send_logwindow_message("Uploading UJ20 kernel, please wait...", true, true);
        qDebug() << "Uploading kernel, please wait...";
        for (int i = 0; i < kernelfilesize; i++)
        {
            output.clear();
            output.append(kerneldata.at(i));
            serial->write_data(output, false);
            progressBarValue = (float)i / (float)kernelfilesize;
            emit set_progressbar_value(progressBarValue * 100.0f);
        }
        delay(500);
        received = serial->read_data();
        received.clear();
    }

    emit set_progressbar_value(0);

    //serial->close_serialport();

    emit set_baudrate_combobox_index(baudRateComboBoxIndex);
    emit set_parity_combobox_index(parityComboBoxIndex);

    return STATUS_SUCCESS;
}

bool UJ20_Unbrick::sendFileToFlash()
{
    QByteArray output;
    QByteArray received;
    QString filename_flash = emit get_filename_to_write();
    QDir dir("./");
    float progressBarValue = 0;
    uint32_t dataaddr = 0;

    int baudRateComboBoxIndex = emit get_baudrate_combobox_index();
    int parityComboBoxIndex = emit get_parity_combobox_index();

    QFile file_flash(filename_flash);

    if (filename_flash.isEmpty())
    {
        QFileDialog openDialog;
        openDialog.setDefaultSuffix("bin");
        filename_flash = QFileDialog::getOpenFileName(this, tr("Open ROM file"), "./", tr("Calibration file (*.bin *.hex)"));

        if (filename_flash.isEmpty()){
            qDebug() << "No ROM file selected!";
            QMessageBox::information(this, tr("ROM file"), "No ROM file selected!");
            return STATUS_ERROR;
        }
        filename_flash = "./" + dir.relativeFilePath(filename_flash);
        emit set_filename_to_write(filename_flash);
    }

    QMessageBox::information(this, tr("Flash file"), "Remove MOD1 voltage and press ok to continue.");

    emit set_baudrate_combobox_index(2);
    emit set_parity_combobox_index(0);
    serial->close_serialport();

    emit send_logwindow_message("Initialising serial port, please wait...", true, true);
    qDebug() << "Initialising serial port, please wait...";
    if(open_serialport())
    {
        emit send_logwindow_message("Serial port error!", true, true);
        qDebug() << "Serial port error!";
        serial->close_serialport();

        emit set_baudrate_combobox_index(baudRateComboBoxIndex);
        emit set_parity_combobox_index(parityComboBoxIndex);
        return STATUS_ERROR;
    }
    else
    {
        emit send_logwindow_message("Requesting flash erase, please wait...", true, true);
        qDebug() << "Requesting flash erase, please wait...";

        output.clear();
        output.append((uint8_t)0x80);
        output.append((uint8_t)0x10);
        output.append((uint8_t)0xF0);
        output.append((uint8_t)0x02);
        output.append((uint8_t)0xAF);
        output.append((uint8_t)0x31);
        output.append(calculate_checksum(output, false));
        serial->write_data(output, true);
        delay(500);

        emit send_logwindow_message("", true, false);
        received.clear();
        for (int i = 0; i < 20; i++)
        {
            received.append(serial->read_data());
            emit send_logwindow_message(".", false, false);
            qDebug() << ".";
            if (received != "")
            {
                if (received.length() > 6)
                {
                    if ((uint8_t)received.at(0) == 0x80 && (uint8_t)received.at(1) == 0xf0 && (uint8_t)received.at(2) == 0x10 && (uint8_t)received.at(3) == 0x02 && (uint8_t)received.at(4) == 0xEF && (uint8_t)received.at(5) == 0x42)
                    {
                        emit send_logwindow_message("", false, true);
                        emit send_logwindow_message("Flash erase in progress, please wait...", true, true);
                        qDebug() << "Flash erase in progress, please wait...";
                        break;
                    }
                    else
                    {
                        emit send_logwindow_message("", false, true);
                        emit send_logwindow_message("Flash erase cmd failed!", true, true);
                        qDebug() << "Flash erase cmd failed!";
                        emit send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
                        qDebug() << "Received: " + parse_message_to_hex(received);
                        return STATUS_ERROR;
                    }
                }
            }
            delay(500);
        }
        if (received == "")
        {
            emit send_logwindow_message("", false, true);
            emit send_logwindow_message("Flash erase cmd failed!", true, true);
            qDebug() << "Flash erase cmd failed!";
            emit send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
            qDebug() << "Received: " + parse_message_to_hex(received);
            serial->close_serialport();

            emit set_baudrate_combobox_index(baudRateComboBoxIndex);
            emit set_parity_combobox_index(parityComboBoxIndex);
            return STATUS_ERROR;
        }
    }
    emit send_logwindow_message("", true, false);
    received.clear();
    for (int i = 0; i < 20; i++)
    {
        received.append(serial->read_data());
        emit send_logwindow_message(".", false, false);
        qDebug() << ".";
        if (received != "")
        {
            if (received.length() > 6)
            {
                if ((uint8_t)received.at(0) == 0x80 && (uint8_t)received.at(1) == 0xf0 && (uint8_t)received.at(2) == 0x10 && (uint8_t)received.at(3) == 0x02 && (uint8_t)received.at(4) == 0xEF && (uint8_t)received.at(5) == 0x52)
                {
                    emit send_logwindow_message("", false, true);
                    emit send_logwindow_message("Flash erased!", true, true);
                    qDebug() << "Flash erased!";
                    break;
                }
                else
                {
                    emit send_logwindow_message("", false, true);
                    emit send_logwindow_message("Flash erase failed!", true, true);
                    qDebug() << "Flash erase failed!";
                    emit send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
                    qDebug() << "Received: " + parse_message_to_hex(received);
                    return STATUS_ERROR;
                }
            }
        }
        delay(1000);
    }

    delay(1000);

    if (!file_flash.open(QIODevice::ReadOnly ))
    {
        QMessageBox::warning(this, tr("Flash file"), "Unable to open flash file for reading");
        return STATUS_ERROR;
    }
    QByteArray flashdata = file_flash.readAll();
    file_flash.close();

    int flashfilesize = flashdata.length();
    dataaddr = 0;
    int blocksize = 128;
    int blocks = flashfilesize / blocksize;
    //int encrypt = 0x82;

    qDebug() << "Uploading file " + filename_flash + " to flash";
    emit send_logwindow_message("Uploading file " + filename_flash + " to flash, please wait...", true, true);

    for (int i = 0; i < blocks; i++)
    {
        output.clear();
        output.append((uint8_t)0x80);
        output.append((uint8_t)0x10);
        output.append((uint8_t)0xF0);
        output.append((uint8_t)0x85);
        output.append((uint8_t)0xAF);
        if (i < (blocks - 1))
            output.append((uint8_t)0x61);
        else
            output.append((uint8_t)0x69);
        output.append((uint8_t)(dataaddr >> 16) & 0xFF);
        output.append((uint8_t)(dataaddr >> 8) & 0xFF);
        output.append((uint8_t)dataaddr & 0xFF);
        for (int j = 0; j < blocksize; j++)
        {
            output.append((flashdata.at(i * blocksize + j)));// ^ encrypt));
        }
        output.append(calculate_checksum(output, false));

        serial->write_data(output, true);
        emit send_logwindow_message("Sent: " + parse_message_to_hex(output), true, true);
        qDebug() << "Sent: " + parse_message_to_hex(output);
        delay(5);
        received.clear();
        if (output.at(5) == 0x61)
        {
            for (int i = 0; i < 500; i++)
            {
                received.append(serial->read_data());
                if (received.length() > 6)
                {
                    if ((uint8_t)received.at(0) == 0x80 && (uint8_t)received.at(1) == 0xf0 && (uint8_t)received.at(2) == 0x10 && (uint8_t)received.at(3) == 0x02 && (uint8_t)received.at(4) == 0xEF && (uint8_t)received.at(5) == 0x52)
                    {
                        emit send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
                        qDebug() << "Received: " + parse_message_to_hex(received);
                        break;
                    }
                    else
                    {
                        emit send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
                        qDebug() << "Received: " + parse_message_to_hex(received);
                        emit send_logwindow_message("Block flash failed!", true, true);
                        qDebug() << "Block flash failed!";
                        return STATUS_ERROR;
                    }
                }
                delay(50);
            }
        }
        else
        {
            emit send_logwindow_message("File written to flash, please wait...", true, true);
            delay(5);
            emit send_logwindow_message("Done! Please remove VPP voltage, power cycle ECU and request SSM Init to .", true, true);
        }
        if ((uint8_t)output.at(5) != 0x69 && received == "")
        {
            emit send_logwindow_message("Flash failed!", true, true);
            qDebug() << "Flash failed!";
            return STATUS_ERROR;
        }
        dataaddr += blocksize;
        progressBarValue = (float)(i * blocksize) / (float)flashfilesize;
        set_progressbar_value(progressBarValue * 100.0f);
    }
    emit send_logwindow_message("File " + filename_flash + " written to flash.", true, true);
    qDebug() << "File" << filename_flash << "written to flash";

    emit set_progressbar_value(0);

    serial->close_serialport();

    emit set_baudrate_combobox_index(baudRateComboBoxIndex);
    emit set_parity_combobox_index(parityComboBoxIndex);

    return STATUS_SUCCESS;
}

bool UJ20_Unbrick::unbrick_uj20()
{
    int baudRateComboBoxIndex = emit get_baudrate_combobox_index();
    int parityComboBoxIndex = emit get_parity_combobox_index();

    if (sendKernelToRam())
    {
        emit send_logwindow_message("ERROR uploading kernel to RAM.", true, true);
        serial->close_serialport();
        return STATUS_ERROR;
    }
    if (sendFileToFlash())
    {
        emit send_logwindow_message("ERROR uploading file to FLASH.", true, true);
        serial->close_serialport();
        return STATUS_ERROR;
    }


    serial->close_serialport();

    emit set_baudrate_combobox_index(baudRateComboBoxIndex);
    emit set_parity_combobox_index(parityComboBoxIndex);

    return STATUS_SUCCESS;

}




bool UJ20_Unbrick::open_serialport()
{
    QStringList serial_port = emit get_serialport_combobox_text().split(" - ");
    //QComboBox *serial_port_list = ui->serialPortComboBox;
    QString baudrate = emit get_baudrate_combobox_text();
    QString parity = emit get_parity_combobox_text();
//    QString selected_serial_port = ui->serialPortComboBox->currentText();

    //serial_port.append(ui->serialPortComboBox->currentText().split(" - ").at(0));
    //serial_port.append(ui->serialPortComboBox->currentText().split(" - ").at(1));

    emit send_logwindow_message("Open serial port: " + serial_port.at(0) + " | baudrate: " + baudrate + " | parity: " + parity, true, true);
    qDebug() << "Baudrate: " + baudrate + " | Parity: " + parity;
    //if (serial_port_list)
    //{
        if (serial_port.length() > 0)
        {
            QString opened_serial_port = serial->open_serialport(serial_port.at(0), baudrate, parity);
            //QString opened_serial_port = serial->open_serialport(serial_port.at(0), baudrate, parity);
            if (opened_serial_port != "")
            {
                if (opened_serial_port != prev_serialport)
                {

                }
                emit send_logwindow_message("Serial port " + opened_serial_port + " ready!", true, true);
                qDebug() << "Serial port " + opened_serial_port + " ready!";
                prev_serialport = opened_serial_port;

                return STATUS_SUCCESS;
            }
            else
            {
                emit send_logwindow_message("Serial port " + opened_serial_port + " NOT ready!", true, true);
                qDebug() << "Serial port " + opened_serial_port + " NOT ready!";
                return STATUS_ERROR;
            }
        }
    //}
    else
    {
        emit send_logwindow_message("No serialports available!", true, true);
        qDebug() << "No serialports available!";
    }

    return STATUS_ERROR;
}

uint8_t UJ20_Unbrick::calculate_checksum(QByteArray output, bool dec_0x100)
{
    uint8_t checksum = 0;

    for (uint16_t i = 0; i < output.length(); i++)
        checksum += (uint8_t)output.at(i);

    if (dec_0x100)
        checksum = (uint8_t) (0x100 - checksum);

    return checksum;
}

QString UJ20_Unbrick::parse_message_to_hex(QByteArray received)
{
    QByteArray msg;

    for (int i = 0; i < received.length(); i++)
    {
        msg.append(QString("0x%1 ").arg((uint8_t)received.at(i),2,16,QLatin1Char('0')).toUtf8());
    }

    return msg;
}

void UJ20_Unbrick::delay(int timeout)
{
    QTime dieTime = QTime::currentTime().addMSecs(timeout);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

