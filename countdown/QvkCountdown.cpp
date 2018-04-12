#include "QvkCountdown.h"
#include <QDesktopWidget>
#include <QTest>
#include <QTimer>

QvkCountdown::QvkCountdown( int value )
{
    countValue = value;
    gradValue = 0;
  
    QDesktopWidget *desk = QApplication::desktop();
    int Width = 300;
    int Height = 300;;
    int x = ( desk->screenGeometry().width() / 2 ) - ( Width / 2 );
    int y = ( desk->screenGeometry().height() / 2 ) -( Height / 2 );
  
    setGeometry( x, y, Width, Height );
    setWindowFlags( Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint );
    setAttribute( Qt::WA_TranslucentBackground, true );
    show();
    
    timer = new QTimer( this );
    connect( timer, SIGNAL( timeout() ), this, SLOT( updateTimer() ) ); 
    timer->start( 1000 );
    
    animationTimer = new QTimer( this );
    connect( animationTimer, SIGNAL( timeout() ), this, SLOT( updateAnimationTimer() ) ); 
    animationTimer->start( 25 );

    
    // Mit screencast erst beginnen, also hier warten bis 0 erreicht ist 
    while ( countValue > 0 )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents );     
        QTest::qSleep( 50 );
    }
    
    // Der Desktopanimation "Langsames ausblenden" entgegenwirken
    clearMask();
    QRegion RegionWidget( x, y, width(), height() );
    QRegion RegionSpace( x+1, y+1, width()-2, height()-2 );
    QRegion RegionNeu = RegionWidget.subtracted( RegionSpace );
    setMask( RegionNeu );
    
    close();
}

QvkCountdown::~QvkCountdown()
{
}


void QvkCountdown::updateTimer()
{
  gradValue = 0;
  countValue--;

  if ( countValue == 0 )
  {
    timer->stop();
    animationTimer->stop();
  }
}

void QvkCountdown::updateAnimationTimer()
{
  gradValue = gradValue - 20;
  update();
}


void QvkCountdown::paintEvent( QPaintEvent *event )
{
  (void)event;
  painter.begin( this );
    painter.setRenderHints( QPainter::Antialiasing, true );
    QPen pen;
    QBrush brush;
    
    brush.setColor( Qt::darkGray );
    brush.setStyle( Qt::SolidPattern );
    pen.setWidth( 0 );
    pen.setColor( Qt::darkGray );
    painter.setBrush( brush );
    painter.setPen( pen );
    painter.setOpacity( 0.3 );
    painter.drawPie( 0, 0, 300, 300, 90*16, gradValue*16 );

    painter.setOpacity( 1.0 );
    pen.setColor( Qt::darkGray );
    pen.setWidth( 6 );
    painter.setPen( pen );
    brush.setStyle( Qt::NoBrush );
    painter.setBrush( brush );
    painter.drawEllipse( QPoint( width()/2, height()/2), 125-3, 125-3 );
    painter.drawEllipse( QPoint( width()/2, height()/2), 100, 100 );
    painter.drawLine( 0, height()/2, width(), height()/2 );
    painter.drawLine( width()/2, 0, width()/2, height() );
    
    int fontSize = 110;
    QFont font;
    font.setPointSize( fontSize );
    painter.setFont( font );
    painter.setPen( Qt::red );
    QFontMetrics fontMetrics( font );
    int fontWidth = fontMetrics.width( QString::number( countValue ) );
    painter.drawText( width()/2-fontWidth/2, height()/2+fontSize/2, QString::number( countValue ) );
  painter.end();
}
