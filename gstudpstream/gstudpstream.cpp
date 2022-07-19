#include "gstudpstream.h"

#include <QThread>

static GstUdpStream *G_GstUdpStream = nullptr;

GstUdpStream::GstUdpStream(GstElement *pPipeline, GstElement *pTee)
    : GstBase(pPipeline, pTee)
    , mQueue(nullptr)
    , mEncoder(nullptr)
    , mVideoConvert(nullptr)
    , mRtph264pay(nullptr)
    , mUdpsink(nullptr)
    , mTeepad(nullptr)
{
    G_GstUdpStream = this;
}

bool GstUdpStream::link()
{
    if(mQueue){
        qDebug() << "GstUdpStream::link has been linked";
        return false;
    }

    mQueue = gst_element_factory_make("queue", "queue_stream");
    if(!mQueue){
        qDebug() << "GstUdpStream::link Failed to create mQueue elements";
        gst_object_unref(mQueue);
        mQueue = nullptr;
        return false;
    }

    mVideoConvert = gst_element_factory_make("videoconvert", "videoconvert_stream");
    if(!mVideoConvert){
        qDebug() << "GstUdpStream::link Failed to create mVideoConvert elements";
        gst_object_unref(mQueue);
        mQueue = nullptr;
        return false;
    }

    mEncoder = gst_element_factory_make("x264enc", "x264enc_stream");
    if(!mEncoder){
        qDebug() << "GstUdpStream::link Failed to create mEncoder elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        mQueue = nullptr;
        return false;
    }

    mRtph264pay = gst_element_factory_make("rtph264pay", NULL);
    if(!mRtph264pay){
        qDebug() << "GstUdpStream::link Failed to create mRtph264pay elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        gst_object_unref(mEncoder);
        mQueue = nullptr;
        return false;
    }

    mUdpsink = gst_element_factory_make("udpsink", NULL);
    if(!mUdpsink){
        qDebug() << "GstUdpStream::link Failed to create mUdpsink elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        gst_object_unref(mEncoder);
        gst_object_unref(mRtph264pay);
        mQueue = nullptr;
        return false;
    }

    g_object_set(G_OBJECT(mQueue), "leaky", 1, NULL);
    g_object_set(G_OBJECT(mEncoder), "tune", 4, NULL);
    g_object_set(G_OBJECT(mUdpsink), "host", UDP_STREAM_IP.toString().toStdString().c_str(), NULL);
    g_object_set(G_OBJECT(mUdpsink), "port", UDP_STREAM_PORT, NULL);
    g_object_set(G_OBJECT(mUdpsink), "auto-multicast", UDP_STREAM_IP.isMulticast(), NULL);
    g_object_set(G_OBJECT(mUdpsink), "multicast-iface", UDP_STREAM_MULTICAST_IFACE.toStdString().c_str(), NULL);

    //bind element to pipeline
    gst_bin_add_many(GST_BIN(mPipeline), mQueue, mVideoConvert, mEncoder, mRtph264pay, mUdpsink, NULL);

    //link base pipeline
    if (!gst_element_link_many(mQueue, mVideoConvert, mEncoder, mRtph264pay, mUdpsink, NULL)){
        qDebug() << "GstUdpStream::link Failed to link elements";
        gst_bin_remove_many(GST_BIN(mPipeline), mQueue, mVideoConvert, mEncoder, mRtph264pay, mUdpsink, NULL);
        mQueue = nullptr;
        return false;
    }

    GstPad *tSinkpad;
    GstPadTemplate *tTempl;

    tTempl = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mTee), "src_%u");
    mTeepad = gst_element_request_pad(mTee, tTempl, NULL, NULL);

    gst_element_sync_state_with_parent(mQueue);
    gst_element_sync_state_with_parent(mVideoConvert);
    gst_element_sync_state_with_parent(mEncoder);
    gst_element_sync_state_with_parent(mRtph264pay);
    gst_element_sync_state_with_parent(mUdpsink);

    tSinkpad = gst_element_get_static_pad(mQueue, "sink");
    gst_pad_link(mTeepad, tSinkpad);
    gst_object_unref(tSinkpad);

    qDebug() << "GstUdpStream::link_cb linked";
    return true;
}
 GstPadProbeReturn GstUdpStream::unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    (void)pad;
    (void)info;
    (void)user_data;

    GstPad *tSinkpad = gst_element_get_static_pad (G_GstUdpStream->mQueue, "sink");
    gst_pad_unlink (G_GstUdpStream->mTeepad, tSinkpad);
    gst_object_unref (tSinkpad);

    gst_element_send_event(G_GstUdpStream->mQueue, gst_event_new_eos());
    gst_element_send_event(G_GstUdpStream->mVideoConvert, gst_event_new_eos());
    gst_element_send_event(G_GstUdpStream->mEncoder, gst_event_new_eos());
    gst_element_send_event(G_GstUdpStream->mRtph264pay, gst_event_new_eos());
    gst_element_send_event(G_GstUdpStream->mUdpsink, gst_event_new_eos());

    QThread::msleep(1000);

    gst_element_set_state(G_GstUdpStream->mQueue, GST_STATE_NULL);
    gst_element_set_state(G_GstUdpStream->mVideoConvert, GST_STATE_NULL);
    gst_element_set_state(G_GstUdpStream->mEncoder, GST_STATE_NULL);
    gst_element_set_state(G_GstUdpStream->mRtph264pay, GST_STATE_NULL);
    gst_element_set_state(G_GstUdpStream->mUdpsink, GST_STATE_NULL);

    gst_bin_remove_many(GST_BIN(G_GstUdpStream->mPipeline), G_GstUdpStream->mQueue, G_GstUdpStream->mVideoConvert, G_GstUdpStream->mEncoder, G_GstUdpStream->mRtph264pay, G_GstUdpStream->mUdpsink, NULL);

    gst_object_unref(G_GstUdpStream->mQueue);
    gst_object_unref(G_GstUdpStream->mVideoConvert);
    gst_object_unref(G_GstUdpStream->mEncoder);
    gst_object_unref(G_GstUdpStream->mRtph264pay);
    gst_object_unref(G_GstUdpStream->mUdpsink);

    gst_element_release_request_pad(G_GstUdpStream->mTee, G_GstUdpStream->mTeepad);
    gst_object_unref (G_GstUdpStream->mTeepad);

    G_GstUdpStream->mQueue = NULL;
    G_GstUdpStream->mEncoder = NULL;
    G_GstUdpStream->mVideoConvert = NULL;
    G_GstUdpStream->mRtph264pay = NULL;
    G_GstUdpStream->mUdpsink = NULL;
    G_GstUdpStream->mTeepad = NULL;

    qDebug() << "GstUdpStream::unlink_cb unlinked";

    return GST_PAD_PROBE_REMOVE;
}


bool GstUdpStream::unlink()
{
    if(!mQueue){
        qDebug() << "GstUdpStream::unlink has been unlinked";
        return false;
    }
    gst_pad_add_probe(mTeepad, GST_PAD_PROBE_TYPE_IDLE, GstUdpStream::unlink_cb, NULL, (GDestroyNotify) g_free);
    return true;
}

bool GstUdpStream::isRunning()
{
    if(mQueue){
        return true;
    }
    return false;
}
