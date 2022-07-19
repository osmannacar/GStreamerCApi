#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include <gst/gst.h>
#include <gstmanager/gstmanager.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    gst_init (&argc, &argv);

    GstManager tGstManager;
    qDebug() << "GstManager::init :" << tGstManager.init();
    qDebug() << "GstManager::start GST_DISPLAY:" << tGstManager.start(Enums::GstProcessType::GST_DISPLAY);
    qDebug() << "GstManager::start GST_RECORD:" << tGstManager.start(Enums::GstProcessType::GST_RECORD);
    qDebug() << "GstManager::start GST_UDP_STREAM:" << tGstManager.start(Enums::GstProcessType::GST_UDP_STREAM);
    qDebug() << "GstManager::start GST_DISPLAY_CAPTURE:" << tGstManager.start(Enums::GstProcessType::GST_DISPLAY_CAPTURE);


    //    QTimer timerRecord;
    //    QTimer timerStream;
    //    QTimer timerDisplayCapture;

    //    QObject::connect(&timerRecord, &QTimer::timeout, [&](){
    //        if(tGstSourceManager.isRunning(Enums::GstProcessType::GST_RECORD)) {
    //            tGstSourceManager.stop(Enums::GstProcessType::GST_RECORD);
    //        }
    //        else {
    //            tGstSourceManager.start(Enums::GstProcessType::GST_RECORD);
    //        }
    //    });
    //    QObject::connect(&timerStream, &QTimer::timeout, [&](){
    //        if(tGstSourceManager.isRunning(Enums::GstProcessType::GST_UDP_STREAM)) {
    //            tGstSourceManager.stop(Enums::GstProcessType::GST_UDP_STREAM);
    //        }
    //        else {
    //            tGstSourceManager.start(Enums::GstProcessType::GST_UDP_STREAM);
    //        }
    //    });
    //    QObject::connect(&timerDisplayCapture, &QTimer::timeout, [&](){
    //        if(!tGstSourceManager.isRunning(Enums::GstProcessType::GST_DISPLAY_CAPTURE)) {
    //            tGstSourceManager.start(Enums::GstProcessType::GST_DISPLAY_CAPTURE);
    //        }

    //        QTimer::singleShot(200, [&]{
    //            if(tGstSourceManager.isRunning(Enums::GstProcessType::GST_DISPLAY_CAPTURE)) {
    //                tGstSourceManager.stop(Enums::GstProcessType::GST_DISPLAY_CAPTURE);
    //            }
    //        });
    //    });

    //    timerRecord.start(5000);
    //    timerStream.start(5000);
    //    timerDisplayCapture.start(5000);


    return a.exec();
}
