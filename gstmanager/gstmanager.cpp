#include "gstmanager.h"
#include "gstsource/gstwebcamsource.h"
#include "gstrecord/gstrecord.h"
#include "gstdisplay/gstdisplay.h"
#include "gstudpstream/gstudpstream.h"
#include "gstdisplaycapture/gstdisplaycapture.h"

#include <QDebug>

GstManager::GstManager(QObject *parent)
    : QObject(parent)
    , mGstRecord(nullptr)
    , mGstDisplay(nullptr)
    , mGstUdpStream(nullptr)
    , mGstWebcamSource(nullptr)
    , mGstDisplayCapture(nullptr)
{

}

GstManager::~GstManager()
{
    for(auto tIter = mProcessContainer.begin(); tIter != mProcessContainer.end(); ++tIter){
        tIter.value()->unlink();
    }
    qDeleteAll(mProcessContainer);
    mProcessContainer.clear();

    delete mGstWebcamSource;
}

bool GstManager::init()
{
    if(mGstWebcamSource){
        qDebug() << "GstManager::init has been inited";
        return false;
    }

    mGstWebcamSource = new GstWebcamSource();
    if(!mGstWebcamSource->link()){
        qDebug() << "GstManager::init error while linking GstWebcamSource";
        return false;
    }
    mGstRecord = new GstRecord(mGstWebcamSource->pipeline(), mGstWebcamSource->tee());
    mGstDisplay = new GstDisplay(mGstWebcamSource->pipeline(), mGstWebcamSource->tee());
    mGstUdpStream = new GstUdpStream(mGstWebcamSource->pipeline(), mGstWebcamSource->tee());
    mGstDisplayCapture = new GstDisplayCapture(mGstWebcamSource->pipeline(), mGstWebcamSource->tee());

    mProcessContainer.insert(Enums::GstProcessType::GST_DISPLAY, mGstDisplay);
    mProcessContainer.insert(Enums::GstProcessType::GST_RECORD, mGstRecord);
    mProcessContainer.insert(Enums::GstProcessType::GST_UDP_STREAM, mGstUdpStream);
    mProcessContainer.insert(Enums::GstProcessType::GST_DISPLAY_CAPTURE, mGstDisplayCapture);

    return true;
}

bool GstManager::isRunning(Enums::GstProcessType pProcessType)
{
    auto tIter = mProcessContainer.find(pProcessType);
    if(tIter == mProcessContainer.end()){
        qDebug() << "GstManager::isRunning process type did not find, pProcessType:" << pProcessType;
        return false;
    }
    return tIter.value()->isRunning();
}

bool GstManager::start(Enums::GstProcessType pProcessType)
{
    auto tIter = mProcessContainer.find(pProcessType);
    if(tIter == mProcessContainer.end()){
        qDebug() << "GstManager::start process type did not find, pProcessType:" << pProcessType;
        return false;
    }
    return tIter.value()->link();
}

bool GstManager::stop(Enums::GstProcessType pProcessType)
{
    auto tIter = mProcessContainer.find(pProcessType);
    if(tIter == mProcessContainer.end()){
        qDebug() << "GstManager::stop process type did not find, pProcessType:" << pProcessType;
        return false;
    }
    return tIter.value()->unlink();
}
