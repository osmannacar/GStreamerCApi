#ifndef GSTWEBCAMSOURCE_H
#define GSTWEBCAMSOURCE_H

#include <QMap>
#include <gst/gst.h>
#include "Enums.h"
#include "GstBase.h"

class GstDisplay;
class GstRecord;
class GstUdpStream;
class GstDisplayCapture;

class GstWebcamSource
{
public:
    GstWebcamSource();
    ~GstWebcamSource();

    GstElement *pipeline() const;

    GstElement *tee() const;

    // GstBase interface
public:
    bool link();
    bool unlink();
    bool isRunning();

private:
    static gboolean message_cb (GstBus * bus, GstMessage * message, gpointer user_data);

private:
    GstElement *mPipeline, *mSource, *mClockOverlay, *mTee;
    GstBus *mBus;
};

#endif // GSTWEBCAMSOURCE_H
