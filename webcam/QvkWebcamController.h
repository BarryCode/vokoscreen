#ifndef QvkWebcamController_H 
#define QvkWebcamController_H

#include "QvkWebcamWatcher.h"
#include "QvkCapturethread.h"
#include "QvkWebcamWatcher.h"
#include "QvkWebcamWindow.h"
#include "QvkWebcamBusyDialog.h"
#include "QvkSettings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QMessageBox>
#include <QMainWindow>
#include <QLabel>
#include <QDial>
#include <QRadioButton>
#include <QProcess>
#include <QTimer>

#include "ui_vokoscreen.h"

class QvkWebcamController : public QObject
{
    Q_OBJECT

public:
//   QvkWebcamController( QCheckBox *myCheckBox, QComboBox *myComboBox, QCheckBox *myMirrorCheckBox,
//					  QFrame *myRotateFrame ,QDial *myRotateDial, QRadioButton *myRadioButtonTopMiddle, QRadioButton *myRadioButtonRightMiddle, QRadioButton *myRadioButtonBottomMiddle, QRadioButton *myRadioButtonLeftMiddle );
   QvkWebcamController(Ui_screencast value);

  virtual ~QvkWebcamController();
  
  QStringList webcamList;
 
  QvkWebcamWindow *webcamWindow;
  
  
public slots:
  void webcamCloseEvent();

  
private slots:
  void webcamChangedEvent( QStringList deviceList );
  void webcamAddedEvent( QStringList deviceList, QStringList addedDevices );
  void webcamRemovedEvent( QStringList deviceList, QString removedDevice );
  void setWebcamOnOff( bool value );
  
  void setNewImage( QImage image );
  void setMirrorOnOff( bool value );
  void rotateDialclicked();
  
  void readWebcams( QStringList );
  void webcams();
  void checkBox_OnOff();

  
private:
  QvkSettings vkSettings;
  /*QFrame *rotateFrame;
  QCheckBox *checkBox;
  QComboBox *comboBox;
  QCheckBox *mirrorCheckBox;
  QDial *rotateDial;
  QRadioButton *radioButtonTopMiddle;
  QRadioButton *radioButtonRightMiddle;
  QRadioButton *radioButtonBottomMiddle;
  QRadioButton *radioButtonLeftMiddle; */
  QvkWebcamWatcher * myWebcamWatcher;
  CaptureThread *captureThread;
  bool mirrored;
  Ui_screencast myUi;

protected:
  
  
};

#endif
