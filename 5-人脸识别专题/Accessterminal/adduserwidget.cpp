#include "adduserwidget.h"
#include "ui_adduserwidget.h"
#include "widget.h"

AddUserWidget::AddUserWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddUserWidget)
{
    ui->setupUi(this);
}

AddUserWidget::~AddUserWidget()
{
    delete ui;
}

void AddUserWidget::on_btnadduserface_clicked()
{
    Widget *mainWidget = qobject_cast<Widget*>(parent());
    if(mainWidget){
        mainWidget->getFaceFeatureInVector();
    }else{
        return;
    }
}
