#include "AddPubKeyDialog.h"
#include "ui_AddPubKeyDialog.h"

#include "wallet.h"

AddPubKeyDialog::AddPubKeyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddPubKeyDialog)
{
    ui->setupUi(this);

    setParent(HXChain::getInstance()->mainFrame);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::FramelessWindowHint);

    ui->widget->setObjectName("widget");
    ui->widget->setStyleSheet(BACKGROUNDWIDGET_STYLE);
    ui->containerWidget->setObjectName("containerwidget");
    ui->containerWidget->setStyleSheet(CONTAINERWIDGET_STYLE);

    ui->okBtn->setStyleSheet(OKBTN_STYLE);
    ui->cancelBtn->setStyleSheet(CANCELBTN_STYLE);
    ui->closeBtn->setStyleSheet(CLOSEBTN_STYLE);

    QRegExp regx("[a-zA-Z0-9]{52}");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->pubKeyLineEdit->setValidator( validator );
}

AddPubKeyDialog::~AddPubKeyDialog()
{
    delete ui;
}

void AddPubKeyDialog::pop()
{
    move(0,0);
    exec();
}

void AddPubKeyDialog::on_closeBtn_clicked()
{
    close();
}

void AddPubKeyDialog::on_okBtn_clicked()
{
    if(checkAddress(ui->pubKeyLineEdit->text(), PubKey) == PubKey)
    {
        pubKey = ui->pubKeyLineEdit->text();
    }
    close();
}

void AddPubKeyDialog::on_cancelBtn_clicked()
{
    close();
}
