#ifndef TESTWIDGET_H
#define TESTWIDGET_H

#include "ui_testwidget.h"

class TestWidget : public QWidget, private Ui::TestWidget
{
    Q_OBJECT

  public:
    explicit TestWidget( QWidget *parent = 0 );
    ~TestWidget();

  private slots:
    // test slots
//    void test_OAuth2Config_save2Json();

    // base slots
    void on_teInput_textChanged();

    void on_btnOne_clicked();
    void on_btnTwo_clicked();
    void on_btnThree_clicked();
    void on_btnFour_clicked();

  private:
    void setUpTests();
};

#endif // TESTWIDGET_H
