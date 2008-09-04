#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include <QActionGroup>
#include <QSignalMapper>

#include "wpsstate.h"
#include "ui_mainwindow.h"
#include "wpsstate.h"
#include "qwpsstate.h"
#include "qtrackstate.h"
#include "qwpsdrawer.h"

class QWpsEditorWindow : public QMainWindow, public Ui::MainWindow {
    Q_OBJECT
    
    QWpsState wpsState;
    QTrackState trackState;
    QPointer<QWpsDrawer> drawer;

    QHash<int, QAction*> actAudios;
    QActionGroup         *actGroupAudios;
    QSignalMapper        *audiosSignalMapper;

    QHash<QString,QAction *> actTargets;
    QActionGroup             *actGroupTargets;
    QSignalMapper            *targetsSignalMapper;
    
protected:
    void connectActions();
public:
    QWpsEditorWindow( QWidget * parent = 0, Qt::WFlags f = 0 );
    void logMsg(QString s);
private slots:
    void slotOpenWps();
    void slotVerboseLevel();
    void slotWpsStateChanged(wpsstate);
    void slotTrackStateChanged(trackstate);

    void slotUpdatePlainWps();
    void slotPlainDocModChanged(bool m);
    void slotSetTarget(const QString &);

signals:
    void signalAudioStatusChanged(int);
    void signalSetTarget(const QString &);

};
#endif




