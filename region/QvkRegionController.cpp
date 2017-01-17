
#include "QvkRegionController.h"

QvkRegionController::QvkRegionController()
{
  regionChoise = new QvkRegionChoise();
  regionRecord = new QvkRegionRecord();
  
  // Unter Ubuntu 16.04 bleibt beim starten der Hintergrund im Rahmen hängen
  // Um dem entgegezuwirken wird einfach x-mal umgeschaltet
  // Irgenwo fehlt an de richtigen Stelle ein repaint und ein update
  lockFrame( false );
  lockFrame( true );
  lockFrame( false );
  
}

QvkRegionController::~QvkRegionController()
{
}

void QvkRegionController::hide() // neu
{
  regionChoise->hide();
}



void QvkRegionController::show()
{
  regionChoise->show();
}

void QvkRegionController::close()
{
  regionChoise->close();
}

int QvkRegionController::getX()
{
  return regionChoise->geometry().x(); 
}

int QvkRegionController::getY()
{
  return regionChoise->geometry().y();
}

int QvkRegionController::getWidth()
{
  return regionChoise->getWidth();
}

int QvkRegionController::getHeight()
{
  return regionChoise->getHeight();
}

int QvkRegionController::getXRecordArea()
{
  return regionChoise->getXRecordArea();
}

int QvkRegionController::getYRecordArea()
{
  return regionChoise->getYRecordArea();
}

void QvkRegionController::lockFrame( bool status )
{
  regionRecord->setRecordGeometry( getX(), getY(), getWidth(), getHeight() );
  
  regionChoise->lockFrame( status );
  
  if ( status == true )
  {
    regionChoise->hide();
    regionRecord->show();
    qDebug() << "QvkRegionController::lockFrame( bool status ) 0";
  }
  else
  {
    regionRecord->hide();
    regionChoise->show();
    qDebug() << "QvkRegionController::lockFrame( bool status ) 1";
  }
  // regionChoise->lockFrame( status );
    
}
