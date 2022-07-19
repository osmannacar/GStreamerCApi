#include "gstwebcamsource.h"
#include <QDebug>
#include <gstdisplay/gstdisplay.h>
#include <gstdisplaycapture/gstdisplaycapture.h>
#include <gstrecord/gstrecord.h>
#include <gstudpstream/gstudpstream.h>

GstWebcamSource::GstWebcamSource()
    : mPipeline(nullptr)
    , mSource(nullptr)
    , mClockOverlay(nullptr)
    , mTee(nullptr)
    , mBus(nullptr)
{

}

GstWebcamSource::~GstWebcamSource()
{
    unlink();
}

bool GstWebcamSource::link()
{
    if(mPipeline){
        qDebug() << "GstWebcamSource::init has benn inited";
        return false;
    }

    mPipeline = gst_pipeline_new(NULL);
    if(!mPipeline){
        qDebug() << "GstWebcamSource::init Failed to create mPipeline elements";
        return false;
    }

    mSource = gst_element_factory_make("v4l2src", NULL);
    if(!mSource){
        qDebug() << "GstWebcamSource::init Failed to create mSource elements";
        gst_object_unref(mPipeline);
        mPipeline = nullptr;
        return false;
    }

    mClockOverlay = gst_element_factory_make("clockoverlay", NULL);
    if( !mClockOverlay){
        qDebug() << "GstWebcamSource::init Failed to create mClockOverlay elements";
        gst_object_unref(mPipeline);
        gst_object_unref(mSource);
        mPipeline = nullptr;
        return false;
    }


    mTee = gst_element_factory_make("tee", "tee");
    if(!mTee){
        qDebug() << "GstWebcamSource::init Failed to create mTee elements";
        gst_object_unref(mPipeline);
        gst_object_unref(mSource);
        gst_object_unref(mClockOverlay);
        mPipeline = nullptr;
        return false;
    }

    g_object_set(mClockOverlay, "time-format", "%Y-%m-%d %H:%M:%S", NULL);

    //bind element to pipeline
    gst_bin_add_many(GST_BIN(mPipeline), mSource, mClockOverlay, mTee, NULL);

    //link base pipeline
    if (!gst_element_link_many(mSource, mClockOverlay, mTee, NULL)){
        qDebug() << "GstWebcamSource::init Failed to link elements";
        gst_bin_remove_many(GST_BIN(mPipeline), mSource, mClockOverlay, mTee, NULL);
        mPipeline = nullptr;
        return false;
    }

    mBus = gst_pipeline_get_bus(GST_PIPELINE (mPipeline));
    gst_bus_add_signal_watch(mBus);
    g_signal_connect(G_OBJECT(mBus), "message", G_CALLBACK(GstWebcamSource::message_cb), this);

    gst_element_set_state(mPipeline, GST_STATE_PLAYING);

    return true;
}

bool GstWebcamSource::unlink()
{
    if(!mPipeline){
        qDebug() << "GstWebcamSource::deInit has been de inited";
        return false;
    }

    gst_bus_remove_signal_watch(mBus);

    gst_element_set_state(mSource, GST_STATE_NULL);
    gst_element_set_state(mClockOverlay, GST_STATE_NULL);
    gst_element_set_state(mTee, GST_STATE_NULL);

    gst_bin_remove_many(GST_BIN(mPipeline), mSource, mClockOverlay, mTee, NULL);

    gst_element_set_state(mPipeline, GST_STATE_PAUSED);
    gst_element_set_state(mPipeline, GST_STATE_NULL);


    gst_object_unref(GST_OBJECT(mBus));
    gst_object_unref(GST_OBJECT(mSource));
    gst_object_unref(GST_OBJECT(mClockOverlay));
    gst_object_unref(GST_OBJECT(mTee));
    gst_object_unref(GST_OBJECT(mPipeline));

    mPipeline = nullptr;
    mSource = nullptr;
    mClockOverlay = nullptr;
    mTee = nullptr;
    mBus = nullptr;

    return true;
}

bool GstWebcamSource::isRunning()
{
    if(mPipeline){
        return true;
    }
    return false;
}

gboolean GstWebcamSource::message_cb(GstBus *bus, GstMessage *message, gpointer user_data)
{
    (void)bus;
    (void)user_data;
//    GstWebcamSource * tGstWebcamSource = static_cast<GstWebcamSource*>(user_data);
    switch (GST_MESSAGE_TYPE (message)) {
    case GST_MESSAGE_ERROR:{
        GError *err = NULL;
        gchar *name, *debug = NULL;

        name = gst_object_get_path_string (message->src);
        gst_message_parse_error (message, &err, &debug);

        g_printerr ("ERROR: from element %s: %s\n", name, err->message);
        if (debug != NULL)
            g_printerr ("Additional debug info:\n%s\n", debug);

        g_error_free (err);
        g_free (debug);
        g_free (name);

        //        g_main_loop_quit (loop);
        break;
    }
    case GST_MESSAGE_WARNING:{
        GError *err = NULL;
        gchar *name, *debug = NULL;

        name = gst_object_get_path_string (message->src);
        gst_message_parse_warning (message, &err, &debug);

        g_printerr ("ERROR: from element %s: %s\n", name, err->message);
        if (debug != NULL)
            g_printerr ("Additional debug info:\n%s\n", debug);

        g_error_free (err);
        g_free (debug);
        g_free (name);
        break;
    }
    case GST_MESSAGE_EOS:{
        g_print ("Got EOS\n");
//        gst_element_set_state (tGstWebcamSource->mPipeline, GST_STATE_NULL);
//        gst_object_unref (tGstWebcamSource->mPipeline);
//        exit(0);
        break;
    }
    default:
        break;
    }

    return TRUE;
}

GstElement *GstWebcamSource::tee() const
{
    return mTee;
}

GstElement *GstWebcamSource::pipeline() const
{
    return mPipeline;
}
