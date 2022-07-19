#ifndef GSTDISPLAY_H
#define GSTDISPLAY_H

#include <GstBase.h>

class GstDisplay : public GstBase
{
public:
    GstDisplay(GstElement *pPipeline, GstElement *pTee);

    // GstBase interface
public:
    bool link() override;
    bool unlink() override;
    bool isRunning() override;

private:
    static GstPadProbeReturn unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

private:
    GstElement *mQueueDisplay, *mVideoConvert, *mVideosink;
    GstPad *mTeepad;
};

#endif // GSTDISPLAY_H
