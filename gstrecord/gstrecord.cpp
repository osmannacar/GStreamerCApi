#include "gstrecord.h"
#include <QDateTime>
#include <QDebug>
#include <QThread>

static GstRecord *G_GstRecord = nullptr;

GstRecord::GstRecord(GstElement *pPipeline, GstElement *pTee)
    : GstBase(pPipeline, pTee)
    , mQueue(nullptr)
    , mVideoConvert(nullptr)
    , mEncoder(nullptr)
    , mMuxer(nullptr)
    , mFilesink(nullptr)
    , mTeepad(nullptr)
{
    G_GstRecord = this;
}

bool GstRecord::link()
{
    if(mQueue){
        qDebug() << "GstRecord::link has been linked";
        return false;
    }

    mQueue = gst_element_factory_make("queue", "queue_record");
    if(!mQueue){
        qDebug() << "GstRecord::link Failed to create mQueue elements";
        mQueue = nullptr;
        return false;
    }

    mVideoConvert = gst_element_factory_make("videoconvert", "videoconvert_record");
    if(!mVideoConvert){
        qDebug() << "GstRecord::link Failed to create mVideoConvert elements";
        gst_object_unref(mQueue);
        mQueue = nullptr;
        return false;
    }

    mEncoder = gst_element_factory_make("x264enc", "x264enc_record");
    if(!mEncoder){
        qDebug() << "GstRecord::link Failed to create mEncoder elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        mQueue = nullptr;
        return false;
    }

    mMuxer = gst_element_factory_make("mpegtsmux", NULL);
    if(!mMuxer){
        qDebug() << "GstRecord::link Failed to create mMpegtsmux elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        gst_object_unref(mEncoder);
        mQueue = nullptr;
        return false;
    }

    mFilesink = gst_element_factory_make("filesink", NULL);
    if(!mFilesink){
        qDebug() << "GstRecord::link Failed to create mFilesink elements";
        gst_object_unref(mQueue);
        gst_object_unref(mVideoConvert);
        gst_object_unref(mEncoder);
        gst_object_unref(mMuxer);
        mQueue = nullptr;
        return false;
    }

    QDateTime tRecordDateTime = QDateTime::currentDateTime();
    QString tFileLocation = RECORD_FOLDER_PATH + RECORD_PREFIX_NAME + tRecordDateTime.toString("yyyy-MM-dd_hh:mm:ss") + EXTENTION;

    g_object_set(G_OBJECT(mQueue), "leaky", 1, NULL);
    g_object_set(G_OBJECT(mEncoder), "tune", 4, NULL);
    g_object_set(G_OBJECT(mMuxer), "name", "mux", NULL);
    g_object_set(G_OBJECT(mFilesink), "location", tFileLocation.toStdString().c_str(), NULL);

    //bind element to pipeline
    gst_bin_add_many(GST_BIN(mPipeline), mQueue, mVideoConvert, mEncoder, mMuxer, mFilesink, NULL);

    //link base pipeline
    if (!gst_element_link_many(mQueue, mVideoConvert, mEncoder, mMuxer, mFilesink, NULL)){
        qDebug() << "GstRecord::link Failed to link elements";
        gst_bin_remove_many(GST_BIN(mPipeline), mQueue, mVideoConvert, mEncoder, mMuxer, mFilesink, NULL);
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
    gst_element_sync_state_with_parent(mMuxer);
    gst_element_sync_state_with_parent(mFilesink);

    tSinkpad = gst_element_get_static_pad(mQueue, "sink");
    gst_pad_link(mTeepad, tSinkpad);
    gst_object_unref(tSinkpad);

    qDebug() << "GstRecord::link_cb linked";
    return true;
}


GstPadProbeReturn GstRecord::unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    (void)pad;
    (void)info;
    (void)user_data;

    GstPad *tSinkpad = gst_element_get_static_pad (G_GstRecord->mQueue, "sink");
    gst_pad_unlink (G_GstRecord->mTeepad, tSinkpad);
    gst_object_unref (tSinkpad);

    gst_element_send_event(G_GstRecord->mQueue, gst_event_new_eos());
    gst_element_send_event(G_GstRecord->mVideoConvert, gst_event_new_eos());
    gst_element_send_event(G_GstRecord->mEncoder, gst_event_new_eos());
    gst_element_send_event(G_GstRecord->mMuxer, gst_event_new_eos());
    gst_element_send_event(G_GstRecord->mFilesink, gst_event_new_eos());

    QThread::msleep(1000);

    gst_element_set_state(G_GstRecord->mQueue, GST_STATE_NULL);
    gst_element_set_state(G_GstRecord->mVideoConvert, GST_STATE_NULL);
    gst_element_set_state(G_GstRecord->mEncoder, GST_STATE_NULL);
    gst_element_set_state(G_GstRecord->mMuxer, GST_STATE_NULL);
    gst_element_set_state(G_GstRecord->mFilesink, GST_STATE_NULL);


    gst_bin_remove_many(GST_BIN(G_GstRecord->mPipeline), G_GstRecord->mQueue, G_GstRecord->mVideoConvert, G_GstRecord->mEncoder, G_GstRecord->mMuxer, G_GstRecord->mFilesink, NULL);

    gst_object_unref(GST_OBJECT(G_GstRecord->mQueue));
    gst_object_unref(GST_OBJECT(G_GstRecord->mVideoConvert));
    gst_object_unref(GST_OBJECT(G_GstRecord->mEncoder));
    gst_object_unref(GST_OBJECT(G_GstRecord->mMuxer));
    gst_object_unref(GST_OBJECT(G_GstRecord->mFilesink));

    gst_element_release_request_pad(G_GstRecord->mTee, G_GstRecord->mTeepad);
    gst_object_unref (G_GstRecord->mTeepad);

    G_GstRecord->mQueue = NULL;
    G_GstRecord->mVideoConvert = NULL;
    G_GstRecord->mEncoder = NULL;
    G_GstRecord->mMuxer = NULL;
    G_GstRecord->mFilesink = NULL;
    G_GstRecord->mTeepad = NULL;

    qDebug() << "GstRecord::unlink_cb unlinked";

    return GST_PAD_PROBE_REMOVE;
}


bool GstRecord::unlink()
{
    if(!mQueue){
        qDebug() << "GstDisplay::unlink has been unlinked";
        return false;
    }
    gst_pad_add_probe(mTeepad, GST_PAD_PROBE_TYPE_IDLE, GstRecord::unlink_cb, NULL, (GDestroyNotify) g_free);
    return true;
}

bool GstRecord::isRunning()
{
    if(mQueue){
        return true;
    }
    return false;
}
