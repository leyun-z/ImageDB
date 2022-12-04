#include "Window.h"
#include "ui_Window.h"

#include "Dialog.h"

#include <QApplication>
#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QGraphicsDropShadowEffect>
#include <QImage>
#include <QMouseEvent>
#include <QPalette>
#include <QPixmap>
#include <QTableWidgetItem>
#include <QFileInfo>

#include <Windows.h>
#pragma comment(lib, "user32.lib") //  链接user32动态库，调用WindowsAPI

Window::Window(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Window)
    , m_currentRowId(-1)
{
    ui->setupUi(this);

    setUpEffect();     // 初始化界面效果
    setUpDataBase();   //  初始化数据库
    setUpImageTable(); //  初始化设置图片表格
    setUpSlots();      //  初始化设置槽函数连接

    onUpdateDBClicked(); //更新数据库显示
}

Window::~Window()
{
    delete ui;
}

void Window::onOpenImageClicked()
{
    //  打开文件对话框，获取文件名
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "打开一张图片",
        QApplication::applicationDirPath(),
        "图片文件(*.jpg *.png *.bmp *.gif *.jpeg *.pbm *.pgm *.ppm *.xbm *.xpm)");

    //  未选择图片是空字符串，返回
    if (fileName.isEmpty())
        return;

    //  打开图片
    QPixmap image;
    image.load(fileName);
    if (image.isNull()) {
        Dialog *d = new Dialog(this);
        d->setTitle("错误");
        d->setContent("图片加载失败！");
        d->exec();
        d->deleteLater();
        return;
    }

    //  保存当前图片文件名
    m_currentImageFileName = fileName;
    m_currentRowId = -1; //  表示未选中数据库图片

    //  设置Label图片
    ui->Image->setPixmap(image.scaled(ui->Image->size(), Qt::KeepAspectRatio));
}

void Window::onSaveImageClicked()
{
    if (m_currentImageFileName.isEmpty()) {
        Dialog *d = new Dialog(this);
        d->setTitle("错误");
        d->setContent("未加载任何图片！");
        d->exec();
        d->deleteLater();
        return;
    }

    //  保存图片
    auto selected = "(*."
                    + m_currentImageFileName.right(m_currentImageFileName.length()
                                                   - m_currentImageFileName.lastIndexOf('.') - 1)
                    + ")";
    auto path = QFileDialog::getSaveFileName(this,
                                             "保存图片",
                                             QApplication::applicationDirPath(),
                                             "(*.jpg);;(*.png);;(*.bmp);;(*.gif);;(*.jpeg);;(*.pbm)"
                                             ";;(*.pgm);;(*.ppm);;(*.xbm);;(*.xpm)",
                                             &selected);
    ui->Image->pixmap().save(path);
}

void Window::onUploadDBClicked()
{
    Dialog *d = new Dialog(this);

    //  未选中图片的提醒
    if (m_currentImageFileName.isEmpty()) {
        d->setTitle("错误");
        d->setContent("未选择图片！");
        d->exec();
        d->deleteLater();
        return;
    }

    QSqlQuery query;
    query.prepare("insert into images(fileName, imageData) values(:fileName, :imageData);");
    query.bindValue(":fileName", m_currentImageFileName);

    QBuffer buff; //创建内存读写器

    ui->Image->pixmap().save(&buff,
                             m_currentImageFileName
                                 .right(m_currentImageFileName.length()
                                        - m_currentImageFileName.lastIndexOf('.') - 1)
                                 .toLatin1());
    QByteArray dataimg;
    dataimg.append(buff.data()); //图像转换为数据
    query.bindValue(":imageData", dataimg);

    //发送提示消息
    if (!query.exec()) {
        d->setTitle("错误");
        d->setContent("上传数据库失败！");
        qDebug() << query.lastError();
    } else {
        d->setTitle("提示");
        d->setContent("上传数据库成功！");

        //  更新数据库
        onUpdateDBClicked();
        ui->ImageTable->setCurrentCell(ui->ImageTable->rowCount() - 1, 0);
    }
    d->exec();
    d->deleteLater(); //  删除对话框
}

