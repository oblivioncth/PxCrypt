# PxCrypt
Unaware of what's right in front of you

# WIP

Store a single or two byte value in the header that denotes the type/version of the magic image.

The primary form is the one that directly adds/subtracts the payload to/from each color channel and requires the original image to decrypt. This is the most secure and most efficient in terms of how much the image needs to be distorted for a given amount of data.

A second form could be one where the data is directly replaces the bits of each channel starting from least significant. The header of the format will need to be stored unencrpyted in some standard fashion (e.g. the first 1-2 bits of each channel of the 4 pixels in each corner
of the image) so that the decrypter knows how many bits per channel were stored. This is the least secure and most destructive, but has the covenience of only relying on a password.
