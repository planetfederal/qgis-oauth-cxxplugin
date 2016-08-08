/***************************************************************************
                              qgspasswordineedit.h
                              ------------------------
  begin                : August 03, 2016
  copyright            : (C) 2016 by Boundless Spatial, Inc. USA
  author               : Larry Shaffer
  email                : lshaffer at boundlessgeo dot com

  based on             : QgsPasswordLineEdit by Alex Bruy
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPASSWORDLINEEDIT_H
#define QGSPASSWORDLINEEDIT_H

#include <QLineEdit>
#include <QIcon>


class QToolButton;

/** \ingroup gui
 * Lineedit with password show toggle button
 **/
class GUI_EXPORT QgsPasswordLineEdit : public QLineEdit
{
  Q_OBJECT

  public:
    QgsPasswordLineEdit( QWidget* parent = nullptr );

  protected:
    void resizeEvent( QResizeEvent* e ) override;

  private slots:
    void togglePassword( bool toggled );

  private:
    QToolButton *btnToggle;
    QString mStyleSheet;
    QIcon mHiddenIcon;
    QIcon mShownIcon;
};

#endif // QGSPASSWORDLINEEDIT_H
