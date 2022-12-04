#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
class Dialog;
}

/**
 * @brief 对话窗窗口，用于显示提示信息
 */
class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = nullptr);
    ~Dialog();

    /**
     * @brief 设置窗口的标题
     * @param title 窗口标题
     */
    void setTitle(const QString &title);

    /**
     * @brief 设置窗口提示内容
     * @param content 窗口提示内容
     */
    void setContent(const QString &content);

    /**
     * @brief 获得窗口标题
     * @return 窗口标题
     */
    QString title();

    /**
     * @brief 获得窗口内容
     * @return 返回窗口内容
     */
    QString content();

protected:
    /**
     * @brief 重写鼠标按下事件，捕获该事件，发送Windows消息，移动窗口
     */
    virtual void mousePressEvent(QMouseEvent *event) override;

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
