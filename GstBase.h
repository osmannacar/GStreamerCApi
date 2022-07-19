#ifndef GSTBASE_H
#define GSTBASE_H

#include <gst/gst.h>

class GstBase
{
public:
    GstBase(GstElement *pPipeline, GstElement *pTee)
        : mPipeline(pPipeline)
        , mTee(pTee)
    {

    }
    virtual ~GstBase(){

    }

public:
    virtual bool link() = 0;
    virtual bool unlink() = 0;
    virtual bool isRunning() = 0;

protected:
    GstElement *mPipeline;
    GstElement *mTee;
};

#endif // GSTBASE_H
