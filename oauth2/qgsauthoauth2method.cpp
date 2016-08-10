/***************************************************************************
    begin                : July 13, 2016
    copyright            : (C) 2016 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthoauth2method.h"

#include "o0globals.h"
#include "o0requestparameter.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsauthmanager.h"
#include "qgso2.h"
#include "qgsauthoauth2config.h"
#include "qgsauthoauth2edit.h"
#include "qgsnetworkaccessmanager.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QEventLoop>
#include <QSettings>
#include <QString>


static const QString AUTH_METHOD_KEY = "OAuth2";
static const QString AUTH_METHOD_DESCRIPTION = "OAuth2 authentication";

QMap<QString, QgsO2* > QgsAuthOAuth2Method::smOAuth2ConfigCache =
  QMap<QString, QgsO2* >();


QgsAuthOAuth2Method::QgsAuthOAuth2Method()
    : QgsAuthMethod()
{
  setVersion( 1 );
  setExpansions( QgsAuthMethod::NetworkRequest | QgsAuthMethod::NetworkReply );
  setDataProviders( QStringList()
                    << "ows"
                    << "wfs"  // convert to lowercase
                    << "wcs"
                    << "wms" );

  QStringList cachedirpaths;
  cachedirpaths << QgsAuthOAuth2Config::tokenCacheDirectory()
  << QgsAuthOAuth2Config::tokenCacheDirectory( true );

  Q_FOREACH ( const QString &cachedirpath, cachedirpaths )
  {
    QDir cachedir( cachedirpath );
    if ( !cachedir.mkpath( cachedirpath ) )
    {
      QgsDebugMsg( QString( "FAILED to create cache dir: %1" ).arg( cachedirpath ) );
    }
  }
}

QgsAuthOAuth2Method::~QgsAuthOAuth2Method()
{
  QDir tempdir( QgsAuthOAuth2Config::tokenCacheDirectory( true ) );
  QStringList dirlist = tempdir.entryList( QDir::Files | QDir::NoDotAndDotDot );
  Q_FOREACH ( QString f, dirlist )
  {
    QString tempfile( tempdir.path() + "/" + f );
    if ( !QFile::remove( tempfile ) )
    {
      QgsDebugMsg( QString( "FAILED to delete temp token cache file: %1" ).arg( tempfile ) );
    }
  }
  if ( !tempdir.rmdir( tempdir.path() ) )
  {
    QgsDebugMsg( QString( "FAILED to delete temp token cache directory: %1" ).arg( tempdir.path() ) );
  }
}

QString QgsAuthOAuth2Method::key() const
{
  return AUTH_METHOD_KEY;
}

QString QgsAuthOAuth2Method::description() const
{
  return AUTH_METHOD_DESCRIPTION;
}

QString QgsAuthOAuth2Method::displayDescription() const
{
  return tr( "OAuth2 authentication" );
}

bool QgsAuthOAuth2Method::updateNetworkRequest( QNetworkRequest &request, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( dataprovider )

  QgsO2* o2 = getOAuth2Bundle( authcfg );
  if ( !o2 )
  {
    QString msg = QString( "Update request FAILED for authcfg %1: null object for requestor" ).arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY );
    QgsDebugMsg( msg );
    return false;
  }

  // if linked, use the available access token, unless a refresh is needed
  if ( o2->linked() )
  {
    // TODO: handle impending expiration of access token

    // if ( expires and refresh token and refresh token url ... )

    // else unlink the authenticator

    // from O2::onTokenReplyFinished()...
    //   qDebug() << "O2::onTokenReplyFinished: Token expires in" << expiresIn << "seconds";
    //   setExpires(QDateTime::currentMSecsSinceEpoch() / 1000 + expiresIn);
    int cursecs = static_cast<int>( QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000 );
  }
  else
  {
    // link app
    // clear any previous token session properties
    o2->unlink();

    connect( o2, SIGNAL( linkedChanged() ), this, SLOT( onLinkedChanged() ) );
    connect( o2, SIGNAL( linkingFailed() ), this, SLOT( onLinkingFailed() ) );
    connect( o2, SIGNAL( linkingSucceeded() ), this, SLOT( onLinkingSucceeded() ) );
    connect( o2, SIGNAL( openBrowser( QUrl ) ), this, SLOT( onOpenBrowser( QUrl ) ) );
    connect( o2, SIGNAL( closeBrowser() ), this, SLOT( onCloseBrowser() ) );

    QSettings settings;
    QString timeoutkey( "/qgis/networkAndProxy/networkTimeout" );
    int prevtimeout = settings.value( timeoutkey, "-1" ).toInt();
    int reqtimeout = o2->oauth2config()->requestTimeout() * 1000;
    settings.setValue( timeoutkey, reqtimeout );

    // go into local event loop and wait for a fired linking-related slot
    QEventLoop loop( qApp );
    loop.connect( o2, SIGNAL( linkingFailed() ), SLOT( quit() ) );
    loop.connect( o2, SIGNAL( linkingSucceeded() ), SLOT( quit() ) );

    // add singlshot timer to quit linking after an alloted timeout
    // this should keep the local event loop from blocking forever
    QTimer timer( this );
    timer.setInterval( reqtimeout * 5 );
    timer.setSingleShot( true );
    connect( &timer, SIGNAL( timeout() ), o2, SIGNAL( linkingFailed() ) );
    timer.start();

    // asynchronously attempt the linking
    o2->link();

    // block request update until asynchronous linking loop is quit
    loop.exec();
    if ( timer.isActive() )
    {
      timer.stop();
    }

    // don't re-apply a setting that wasn't already set
    if ( prevtimeout == -1 )
    {
      settings.remove( timeoutkey );
    }
    else
    {
      settings.setValue( timeoutkey, prevtimeout );
    }

    if ( !o2->linked() )
    {
      QString msg = QString( "Update request FAILED for authcfg %1: requestor could not link app" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, QgsMessageLog::WARNING );
      return false;
    }
  }

  if ( o2->token().isEmpty() )
  {
    QString msg = QString( "Update request FAILED for authcfg %1: access token is empty" ).arg( authcfg );
    QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, QgsMessageLog::WARNING );
    return false;
  }

  // update the request
  QgsAuthOAuth2Config::AccessMethod accessmethod = o2->oauth2config()->accessMethod();

  QString msg;
  QUrl url = request.url();
#if QT_VERSION >= 0x050000
  QUrlQuery query( url );
#endif
  switch ( accessmethod )
  {
    case QgsAuthOAuth2Config::Header:
      request.setRawHeader( O2_HTTP_AUTHORIZATION_HEADER, QString( "Bearer %1" ).arg( o2->token() ).toAscii() );
      msg += QString( "Updated request HEADER with access token for authcfg: %1" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, QgsMessageLog::INFO );
      break;
    case QgsAuthOAuth2Config::Form:
      // FIXME: what to do here if the parent request is not POST?
      //        probably have to skip this until auth system support is moved into QgsNetworkAccessManager
      msg += QString( "Update request FAILED for authcfg %1: form POST token update is unsupported" ).arg( authcfg );
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, QgsMessageLog::WARNING );
      break;
    case QgsAuthOAuth2Config::Query:
#if QT_VERSION < 0x050000
      if ( !url.hasQueryItem( O2_OAUTH2_ACCESS_TOKEN ) )
      {
        url.addQueryItem( O2_OAUTH2_ACCESS_TOKEN, o2->token() );
#else
      if ( !query.hasQueryItem( O2_OAUTH2_ACCESS_TOKEN ) )
      {
        query.addQueryItem( O2_OAUTH2_ACCESS_TOKEN, o2->token() );
        url.setQuery( query );
#endif
        request.setUrl( url );
        msg += QString( "Updated request QUERY with access token for authcfg: %1" ).arg( authcfg );
      }
      else
      {
        msg += QString( "Updated request QUERY with access token SKIPPED (existing token) for authcfg: %1" ).arg( authcfg );
      }
      QgsMessageLog::logMessage( msg, AUTH_METHOD_KEY, QgsMessageLog::INFO );
      break;
  }

  return true;
}

bool QgsAuthOAuth2Method::updateNetworkReply( QNetworkReply *reply, const QString &authcfg, const QString &dataprovider )
{
  Q_UNUSED( reply )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  // TODO: handle token refresh error on the reply, see O2Requestor::onRequestError()
  // Is this doable if the errors are also handled in qgsapp (and/or elsewhere)?
  // Can we block as long as needed if the reply gets deleted elsewhere,
  // or will a local loop's connection keep it alive after a call to deletelater()?

  return true;
}

void QgsAuthOAuth2Method::onLinkedChanged()
{
  // Linking (login) state has changed.
  // Use o2->linked() to get the actual state
  QgsDebugMsg( "Link state changed" );
}

void QgsAuthOAuth2Method::onLinkingFailed()
{
  // Login has failed
  QgsMessageLog::logMessage( "Login has failed", AUTH_METHOD_KEY );
}

void QgsAuthOAuth2Method::onLinkingSucceeded()
{
  QgsO2 *o2 = qobject_cast<QgsO2 *>( sender() );
  if ( !o2 )
  {
    QgsMessageLog::logMessage( "Linking succeeded, but authenticator access FAILED: null object",
                               AUTH_METHOD_KEY, QgsMessageLog::WARNING );
    return;
  }

  if ( !o2->linked() )
  {
    QgsMessageLog::logMessage( "Linking apparently succeeded, but authenticator FAILED to verify it is linked",
                               AUTH_METHOD_KEY, QgsMessageLog::WARNING );
    return;
  }

  QgsMessageLog::logMessage( "Linking succeeded", AUTH_METHOD_KEY, QgsMessageLog::INFO );

  //###################### DO NOT LEAVE ME UNCOMMENTED ######################
  //QgsDebugMsg( QString( "Access token: %1" ).arg( o2->token() ) );
  //QgsDebugMsg( QString( "Access token secret: %1" ).arg( o2->tokenSecret() ) );
  //###################### DO NOT LEAVE ME UNCOMMENTED ######################

  QVariantMap extraTokens = o2->extraTokens();
  if ( !extraTokens.isEmpty() )
  {
    QString msg( "Extra tokens in response:\n" );
    Q_FOREACH ( QString key, extraTokens.keys() )
    {
      // don't expose the values in a log (unless they are only 3 chars long, of course)
      msg += QString( "    %1:%2...\n" ).arg( key, extraTokens.value( key ).toString().left( 3 ) );
    }
    QgsDebugMsg( msg );
  }
}

void QgsAuthOAuth2Method::onOpenBrowser( const QUrl &url )
{
  // Open a web browser or a web view with the given URL.
  // The user will interact with this browser window to
  // enter login name, password, and authorize your application
  // to access the Twitter account
  QgsMessageLog::logMessage( "Open browser requested", AUTH_METHOD_KEY, QgsMessageLog::INFO );

  QDesktopServices::openUrl( url );
}

void QgsAuthOAuth2Method::onCloseBrowser()
{
  // Close the browser window opened in openBrowser()
  QgsMessageLog::logMessage( "Close browser requested", AUTH_METHOD_KEY, QgsMessageLog::INFO );

  // Bring focus back to QGIS app
  if ( QgsApplication::type() == QApplication::GuiClient )
  {
    Q_FOREACH ( QWidget *topwdgt, QgsApplication::topLevelWidgets() )
    {
      if ( topwdgt->objectName() == QString( "MainWindow" ) )
      {
        topwdgt->raise();
        topwdgt->activateWindow();
        topwdgt->show();
        break;
      }
    }
  }
}

void QgsAuthOAuth2Method::onReplyFinished()
{
  QgsMessageLog::logMessage( "Network reply finished", AUTH_METHOD_KEY, QgsMessageLog::INFO );
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  QgsMessageLog::logMessage( QString( "Results: %1" ).arg( QString( reply->readAll() ) ),
                             AUTH_METHOD_KEY, QgsMessageLog::INFO );
}

void QgsAuthOAuth2Method::onNetworkError( QNetworkReply::NetworkError error )
{
  Q_UNUSED( error );

  QgsMessageLog::logMessage( "Network error occurred", AUTH_METHOD_KEY, QgsMessageLog::WARNING );
  QNetworkReply *reply = qobject_cast<QNetworkReply *>( sender() );
  QgsMessageLog::logMessage( QString( "Error: %1" ).arg( reply->errorString() ),
                             AUTH_METHOD_KEY, QgsMessageLog::WARNING );
}

bool QgsAuthOAuth2Method::updateDataSourceUriItems( QStringList &connectionItems, const QString &authcfg,
    const QString &dataprovider )
{
  Q_UNUSED( connectionItems )
  Q_UNUSED( authcfg )
  Q_UNUSED( dataprovider )

  return true;
}

void QgsAuthOAuth2Method::updateMethodConfig( QgsAuthMethodConfig &mconfig )
{
  if ( mconfig.hasConfig( "oldconfigstyle" ) )
  {
    QgsDebugMsg( "Updating old style auth method config" );
  }

  // NOTE: add updates as method version() increases due to config storage changes
}

void QgsAuthOAuth2Method::clearCachedConfig( const QString &authcfg )
{
  removeOAuth2Bundle( authcfg );
}

QgsO2* QgsAuthOAuth2Method::getOAuth2Bundle( const QString &authcfg, bool fullconfig )
{
  // TODO: update to QgsMessageLog output where appropriate

  // check if it is cached
  if ( smOAuth2ConfigCache.contains( authcfg ) )
  {
    QgsDebugMsg( QString( "Retrieving OAuth bundle for authcfg: %1" ).arg( authcfg ) );
    return smOAuth2ConfigCache.value( authcfg );
  }

  QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( qApp );
  QgsO2* nullbundle =  nullptr;

  // else build oauth2 config
  QgsAuthMethodConfig mconfig;
  if ( !QgsAuthManager::instance()->loadAuthenticationConfig( authcfg, mconfig, fullconfig ) )
  {
    QgsDebugMsg( QString( "Retrieve config FAILED for authcfg: %1" ).arg( authcfg ) );
    config->deleteLater();
    return nullbundle;
  }

  QgsStringMap configmap = mconfig.configMap();

  // do loading of method config into oauth2 config

  if ( configmap.contains( "oauth2config" ) )
  {
    QByteArray configtxt = configmap.value( "oauth2config" ).toUtf8();
    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QString( "FAILED to load OAuth2 config: empty config txt" ) );
      config->deleteLater();
      return nullbundle;
    }
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################
    //QgsDebugMsg( QString( "LOAD oauth2config configtxt: \n\n%1\n\n" ).arg( QString( configtxt ) ) );
    //###################### DO NOT LEAVE ME UNCOMMENTED #####################

    if ( !config->loadConfigTxt( configtxt, QgsAuthOAuth2Config::JSON ) )
    {
      QgsDebugMsg( QString( "FAILED to load OAuth2 config into object" ) );
      config->deleteLater();
      return nullbundle;
    }
  }
  else if ( configmap.contains( "definedid" ) )
  {
    bool ok = false;
    QString definedid = configmap.value( "definedid" );
    if ( definedid.isEmpty() )
    {
      QgsDebugMsg( QString( "FAILED to load a defined ID for OAuth2 config" ) );
      config->deleteLater();
      return nullbundle;
    }

    QString extradir = configmap.value( "defineddirpath" );
    if ( extradir.isEmpty() )
    {
      QgsDebugMsg( QString( "No custom defined dir path to load OAuth2 config" ) );
    }

    QgsStringMap definedcache = QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( extradir );

    if ( !definedcache.contains( definedid ) )
    {
      QgsDebugMsg( QString( "FAILED to load OAuth2 config for defined ID: missing ID or file for %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    QByteArray definedtxt = definedcache.value( definedid ).toUtf8();
    if ( definedtxt.isNull() || definedtxt.isEmpty() )
    {
      QgsDebugMsg( QString( "FAILED to load config text for defined ID: empty text for %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    if ( !config->loadConfigTxt( definedtxt, QgsAuthOAuth2Config::JSON ) )
    {
      QgsDebugMsg( QString( "FAILED to load config text for defined ID: %1" ).arg( definedid ) );
      config->deleteLater();
      return nullbundle;
    }

    QByteArray querypairstxt = configmap.value( "querypairs" ).toUtf8();
    if ( !querypairstxt.isNull() && !querypairstxt.isEmpty() )
    {
      QVariantMap querypairsmap =
        QgsAuthOAuth2Config::variantFromSerialized( querypairstxt, QgsAuthOAuth2Config::JSON, &ok );
      if ( !ok )
      {
        QgsDebugMsg( QString( "No query pairs to load OAuth2 config: FAILED to parse" ) );
      }
      if ( querypairsmap.isEmpty() )
      {
        QgsDebugMsg( QString( "No query pairs to load OAuth2 config: parsed pairs are empty" ) );
      }
      else
      {
        config->setQueryPairs( querypairsmap );
      }
    }
    else
    {
      QgsDebugMsg( QString( "No query pairs to load OAuth2 config: empty text" ) );
    }
  }

  // TODO: instantiate particular QgsO2 subclassed authenticators relative to config ???

  QgsDebugMsg( QString( "Loading authenticator object with %1 flow properties of OAuth2 config: %2" )
               .arg( QgsAuthOAuth2Config::grantFlowString( config->grantFlow() ), authcfg ) );

  QgsO2* o2 = new QgsO2( authcfg, config, qApp, QgsNetworkAccessManager::instance() );

  // cache bundle
  putOAuth2Bundle( authcfg, o2 );

  return o2;
}

void QgsAuthOAuth2Method::putOAuth2Bundle( const QString &authcfg, QgsO2* bundle )
{
  QgsDebugMsg( QString( "Putting oauth2 bundle for authcfg: %1" ).arg( authcfg ) );
  smOAuth2ConfigCache.insert( authcfg, bundle );
}

void QgsAuthOAuth2Method::removeOAuth2Bundle( const QString &authcfg )
{
  if ( smOAuth2ConfigCache.contains( authcfg ) )
  {
    smOAuth2ConfigCache.value( authcfg )->deleteLater();
    smOAuth2ConfigCache.remove( authcfg );
    QgsDebugMsg( QString( "Removed oauth2 bundle for authcfg: %1" ).arg( authcfg ) );
  }
}


//////////////////////////////////////////////
// Plugin externals
//////////////////////////////////////////////

/**
 * Required class factory to return a pointer to a newly created object
 */
QGISEXTERN QgsAuthOAuth2Method *classFactory()
{
  return new QgsAuthOAuth2Method();
}

/** Required key function (used to map the plugin to a data store type)
 */
QGISEXTERN QString authMethodKey()
{
  return AUTH_METHOD_KEY;
}

/**
 * Required description function
 */
QGISEXTERN QString description()
{
  return AUTH_METHOD_DESCRIPTION;
}

/**
 * Required isAuthMethod function. Used to determine if this shared library
 * is an authentication method plugin
 */
QGISEXTERN bool isAuthMethod()
{
  return true;
}

/**
 * Optional class factory to return a pointer to a newly created edit widget
 */
QGISEXTERN QgsAuthOAuth2Edit *editWidget( QWidget *parent )
{
  return new QgsAuthOAuth2Edit( parent );
}

/**
 * Required cleanup function
 */
QGISEXTERN void cleanupAuthMethod() // pass QgsAuthMethod *method, then delete method  ?
{
}
