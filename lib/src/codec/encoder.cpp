// Unit Includes
#include "pxcrypt/codec/encoder.h"
#include "codec/encoder_p.h"

namespace PxCrypt
{

/*! @cond */

/* TODO: Create a base error type for encoder/decoder to handle the common error types (i.e. PSK, BPC issues,
 * and conversion from innerworking type errors), though this will require reworking Qx::Error so that it's
 * more easily inheritable in a fashion that allows sub error types. A middle ground option is to use the
 * existing technique of creating a derived, but not instantiated class based on Abstract error and then add a
 * convenient way to create derivatives of Qx::Error (the view) itself, that can then match that new derivation
 * for a kind of "sub-interface" (i.e. that new error view is easily constructable from anything derived from
 * that middle derivation and can act as the baseline return to capture multiple sub errors
 */

//===============================================================================================================
// EncoderPrivate
//===============================================================================================================

//-Constructor---------------------------------------------------------------------------------------------------
//Protected:
EncoderPrivate::EncoderPrivate() :
    mBpc(1),
    mEncoding(Encoder::Absolute),
    mPsk()
{}

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
EncoderPrivate::~EncoderPrivate() {};

/*! @endcond */

//===============================================================================================================
// Encoder
//===============================================================================================================

/*!
 *  @class Encoder <pxcrypt/codec/encoder.h>
 *
 *  @brief The Encoder class is a base class from which all PxCrypt encoders derive in order to
 *  provide serialization of binary data to portions of an arbitrary image's color channels.
 */

//-Class Enums-----------------------------------------------------------------------------------------------------
/*!
 *  @enum Encoder::Encoding
 *
 *  This enum specifies the encoding strategy utilized when encoding/decoding data, which
 *  affects several aspects of the resultant image(s).
 *
 *  @par Relative
 *  @parblock
 *  The input data is broken up into 1-7 bit-wide frames in according to the value selected for
 *  bits per channel which are then woven into existing pixel data with each frame being mapped to
 *  one channel of a given pixel. This is accomplished by applying an offset to the original color
 *  channel value that matches the magnitude of the frame. The frame is subtracted from the original
 *  value if it is greater than @c 127; otherwise, the frame is added to the original value.
 *
 *  Because the data is not stored directly in the image, but rather as the difference between the
 *  original image and the encrypted image, the is no way to know what the offset for each channel
 *  was without a point of reference, and thus the original medium image is required in order to
 *  decode the encoded data. Payloads are more securely protected using this method.
 *
 *  The effect each frame has on a given channel is directly proportional to its magnitude, with
 *  higher value frames causing greater distortion in the original image. The best case scenario
 *  occurs when the frame has a value of zero, while the worst is when the frame contains the maximum
 *  value allowed by the BPC setting. Overall this means that the choice of medium has no effect on
 *  the amount of distortion and that the degree of distortion at a given BPC is influenced entirely
 *  by the input payload when using this strategy, with ideal data consisting largely of low bits.
 *
 *  Best Case) 0
 *
 *  Worst Case) 2<SUP>N</SUP> - 1 (where N = bits per channel)
 *  @endparblock
 *
 *  @par Absolute
 *  @parblock
 *  This is the default encoding strategy.
 *
 *  Input data is broken up into frames in the same manner as the Relative strategy, but are simply
 *  inserted directly into a pixel's channel data by replacing the lowest bits up to amount allowed
 *  by the BPC value. This allows the convenience of not requiring the original medium to decode
 *  the encrypted data, but at the cost of security since the output image relies entirely on the
 *  strength of the pre shared key for protection.
 *
 *  With this strategy the potential distortion caused by each frame depends on both the input data
 *  and the selected medium, as the degree of change is dictated by how different the frame is compared
 *  to the original bits that are being replaced. The best case scenario occurs when the frames data
 *  matches the original exactly, while the worst is when it is completely different. This leads to the
 *  same range of potential impact on the original image as with the Relative strategy, but in theory
 *  means that this method can out perform said strategy on average when the variance of the payload
 *  closes matches the variance of the image's color data
 *
 *  Best Case) 0
 *
 *  Worst Case) 2<SUP>N</SUP> - 1 (where N = bits per channel)
 *  @endparblock
 *
 *  @var Encoder::Encoding Encoder::Relative
 *  Requires the original medium(s) in order to decode the encrypted data.
 *
 *  @var Encoder::Encoding Encoder::Absolute
 *  Does not require the original medium(s) in order to decode the encrypted data.
 */

//-Constructor---------------------------------------------------------------------------------------------------
//Protected:
/*! @cond */
Encoder::Encoder(std::unique_ptr<EncoderPrivate> d) : d_ptr(std::move(d)) {}
/*! @endcond */

//-Destructor---------------------------------------------------------------------------------------------------
//Public:
/*!
 *  Destroys the encoder.
 */
Encoder::~Encoder() {}
// Definition here is required so that private implementation is visible from the same TU when this is deleted

//-Instance Functions----------------------------------------------------------------------------------------------
//Public:
/*!
 *  Returns the number of bits-per-channel the encoder is configured to use.
 *
 *  @sa setBpc().
 */
quint8 Encoder::bpc() const { Q_D(const Encoder); return d->mBpc; }

/*!
 *  Returns the encoding strategy the encoder is configured to use.
 *
 *  @sa setEncoding().
 */
Encoder::Encoding Encoder::encoding() const { Q_D(const Encoder); return d->mEncoding; }

/*!
 *  Returns the key the encoder is configured to use for data scrambling.
 *
 *  @sa setPresharedKey().
 */
QByteArray Encoder::presharedKey() const { Q_D(const Encoder); return d->mPsk; }

/*!
 *  Sets the number of bits-per-channel the encoder is configured to use to @a bpc.
 *
 *  This settings directly determines the number of bits woven into each color channel of a pixel and affects
 *  total encoding capacity. A BPC of 0 will instruct the encoder to determine the optimal BPC count automatically.
 *
 *  @sa bpc().
 */
void Encoder::setBpc(quint8 bpc) { Q_D(Encoder); d->mBpc = bpc; }

/*!
 *  Sets the encoding strategy to use to @a enc.
 *
 *  @sa encoding() and Encoding.
 */
void Encoder::setEncoding(Encoding enc) { Q_D(Encoder); d->mEncoding = enc; }

/*!
 *  Sets key used for scrambling the encoding sequence to @a key.
 *
 *  This key will be required in order to decode the resultant encoded data. An empty string results in the use
 *  of a known/default encoding sequence.
 *
 *  @sa presharedKey().
 */
void Encoder::setPresharedKey(const QByteArray& key) { Q_D(Encoder); d->mPsk = key;}

}
