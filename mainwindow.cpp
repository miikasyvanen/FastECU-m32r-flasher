#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle(title + " " + version);

    set_progressbar_value(0);

    ui->mcuModelComboBox->addItem("WA12212920WWW");
    ui->mcuModelComboBox->addItem("WA12212930WWW");
    ui->mcuModelComboBox->addItem("WA12212970WWW (M32170) in progress");

    //ui->writeFlashLineEdit->setText("./roms/0D04680805.bin");

    ui->baudRateComboBox->addItem("4800");
    ui->baudRateComboBox->addItem("9600");
    ui->baudRateComboBox->addItem("19200");
    ui->baudRateComboBox->addItem("38400");
    ui->baudRateComboBox->addItem("39063");
    ui->baudRateComboBox->addItem("48828");

    ui->parityComboBox->addItem("No parity");
    ui->parityComboBox->addItem("Even parity");

    connect(ui->writeFlashFileDialogButton, SIGNAL(clicked()), this, SLOT(add_file_to_write()));

    serial = new serialport();

    serialport_check_timer = new QTimer(this);
    serialport_check_timer->setInterval(serialport_check_timer_timeout);
    connect(serialport_check_timer, SIGNAL(timeout()), this, SLOT(check_serialports()));
    serialport_check_timer->start();

    connect(ui->ssmInitButton, SIGNAL(clicked()), this, SLOT(requestSsmInit()));
    connect(ui->versionButton, SIGNAL(clicked()), this, SLOT(requestVersion()));
    connect(ui->versionWithSyncButton, SIGNAL(clicked()), this, SLOT(requestVersionWithSync()));
    connect(ui->statusButton, SIGNAL(clicked()), this, SLOT(requestStatus()));
    connect(ui->idCheckButton, SIGNAL(clicked()), this, SLOT(requestId()));

    connect(ui->writeFlashButton, SIGNAL(clicked()), this, SLOT(writeFileToFlash()));
}

MainWindow::~MainWindow()
{
    serial->close_serialport();
    delete ui;
}

int MainWindow::add_file_to_write()
{
    QDir dir("./");
    QString filename;

    QFileDialog openDialog;
    openDialog.setDefaultSuffix("bin");
    filename = QFileDialog::getOpenFileName(this, tr("Open ROM file"), "./", tr("Calibration file (*.bin *.hex)"));

    if (filename.isEmpty()){
        qDebug() << "No ROM file selected!";
        QMessageBox::information(this, tr("ROM file"), "No ROM file selected!");
        return STATUS_ERROR;
    }
    filename = "./" + dir.relativeFilePath(filename);
    set_filename_to_write(filename);

    return STATUS_SUCCESS;
}

QString MainWindow::requestSsmInit()
{
    QByteArray cmd_code;
    QByteArray received;
    int loop = 0;

    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    ui->baudRateComboBox->setCurrentIndex(0);
    ui->parityComboBox->setCurrentIndex(0);

    // 0x80 0xf0 0x10 0x01 0xbf 0x40
    cmd_code.append((uint8_t)0x80);
    cmd_code.append((uint8_t)0x10);
    cmd_code.append((uint8_t)0xf0);
    cmd_code.append((uint8_t)0x01);
    cmd_code.append((uint8_t)0xbf);
    cmd_code.append((uint8_t)0x40);

    send_logwindow_message("Requesting ECU ID, please wait...", true, true);
    send_logwindow_message("Initialising serial port, please wait...", true, true);
    if(open_serialport())
        send_logwindow_message("Serial port error!", true, true);
    else
    {
        send_logwindow_message("Port open, sending request to ECU, please wait...", true, true);

        while (received == "" && loop < 10)
        {
            serial->write_data(cmd_code, true);
            qDebug() << "Try " + QString::number(loop + 1) + ": Sent: " + parse_message_to_hex(cmd_code);
            send_logwindow_message("Try " + QString::number(loop + 1) + ": Sent: " + parse_message_to_hex(cmd_code), true, true);

            delay(250);
            received = serial->read_data();
            if (received != "")
            {
                qDebug()  << "Received:" + parse_message_to_hex(received);
                send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
            }
            loop++;
        }
        if (received == "")
        {
            qDebug()  << "No answer from ECU!";
            send_logwindow_message("No answer from ECU!", true, true);
        }
    }
    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return NULL;
}

QString MainWindow::requestVersion()
{
    QByteArray cmd_code;
    QByteArray received;

    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    ui->baudRateComboBox->setCurrentIndex(1);
    ui->parityComboBox->setCurrentIndex(0);

    send_logwindow_message("Requesting ECU version, please wait...", true, true);
    send_logwindow_message("Initialising serial port, please wait...", true, true);
    if(open_serialport())
    {
        send_logwindow_message("Serial port error!", true, true);
        return NULL;
    }
    else
    {
        send_logwindow_message("Port open, sending request to ECU, please wait...", true, true);
        cmd_code.clear();
        cmd_code.append((uint8_t)M16C_BL_CMD_CODE_VERSION);
        serial->write_data(cmd_code, true);
        send_logwindow_message("Sent: " + parse_message_to_hex(cmd_code), true, true);
        delay(250);
        received = serial->read_data();
        send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
    }
    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return NULL;
}

