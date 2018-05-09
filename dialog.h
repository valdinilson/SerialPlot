#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class Dialog;
}

class QSerialPort;
class QSerialPortInfo;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private slots:
  void timerSlot();

  void on_cmbPortaSerial_currentIndexChanged(int index);

  void on_btnConectar_clicked();

  void on_btnIniciar_clicked();

  void on_rdoTensao_toggled(bool checked);

  void on_rdoTempo_toggled(bool checked);

private:
    Ui::Dialog *ui;
    QTimer dataTimer;
    QSerialPort *arduino;
    QSerialPortInfo *portInfo;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;
    QString arduino_port_name;
    bool arduino_is_available;
    bool arduino_is_connect;
    QByteArray serialData;
    QString serialBuffer;
};

#endif // DIALOG_H
