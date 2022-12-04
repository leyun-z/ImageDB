#ifndef WINDOW_H
#define WINDOW_H

#include <QThread>
#include <QWidget>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlQueryModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class Window;
}
QT_END_NAMESPACE

class Window : public QWidget
{
    Q_OBJECT

public:
    Window(QWidget *parent = nullptr);
    ~Window();

public slots:

    /**
     * @brief 打开图片按钮的槽函数，通过文件对话框，选择一张图片，并将其显示在右侧区域
     */
    void onOpenImageClicked();

    /**
     * @brief 保存图片按钮的槽函数，通过文件对话框，将右侧图片文件保存
     */
    void onSaveImageClicked();

    /**
     * @brief 上传数据库按钮点击，将右侧文件和文件对应的文件名保存到数据库中
     */
    void onUploadDBClicked();

    /**
     * @brief 更新数据库按钮被点击，更新左侧图片表格显示
     * @param message 是否根据更新成功与否发送提示信息，为true则不发送消息
     */
    void onUpdateDBClicked(bool message = true);

    /**
     * @brief 删除图片按钮被点击，如果右侧图片存在数据库中，则删除该图片，否则给出提醒
     */
    void onDeleteImageClicked();

    /**
     * @brief 搜索按钮被点击
     */
    void onSearchClicked();

protected:
    /**
     * @brief 重写鼠标按下事件，捕获该事件，发送Windows消息，移动窗口
     */
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    /**
     * @brief 初始化设置图片表格属性
     */
    void setUpImageTable();

    /**
     * @brief 初始化设置界面效果
     */
    void setUpEffect();

    /**
     * @brief 初始化数据库
     */
    void setUpDataBase();

    /**
     * @brief 初始化按钮槽函数的连接
     */
    void setUpSlots();

private:
    Ui::Window *ui;
    QSqlDatabase m_db;
    QString m_currentImageFileName; //  当前打开图片的文件名
    qlonglong m_currentRowId;       //  当前选中的图片Id
};
#endif // WINDOW_H