QString MainWindow::requestVersionWithSync()
{
    QByteArray cmd_code;
    QByteArray received;

    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    ui->baudRateComboBox->setCurrentIndex(1);
    ui->parityComboBox->setCurrentIndex(0);

    send_logwindow_message("Requesting ECU version, please wait...", true, true);
    send_logwindow_message("Initialising serial port, please wait...", true, true);
    if(open_serialport())
    {
        send_logwindow_message("Serial port error!", true, true);
        return NULL;
    }
    else
    {
        send_logwindow_message("Port open, syncing, please wait...", true, true);
        cmd_code.clear();
        cmd_code.append((uint8_t)0x00);
        for (int i = 0; i < 50; i++)
        {
            serial->write_data(cmd_code, false);
            delay(30);
        }
        send_logwindow_message("Sent: " + parse_message_to_hex(cmd_code), true, true);
        delay(250);
        received = serial->read_data();
        send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);

        send_logwindow_message("Sending request to ECU, please wait...", true, true);
        cmd_code.clear();
        cmd_code.append((uint8_t)M16C_BL_CMD_CODE_VERSION);
        serial->write_data(cmd_code, true);
        send_logwindow_message("Sent: " + parse_message_to_hex(cmd_code), true, true);
        delay(1000);
        received = serial->read_data();
        send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
    }
    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return NULL;
}

QString MainWindow::requestStatus()
{
    QByteArray cmd_code;
    QByteArray received;

    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    ui->baudRateComboBox->setCurrentIndex(1);
    ui->parityComboBox->setCurrentIndex(0);

    send_logwindow_message("Requesting ECU status, please wait...", true, true);
    send_logwindow_message("Initialising serial port, please wait...", true, true);
    if(open_serialport())
        send_logwindow_message("Serial port error!", true, true);
    else
    {
        send_logwindow_message("Port open, sending request to ECU, please wait...", true, true);
        cmd_code.clear();
        cmd_code.append((uint8_t)M16C_BL_CMD_CODE_READ_STATUS_REG);
        serial->write_data(cmd_code, true);
        send_logwindow_message("Sent: " + parse_message_to_hex(cmd_code), true, true);

        delay(250);
        received = serial->read_data();
        send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
    }
    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return NULL;
}

QString MainWindow::requestId()
{
    QByteArray cmd_code;
    QByteArray received;

    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    ui->baudRateComboBox->setCurrentIndex(1);
    ui->parityComboBox->setCurrentIndex(0);

    send_logwindow_message("Requesting ECU ID, please wait...", true, true);
    send_logwindow_message("Initialising serial port, please wait...", true, true);
    if(open_serialport())
        send_logwindow_message("Serial port error!", true, true);
    else
    {
        send_logwindow_message("Port open, sending request to ECU, please wait...", true, true);
        cmd_code.clear();
        cmd_code.append((uint8_t)M16C_BL_CMD_CODE_ID_CHECK);
        serial->write_data(cmd_code, true);
        send_logwindow_message("Sent: " + parse_message_to_hex(cmd_code), true, true);

        delay(250);
        received = serial->read_data();
        send_logwindow_message("Received: " + parse_message_to_hex(received), true, true);
    }
    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return NULL;
}

bool MainWindow::writeFileToFlash()
{
    int baudRateComboBoxIndex = ui->baudRateComboBox->currentIndex();
    int parityComboBoxIndex = ui->parityComboBox->currentIndex();

    if (ui->mcuModelComboBox->currentText() == "WA12212920WWW")
    {
        UJ20_Unbrick *uj20_unbrick = new UJ20_Unbrick(this);
        uj20_unbrick->unbrick_uj20();
    }
    if (ui->mcuModelComboBox->currentText() == "WA12212930WWW")
    {
        UJ30_Unbrick *uj30_unbrick = new UJ30_Unbrick(this);
        uj30_unbrick->unbrick_uj30();
    }

    serial->close_serialport();

    ui->baudRateComboBox->setCurrentIndex(baudRateComboBoxIndex);
    ui->parityComboBox->setCurrentIndex(parityComboBoxIndex);

    return STATUS_SUCCESS;
}

QString MainWindow::get_filename_to_write()
{
    return ui->writeFlashLineEdit->text();
}

void MainWindow::set_filename_to_write(QString filename)
{
    ui->writeFlashLineEdit->setText(filename);
}

QString MainWindow::get_serialport_combobox_text()
{
    return ui->serialPortComboBox->currentText();
}