void Window::onUpdateDBClicked(bool message)
{
    QSqlQuery query;

    Dialog *d = new Dialog(this); //发送提示消息
    if (!query.exec("select rowId,fileName,imageData from images;")) {
        d->setTitle("错误");
        d->setContent("更新数据库失败！");
        qDebug() << query.lastError();
    } else {
        d->setTitle("提示");
        d->setContent("更新数据库成功！");
    }

    if (!message) {
        d->exec();
    }
    d->deleteLater(); //  删除对话框

    //  装载数据
    ui->ImageTable->clearContents(); //  清除所有Item
    int index = 0;
    ui->ImageTable->setRowCount(index);
    while (query.next()) {
        ui->ImageTable->setRowCount(index + 1);

        //  每行添加三个item
        QTableWidgetItem *item0 = new QTableWidgetItem();
        QTableWidgetItem *item1 = new QTableWidgetItem();
        QTableWidgetItem *item2 = new QTableWidgetItem();
        item0->setText(query.value("rowId").toString()); //  行号
        item0->setTextAlignment(Qt::AlignCenter);
        QString fileName = query.value("fileName").toString(); //  文件名
        QFileInfo fileInfo(fileName);
        item1->setText(fileInfo.fileName());
        item2->setSizeHint(QSize(160, 80)); //  图片显示区域大小

        ui->ImageTable->setItem(index, 0, item0);
        ui->ImageTable->setItem(index, 1, item1);
        ui->ImageTable->setItem(index, 2, item2);

        //  从数据库中加载图片
        QPixmap image = QPixmap::fromImage(
            QImage::fromData(query.value("imageData").toByteArray(),
                             fileName.right(fileName.length() - fileName.lastIndexOf('.') - 1)
                                 .toLatin1())
                .scaled(item2->sizeHint(), Qt::KeepAspectRatio));

        //如果图片加载失败，则显示一则消息
        if (image.isNull()) {
            item2->setText("图片加载失败！");
        } else {
            QLabel *imageLabel = new QLabel(ui->ImageTable);
            imageLabel->setPixmap(image);
            imageLabel->setAlignment(Qt::AlignCenter);
            ui->ImageTable->setCellWidget(index, 2, imageLabel);
        }

        ui->ImageTable->setRowHeight(index, 100);
        ui->ImageTable->resizeColumnToContents(1);

        index++;
    }
}

void Window::onDeleteImageClicked()
{
    Dialog *d = new Dialog(this);

    //  未选中图片的提醒
    if (m_currentImageFileName.isEmpty()) {
        d->setTitle("错误");
        d->setContent("未选择图片！");
        d->exec();
        d->deleteLater();
        return;
    }

    //  若当前未选中任何数据库图片，则给出提醒
    if (m_currentRowId < 0) {
        d->setTitle("错误");
        d->setContent("当前显示的图片尚未存入数据库！");
        d->exec();
        d->deleteLater();
        return;
    }

    d->setTitle("提醒");
    d->setContent("您确认删除该图片嘛？"
                  "\r\n行号："
                  + QString::number(m_currentRowId) + "\r\n图片：" + m_currentImageFileName);
    if (d->exec() == Dialog::Accepted) { //  用户接受才删除图片

        //  根据行号删除图片
        QSqlQuery query;
        query.prepare("delete from images where rowId=:rowId;");
        query.bindValue(":rowId", m_currentRowId);

        if (query.exec()) {
            d->setTitle("提示");
            d->setContent("图片删除成功！");
            d->exec();
            d->deleteLater();

            qlonglong index = ui->ImageTable->currentRow(); //更新前当前行

            onUpdateDBClicked(); //  更新表格

            //  删除成功
            if (index < ui->ImageTable->rowCount()) {
                ui->ImageTable->setCurrentCell(index, 0);
            } else {
                m_currentRowId = -1;
                m_currentImageFileName.clear();
                ui->Image->setText("还没有图片哦~");
            }

        } else {
            d->setTitle("错误");
            d->setContent("图片删除失败！");
            d->exec();
            d->deleteLater();
            qDebug() << query.lastError();
        }
    }
}

