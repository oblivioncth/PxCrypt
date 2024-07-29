#ifndef CANVAS_H
#define CANVAS_H

// Qt Includes
#include <QImage>
#include <QIODevice>

// Project Includes
#include "medium_io/frame.h"
#include "medium_io/operate/px_access.h"
#include "medium_io/operate/data_translator.h"

namespace PxCryptPrivate
{

class Canvas final : public QIODevice
{
//-Instance Variables----------------------------------------------------------------------------------------------
private:
    PxAccess mAccess;
    DataTranslator mTranslator;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Canvas(QImage* image, const Frame& frame, const QImage* refImage = nullptr);

//-Destructor---------------------------------------------------------------------------------------------------
public:
    ~Canvas();

//-Instance Functions----------------------------------------------------------------------------------------------
private:
    void _reset(); // Don't overlap with QIODevice::reset()

protected:
    qint64 readData(char* data, qint64 maxlen) override;
    qint64 skipData(qint64 maxSize) override;
    qint64 writeData(const char* data, qint64 len) override;

public:
    bool isSequential() const override;
    bool open(OpenMode mode) override;
    void close() override;
    bool atEnd() const override;
};

}

#endif // CANVAS_H
