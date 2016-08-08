#include "testwidget.h"
#include "ui_testwidget.h"

#include <QFileInfo>

#include <QtCrypto>

#include "qgsapplication.h"
#include "qgsauthconfig.h"
#include "qgsauthcrypto.h"
#include "qgsauthmanager.h"

TestWidget::TestWidget( QWidget *parent )
    : QWidget( parent )
{
  setupUi( this );
}

TestWidget::~TestWidget()
{
}

void TestWidget::on_teInput_textChanged()
{
}


void TestWidget::on_btnOne_clicked()
{
}

void TestWidget::on_btnTwo_clicked()
{
}

void TestWidget::on_btnThree_clicked()
{
}

void TestWidget::on_btnFour_clicked()
{
}
