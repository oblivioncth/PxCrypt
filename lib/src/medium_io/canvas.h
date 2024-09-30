#ifndef CANVAS_H
#define CANVAS_H

// Qt Includes
#include <QImage>
#include <QIODevice>

// Project Includes
#include "pxcrypt/codec/encoder.h"
#include "medium_io/operate/meta_access.h"
#include "medium_io/operate/px_access.h"
#include "medium_io/operate/data_translator.h"

using namespace Qt::StringLiterals;

namespace PxCryptPrivate
{

class Canvas final : public QIODevice
{
//-Aliases----------------------------------------------------------------------------------------------------------
private:
    using Encoding = PxCrypt::Encoder::Encoding;

public:
    using metavalue_t = quint8;

//-Class Variables----------------------------------------------------------------------------------------------
private:
    static inline const QByteArray DEFAULT_SEED = "The best and most secure seed that is possible to exist!"_ba;

//-Instance Variables----------------------------------------------------------------------------------------------
private:
    QSize mSize;
    MetaAccess mMetaAccess;
    PxAccess mPxAccess;
    DataTranslator mTranslator;

//-Constructor---------------------------------------------------------------------------------------------------
public:
    Canvas(QImage& image, const QByteArray& psk = {});

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
    // IO
    bool isSequential() const override;
    bool open(OpenMode mode) override;
    void close() override;
    bool atEnd() const override;

    // Other
    metavalue_t bpc() const;
    Encoding encoding() const;

    void setBpc(metavalue_t bpc);
    void setEncoding(Encoding enc);
    void setReference(const QImage* ref = nullptr);
};

}

#endif // CANVAS_H
