/***************************************************************************
                              qgspasswordineedit.cpp
                              ------------------------
  begin                : August 03, 2016
  copyright            : (C) 2016 by Boundless Spatial, Inc. USA
  author               : Larry Shaffer
  email                : lshaffer at boundlessgeo dot com

  based on             : QgsFilterLineEdit by Alex Bruy
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspasswordlineedit.h"

#include <QToolButton>
#include <QStyle>


QgsPasswordLineEdit::QgsPasswordLineEdit( QWidget* parent )
    : QLineEdit( parent )
    , btnToggle( nullptr )
{
  mHiddenIcon = QIcon( ":/oauth2method/oauth2_resources/hidden.svg" );
  mShownIcon = QIcon( ":/oauth2method/oauth2_resources/visible_red.svg" );

  setPlaceholderText( tr( "Password" ) );
  setEchoMode( QLineEdit::Password) ;

  btnToggle = new QToolButton( this );
  btnToggle->setIcon( mHiddenIcon );
  btnToggle->setCheckable( true );
  btnToggle->setToolTip( tr( "Toggle password visibility" ) );
  btnToggle->setCursor( Qt::ArrowCursor );
  btnToggle->setStyleSheet( "QToolButton { border: none; padding: 0px; }" );

  connect( btnToggle, SIGNAL( toggled( bool ) ), this, SLOT( togglePassword( bool ) ) );

  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  mStyleSheet = QString( "QLineEdit { padding-right: %1px; } " )
                .arg( btnToggle->sizeHint().width() + frameWidth + 1 );

  QSize msz = minimumSizeHint();
  setMinimumSize( qMax( msz.width(), btnToggle->sizeHint().height() + frameWidth * 2 + 2 ),
                  qMax( msz.height(), btnToggle->sizeHint().height() + frameWidth * 2 + 2 ) );
}

void QgsPasswordLineEdit::resizeEvent( QResizeEvent * )
{
  QSize sz = btnToggle->sizeHint();
  int frameWidth = style()->pixelMetric( QStyle::PM_DefaultFrameWidth );
  btnToggle->move( rect().right() - frameWidth - sz.width(),
                  ( rect().bottom() + 1 - sz.height() ) / 2 );
}

void QgsPasswordLineEdit::togglePassword( bool toggled )
{
  setEchoMode(toggled ? QLineEdit::Normal : QLineEdit::Password);
  btnToggle->setIcon( toggled ? mShownIcon : mHiddenIcon );
}
