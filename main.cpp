/* vokoscreen - A desktop recorder
 * Copyright (C) 2011-2014 Volker Kohaupt
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301 USA
 */

#include "screencast.h"

#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QtSingleApplication>


int main(int argc, char** argv)
{
    QtSingleApplication app( "vokoscreen", argc, argv);

    QStringList arguments = QApplication::instance()->arguments();
    for( int i = 1; i < arguments.count(); ++i )
    {
      if ( arguments[ i ] == "--help" )
      {
         qDebug() << "Usage: vokoscreen [OPTIONS]";
         qDebug( " " );
         qDebug() << "Options:";
         qDebug() << "  --help              Show this help message";
	 qDebug( " " );
	 return close( 0 );
      }
    }
    
    qDebug() << "[vokoscreen] For comanndline option take: vokoscreen --help";
    
    if (app.isRunning())
    {
      QString msg = "vokoscreen can be started only once";
      qDebug() << "[vokoscreen]" << msg;
      QMessageBox::StandardButton ret = QMessageBox::information( NULL,
                             "Info",
                             msg,
                             QMessageBox::Close );
      (void)ret;
      return !app.sendMessage("aa");
    }
    
    QTranslator * qtTranslator = new QTranslator();
    qtTranslator->load( "qt_" + QLocale::system().name(), QLibraryInfo::location( QLibraryInfo::TranslationsPath ) );
    app.installTranslator( qtTranslator );

    QTranslator translator;
    translator.load( "vokoscreen_" + QLocale::system().name(), ":/language" );
    app.installTranslator( &translator );
    
    screencast foo;
      app.setActivationWindow( &foo );
    foo.show();

    return app.exec();
}
