﻿#include <QListView>

#ifdef WIN32
#include <windows.h>
#endif



#include "transferpage.h"
#include "ui_transferpage.h"
#include "wallet.h"

#include "contactdialog.h"
#include "remarkdialog.h"
#include "commondialog.h"
#include "transferconfirmdialog.h"
#include "transferrecordwidget.h"
#include "ContactChooseWidget.h"
#include "BlurWidget.h"

TransferPage::TransferPage(QString name,QWidget *parent,QString assettype) :
    QWidget(parent),
    accountName(name),
    assetType(assettype),
    inited(false),
    assetUpdating(false),
    contactUpdating(false),
    currentTopWidget(NULL),
    ui(new Ui::TransferPage)
{
	

    ui->setupUi(this);

    InitStyle();

    //初始化账户comboBox
    // 账户下拉框按字母顺序排序
    QStringList keys = UBChain::getInstance()->accountInfoMap.keys();
    ui->accountComboBox->addItems( keys);
    if(ui->accountComboBox->count() != 0)
    {
        if( accountName.isEmpty() )
        {
            ui->accountComboBox->setCurrentIndex(0);
        }
        else
        {
            ui->accountComboBox->setCurrentText( accountName);
        }
    }

    if( accountName.isEmpty())  // 如果是点击账单跳转
    {
        if( UBChain::getInstance()->addressMap.size() > 0)
        {
            accountName = UBChain::getInstance()->addressMap.keys().at(0);
        }
        else  // 如果还没有账户
        {
            emit back();    // 跳转在functionbar  这里并没有用
            return;
        }
    }

    connect( UBChain::getInstance(), SIGNAL(jsonDataUpdated(QString)), this, SLOT(jsonDataUpdated(QString)));

    connect(ui->toolButton_chooseContact,&QToolButton::clicked,this,&TransferPage::chooseContactSlots);
    connect(ui->checkBox,&QCheckBox::stateChanged,this,&TransferPage::checkStateChangedSlots);

    ui->amountLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    setAmountPrecision();

    QRegExp regx("[a-zA-Z0-9\-\.\ \n]+$");
    QValidator *validator = new QRegExpValidator(regx, this);
    ui->sendtoLineEdit->setValidator( validator );
    ui->sendtoLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->tipLabel3->hide();
    ui->tipLabel4->hide();
    ui->tipLabel6->hide();

    getAssets();
    if(!assetType.isEmpty())
    {
        ui->assetComboBox->setCurrentText(assetType);
    }

    inited = true;
	
    updateAmountSlots();

    updatePoundage();
}

TransferPage::~TransferPage()
{
    delete ui;
}



void TransferPage::on_accountComboBox_currentIndexChanged(const QString &arg1)
{
    if( !inited)  return;

    accountName = arg1;
    UBChain::getInstance()->mainFrame->setCurrentAccount(accountName);

    ui->amountLineEdit->clear();
    updateAmountSlots();
}


void TransferPage::on_sendBtn_clicked()
{
    if(ui->amountLineEdit->text().size() == 0 || ui->sendtoLineEdit->text().size() == 0)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("Please enter the amount and address."));
        tipDialog.pop();
        return;
    }

    if( ui->amountLineEdit->text().toDouble()  <= 0)
    {
        CommonDialog tipDialog(CommonDialog::OkOnly);
        tipDialog.setText( tr("The amount can not be 0"));
        tipDialog.pop();
        return;
    }


    QString remark = ui->memoTextEdit->toPlainText();


    AddressType type = checkAddress(ui->sendtoLineEdit->text(),AccountAddress | MultiSigAddress);
    if( type == AccountAddress)
    {
        TransferConfirmDialog transferConfirmDialog( ui->sendtoLineEdit->text(), ui->amountLineEdit->text(), "20", remark, ui->assetComboBox->currentText());
        bool yOrN = transferConfirmDialog.pop();
        if( yOrN)
        {

            UBChain::getInstance()->postRPC( "id-transfer_to_address-" + accountName,
                                             toJsonFormat( "transfer_to_address",
                                                           QJsonArray() << accountName << ui->sendtoLineEdit->text()
                                                           << ui->amountLineEdit->text() << ui->assetComboBox->currentText()
                                                           << remark << true ));
qDebug() << toJsonFormat( "transfer_to_address",
                          QJsonArray() << accountName << ui->sendtoLineEdit->text()
                          << ui->amountLineEdit->text() << ui->assetComboBox->currentText()
                          << remark << true );
        }

    }
}

void TransferPage::on_amountLineEdit_textChanged(const QString &arg1)
{

}

void TransferPage::refresh()
{

}

