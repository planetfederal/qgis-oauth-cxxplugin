/***************************************************************************
    webpage.cpp  -  test app for PKI integration in QGIS
                             -------------------
    begin                : 2014-09-12
    copyright            : (C) 2014 by Boundless Spatial, Inc.
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

#include "browser.h"
#include "ui_browser.h"

#include "testwidget.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QInputDialog>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QSslCertificate>
#include <QSslConfiguration>
#include <QSslError>
#include <QSslKey>
#include <QTimer>

#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgsauthmethod.h"
#include "qgsauthmethodregistry.h"
#include "qgsautheditorwidgets.h"
#include "qgsauthconfigselect.h"
#include "qgsauthconfigedit.h"
#include "qgsauthoauth2edit.h"
#include "qgsauthsslerrorsdialog.h"
#include "qgscredentialdialog.h"
#include "qgslogger.h"
#include "qgsnetworkaccessmanager.h"


WebBrowser::WebBrowser( QWidget *parent )
    : QMainWindow( parent )
    , mPage( 0 )
    , mReply( 0 )
    , mLoaded( false )
    , mAuth( 0 )
    , mNam( 0 )
    , mTestWidget( 0 )
{
  if ( !initQGIS() )
  {
    return;
  }

  setupUi( this );

  comboBox->lineEdit()->setAlignment( Qt::AlignLeft );

  QStringList urlList;
  urlList << QString( "http://localhost" )
  << QString( "http://localhost:8080" )
  << QString( "https://localhost:8443" )
  << QString( "https://localhost:8443/geoserver/web/" )
  << QString( "https://api.github.com/user/repos" )
  << QString( "http://ckan.dev.flightopseng.com:5000/api/3/action/package_list" )
  << QString( "http://boundless-test:8080" )
  << QString( "https://boundless-test:8443" )
  << QString( "https://boundless-test:8443/geoserver/web/" )
  << QString( "http://www.google.com" )
  << QString( "https://www.googleapis.com/drive/v3/files" )
  << QString( "https://localhost:8443/geoserver/opengeo/wms?service=WMS&version=1.1.0"
              "&request=GetMap&layers=opengeo:countries&styles=&bbox=-180.0,-90.0,180.0,90.0"
              "&width=720&height=400&srs=EPSG:4326&format=application/openlayers" )
  << QString( "https://boundless-test:8443/geoserver/opengeo/wms?service=WMS&version=1.1.0"
              "&request=GetMap&layers=opengeo:countries&styles=&bbox=-180.0,-90.0,180.0,90.0"
              "&width=720&height=400&srs=EPSG:4326&format=application/openlayers" );

  comboBox->addItems( urlList );

  connect( webView, SIGNAL( linkClicked( QUrl ) ), this, SLOT( loadUrl( QUrl ) ) );
  connect( webView, SIGNAL( urlChanged( QUrl ) ), this, SLOT( setLocation( QUrl ) ) );
  connect( webView, SIGNAL( titleChanged( const QString& ) ), SLOT( updateTitle( const QString& ) ) );

  connect( backButton, SIGNAL( clicked() ), webView, SLOT( back() ) );
  connect( forwardButton, SIGNAL( clicked() ), webView, SLOT( forward() ) );
  connect( reloadButton, SIGNAL( clicked() ), webView, SLOT( reload() ) );
  connect( stopButton, SIGNAL( clicked() ), webView, SLOT( stop() ) );

//  connect( comboBox->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( loadUrl() ) );
  connect( comboBox, SIGNAL( activated( const QString& ) ), this, SLOT( loadUrl( const QString& ) ) );
  connect( btnClear, SIGNAL( clicked() ), this, SLOT( clearLog() ) );

  connect( this, SIGNAL( messageOut( const QString&, const QString&, MessageLevel ) ),
           this, SLOT( writeDebug( const QString&, const QString&, MessageLevel ) ) );

  setWebPage();

//  appendLog( QgsApplication::showSettings() );

}

WebBrowser::~WebBrowser()
{
  QgsApplication::exitQgis();
}

bool WebBrowser::initQGIS()
{
  QgsDebugMsg( "Setting up network access manager" );
  mNam = new QgsNetworkAccessManager( qApp );
  mNam->setupDefaultProxyAndCache();

  connect( mNam, SIGNAL( finished( QNetworkReply* ) ),
           this, SLOT( requestReply( QNetworkReply* ) ) );
  connect( mNam, SIGNAL( requestTimedOut( QNetworkReply* ) ),
           this, SLOT( requestTimeout( QNetworkReply* ) ) );
  connect( mNam, SIGNAL( sslErrors( QNetworkReply*, const QList<QSslError>& ) ),
           this, SLOT( onSslErrors( QNetworkReply*, const QList<QSslError>& ) ) );

  QgsDebugMsg( "Checking database" );
  // Do this early on before anyone else opens it and prevents us copying it
  QString dbError;
#ifdef QGIS_2
  if ( !QgsApplication::createDB( &dbError ) )
#else
  if ( !QgsApplication::createDatabase( &dbError ) )
#endif
  {
    QgsDebugMsg( "Private qgis.db!" );
    return false;
  }

  mAuth = QgsAuthManager::instance();
  if ( !mAuth->init( QgsApplication::pluginPath() ) )
  {
    QgsDebugMsg( "Authentication system FAILED to initialize" );
    return false;
  }

  // set graphical credential requester
  new QgsCredentialDialog( this );

  QgsApplication::initQgis();

  if ( mAuth->isDisabled() )
  {
    QgsDebugMsg( "Authentication system is DISABLED" );
    return false;
  }
  else
  {
    connect( mAuth, SIGNAL( messageOut( const QString&, const QString&, MessageLevel ) ),
             this, SLOT( writeDebug( const QString&, const QString&, MessageLevel ) ) );
  }

  return true;
}

void WebBrowser::updateTitle( const QString& title )
{
  setWindowTitle( title );
}

void WebBrowser::setLocation( const QUrl& url )
{
  comboBox->lineEdit()->setText( url.toString() );
}

void WebBrowser::appendLog( const QString& msg )
{
  plainTextEdit->appendPlainText( msg );
}

void WebBrowser::clearLog()
{
  plainTextEdit->clear();
}

void WebBrowser::showEvent( QShowEvent * e )
{
  // only load on demand
//  if ( !mLoaded )
//  {
//    loadUrl();
//    mLoaded = true;
//  }

  QMainWindow::showEvent( e );

  // temporarily do this for testing
  //  QTimer::singleShot( 250, this, SLOT( on_btnAuthSettings_clicked() ) );
}

void WebBrowser::setConfigId( const QString &id )
{
  leAuthId->setText( id );
}

void WebBrowser::loadUrl( const QString& url )
{
  QString curText( comboBox->lineEdit()->text() );
  if ( url.isEmpty() && curText.isEmpty() )
  {
    return;
  }

  QUrl reqUrl( url.isEmpty() ? comboBox->lineEdit()->text() : url );
  loadUrl( reqUrl );
}

void WebBrowser::loadUrl( const QUrl& url )
{
  if ( url.isEmpty() || !url.isValid() )
  {
    return;
  }

  QNetworkRequest req;
  req.setUrl( url );
  req.setRawHeader( "User-Agent", "Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:32.0) Gecko/20100101 Firefox/32.0" );

  QString authcfg = leAuthId->text();

  if ( !authcfg.isEmpty() )
    mAuth->updateNetworkRequest( req, authcfg );

  //webView->load( req ); // hey, why doesn't this work? doesn't pass ssl cert/key

  appendLog( "Request headers:" );
  Q_FOREACH ( QByteArray header, req.rawHeaderList() )
  {
    appendLog( QString( "    %1: %2" ).arg( QString( header ), QString( req.rawHeader( header ) ) ) );
  }

  mReply = mNam->get( req );

  if ( !leAuthId->text().isEmpty() )
    mAuth->updateNetworkReply( mReply, authcfg );

  clearWebView();
  setLocation( mReply->request().url() );
  webView->setFocus();
  connect( mReply, SIGNAL( readyRead() ), this, SLOT( loadReply() ) );
}

void WebBrowser::loadReply()
{
  appendLog( "Reponse headers:" );
  Q_FOREACH ( QByteArray header, mReply->rawHeaderList() )
  {
    appendLog( QString( "    %1: %2" ).arg( QString( header ), QString( mReply->rawHeader( header ) ) ) );
  }

  QUrl url( mReply->url() );
  webView->setContent( mReply->readAll(), QString(), url );
}

void WebBrowser::setWebPage()
{
  mPage = new QWebPage( this );
//  mPage->setNetworkAccessManager( mNam );
  webView->setPage( mPage );
}

void WebBrowser::clearWebView()
{
  webView->setContent( QString( "" ).toAscii() );
}

void WebBrowser::requestReply( QNetworkReply * reply )
{
  if ( reply->error() != QNetworkReply::NoError )
  {
    appendLog( QString( "Network error #%1: %2" ).arg( reply->error() ).arg( reply->errorString() ) );
  }
  if ( reply->isRunning() )
  {
    reply->close();
  }
  reply->deleteLater();
}

void WebBrowser::requestTimeout( QNetworkReply *reply )
{
  QgsDebugMsg( QString( "Request timeout for: %1" ).arg( reply->url().toString() ) );
}

void WebBrowser::onSslErrors( QNetworkReply* reply, const QList<QSslError>& errors )
{
  // Culled from QgisApp::namSslErrors (2.14.3)

  //  reply->ignoreSslErrors( expectedSslErrors() );

  QString msg = QString( "SSL errors occured accessing URL %1:" ).arg( reply->request().url().toString() );

  foreach ( const QSslError& error, errors )
  {
    if ( error.error() == QSslError::NoError )
      continue;

    msg += "\n" + error.errorString();
  }

  appendLog( msg );

//  // check for SSL cert custom config
//  QString sha( QgsAuthCertUtils::shaHexForCert( reply->sslConfiguration().peerCertificate() ) );
//  QgsAuthConfigSslServer config( mAuth->getSslCertCustomConfig( sha ) );
//  if ( !config.isNull() )
//  {
//    appendLog( "SSL cert custom config found, ignoring errors if they match" );
//    const QList<QSslError::SslError>& configerrenums( config.sslIgnoredErrorEnums() );
//    bool ignore = true;
//    Q_FOREACH ( const QSslError& error, errors )
//    {
//      ignore = ignore && configerrenums.contains( error.error() );
//    }
//    if ( ignore )
//    {
//      appendLog( "Errors matched custom config's, ignoring all" );
//      reply->ignoreSslErrors();
//    }
//  }
  // stop the timeout timer, or app crashes if the user (or slot) takes longer than
  // singleshot timeout and tries to update the closed QNetworkReply
  QTimer *timer = reply->findChild<QTimer *>( "timeoutTimer" );
  if ( timer )
  {
    QgsDebugMsg( "Stopping network reply timeout" );
    timer->stop();
  }

  QString requesturl = reply->request().url().toString();
  QgsDebugMsg( QString( "SSL errors occurred accessing URL:\n%1" ).arg( requesturl ) );

  QString hostport( QString( "%1:%2" )
                    .arg( reply->url().host() )
                    .arg( reply->url().port() != -1 ? reply->url().port() : 443 )
                    .trimmed() );
  QString digest( QgsAuthCertUtils::shaHexForCert( reply->sslConfiguration().peerCertificate() ) );
  QString dgsthostport( QString( "%1:%2" ).arg( digest, hostport ) );

  const QHash<QString, QSet<QSslError::SslError> > &errscache( mAuth->getIgnoredSslErrorCache() );

  if ( errscache.contains( dgsthostport ) )
  {
    QgsDebugMsg( QString( "Ignored SSL errors cahced item found, ignoring errors if they match for %1" ).arg( hostport ) );
    const QSet<QSslError::SslError>& errenums( errscache.value( dgsthostport ) );
    bool ignore = !errenums.isEmpty();
    int errmatched = 0;
    if ( ignore )
    {
      Q_FOREACH ( const QSslError& error, errors )
      {
        if ( error.error() == QSslError::NoError )
          continue;

        bool errmatch = errenums.contains( error.error() );
        ignore = ignore && errmatch;
        errmatched += errmatch ? 1 : 0;
      }
    }

    if ( ignore && errenums.size() == errmatched )
    {
      QgsDebugMsg( QString( "Errors matched cached item's, ignoring all for %1" ).arg( hostport ) );
      reply->ignoreSslErrors();
      return;
    }

    QgsDebugMsg( QString( "Errors %1 for cached item for %2" )
                 .arg( errenums.isEmpty() ? "not found" : "did not match",
                       hostport ) );
  }


  QgsAuthSslErrorsDialog *dlg = new QgsAuthSslErrorsDialog( reply, errors, this, digest, hostport );
  dlg->setWindowModality( Qt::ApplicationModal );
  dlg->resize( 580, 512 );
  if ( dlg->exec() )
  {
    if ( reply )
    {
      QgsDebugMsg( QString( "All SSL errors ignored for %1" ).arg( hostport ) );
      reply->ignoreSslErrors();
    }
  }
  dlg->deleteLater();

  // restart network request timeout timer
  if ( reply )
  {
    QSettings s;
    QTimer *timer = reply->findChild<QTimer *>( "timeoutTimer" );
    if ( timer )
    {
      QgsDebugMsg( "Restarting network reply timeout" );
      timer->setSingleShot( true );
      timer->start( s.value( "/qgis/networkAndProxy/networkTimeout", "60000" ).toInt() );
    }
  }
}

void WebBrowser::on_btnAuthSettings_clicked()
{
  QDialog * dlg = new QDialog( this );
  dlg->setWindowTitle( tr( "Authentication Settings" ) );
  QVBoxLayout *layout = new QVBoxLayout( dlg );

  QgsAuthEditorWidgets * ae = new QgsAuthEditorWidgets( dlg );
  layout->addWidget( ae );

  QDialogButtonBox *buttonBox = new QDialogButtonBox( QDialogButtonBox::Close,
      Qt::Horizontal, dlg );
  buttonBox->button( QDialogButtonBox::Close )->setDefault( true );

  layout->addWidget( buttonBox );

  connect( buttonBox, SIGNAL( rejected() ), dlg, SLOT( close() ) );

  dlg->setLayout( layout );
  dlg->setWindowModality( Qt::WindowModal );
  dlg->resize( 800, 512 );
  dlg->exec();

}

void WebBrowser::on_btnAuthSelect_clicked()
{
  QDialog * dlg = new QDialog( this );
  dlg->setWindowTitle( tr( "Select Authentication" ) );
  QVBoxLayout *layout = new QVBoxLayout( dlg );

  QgsAuthConfigSelect * as = new QgsAuthConfigSelect( dlg );
  if ( !leAuthId->text().isEmpty() )
    as->setConfigId( leAuthId->text() );
  layout->addWidget( as );

  QDialogButtonBox *buttonBox = new QDialogButtonBox(
    QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
    Qt::Horizontal, dlg );
  layout->addWidget( buttonBox );

  connect( buttonBox, SIGNAL( accepted() ), dlg, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), dlg, SLOT( close() ) );

  dlg->setLayout( layout );
//  dlg->setWindowModality( Qt::WindowModal );
  if ( dlg->exec() )
  {
    emit messageOut( QString( "Selected authid: %1" ).arg( as->configId() ) );
    leAuthId->setText( as->configId() );
  }
  else
  {
    emit messageOut( "QgsAuthConfigWidget->exec() = 0" );
  }
}

void WebBrowser::on_btnTests_clicked()
{
  if ( !mTestWidget )
  {
    mTestWidget = new TestWidget();
    mTestWidget->setWindowModality( Qt::WindowModal );
  }
  mTestWidget->show();
}

void WebBrowser::on_btnAuthClearCfg_clicked()
{
  if ( !leAuthId->text().isEmpty() )
    mAuth->clearCachedConfig( leAuthId->text() );
}

void WebBrowser::on_btnAuthAdd_clicked()
{
  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this, QString(), QString() );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    setConfigId( ace->configId() );
  }
  ace->deleteLater();
}

void WebBrowser::on_btnAuthEdit_clicked()
{
  if ( leAuthId->text().isEmpty() )
  {
    return;
  }

  if ( !QgsAuthManager::instance()->setMasterPassword( true ) )
    return;

  QgsAuthConfigEdit * ace = new QgsAuthConfigEdit( this, leAuthId->text(), QString() );
  ace->setWindowModality( Qt::WindowModal );
  if ( ace->exec() )
  {
    setConfigId( ace->configId() );
  }
  ace->deleteLater();
}

void WebBrowser::writeDebug( const QString& message, const QString& tag, WebBrowser::MessageLevel level )
{
  Q_UNUSED( tag );

  QString msg;
  switch ( level )
  {
    case INFO:
      break;
    case WARNING:
      msg += "WARNING: ";
      break;
    case CRITICAL:
      msg += "ERROR: ";
      break;
  }

  if ( !tag.isEmpty() )
  {
    msg += QString( "( %1 ) " ).arg( tag );
  }

  msg += message;
  QgsDebugMsg( msg );
}
