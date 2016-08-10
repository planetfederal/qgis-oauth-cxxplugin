/***************************************************************************
    begin                : July 30, 2016
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

#include "qgsauthoauth2config.h"

#include <QDir>
#include <QSettings>

#if QT_VERSION < 0x050000
// use QJson third-party lib
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <qjson/qobjecthelper.h>
#else
// TODO include <QJsonDocument> etc from QtCore?
#endif

#include "qgsapplication.h"
#include "qgslogger.h"


QgsAuthOAuth2Config::QgsAuthOAuth2Config( QObject *parent )
    : QObject( parent )
    , mVersion( 1 )
    , mConfigType( Custom )
    , mGrantFlow( AuthCode )
    , mRedirectPort( 7070 )
    , mPersistToken( false )
    , mAccessMethod( Header )
    , mRequestTimeout( 30 ) // in seconds
    , mQueryPairs( QVariantMap() )
    , mValid( false )
{

  // internal signal bounces
  connect( this, SIGNAL( idChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( versionChanged( int ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( configTypeChanged( ConfigType ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( grantFlowChanged( GrantFlow ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( nameChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( descriptionChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( requestUrlChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( tokenUrlChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( refreshTokenUrlChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( redirectUrlChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( redirectPortChanged( int ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( clientIdChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( clientSecretChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( usernameChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( passwordChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( scopeChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( stateChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( apiKeyChanged( const QString& ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( persistTokenChanged( bool ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( accessMethodChanged( AccessMethod ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( requestTimeoutChanged( int ) ), this, SIGNAL( configChanged() ) );
  connect( this, SIGNAL( queryPairsChanged( const QVariantMap& ) ), this, SIGNAL( configChanged() ) );

  // always recheck validity on any change
  // this, in turn, may emit validityChanged( bool )
  connect( this, SIGNAL( configChanged() ), this, SLOT( validateConfig() ) );

  validateConfig();
}

QgsAuthOAuth2Config::~QgsAuthOAuth2Config()
{
}

void QgsAuthOAuth2Config::setId( const QString &value )
{
  QString preval( mId );
  mId = value;
  if ( preval != value ) emit idChanged( mId );
}

void QgsAuthOAuth2Config::setVersion( int value )
{
  int preval( mVersion );
  mVersion = value;
  if ( preval != value ) emit versionChanged( mVersion );
}

void QgsAuthOAuth2Config::setConfigType( QgsAuthOAuth2Config::ConfigType value )
{
  ConfigType preval( mConfigType );
  mConfigType = value;
  if ( preval != value ) emit configTypeChanged( mConfigType );
}

void QgsAuthOAuth2Config::setGrantFlow( QgsAuthOAuth2Config::GrantFlow value )
{
  GrantFlow preval( mGrantFlow );
  mGrantFlow = value;
  if ( preval != value ) emit grantFlowChanged( mGrantFlow );
}

void QgsAuthOAuth2Config::setName( const QString &value )
{
  QString preval( mName );
  mName = value;
  if ( preval != value ) emit nameChanged( mName );
}

void QgsAuthOAuth2Config::setDescription( const QString &value )
{
  QString preval( mDescription );
  mDescription = value;
  if ( preval != value ) emit descriptionChanged( mDescription );
}

void QgsAuthOAuth2Config::setRequestUrl( const QString &value )
{
  QString preval( mRequestUrl );
  mRequestUrl = value;
  if ( preval != value ) emit requestUrlChanged( mRequestUrl );
}

void QgsAuthOAuth2Config::setTokenUrl( const QString &value )
{
  QString preval( mTokenUrl );
  mTokenUrl = value;
  if ( preval != value ) emit tokenUrlChanged( mTokenUrl );
}

void QgsAuthOAuth2Config::setRefreshTokenUrl( const QString &value )
{
  QString preval( mRefreshTokenUrl );
  mRefreshTokenUrl = value;
  if ( preval != value ) emit refreshTokenUrlChanged( mRefreshTokenUrl );
}

void QgsAuthOAuth2Config::setRedirectUrl( const QString &value )
{
  QString preval( mRedirectURL );
  mRedirectURL = value;
  if ( preval != value ) emit redirectUrlChanged( mRedirectURL );
}

void QgsAuthOAuth2Config::setRedirectPort( int value )
{
  int preval( mRedirectPort );
  mRedirectPort = value;
  if ( preval != value ) emit redirectPortChanged( mRedirectPort );
}

void QgsAuthOAuth2Config::setClientId( const QString &value )
{
  QString preval( mClientId );
  mClientId = value;
  if ( preval != value ) emit clientIdChanged( mClientId );
}

void QgsAuthOAuth2Config::setClientSecret( const QString &value )
{
  QString preval( mClientSecret );
  mClientSecret = value;
  if ( preval != value ) emit clientSecretChanged( mClientSecret );
}

void QgsAuthOAuth2Config::setUsername( const QString &value )
{
  QString preval( mUsername );
  mUsername = value;
  if ( preval != value ) emit usernameChanged( mUsername );
}

void QgsAuthOAuth2Config::setPassword( const QString &value )
{
  QString preval( mPassword );
  mPassword = value;
  if ( preval != value ) emit passwordChanged( mPassword );
}

void QgsAuthOAuth2Config::setScope( const QString &value )
{
  QString preval( mScope );
  mScope = value;
  if ( preval != value ) emit scopeChanged( mScope );
}

void QgsAuthOAuth2Config::setState( const QString &value )
{
  QString preval( mState );
  mState = value;
  if ( preval != value ) emit stateChanged( mState );
}

void QgsAuthOAuth2Config::setApiKey( const QString &value )
{
  QString preval( mApiKey );
  mApiKey = value;
  if ( preval != value ) emit apiKeyChanged( mApiKey );
}

void QgsAuthOAuth2Config::setPersistToken( bool persist )
{
  bool preval( mPersistToken );
  mPersistToken = persist;
  if ( preval != persist ) emit persistTokenChanged( mPersistToken );
}

void QgsAuthOAuth2Config::setAccessMethod( QgsAuthOAuth2Config::AccessMethod value )
{
  AccessMethod preval( mAccessMethod );
  mAccessMethod = value;
  if ( preval != value ) emit accessMethodChanged( mAccessMethod );
}

void QgsAuthOAuth2Config::setRequestTimeout( int value )
{
  int preval( mRequestTimeout );
  mRequestTimeout = value;
  if ( preval != value ) emit requestTimeoutChanged( mRequestTimeout );
}

void QgsAuthOAuth2Config::setQueryPairs( const QVariantMap &pairs )
{
  QVariantMap preval( mQueryPairs );
  mQueryPairs = pairs;
  if ( preval != pairs ) emit queryPairsChanged( mQueryPairs );
}

void QgsAuthOAuth2Config::setToDefaults()
{
  setId( QString::null );
  setVersion( 1 );
  setConfigType( QgsAuthOAuth2Config::Custom );
  setGrantFlow( QgsAuthOAuth2Config::AuthCode );
  setName( QString::null );
  setDescription( QString::null );
  setRequestUrl( QString::null );
  setTokenUrl( QString::null );
  setRefreshTokenUrl( QString::null );
  setRedirectUrl( QString::null );
  setRedirectPort( 7070 );
  setClientId( QString::null );
  setClientSecret( QString::null );
  setUsername( QString::null );
  setPassword( QString::null );
  setScope( QString::null );
  setState( QString::null );
  setApiKey( QString::null );
  setPersistToken( false );
  setAccessMethod( QgsAuthOAuth2Config::Header );
  setRequestTimeout( 30 ); // in seconds
  setQueryPairs( QVariantMap() );
}

bool QgsAuthOAuth2Config::operator==( const QgsAuthOAuth2Config &other ) const
{
  return ( other.version() == this->version()
           && other.configType() == this->configType()
           && other.grantFlow() == this->grantFlow()
           && other.name() == this->name()
           && other.description() == this->description()
           && other.requestUrl() == this->requestUrl()
           && other.tokenUrl() == this->tokenUrl()
           && other.refreshTokenUrl() == this->refreshTokenUrl()
           && other.redirectUrl() == this->redirectUrl()
           && other.redirectPort() == this->redirectPort()
           && other.clientId() == this->clientId()
           && other.clientSecret() == this->clientSecret()
           && other.username() == this->username()
           && other.password() == this->password()
           && other.scope() == this->scope()
           && other.state() == this->state()
           && other.apiKey() == this->apiKey()
           && other.persistToken() == this->persistToken()
           && other.accessMethod() == this->accessMethod()
           && other.requestTimeout() == this->requestTimeout()
           && other.queryPairs() == this->queryPairs() );
}

bool QgsAuthOAuth2Config::operator!=( const QgsAuthOAuth2Config &other ) const
{
  return  !( *this == other );
}

bool QgsAuthOAuth2Config::isValid() const
{
  return mValid;
}

// slot
void QgsAuthOAuth2Config::validateConfig( bool needsId )
{
  bool oldvalid = mValid;

  if ( mGrantFlow == AuthCode || mGrantFlow == Implicit )
  {
    mValid = ( !requestUrl().isEmpty()
               && !tokenUrl().isEmpty()
               && !clientId().isEmpty()
               && ( mGrantFlow == AuthCode ? !clientSecret().isEmpty() : true )
               && redirectPort() > 0
               && ( needsId ? !id().isEmpty() : true ) );
  }
  else if ( mGrantFlow == ResourceOwner )
  {
    mValid = ( !tokenUrl().isEmpty()
               && !clientId().isEmpty()
               && !clientSecret().isEmpty()
               && !username().isEmpty()
               && !password().isEmpty()
               && ( needsId ? !id().isEmpty() : true ) );
  }

  if ( mValid != oldvalid ) emit validityChanged( mValid );
}

bool QgsAuthOAuth2Config::loadConfigTxt(
  const QByteArray &configtxt, QgsAuthOAuth2Config::ConfigFormat format )
{
  bool res = false;

  if ( format == JSON )
  {
#if QT_VERSION < 0x050000
    QJson::Parser parser;
    QVariant variant = parser.parse( configtxt, &res );
    if ( !res )
    {
      QgsDebugMsg( QString( "Error parsing JSON: %1" ).arg( parser.errorString() ) );
      return res;
    }
    QJson::QObjectHelper::qvariant2qobject( variant.toMap(), this );
#else
    // TODO add QJsonDocument support
#endif
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
  }
  return true;
}

QByteArray QgsAuthOAuth2Config::saveConfigTxt(
  QgsAuthOAuth2Config::ConfigFormat format, bool pretty, bool *ok ) const
{
  QByteArray out;
  bool res = false;

  if ( !isValid() )
  {
    QgsDebugMsg( "FAILED, config is not valid" );
    if ( ok ) *ok = res;
    return out;
  }

  if ( format == JSON )
  {
#if QT_VERSION < 0x050000
    QVariantMap variant = QJson::QObjectHelper::qobject2qvariant( this );
    QJson::Serializer serializer;
    serializer.setIndentMode( pretty ? QJson::IndentFull : QJson::IndentCompact );
    out = serializer.serialize( variant, &res );
    if ( !res )
    {
      QgsDebugMsg( QString( "Error serializing JSON: %1" ).arg( serializer.errorMessage() ) );
      if ( ok ) *ok = res;
      return out;
    }
#else
    // TODO add Qt5 QJson* support
#endif
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
  }

  if ( ok ) *ok = res;
  return out;
}

QVariantMap QgsAuthOAuth2Config::mappedProperties() const
{
  QVariantMap vmap;
  vmap.insert( "apiKey", this->apiKey() );
  vmap.insert( "clientId", this->clientId() );
  vmap.insert( "clientSecret", this->clientSecret() );
  vmap.insert( "configType", static_cast<int>( this->configType() ) );
  vmap.insert( "description", this->description() );
  vmap.insert( "grantFlow", static_cast<int>( this->grantFlow() ) );
  vmap.insert( "id", this->id() );
  vmap.insert( "name", this->name() );
  vmap.insert( "password", this->password() );
  vmap.insert( "persistToken", this->persistToken() );
  vmap.insert( "queryPairs", this->queryPairs() );
  vmap.insert( "redirectPort", this->redirectPort() );
  vmap.insert( "redirectUrl", this->redirectUrl() );
  vmap.insert( "refreshTokenUrl", this->refreshTokenUrl() );
  vmap.insert( "accessMethod", static_cast<int>( this->accessMethod() ) );
  vmap.insert( "requestTimeout", this->requestTimeout() );
  vmap.insert( "requestUrl", this->requestUrl() );
  vmap.insert( "scope", this->scope() );
  vmap.insert( "state", this->state() );
  vmap.insert( "tokenUrl", this->tokenUrl() );
  vmap.insert( "username", this->username() );
  vmap.insert( "version", this->version() );

  return vmap;
}

// static
QByteArray QgsAuthOAuth2Config::serializeFromVariant(
  const QVariantMap &variant,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool pretty,
  bool *ok )
{
  QByteArray out;
  bool res = false;

  if ( format == JSON )
  {
#if QT_VERSION < 0x050000
    QJson::Serializer serializer;
    serializer.setIndentMode( pretty ? QJson::IndentFull : QJson::IndentCompact );
    out = serializer.serialize( variant, &res );
    if ( !res )
    {
      QgsDebugMsg( QString( "Error serializing JSON: %1" ).arg( serializer.errorMessage() ) );
      if ( ok ) *ok = res;
      return out;
    }
#else
    // TODO add Qt5 QJson* support
#endif
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
  }

  if ( ok ) *ok = res;
  return out;
}

// static
QVariantMap QgsAuthOAuth2Config::variantFromSerialized(
  const QByteArray &serial,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QVariantMap vmap;
  bool res = false;

  if ( format == JSON )
  {
#if QT_VERSION < 0x050000
    QJson::Parser parser;
    QVariant var = parser.parse( serial, &res );
    if ( !res )
    {
      QgsDebugMsg( QString( "Error parsing JSON to variant: %1" ).arg( parser.errorString() ) );
      if ( ok ) *ok = res;
      return vmap;
    }
    if ( !var.isValid() || var.isNull() )
    {
      QgsDebugMsg( QString( "Error parsing JSON to variant: %1" ).arg( "invalid or null" ) );
      if ( ok ) *ok = res;
      return vmap;
    }
    vmap = var.toMap();
    if ( vmap.isEmpty() )
    {
      QgsDebugMsg( QString( "Error parsing JSON to variantmap: %1" ).arg( "map empty" ) );
      if ( ok ) *ok = res;
      return vmap;
    }
#else
    // TODO add QJsonDocument support
#endif
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
  }

  if ( ok ) *ok = res;
  return vmap;
}

//static
bool QgsAuthOAuth2Config::writeOAuth2Config(
  const QString &filepath,
  QgsAuthOAuth2Config *config,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool pretty )
{
  bool res = false;
  QByteArray configtxt = config->saveConfigTxt( format, pretty, &res );
  if ( !res )
  {
    QgsDebugMsg( "FAILED to save config to text" );
    return false;
  }

  QFile config_file( filepath );
  QString file_path( config_file.fileName() );

  if ( config_file.open( QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text ) )
  {
    qint64 bytesWritten = config_file.write( configtxt );
    config_file.close();
    if ( bytesWritten == -1 )
    {
      QgsDebugMsg( QString( "FAILED to write config file: %1" ).arg( file_path ) );
      return false;
    }
  }
  else
  {
    QgsDebugMsg( QString( "FAILED to open for writing config file: %1" ).arg( file_path ) );
    return false;
  }

  if ( !config_file.setPermissions( QFile::ReadOwner | QFile::WriteOwner ) )
  {
    QgsDebugMsg( QString( "FAILED to set permissions config file: %1" ).arg( file_path ) );
    return false;
  }

  return true;
}

// static
QList<QgsAuthOAuth2Config*> QgsAuthOAuth2Config::loadOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QList<QgsAuthOAuth2Config*> configs = QList<QgsAuthOAuth2Config*>();
  bool res = false;
  QStringList namefilters;

  if ( format == JSON )
  {
    namefilters << "*.json";
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
    if ( ok ) *ok = res;
    return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsg( QString( "Config files found in: %1...\n%2" )
                 .arg( configdir.path(), configfiles.join( ", " ) ) );
  }
  else
  {
    QgsDebugMsg( QString( "No config files found in: %1" ).arg( configdir.path() ) );
    if ( ok ) *ok = res;
    return configs;
  }

  // Add entries
  Q_FOREACH ( const QString &configfile, configfiles )
  {
    QByteArray configtxt;
    QFile cfile( configdir.path() + "/" + configfile );
    if ( cfile.exists() )
    {
      bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugMsg( QString( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QString( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( parent );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugMsg( QString( "FAILED to load config: %1" ).arg( configfile ) );
      config->deleteLater();
      continue;
    }
    configs << config;
  }

  if ( ok ) *ok = true;
  return configs;
}

// static
QgsStringMap QgsAuthOAuth2Config::mapOAuth2Configs(
  const QString &configdirectory,
  QObject *parent,
  QgsAuthOAuth2Config::ConfigFormat format,
  bool *ok )
{
  QgsStringMap configs = QgsStringMap();
  bool res = false;
  QStringList namefilters;

  if ( format == JSON )
  {
    namefilters << "*.json";
  }
  else
  {
    QgsDebugMsg( "Unsupported output format" );
    if ( ok ) *ok = res;
    return configs;
  }

  QDir configdir( configdirectory );
  configdir.setNameFilters( namefilters );
  QStringList configfiles = configdir.entryList( namefilters );

  if ( configfiles.size() > 0 )
  {
    QgsDebugMsg( QString( "Config files found in: %1...\n%2" )
                 .arg( configdir.path(), configfiles.join( ", " ) ) );
  }
  else
  {
    QgsDebugMsg( QString( "No config files found in: %1" ).arg( configdir.path() ) );
    if ( ok ) *ok = res;
    return configs;
  }

  // Add entries
  Q_FOREACH ( const QString &configfile, configfiles )
  {
    QByteArray configtxt;
    QFile cfile( configdir.path() + "/" + configfile );
    if ( cfile.exists() )
    {
      bool ret = cfile.open( QIODevice::ReadOnly | QIODevice::Text );
      if ( ret )
      {
        configtxt = cfile.readAll();
      }
      else
      {
        QgsDebugMsg( QString( "FAILED to open config for reading: %1" ).arg( configfile ) );
      }
      cfile.close();
    }

    if ( configtxt.isEmpty() )
    {
      QgsDebugMsg( QString( "EMPTY read of config: %1" ).arg( configfile ) );
      continue;
    }

    // validate the config before caching it
    QgsAuthOAuth2Config *config = new QgsAuthOAuth2Config( parent );
    if ( !config->loadConfigTxt( configtxt, format ) )
    {
      QgsDebugMsg( QString( "FAILED to load config: %1" ).arg( configfile ) );
      config->deleteLater();
      continue;
    }
    if ( config->id().isEmpty() )
    {
      QgsDebugMsg( QString( "NO ID SET for config: %1" ).arg( configfile ) );
      config->deleteLater();
      continue;
    }
    configs.insert( config->id(), configtxt );
    config->deleteLater();
  }

  if ( ok ) *ok = true;
  return configs;
}

QgsStringMap QgsAuthOAuth2Config::mappedOAuth2ConfigsCache( const QString &extradir )
{
  QgsStringMap configs;
  bool ok = false;

  // Load from default locations
  QStringList configdirs;
  // in order of override preference, i.e. user over pkg dir
  configdirs << QgsAuthOAuth2Config::oauth2ConfigsPkgDataDir()
  << QgsAuthOAuth2Config::oauth2ConfigsUserSettingsDir();

  if ( !extradir.isEmpty() )
  {
    // configs of similar IDs in this dir will override existing in standard dirs
    configdirs << extradir;
  }

  Q_FOREACH ( QString configdir, configdirs )
  {
    QFileInfo configdirinfo( configdir );
    if ( !configdirinfo.exists() || !configdirinfo.isDir() )
    {
      continue;
    }
    QgsStringMap newconfigs = QgsAuthOAuth2Config::mapOAuth2Configs(
                                configdirinfo.canonicalFilePath(), qApp, QgsAuthOAuth2Config::JSON, &ok );
    if ( ok )
    {
      QgsStringMap::const_iterator i = newconfigs.constBegin();
      while ( i != newconfigs.constEnd() )
      {
        configs.insert( i.key(), i.value() );
        ++i;
      }
    }
  }
  return configs;
}

// static
QString QgsAuthOAuth2Config::oauth2ConfigsPkgDataDir()
{
  return QgsApplication::pkgDataPath() + "/oauth2_configs";
}

// static
QString QgsAuthOAuth2Config::oauth2ConfigsUserSettingsDir()
{
  return QgsApplication::qgisSettingsDirPath() + "/oauth2_configs";
}

// static
QString QgsAuthOAuth2Config::configTypeString( QgsAuthOAuth2Config::ConfigType configtype )
{
  switch ( configtype )
  {
    case QgsAuthOAuth2Config::Custom:
      return tr( "Custom" );
    case QgsAuthOAuth2Config::Predefined:
      return tr( "Predefined" );
  }
}

// static
QString QgsAuthOAuth2Config::grantFlowString( QgsAuthOAuth2Config::GrantFlow flow )
{
  switch ( flow )
  {
    case QgsAuthOAuth2Config::AuthCode:
      return tr( "Authorization Code" );
    case QgsAuthOAuth2Config::Implicit:
      return tr( "Implicit" );
    case QgsAuthOAuth2Config::ResourceOwner:
      return tr( "Resource Owner" );
  }
}

// static
QString QgsAuthOAuth2Config::accessMethodString( QgsAuthOAuth2Config::AccessMethod method )
{
  switch ( method )
  {
    case QgsAuthOAuth2Config::Header:
      return tr( "Header" );
    case QgsAuthOAuth2Config::Form:
      return tr( "Form (POST only)" );
    case QgsAuthOAuth2Config::Query:
      return tr( "URL Query" );
  }
}

// static
QString QgsAuthOAuth2Config::tokenCacheDirectory( bool temporary )
{
  QDir setdir( QgsApplication::qgisSettingsDirPath() );
  return  QString( "%1/oauth2-cache" ).arg( temporary ? QDir::tempPath() : setdir.canonicalPath() );
}

// static
QString QgsAuthOAuth2Config::tokenCacheFile( const QString &suffix )
{
  return QString( "authcfg-%1.ini" ).arg( !suffix.isEmpty() ? suffix : "cache" );
}
