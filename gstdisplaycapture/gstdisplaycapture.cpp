#include "gstdisplaycapture.h"
#include <QDebug>
#include <QDateTime>
#include <QThread>


static GstDisplayCapture *G_GstDisplayCapture = nullptr;

GstDisplayCapture::GstDisplayCapture(GstElement *pPipeline, GstElement *pTee)
    : GstBase(pPipeline, pTee)
    , mQueue(nullptr)
    , mAutoVideoConvert(nullptr)
    , mPngEnc(nullptr)
    , mMultifilesink(nullptr)
    , mTeepad(nullptr)
{
    G_GstDisplayCapture = this;
}

bool GstDisplayCapture::link()
{
    if(mQueue){
        qDebug() << "GstDisplayCapture::link has been linked";
        return false;
    }

    mQueue = gst_element_factory_make("queue", "queue_screencapture");
    if(!mQueue){
        qDebug() << "GstDisplayCapture::link Failed to create mQueue elements";
        mQueue = nullptr;
        return false;
    }

    mAutoVideoConvert = gst_element_factory_make("autovideoconvert", "autovideoconvert_screencapture");
    if(!mAutoVideoConvert){
        qDebug() << "GstRecord::link Failed to create mAutoVideoConvert elements";
        gst_object_unref(mQueue);
        mQueue = nullptr;
        return false;
    }

    mPngEnc = gst_element_factory_make("pngenc", "pngenc_screencapture");
    if(!mPngEnc){
        qDebug() << "GstRecord::link Failed to create mPngEnc elements";
        gst_object_unref(mQueue);
        gst_object_unref(mAutoVideoConvert);
        mQueue = nullptr;
        return false;
    }

    mMultifilesink = gst_element_factory_make("multifilesink", "multifilesink_screencapture");
    if(!mMultifilesink){
        qDebug() << "GstRecord::link Failed to create mMultifilesink elements";
        gst_object_unref(mQueue);
        gst_object_unref(mAutoVideoConvert);
        gst_object_unref(mMultifilesink);
        mQueue = nullptr;
        return false;
    }

    QDateTime tRecordDateTime = QDateTime::currentDateTime();
    QString tFileLocation = RECORD_FOLDER_PATH + RECORD_PREFIX_NAME + tRecordDateTime.toString("yyyy-MM-dd_hh:mm:ss") + EXTENTION;

    g_object_set(G_OBJECT(mQueue), "leaky", 1, NULL);
    g_object_set(G_OBJECT(mMultifilesink), "location", tFileLocation.toStdString().c_str(), NULL);

    //bind element to pipeline
    gst_bin_add_many(GST_BIN(mPipeline), mQueue, mAutoVideoConvert, mPngEnc, mMultifilesink, NULL);

    //link base pipeline
    if (!gst_element_link_many(mQueue, mAutoVideoConvert, mPngEnc, mMultifilesink, NULL)){
        qDebug() << "GstRecord::link Failed to link elements";
        gst_bin_remove_many(GST_BIN(mPipeline), mQueue, mAutoVideoConvert, mPngEnc, mMultifilesink, NULL);
        mQueue = nullptr;
        return false;
    }

    GstPad *tSinkpad;
    GstPadTemplate *tTempl;

    tTempl = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mTee), "src_%u");
    mTeepad = gst_element_request_pad(mTee, tTempl, NULL, NULL);

    gst_element_sync_state_with_parent(mQueue);
    gst_element_sync_state_with_parent(mAutoVideoConvert);
    gst_element_sync_state_with_parent(mPngEnc);
    gst_element_sync_state_with_parent(mMultifilesink);

    tSinkpad = gst_element_get_static_pad(mQueue, "sink");
    gst_pad_link(mTeepad, tSinkpad);
    gst_object_unref(tSinkpad);

    qDebug() << "GstDisplayCapture::link_cb linked";

    return true;
}

bool GstDisplayCapture::unlink()
{
    if(!mQueue){
        qDebug() << "GstDisplayCapture::unlink has been unlinked";
        return false;
    }
    gst_pad_add_probe(mTeepad, GST_PAD_PROBE_TYPE_IDLE, GstDisplayCapture::unlink_cb, NULL, (GDestroyNotify) g_free);
    return true;
}

bool GstDisplayCapture::isRunning()
{
    if(mQueue){
        return true;
    }
    return false;
}

GstPadProbeReturn GstDisplayCapture::unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    (void)pad;
    (void)info;
    (void)user_data;

    GstPad *tSinkpad = gst_element_get_static_pad (G_GstDisplayCapture->mQueue, "sink");
    gst_pad_unlink (G_GstDisplayCapture->mTeepad, tSinkpad);
    gst_object_unref (tSinkpad);

    gst_element_send_event(G_GstDisplayCapture->mQueue, gst_event_new_eos());
    gst_element_send_event(G_GstDisplayCapture->mAutoVideoConvert, gst_event_new_eos());
    gst_element_send_event(G_GstDisplayCapture->mPngEnc, gst_event_new_eos());
    gst_element_send_event(G_GstDisplayCapture->mMultifilesink, gst_event_new_eos());

    QThread::msleep(1000);


    gst_element_set_state(G_GstDisplayCapture->mQueue, GST_STATE_NULL);
    gst_element_set_state(G_GstDisplayCapture->mAutoVideoConvert, GST_STATE_NULL);
    gst_element_set_state(G_GstDisplayCapture->mPngEnc, GST_STATE_NULL);
    gst_element_set_state(G_GstDisplayCapture->mMultifilesink, GST_STATE_NULL);

    gst_bin_remove_many(GST_BIN(G_GstDisplayCapture->mPipeline), G_GstDisplayCapture->mQueue, G_GstDisplayCapture->mAutoVideoConvert, G_GstDisplayCapture->mPngEnc, G_GstDisplayCapture->mMultifilesink, NULL);

    gst_object_unref(GST_OBJECT(G_GstDisplayCapture->mQueue));
    gst_object_unref(GST_OBJECT(G_GstDisplayCapture->mAutoVideoConvert));
    gst_object_unref(GST_OBJECT(G_GstDisplayCapture->mPngEnc));
    gst_object_unref(GST_OBJECT(G_GstDisplayCapture->mMultifilesink));

    gst_element_release_request_pad(G_GstDisplayCapture->mTee, G_GstDisplayCapture->mTeepad);
    gst_object_unref (G_GstDisplayCapture->mTeepad);

    G_GstDisplayCapture->mQueue = NULL;
    G_GstDisplayCapture->mAutoVideoConvert = NULL;
    G_GstDisplayCapture->mPngEnc = NULL;
    G_GstDisplayCapture->mMultifilesink = NULL;

    qDebug() << "GstDisplayCapture::unlink_cb unlinked";

    return GST_PAD_PROBE_REMOVE;
}
