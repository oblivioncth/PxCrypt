#ifndef ERRORCODE_H
#define ERRORCODE_H

enum ErrorCode{
    // Common
    NO_ERR = 0,
    INVALID_ARGS = 1,

    // Encode
    ENCODE_FAILED = 100,

    // Decode
    DECODE_FAILED = 200
};

#endif // ERRORCODE_H