void TransferPage::setAmountPrecision()
{
    QRegExp rx1(QString("^([0]|[1-9][0-9]{0,10})(?:\\.\\d{0,%1})?$|(^\\t?$)").arg(ASSET_PRECISION));
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    ui->amountLineEdit->setValidator(pReg1);
}

void TransferPage::InitStyle()
{
    setAutoFillBackground(true);
    QPalette palette;
    palette.setColor(QPalette::Window, QColor(248,249,253));
    setPalette(palette);
    setStyleSheet("QLineEdit{border:none;background:transparent;color:#5474EB;margin-left:2px;}"
                  "Qline{color:#5474EB;background:#5474EB;}"
                  "QComboBox{border:none;background:transparent;color:#5474EB;font-size:12pt;margin-left:3px;}"
                  "QPushButton{border:none;background-color:#5474EB;color:white;width:60px;height:20px;\
                   border-radius:10px;font-size:12pt;}"
                  "QPushButton::hover{background-color:#00D2FF;}"
                  "QLabel{background:transparent;color:black:font-family:Microsoft YaHei UI Light;}"
                  );
    ui->sendBtn->setStyleSheet("QToolButton{background-color:#5474EB; border:none;border-radius:10px;color: rgb(255, 255, 255);}"
                               "QToolButton:hover{background-color:#00D2FF;}");
    ui->toolButton->setStyleSheet("QToolButton{background-color:#5474EB; border:none;border-radius:10px;color: rgb(255, 255, 255);}"
                                  "QToolButton:hover{background-color:#00D2FF;}");

    ui->transferRecordBtn->setStyleSheet("QToolButton{background-color:#00D2FF; border:none;border-radius:10px;color: rgb(255, 255, 255);}"
                                         "QToolButton:hover{background-color:#5474EB;}");

    ui->toolButton_chooseContact->setIconSize(QSize(14,14));
    ui->toolButton_chooseContact->setIcon(QIcon(":/ui/wallet_ui/tans.png"));
    ui->memoTextEdit->setStyleSheet("border:none;background:none;color:#5474EB;");

    ui->toolButton_chooseContact->setStyleSheet("QToolButton{background-color:black; border:none;border-radius:7px;color: rgb(255, 255, 255);}"
                                                "QToolButton:hover{background-color:#5474EB;}");
}

void TransferPage::updatePoundage()
{//查询配置文件中的手续费设置
    QString feeID = UBChain::getInstance()->configFile->value("/settings/feeOrderID").toString();
    if(feeID.isEmpty())
    {
        ui->checkBox->setChecked(false);
        ui->checkBox->setEnabled(true);
        ui->toolButton->setText(tr("poundage doesn't exist!"));
        ui->toolButton->setEnabled(false);
    }
    else
    {
        //查询承兑单
    }
}

QString TransferPage::getCurrentAccount()
{
    return accountName;
}

void TransferPage::setAddress(QString address)
{
    ui->sendtoLineEdit->setText(address);
}


void TransferPage::getAssets()
{
    assetUpdating = true;

    QStringList keys = UBChain::getInstance()->assetInfoMap.keys();
    foreach (QString key, keys)
    {
        ui->assetComboBox->addItem(UBChain::getInstance()->assetInfoMap.value(key).symbol);
    }


    assetUpdating = false;
}


void TransferPage::jsonDataUpdated(QString id)
{
    if( id == "id-transfer_to_address-" + accountName)
    {
        QString result = UBChain::getInstance()->jsonDataValue(id);
qDebug() << id << result;
        if( result.startsWith("\"result\":{"))             // 成功
        {

            CommonDialog tipDialog(CommonDialog::OkOnly);
            tipDialog.setText( tr("Transaction has been sent,please wait for confirmation"));
            tipDialog.pop();

        }
        else
        {
            int pos = result.indexOf("\"message\":\"") + 11;
            QString errorMessage = result.mid(pos, result.indexOf("\"", pos) - pos);

            CommonDialog tipDialog(CommonDialog::OkOnly);
            tipDialog.setText( tr("Transaction sent failed: %1").arg(errorMessage));
            tipDialog.pop();

        }
        return;
    }


}


void TransferPage::on_assetComboBox_currentIndexChanged(int index)
{
    if( assetUpdating)  return;

    setAmountPrecision();

    ui->amountLineEdit->clear();
    updateAmountSlots();
}

void TransferPage::on_sendtoLineEdit_textChanged(const QString &arg1)
{
    if( ui->sendtoLineEdit->text().contains(" ") || ui->sendtoLineEdit->text().contains("\n"))   // 不判断就remove的话 右键菜单撤销看起来等于不能用
    {
        ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().simplified().remove(" "));
    }