QString MainWindow::get_baudrate_combobox_text()
{
    return ui->baudRateComboBox->currentText();
}

int MainWindow::get_baudrate_combobox_index()
{
    return ui->baudRateComboBox->currentIndex();
}

void MainWindow::set_baudrate_combobox_index(int index)
{
    ui->baudRateComboBox->setCurrentIndex(index);
}

QString MainWindow::get_parity_combobox_text()
{
    return ui->parityComboBox->currentText();
}

int MainWindow::get_parity_combobox_index()
{
    return ui->parityComboBox->currentIndex();
}

void MainWindow::set_parity_combobox_index(int index)
{
    ui->parityComboBox->setCurrentIndex(index);
}

uint8_t MainWindow::calculate_checksum(QByteArray output, bool dec_0x100)
{
    uint8_t checksum = 0;

    for (uint16_t i = 0; i < output.length(); i++)
        checksum += (uint8_t)output.at(i);

    if (dec_0x100)
        checksum = (uint8_t) (0x100 - checksum);

    return checksum;
}

void MainWindow::check_serialports()
{
    QComboBox *serial_port_list = ui->serialPortComboBox;
    QString prev_serial_port;
    int index = 0;

    if (serial_port_list)
    {
        prev_serial_port.append(serial_port_list->currentText());
        serial_port_list->clear();

        serial_ports.clear();
        serial_ports.append(serial->check_serialports());

        for (int i = 0; i < serial_ports.length(); i++)
        {
            serial_port_list->addItem(serial_ports.at(i));
            //if (prev_serial_port == serial_ports.at(i))
                serial_port_list->setCurrentIndex(index);
            index++;
        }
        index--;
        serial_port_list->setCurrentIndex(index);
    }
    //else
        //qDebug() << "Serialport list combo box NOT found!";

}

bool MainWindow::open_serialport()
{
    QStringList serial_port;
    QComboBox *serial_port_list = ui->serialPortComboBox;
    QString baudrate = ui->baudRateComboBox->currentText();
    QString parity = ui->parityComboBox->currentText();
    QString selected_serial_port = ui->serialPortComboBox->currentText();

    serial_port.append(ui->serialPortComboBox->currentText().split(" - ").at(0));
    serial_port.append(ui->serialPortComboBox->currentText().split(" - ").at(1));

    send_logwindow_message("Open serial port: " + serial_port.at(0) + " | baudrate: " + baudrate + " | parity: " + parity, true, true);
    qDebug() << "Baudrate: " + baudrate + " | Parity: " + parity;
    if (serial_port_list)
    {
        if (serial_ports.length() > 0)
        {
            QString opened_serial_port = serial->open_serialport(serial_port.at(0), baudrate, parity);
            //QString opened_serial_port = serial->open_serialport(serial_port.at(0), baudrate, parity);
            if (opened_serial_port != "")
            {
                if (opened_serial_port != prev_serialport)
                {

                }
                send_logwindow_message("Serial port " + opened_serial_port + " ready!", true, true);
                qDebug() << "Serial port " + opened_serial_port + " ready!";
                prev_serialport = opened_serial_port;

                return STATUS_SUCCESS;
            }
            else
            {
                send_logwindow_message("Serial port " + opened_serial_port + " NOT ready!", true, true);
                qDebug() << "Serial port " + opened_serial_port + " NOT ready!";
                return STATUS_ERROR;
            }
        }
    }
    else
    {
        send_logwindow_message("No serialports available!", true, true);
        qDebug() << "No serialports available!";
    }

    return STATUS_ERROR;
}

void MainWindow::send_logwindow_message(QString message, bool timestamp, bool linefeed)
{
    QDateTime dateTime = dateTime.currentDateTime();
    QString dateTimeString = dateTime.toString("[yyyy-MM-dd hh':'mm':'ss'.'zzz']  ");

    if (timestamp)
        message = dateTimeString + message;
    if (linefeed)
        message = message + "\n";

    QTextEdit *logwindow = ui->logwindow;
    if (logwindow)
    {
        logwindow->insertPlainText(message);
        logwindow->ensureCursorVisible();

        QCoreApplication::processEvents(QEventLoop::AllEvents, 5);

        //return STATUS_SUCCESS;
    }

    //return STATUS_ERROR;
}

QString MainWindow::parse_message_to_hex(QByteArray received)
{
    QByteArray msg;

    for (int i = 0; i < received.length(); i++)
    {
        msg.append(QString("0x%1 ").arg((uint8_t)received.at(i),2,16,QLatin1Char('0')).toUtf8());
    }

    return msg;
}

void MainWindow::set_progressbar_value(int value)
{
    if (ui->progressBar)
        ui->progressBar->setValue(value);
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::delay(int timeout)
{
    QTime dieTime = QTime::currentTime().addMSecs(timeout);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
}

