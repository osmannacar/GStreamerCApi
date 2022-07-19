#ifndef GSTMANAGER_H
#define GSTMANAGER_H

#include <QObject>
#include <QMap>
#include "Enums.h"
#include "GstBase.h"

class GstRecord;
class GstDisplay;
class GstUdpStream;
class GstWebcamSource;
class GstDisplayCapture;

class GstManager : public QObject
{
 Q_OBJECT
public:
    explicit GstManager(QObject *parent = nullptr);
    ~GstManager();

    bool init();

    bool isRunning(Enums::GstProcessType pProcessType);

public slots:
    bool start(Enums::GstProcessType pProcessType);
    bool stop(Enums::GstProcessType pProcessType);

private:
    GstRecord *mGstRecord;
    GstDisplay *mGstDisplay;
    GstUdpStream *mGstUdpStream;
    GstWebcamSource *mGstWebcamSource;
    GstDisplayCapture *mGstDisplayCapture;

private:
    QMap<Enums::GstProcessType, GstBase*> mProcessContainer;
};

#endif // GSTMANAGER_H
