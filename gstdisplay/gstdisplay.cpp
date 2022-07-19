#include "gstdisplay.h"
#include <QDebug>
#include <QThread>

static GstDisplay *G_GstDisplay = nullptr;

GstDisplay::GstDisplay(GstElement *pPipeline, GstElement *pTee)
    : GstBase(pPipeline, pTee)
    , mQueueDisplay(nullptr)
    , mVideoConvert(nullptr)
    , mVideosink(nullptr)
    , mTeepad(nullptr)
{
    G_GstDisplay = this;
}

bool GstDisplay::link()
{
    if(mQueueDisplay){
        qDebug() << "GstDisplay::link has been linked";
        return false;
    }

    mQueueDisplay = gst_element_factory_make("queue", "queue_display");
    if(!mQueueDisplay){
        qDebug() << "GstDisplay::link Failed to create mQueueDisplay elements";
        gst_object_unref(mQueueDisplay);
        mQueueDisplay = nullptr;
        return false;
    }

    mVideoConvert = gst_element_factory_make("videoconvert", NULL);
    if(!mVideoConvert){
        qDebug() << "GstDisplay::link Failed to create mVideoConvert elements";
        gst_object_unref(mQueueDisplay);
        gst_object_unref(mVideoConvert);
        mQueueDisplay = nullptr;
        return false;
    }

    mVideosink = gst_element_factory_make("autovideosink", NULL);
    if(!mVideosink){
        qDebug() << "GstDisplay::link Failed to create mVideosink elements";
        gst_object_unref(mQueueDisplay);
        gst_object_unref(mVideoConvert);
        gst_object_unref(mVideosink);
        mQueueDisplay = nullptr;
        return false;
    }
    //bind element to pipeline
    gst_bin_add_many(GST_BIN(mPipeline), mQueueDisplay, mVideoConvert, mVideosink, NULL);

    //link base pipeline
    if (!gst_element_link_many(mQueueDisplay, mVideoConvert, mVideosink, NULL)){
        qDebug() << "GstDisplay::link Failed to link elements";
        gst_bin_remove_many(GST_BIN(mPipeline), mQueueDisplay, mVideoConvert, mVideosink, NULL);
        mQueueDisplay = nullptr;
        return false;
    }

    GstPad *tSinkpad;
    GstPadTemplate *tTempl;

    tTempl = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(mTee), "src_%u");
    mTeepad = gst_element_request_pad(mTee, tTempl, NULL, NULL);

    gst_element_sync_state_with_parent(mQueueDisplay);
    gst_element_sync_state_with_parent(mVideoConvert);
    gst_element_sync_state_with_parent(mVideosink);

    tSinkpad = gst_element_get_static_pad(mQueueDisplay, "sink");
    gst_pad_link(mTeepad, tSinkpad);
    gst_object_unref(tSinkpad);

    qDebug() << "GstDisplay::link_cb linked";

    return true;
}

GstPadProbeReturn GstDisplay::unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
{
    (void)pad;
    (void)info;
    (void)user_data;


    GstPad *tSinkpad = gst_element_get_static_pad (G_GstDisplay->mQueueDisplay, "sink");
    gst_pad_unlink (G_GstDisplay->mTeepad, tSinkpad);
    gst_object_unref (tSinkpad);

    gst_element_send_event(G_GstDisplay->mQueueDisplay, gst_event_new_eos());
    gst_element_send_event(G_GstDisplay->mVideoConvert, gst_event_new_eos());
    gst_element_send_event(G_GstDisplay->mVideosink, gst_event_new_eos());

    QThread::msleep(1000);

    gst_element_set_state(G_GstDisplay->mQueueDisplay, GST_STATE_NULL);
    gst_element_set_state(G_GstDisplay->mVideoConvert, GST_STATE_NULL);
    gst_element_set_state(G_GstDisplay->mVideosink, GST_STATE_NULL);

    gst_bin_remove_many(GST_BIN(G_GstDisplay->mPipeline), G_GstDisplay->mQueueDisplay, G_GstDisplay->mVideoConvert, G_GstDisplay->mVideosink, NULL);

    gst_object_unref(G_GstDisplay->mQueueDisplay);
    gst_object_unref(G_GstDisplay->mVideoConvert);
    gst_object_unref(G_GstDisplay->mVideosink);

    gst_element_release_request_pad(G_GstDisplay->mTee, G_GstDisplay->mTeepad);
    gst_object_unref (G_GstDisplay->mTeepad);

    G_GstDisplay->mQueueDisplay = NULL;
    G_GstDisplay->mVideoConvert = NULL;
    G_GstDisplay->mVideosink = NULL;
    G_GstDisplay->mTeepad = NULL;

    qDebug() << "GstDisplay::unlink_cb unlinked";

    return GST_PAD_PROBE_REMOVE;
}


bool GstDisplay::unlink()
{
    if(!mQueueDisplay){
        qDebug() << "GstDisplay::unlink has been unlinked";
        return false;
    }
    gst_pad_add_probe(mTeepad, GST_PAD_PROBE_TYPE_IDLE, GstDisplay::unlink_cb, NULL, (GDestroyNotify) g_free);

    return true;
}

bool GstDisplay::isRunning()
{
    if(mQueueDisplay){
        return true;
    }
    return false;
}
