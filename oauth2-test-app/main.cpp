#include <QApplication>
#include <QFile>
#include <QDialog>
#include <QIcon>
#include <QLibraryInfo>
#include <QSettings>
#include <QTranslator>

#include "config.h"
#include "browser.h"
#include "qgsconfig.h"
#include "qgscredentialdialog.h"
#include "qgsapplication.h"
#include "qgsautheditorwidgets.h"
#include "qgsauthmanager.h"
#include "qgsauthmethodedit.h"
#include "qgsauthmethodregistry.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"

int main( int argc, char *argv[] )
{
#ifdef QGIS_LOG_FILE
  if ( QFile::exists( QString( QGIS_LOG_FILE ) ) )
  {
#ifdef Q_OS_WIN
    _putenv( QString( "%1=%2" ).arg( "QGIS_LOG_FILE", QGIS_LOG_FILE ).toUtf8().constData() );
#else
    setenv( "QGIS_LOG_FILE", QGIS_LOG_FILE, 1 );
#endif
          }
#endif

          QString qgis_prefix( "" );
#ifdef QGIS_BUILD_PATH
  QString build_path( QGIS_BUILD_PATH );
  if ( !build_path.isEmpty() )
  {
    qgis_prefix += QString( QGIS_BUILD_PATH ) + "/output/bin";
#ifdef Q_OS_MACX
    qgis_prefix += "/QGIS.app/Contents/MacOS";
#endif
  }
#else
  // this is for CMAKE_INSTALL_PREFIX from qgsconfig.h, not this project
  QString qgis_prefix( CMAKE_INSTALL_PREFIX );
#endif

  if ( !qgis_prefix.isEmpty() && QFile::exists( qgis_prefix ) )
  {
#ifdef Q_OS_WIN
    _putenv( QString( "%1=%2" ).arg( "QGIS_PREFIX_PATH", qgis_prefix.toUtf8().constData() ).toUtf8().constData() );
#else
    setenv( "QGIS_PREFIX_PATH", qgis_prefix.toUtf8().constData(), 1 );
#endif
          }

          QgsApplication app( argc, argv, true );

  QCoreApplication::setOrganizationName( QgsApplication::QGIS_ORGANIZATION_NAME );
  QCoreApplication::setOrganizationDomain( QgsApplication::QGIS_ORGANIZATION_DOMAIN );
  QCoreApplication::setApplicationName( QgsApplication::QGIS_APPLICATION_NAME );

//  QCoreApplication::setAttribute( Qt::AA_DontShowIconsInMenus, false );

//  QCoreApplication::addLibraryPath( QString( EXTRA_QGIS_PLUGIN_DIR ) );

//  QgsApplication::setPrefixPath( qgis_prefix, true );

  QgsDebugMsg( QgsApplication::showSettings() );

  // load default widget
  WebBrowser * browser = new WebBrowser();

  browser->show();
  browser->raise();
  browser->activateWindow();
  browser->move( 200, 50 );
  browser->resize( 1000, 800 );

  app.connect( &app, SIGNAL( lastWindowClosed() ), &app, SLOT( quit() ) );

  int retval = app.exec();
  delete browser;
  return retval;
}