void Window::onSearchClicked()
{
    //  空白字符退出
    auto str = ui->Input->text().trimmed();
    if (str.isEmpty()){
        onUpdateDBClicked();
        return;
    }

    QSqlQuery query;
    if (!query.exec("select rowId,fileName,imageData from images;")) {
        qDebug() << query.lastError();
    }

    //  装载数据
    ui->ImageTable->clearContents(); //  清除所有Item
    int index = 0;
    ui->ImageTable->setRowCount(index);
    while (query.next()) {
        QString fileName = query.value("fileName").toString(); //  文件名
        QFileInfo fileInfo(fileName);

        //  字符匹配不是头部，继续循环
        if (fileInfo.fileName().indexOf(str) != 0)
            continue;

        ui->ImageTable->setRowCount(index + 1);

        //  每行添加三个item
        QTableWidgetItem *item0 = new QTableWidgetItem();
        QTableWidgetItem *item1 = new QTableWidgetItem();
        QTableWidgetItem *item2 = new QTableWidgetItem();
        item0->setText(query.value("rowId").toString()); //  行号
        item0->setTextAlignment(Qt::AlignCenter);
        item1->setText(fileInfo.fileName());
        item2->setSizeHint(QSize(160, 80)); //  图片显示区域大小

        ui->ImageTable->setItem(index, 0, item0);
        ui->ImageTable->setItem(index, 1, item1);
        ui->ImageTable->setItem(index, 2, item2);

        //  从数据库中加载图片
        QPixmap image = QPixmap::fromImage(
            QImage::fromData(query.value("imageData").toByteArray(),
                             fileName.right(fileName.length() - fileName.lastIndexOf('.') - 1)
                                 .toLatin1())
                .scaled(item2->sizeHint(), Qt::KeepAspectRatio));

        //如果图片加载失败，则显示一则消息
        if (image.isNull()) {
            item2->setText("图片加载失败！");
        } else {
            QLabel *imageLabel = new QLabel(ui->ImageTable);
            imageLabel->setPixmap(image);
            imageLabel->setAlignment(Qt::AlignCenter);
            ui->ImageTable->setCellWidget(index, 2, imageLabel);
        }

        ui->ImageTable->setRowHeight(index, 100);
        ui->ImageTable->resizeColumnToContents(1);

        index++;
    }
}

