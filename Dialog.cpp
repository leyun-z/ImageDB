#include "Dialog.h"
#include "ui_Dialog.h"

#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>

#include <Windows.h>
#pragma comment(lib, "user32.lib") //  链接user32动态库，调用WindowsAPI

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    //  去除标题栏，通过自绘标题栏的方式，统一UI风格
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint );

    //  为背景添加边框阴影
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor::fromRgb(100,100,100));
    shadowEffect->setBlurRadius(16);
    ui->Bk->setGraphicsEffect(shadowEffect);

    //  设置窗口背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);

    //  为确认按钮添加边框阴影
    QGraphicsDropShadowEffect *shadowEffectOk = new QGraphicsDropShadowEffect(this);
    shadowEffectOk->setOffset(0, 0);
    shadowEffectOk->setColor(QColor::fromRgb(140,204,244));
    shadowEffectOk->setBlurRadius(16);
    ui->BtnOk->setGraphicsEffect(shadowEffectOk);
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::setTitle(const QString &title)
{
    ui->Title->setText(title);
}

void Dialog::setContent(const QString &content)
{
    ui->Content->setText(content);
}

QString Dialog::title()
{
    return ui->Title->text();
}

QString Dialog::content()
{
    return ui->Content->text();
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN //  Windows平台下发送移动窗口的消息
    if (ReleaseCapture())
        SendMessage(HWND(winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
    event->ignore();
#endif
}
