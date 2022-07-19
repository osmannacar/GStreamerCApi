#ifndef GSTUDPSTREAM_H
#define GSTUDPSTREAM_H

#include <GstBase.h>
#include <QHostAddress>

class GstUdpStream : public GstBase
{
public:
    GstUdpStream(GstElement *pPipeline, GstElement *pTee);

    // GstBase interface
public:
    bool link() override;
    bool unlink() override;
    bool isRunning() override;

private:
    static GstPadProbeReturn unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

private:
    GstElement *mQueue, *mVideoConvert, *mEncoder, *mRtph264pay, *mUdpsink;
    GstPad *mTeepad;

private:
    const static int UDP_STREAM_PORT = 1234;
    inline const static QHostAddress UDP_STREAM_IP = QHostAddress("10.2.1.126");
    inline const static QString UDP_STREAM_MULTICAST_IFACE = QString("eno1");
};

#endif // GSTUDPSTREAM_H