void Window::mousePressEvent(QMouseEvent *event)
{
#ifdef Q_OS_WIN //  Windows平台下发送移动窗口的消息
    if (ReleaseCapture())
        SendMessage(HWND(winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
    event->ignore();
#endif
}

void Window::setUpImageTable()
{
    //  设置表头
    ui->ImageTable->setColumnCount(3);                               //  设置表列数
    ui->ImageTable->horizontalHeader()->setMinimumHeight(40);        //  设置表头最低高度
    ui->ImageTable->horizontalHeader()->setStretchLastSection(true); //  设置表头最后一项填充
    QStringList headers;
    headers << "行号"
            << "文件名"
            << "图片";
    ui->ImageTable->setHorizontalHeaderLabels(headers); //   初始化表头
    ui->ImageTable->setWordWrap(true);
}

void Window::setUpEffect()
{
    //  去除标题栏，通过自绘标题栏的方式，统一UI风格
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    //  为主界面添加边框阴影
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setOffset(0, 0);
    shadowEffect->setColor(QColor::fromRgb(100, 100, 100));
    shadowEffect->setBlurRadius(16);
    ui->Main->setGraphicsEffect(shadowEffect);

    //  为图片添加阴影底座
    QGraphicsDropShadowEffect *shadowEffectImageBottom = new QGraphicsDropShadowEffect(this);
    shadowEffectImageBottom->setOffset(0, 0);
    shadowEffectImageBottom->setColor(ui->BtnOpenImage->palette().color(QPalette::Window));
    shadowEffectImageBottom->setBlurRadius(32);
    ui->ImageBottom->setGraphicsEffect(shadowEffectImageBottom);

    //  设置窗口背景透明
    setAttribute(Qt::WA_TranslucentBackground, true);

    //  为表格控件添加阴影
    QGraphicsDropShadowEffect *shadowEffectTable = new QGraphicsDropShadowEffect(this);
    shadowEffectTable->setOffset(0, 0);
    shadowEffectTable->setColor(ui->BtnOpenImage->palette().color(QPalette::Window));
    shadowEffectTable->setBlurRadius(16);
    ui->ImageTable->setGraphicsEffect(shadowEffectTable);

    //  为功能按钮添加阴影
    QList<QPushButton *> btnList;
    btnList << ui->BtnOpenImage << ui->BtnSaveImage << ui->BtnDeleteImage << ui->BtnUploadDB
            << ui->BtnUpdateDB;
    for (const auto &button : btnList) {
        QGraphicsDropShadowEffect *shadowEffectButton = new QGraphicsDropShadowEffect(this);
        shadowEffectButton->setOffset(0, 0);
        shadowEffectButton->setColor(button->palette().color(QPalette::Window));
        shadowEffectButton->setBlurRadius(4);
        button->setGraphicsEffect(shadowEffectButton);
    }
}

void Window::setUpDataBase()
{
    //  连接数据库
    auto m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("ImageDB.sqlite");

    if (!m_db.open()) {
        qDebug() << "DBService::DBService 数据库初始化失败！" << m_db.lastError();
    } else {
        qDebug() << "数据库连接成功！";
    }
}

void Window::setUpSlots()
{
    //  配置按钮槽函数
    connect(ui->BtnOpenImage,
            &QPushButton::clicked,
            this,
            &Window::onOpenImageClicked); //打开图片按钮
    connect(ui->BtnSaveImage,
            &QPushButton::clicked,
            this,
            &Window::onSaveImageClicked); //保存图片按钮
    connect(ui->BtnUpdateDB,
            &QPushButton::clicked,
            this,
            &Window::onUpdateDBClicked); //更新数据库按钮
    connect(ui->BtnUploadDB,
            &QPushButton::clicked,
            this,
            &Window::onUploadDBClicked); //上传数据库按钮
    connect(ui->BtnDeleteImage,
            &QPushButton::clicked,
            this,
            &Window::onDeleteImageClicked); //删除图片按钮
    connect(ui->BtnSearch,
            &QToolButton::clicked,
            this,
            &Window::onSearchClicked); //搜索按钮

    //  监听当前选择变化，并实时改变右侧图片显示
    connect(ui->ImageTable,
            &QTableWidget::currentCellChanged,
            this,
            [=](qlonglong cr, qlonglong cl, qlonglong pr, qlonglong pl) {
                //  注释掉未使用的变量
                Q_UNUSED(cl);
                Q_UNUSED(pr);
                Q_UNUSED(pl);
                //  如果行号小于0，则立即返回
                if (cr < 0)
                    return;

                //  获取当前选中的图片的数据库行号和文件名
                m_currentRowId = ui->ImageTable->item(cr, 0)->text().toLongLong();
                m_currentImageFileName = ui->ImageTable->item(cr, 1)->text();

                //  根据行号查询图片数据
                QSqlQuery query;
                query.prepare("select imageData from images where rowId=:rowId");
                query.bindValue(":rowId", m_currentRowId);
                if (!query.exec()) {
                    Dialog *d = new Dialog(this);
                    d->setTitle("错误");
                    d->setContent("加载图片失败！");
                    d->exec();
                    d->deleteLater();
                } else {
                    query.next();

                    //  查询成功，则装载图片数据
                    QPixmap image = QPixmap::fromImage(
                        QImage::fromData(query.value("imageData").toByteArray(),
                                         m_currentImageFileName
                                             .right(m_currentImageFileName.length()
                                                    - m_currentImageFileName.lastIndexOf('.') - 1)
                                             .toLatin1())
                            .scaled(ui->Image->size(), Qt::KeepAspectRatio));

                    if (image.isNull()) {
                        ui->Image->setText("图片加载失败！");
                    } else {
                        ui->Image->setPixmap(image);
                    }

                    //  更新窗口
                    update();
                }
            });
}
