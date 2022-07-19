#ifndef ENUMS_H
#define ENUMS_H
#include <QObject>
class Enums : public QObject
{
    Q_OBJECT
public:
    enum class GstProcessType : int{
        GST_DISPLAY = 0,
        GST_RECORD = 1,
        GST_UDP_STREAM = 2,
        GST_DISPLAY_CAPTURE = 3,
        GST_UNKNOWN
    };

    Q_ENUM(GstProcessType)

};

#endif // ENUMS_H