//    ui->sendtoLineEdit->setText( ui->sendtoLineEdit->text().remove("\n"));
    if( ui->sendtoLineEdit->text().isEmpty()  )
    {
        ui->tipLabel4->hide();
        return;
    }

    AddressType type = checkAddress(ui->sendtoLineEdit->text(),AccountAddress | ContractAddress | MultiSigAddress);
    if( type == AccountAddress)
    {
        ui->tipLabel4->setText(tr("Valid account address."));
        ui->tipLabel4->setStyleSheet("color: rgb(43,230,131);");
        ui->tipLabel4->show();
//        calculateCallContractFee();
    }
    else if( type == ContractAddress)
    {
        ui->tipLabel4->setText(tr("Sending coins to contract address is not supported currently."));
        ui->tipLabel4->setStyleSheet("color: rgb(255,34,76);");
        ui->tipLabel4->show();
    }
    else if( type == MultiSigAddress)
    {
        if(ui->assetComboBox->currentIndex() > 0)
        {
            ui->tipLabel4->setText(tr("You can only send %1s to multisig address currently.").arg(ASSET_NAME));
            ui->tipLabel4->setStyleSheet("color: rgb(255,34,76);");
            ui->tipLabel4->show();
        }
        else
        {
            ui->tipLabel4->setText(tr("Valid multisig address."));
            ui->tipLabel4->setStyleSheet("color: rgb(43,230,131);");
            ui->tipLabel4->show();
        }
    }
    else
    {
        ui->tipLabel4->setText(tr("Invalid address."));
        ui->tipLabel4->setStyleSheet("color: rgb(255,34,76);");
        ui->tipLabel4->show();
    }


}


void TransferPage::on_memoTextEdit_textChanged()
{
    QTextCodec* utfCodec = QTextCodec::codecForName("UTF-8");
    QByteArray ba = utfCodec->fromUnicode(ui->memoTextEdit->toPlainText());
    if( ba.size() > 40)
    {
        ui->tipLabel6->show();
    }
    else
    {
        ui->tipLabel6->hide();
    }
}

void TransferPage::on_transferRecordBtn_clicked()
{
    currentTopWidget = new TransferRecordWidget(this);
    currentTopWidget->move(0,0);
    currentTopWidget->show();
    static_cast<TransferRecordWidget*>(currentTopWidget)->showTransferRecord(UBChain::getInstance()->accountInfoMap.value(accountName).address);
}

void TransferPage::chooseContactSlots()
{
    ContactChooseWidget *wi = new ContactChooseWidget(this);
    BlurWidget *blur = new BlurWidget(this);
    connect(wi,&ContactChooseWidget::closeSignal,blur,&BlurWidget::close);
    connect(wi,&ContactChooseWidget::selectContactSignal,this,&TransferPage::selectContactSlots);

    blur->show();
    wi->move(QPoint(160,140));
    wi->show();
    wi->raise();
}

void TransferPage::selectContactSlots(const QString &name, const QString &address)
{
    ui->sendtoLineEdit->setText(address);
}

void TransferPage::updateAmountSlots()
{//根据当前账户、资产类型，判断最大转账数量
    QString name = ui->accountComboBox->currentText();
    QString assetType = ui->assetComboBox->currentText();

    QString assetID ;
    int assetPrecision ;
    foreach (AssetInfo in, UBChain::getInstance()->assetInfoMap) {
        if(in.symbol == assetType){
            assetID = in.id;
            assetPrecision = in.precision;
            break;
        }
    }

    //获取数量
    bool isFindAmmount = false;
    QMapIterator<QString, AccountInfo> i(UBChain::getInstance()->accountInfoMap);
      while (i.hasNext()) {
          i.next();
          if(isFindAmmount) break;
          if(i.key() != name) continue;
          AccountInfo info = i.value();
          QMapIterator<QString,AssetAmount> am(info.assetAmountMap);
          while(am.hasNext()){
              am.next();
              if(am.key() == assetID)
              {
                  ui->amountLineEdit->setPlaceholderText(tr("max muber:") + QString::number(
                       am.value().amount/pow(10,assetPrecision)));
                  isFindAmmount = true;
                  break;
              }
          }
      }
     if(!isFindAmmount)
     {
         ui->amountLineEdit->setPlaceholderText(tr("max muber: 0"));
     }
}

void TransferPage::checkStateChangedSlots(int state)
{
    if(state == Qt::Checked)
    {
        ui->toolButton->setEnabled(true);
    }
    else
    {
        ui->toolButton->setEnabled(false);
    }
}