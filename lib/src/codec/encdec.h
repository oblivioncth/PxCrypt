#ifndef ENCDEC_P_H
#define ENCDEC_P_H

// Qt Includes
#include <QString>
#include <QCryptographicHash>
#include <QSize>
#include <QImage>

// Magic Enum
#include <magic_enum.hpp>

#define ENUM_NAME(eenum) QString(magic_enum::enum_name(eenum).data())

namespace PxCryptPrivate
{

//-Namespace Enums-------------------------------------------------------------------------------------------------
enum Channel : quint8{
    Alpha = 0,
    Red = 1,
    Green = 2,
    Blue = 3,
};

//-Namespace Variables-------------------------------------------------------------------------------------------------
constexpr auto CH_COUNT = magic_enum::enum_count<Channel>();
constexpr quint8 BPC_MIN = 1; // TODO: Can clamp to do away with these
constexpr quint8 BPC_MAX = 7;

//-Namespace Functions-------------------------------------------------------------------------------------------------
QImage standardizeImage(const QImage& img); //TODO: See if this can go somewhere else

}


#endif // ENCDEC_P_H
