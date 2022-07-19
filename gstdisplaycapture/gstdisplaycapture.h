#ifndef GSTDISPLAYCAPTURE_H
#define GSTDISPLAYCAPTURE_H

#include <GstBase.h>
#include <QStandardPaths>

class GstDisplayCapture : public GstBase
{
public:
    GstDisplayCapture(GstElement *pPipeline, GstElement *pTee);

    // GstBase interface
public:
    bool link() override;
    bool unlink() override;
    bool isRunning() override;

private:
    static GstPadProbeReturn unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

private:
    GstElement *mQueue, *mAutoVideoConvert, *mPngEnc, *mMultifilesink;
    GstPad *mTeepad;

private:
    inline static const QString RECORD_FOLDER_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    inline static const QString RECORD_PREFIX_NAME = "/Sceenshot_";
    inline const static QString EXTENTION = QString(".png");
};

#endif // GSTDISPLAYCAPTURE_H
