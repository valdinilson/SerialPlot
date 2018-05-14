#include "dialog.h"
#include "ui_dialog.h"

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMessageBox>
#include <QDebug>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    serialBuffer(""),
    ui(new Ui::Dialog),
    arduino_is_connect(false),
    arduino_is_available(false)
{
    ui->setupUi(this);

    // limpa os labels
    ui->lblPortName->clear();
    ui->lblDescription->clear();
    ui->lblManufacturer->clear();
    ui->lblSerialNumber->clear();
    ui->lblSystemLocation->clear();
    ui->lblVendorID->clear();
    ui->lblProductID->clear();

    // desabilita buttons
    ui->btnConectar->setEnabled(false);
    ui->btnIniciar->setEnabled(false);

    // desabilita radioButtons
    ui->rdoTensao->setEnabled(false);
    ui->rdoTempo->setEnabled(false);

    // desabilita doubleSpinBoxes
    ui->spnTensao->setEnabled(false);
    ui->spnRampa->setEnabled(false);
    ui->spnDegrau->setEnabled(false);

    setWindowTitle("Leitor de Porta Serial");

    arduino = new QSerialPort;
    arduino->setBaudRate(QSerialPort::Baud9600);
    arduino->setDataBits(QSerialPort::Data8);
    arduino->setParity(QSerialPort::NoParity);
    arduino->setStopBits(QSerialPort::OneStop);
    arduino->setFlowControl(QSerialPort::NoFlowControl);

    for(const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts()) {
        ui->cmbPortaSerial->addItem(serialPortInfo.portName());
    }
    ui->cmbPortaSerial->setCurrentIndex(-1);
    arduino_port_name.clear();

    ui->plot->addGraph(); // blue line
    ui->plot->graph()->setPen(QPen(QColor(40, 110, 255)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%s");
    ui->plot->xAxis->setTicker(timeTicker);
    ui->plot->axisRect()->setupFullAxesBox();
    ui->plot->yAxis->setRange(-0.5, 5.5);

    connect(ui->plot->xAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->xAxis2, SLOT(setRange(QCPRange)));
    connect(ui->plot->yAxis, SIGNAL(rangeChanged(QCPRange)), ui->plot->yAxis2, SLOT(setRange(QCPRange)));
}

Dialog::~Dialog()
{
    if (arduino->isOpen()) arduino->close();

    delete portInfo;
    delete arduino;
    delete ui;
}

void Dialog::timerSlot()
{
    static QTime time(QTime::currentTime());

    double key = time.elapsed()/1000.0;
    static double lastPointKey = 0;
    if (key-lastPointKey > 0.002)
    {
        QStringList bufferSplit = serialBuffer.split("\r\n");
        if (bufferSplit.length() < 3) {
            serialData = arduino->readAll();
            serialBuffer += QString::fromStdString(serialData.toStdString());
        } else {
            QString value = bufferSplit[1];
            ui->plot->graph()->addData(key, value.toDouble());
            serialBuffer.clear();
        }

      lastPointKey = key;
    }

    ui->plot->xAxis->setRange(key, 30, Qt::AlignRight);
    ui->plot->replot();

    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key-lastFpsKey > 2)
    {
      lastFpsKey = key;
      frameCount = 0;
    }
}

void Dialog::on_cmbPortaSerial_currentIndexChanged(int index)
{
    if (index != -1) {
        portInfo = new QSerialPortInfo(ui->cmbPortaSerial->currentText());

        ui->lblPortName->setText(arduino_port_name);
        ui->lblDescription->setText(portInfo->description());
        ui->lblManufacturer->setText(portInfo->manufacturer());
        ui->lblSystemLocation->setText(portInfo->systemLocation());
        ui->lblSerialNumber->setText(portInfo->serialNumber());

        if (portInfo->hasVendorIdentifier())
            ui->lblVendorID->setText(QString::number(portInfo->vendorIdentifier()));
        if (portInfo->hasProductIdentifier())
            ui->lblProductID->setText(QString::number(portInfo->productIdentifier()));
        ui->btnConectar->setEnabled(true);

    } else {
        arduino_port_name.clear();
        ui->lblPortName->clear();
        ui->lblDescription->clear();
        ui->lblManufacturer->clear();
        ui->lblSystemLocation->clear();
        ui->lblSerialNumber->clear();
        ui->lblVendorID->clear();
        ui->lblProductID->clear();
        ui->btnConectar->setEnabled(false);
        ui->btnIniciar->setEnabled(false);

        ui->rdoTensao->setEnabled(false);
        ui->rdoTempo->setEnabled(false);

        ui->spnTensao->setEnabled(false);
    }
}

void Dialog::on_btnConectar_clicked()
{
    arduino->setPortName(portInfo->portName());

    if (!arduino->isOpen()) {
        arduino_is_connect = arduino->open(QSerialPort::ReadWrite);
        ui->btnConectar->setText("Desconectar");
        connect(arduino, SIGNAL(readyRead()), this, SLOT(timerSlot()));
        dataTimer.start(0);
    } else {
        arduino_is_connect = false;
        ui->btnConectar->setText("Conectar");
        disconnect(arduino, SIGNAL(readyRead()), this, SLOT(timerSlot()));
        if (arduino->isOpen()) arduino->close();
    }

    ui->btnIniciar->setEnabled(arduino_is_connect);
    ui->rdoTensao->setEnabled(arduino_is_connect);
    ui->rdoTempo->setEnabled(arduino_is_connect);
    ui->spnTensao->setEnabled(arduino_is_connect);
}

void Dialog::on_btnIniciar_clicked()
{
    if (arduino->isWritable()) {
        if (ui->rdoTensao->isChecked())
        {
            QString s = "v:" + QString::number(ui->spnTensao->value());
            arduino->write(s.toStdString().c_str());
        } else if (ui->rdoTempo->isChecked())
        {
            QString s = "t:" + QString::number(ui->spnRampa->value()) + ":" + QString::number(ui->spnDegrau->value());
            arduino->write(s.toStdString().c_str());
        } else
            QMessageBox::information(nullptr, "Informação", "Falta dado para transmissão.");
    }
}

void Dialog::on_rdoTensao_toggled(bool checked)
{
    ui->spnTensao->setEnabled(checked);
}

void Dialog::on_rdoTempo_toggled(bool checked)
{
    ui->spnRampa->setEnabled(checked);
    ui->spnDegrau->setEnabled(checked);
}
