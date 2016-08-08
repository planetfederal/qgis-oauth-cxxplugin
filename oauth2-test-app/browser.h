/***************************************************************************
    browser.h  -  test app for auth method plugin integration in QGIS
                             -------------------
    begin                : 2014-09-12, 07-30-2016
    copyright            : (C) 2014-2016 by Boundless Spatial, Inc.
    web                  : http://boundlessgeo.com
    author               : Larry Shaffer
    email                : larrys at dakotacarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef WEBBROWSER_H
#define WEBBROWSER_H

#include "ui_browser.h"
#include "testwidget.h"

#include <QMainWindow>
#include <QSslCertificate>

#include "qgsauthconfigeditor.h"
#include "qgsauthconfigedit.h"

//
// Mac OS X Includes
// Must include before GEOS 3 due to unqualified use of 'Point'
//
#ifdef Q_OS_MACX
#include <ApplicationServices/ApplicationServices.h>
// check macro breaks QItemDelegate
#ifdef check
#undef check
#endif
#endif

class QgsAuthManager;
class QgsNetworkAccessManager;

class APP_EXPORT WebBrowser : public QMainWindow, private Ui::MainWindow
{
    Q_OBJECT

  public:
    enum MessageLevel
    {
      INFO = 0,
      WARNING = 1,
      CRITICAL = 2
    };

    explicit WebBrowser( QWidget *parent = 0 );
    ~WebBrowser();

  signals:
    void messageOut( const QString& message, const QString& tag = QString(), MessageLevel level = INFO ) const;

  protected:
    void showEvent( QShowEvent * );

  private slots:
    void setConfigId( const QString& id );

    void requestReply( QNetworkReply* reply );
    void requestTimeout( QNetworkReply* reply );
    void onSslErrors( QNetworkReply* reply, const QList<QSslError>& errors );
    void updateTitle( const QString& title );
    void setLocation( const QUrl& url );
    void loadUrl( const QString& url = QString() );
    void loadUrl( const QUrl& url );
    void loadReply();
    void clearWebView();
    void clearLog();

    void writeDebug( const QString& message, const QString& tag = QString(), MessageLevel level = INFO );

    void on_btnTests_clicked();
    void on_btnAuthSettings_clicked();
    void on_btnAuthClearCfg_clicked();
    void on_btnAuthSelect_clicked();
    void on_btnAuthAdd_clicked();

    void on_btnAuthEdit_clicked();

  private:
    bool initQGIS();
    void setWebPage();
    void appendLog( const QString& msg );
    QSslCertificate certAuth();
    QSslCertificate clientCert();
    QSslKey clientKey( const QByteArray& passphrase );
    QList<QSslError> expectedSslErrors();
    QString pkiDir();

    QWebPage *mPage;
    QNetworkReply *mReply;
    bool mLoaded;
    QgsAuthManager *mAuth;
    QgsNetworkAccessManager *mNam;

    TestWidget *mTestWidget;
};

#endif // WEBBROWSER_H
