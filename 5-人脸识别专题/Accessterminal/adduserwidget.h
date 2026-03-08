#ifndef ADDUSERWIDGET_H
#define ADDUSERWIDGET_H

#include <QWidget>

namespace Ui {
class AddUserWidget;
}

class AddUserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AddUserWidget(QWidget *parent = nullptr);
    ~AddUserWidget();

private slots:
    void on_btnadduserface_clicked();

private:
    Ui::AddUserWidget *ui;
};

#endif // ADDUSERWIDGET_H
