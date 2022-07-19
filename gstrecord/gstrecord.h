#ifndef GSTRECORD_H
#define GSTRECORD_H

#include <GstBase.h>
#include <QStandardPaths>
#include <QString>

class GstRecord : public GstBase
{
public:
    GstRecord(GstElement *pPipeline, GstElement *pTee);

    // GstBase interface
public:
    bool link() override;
    bool unlink() override;
    bool isRunning() override;

private:
    static GstPadProbeReturn unlink_cb(GstPad *pad, GstPadProbeInfo *info, gpointer user_data);

private:
    GstElement *mQueue, *mVideoConvert, *mEncoder, *mMuxer, *mFilesink;
    GstPad *mTeepad;

private:
    inline static const QString RECORD_FOLDER_PATH = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    inline static const QString RECORD_PREFIX_NAME = "/RECORD_";
    inline const static QString EXTENTION = QString(".ts");
};

#endif // GSTRECORD_H
