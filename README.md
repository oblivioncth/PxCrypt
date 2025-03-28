# PxCrypt

PxCrypt is a C++ library that facilitates a form of digital visual steganography. More specifically, it allows for storing arbitrary binary data within the color channel data of images.

The project also includes a command-line utility through which one can evaluate the library's functionality, or simply encode/decode images as an end-user.

Overall an image's capacity for encoding is determined by its dimensions. The larger the row and column count, the more data can be stored.

[![Dev Builds](https://github.com/oblivioncth/PxCrypt/actions/workflows/build-project.yml/badge.svg?branch=dev)](https://github.com/oblivioncth/PxCrypt/actions/workflows/build-project.yml)

## Features

- Easy to use command-line encoder/decoder utility for jumping straight into digital image steganography
- Fully documented C++ library available in static and shared flavors
- Supports images across a variety of popular formats as a storage medium
- Stores 1-7 bits of information per-color-channel, per-pixel
- Optionally augments data obfuscation with basic encryption via a pre-shared key that scrambles the payload data sequence
- Optionally encode data in a manner that also requires the original, unaltered image in order to decode the payload (can be combined with a pre-shared key)
- Always produces a 32-bit RGBA images (saves as a PNG from the command-line)
- Allows for splitting payloads across multiple images, similar to split-archive formats

### Note on Security
While the this technique results in embedded data that's highly scrambled and difficult to decode, and the underlying sequence generator cannot be meaningfully sampled for state recreation in this application, it is not a CSPRNG (MT19937). The purpose of this technique is largely data obfuscation, with a basic level of encryption. **For maximum data security, pre-encrypt payloads using a cryptographically secure algorithm, like AES.**

## Library

> [!WARNING]
> While this library is in a pre-release state (i.e. < v1.0) the underlying implementation is subject to change in ways that break compatibility with older encrypted images at any time! The intention is to avoid such changes, but I cannot guarantee they will not occur until 1.0 is finalized. Should a breaking change occur, an older version of the library can always be used to decrypt the contents of older images.

Detailed documentation of this library, facilitated by Doxygen, is available at: https://oblivioncth.github.io/PxCrypt/

### Getting Started
Either grab the latest [release](https://github.com/oblivioncth/PxCrypt/releases/) or [build the library from source](https://oblivioncth.github.io/PxCrypt/index.html#autotoc_md5), and import using CMake.

Building from source is recommended as this library can easily be integrated as a dependency into your project using CMake's FetchContent. An example of this is demonstrated in the documentation.

Finally, the [Minimal Example](https://oblivioncth.github.io/PxCrypt/index.html#autotoc_md4) gives a basic overview of the lib's API.

You can also refer to the encoder/decoder utility source to get a better understanding of how to use the library.

## Encoder/Decoder Utility
This utility is able to encode and decode PxCrypt images via a command-line interface. The **encode** command takes a single arbitrary file along with an image or images as input and produces a single or multiple "magic" image(s) with the provided file encoded within, as well as the file's original name. The **decode** command takes an encoded image or images and simply reverses the process. There are various options for both commands that can be used to customize exactly how data serialization is handled.

Input image format support can vary depending on platform, but usually most popular formats are supported. The exact list on a given system can be checked using the global **-f** option.

Output images are always produced as a 32-bit RGBA PNG. **It is imperative that the encoded images produced by this utility are not edited, converted, or otherwise modified in a manner that perverts color channel data, or the original data will be lost.** This includes converting the image to a lossy format, or format with a different bit-depth per-channel. No data is encoded within the alpha channel so it is safe to discard, and the order of each color channel (i.e. RGB vs BGR) does not matter so long as 8-bits per-channel is maintained.

An optional pre-shared key (i.e. password) can be added during magic image creation in order to strengthen the security of the encoded data.

### Usage

The application uses the following syntax scheme:

    PxCrypt <global options> [command] <command options>

The order of switches within each options section does not matter.

### Example
To encode a file within an image, try:

	PxCrypt encode -i "example/file/path.txt" -m "medium/image/path.jpg"

then to recover the data

	PxCrypt decode -i "example/file/path_enc.png"

The original `"example/file/path.txt"` file will need to be moved before using the `decode` command as the utility will not overwrite files that already exist.

Encoding data with a pre-shared key is recommended as anyone with this utility can decode keyless images. Using the "Relative" encoding type is even better.

### All Commands/Options

#### Global Options:
 -  **-h | --help | -?:** Prints usage information
 -  **-v | --version:** Prints the current version of the tool
 - **-f | --formats:** Prints the image formats supported by the tool (input only)

#### Commands:
**encode** - Stores a file within the color channel data of an image

Options:
 -  **-i | --input:** Path to the input file to encode
 -  **-o | --output:** Path to the encoded output file. Defaults to the input path with a '_enc' suffix
 - **-m | --medium:** Path to the image in which to encode the file, or a directory of images to use for a multi-part encode
 - **-d | --density:** How many bits-per-channel to use when encoding the image (auto | 1-7). Defaults to 'auto'
 - **-k | --key:** An optional key/password to require in order to decode the encoded image
 - **-t | --type:** "The type of encoding to use, choose between 'Relative' and 'Absolute' (defaults to Absolute)

Requires:
**-i** and **-m**

*Notes:*
See the documentation for [PxCrypt::Encoder::Encoding](https://oblivioncth.github.io/PxCrypt/classPxCrypt_1_1Encoder.html#add57a5880fd161dfd3ae3943adb9aed3) for the differences between the encoding types. The gist is that 'Relative' will require the original medium image in order to decode the encoded data, and is therefore can be more secure, while 'Absolute' does not.

--------------------------------------------------------------------------------

**decode** - Retrieves the original file from an encoded image

Options:
 -  **-i | --input:** Path to the encoded image to decode
 -  **-o | --output:** Directory to place the decoded output. Defaults to the input path's directory
 - **-m | --medium:** Path to the original image used to encode the data (ignored for encoding types other than 'Relative')
 - **-k | --key:** The key for images protected with one

Requires:
**-i**

--------------------------------------------------------------------------------

**measure** - Determine how much data can be encoded within a given image

Options:
 -  **-i | --input:** Path to the image to measure
 -  **-f | --filename:** Name of potential payload file. Defaults to 'filename.txt'

Requires:
**-i**

*Notes:*
A filename is needed since the original name of encoded files are stored within PxCrypt images, and therefore consume some of their capacity.

--------------------------------------------------------------------------------

## Image Artifacts
It's surprising how much information can be encoded within an image while inducing little-to-no perceptible distortion.

While the degree to which an image becomes visually distorted when used as an encoding medium is partially influenced by the content of the payload, it is largely a function of how densely the payload is packed within the image.

In this regard there are two factors: How much data is allocated per channel, and how many pixels are allocated for encoded data.

The first metric, known as bits-per-channel, or BPC (which is set with the option **-d** of the utility), determines the maximum distortion severity of a single pixel, while the second affects the overall proximity of distortion artifacts.

The following images demonstrate the effect the encoding process has on an image at various BPC levels and overall capacity usage (i.e. both factors), using the Absolute encoding type:

![Distortion at 1 BPC & 20%, 50%, 80% Capacity](https://i.imgur.com/AoMad91.png)
BPC 1 - 20% Capacity (Left), 50% Capacity (Center), 80% Capacity (Right)

![Distortion at 1 BPC & 20%, 50%, 80% Capacity](https://i.imgur.com/rrt7TMb.png)
BPC 3 - 20% Capacity (Left), 50% Capacity (Center), 80% Capacity (Right)

![Distortion at 1 BPC & 20%, 50%, 80% Capacity](https://i.imgur.com/Jv8wtBq.png)
BPC 5 - 20% Capacity (Left), 50% Capacity (Center), 80% Capacity (Right)

![Distortion at 1 BPC & 20%, 50%, 80% Capacity](https://i.imgur.com/waJEUsd.png)
BPC 7 - 20% Capacity (Left), 50% Capacity (Center), 80% Capacity (Right)

## Source

### Summary

 - C++20
 - CMake >= 3.23.0
 - Targets:
	 - Windows 10+
	 - Linux

### Dependencies
- Qt6
- [Qx](https://github.com/oblivioncth/Qx/)
- [Neargye's Magic Enum](https://github.com/Neargye/magic_enum)
- [OBCMake](https://github.com/oblivioncth/OBCmake) (build script support, fetched automatically)
- [Doxygen](https://www.doxygen.nl/)  (for documentation)

## Pre-built Releases/Artifacts

Releases and some workflows currently provide builds of PxCrypt in various combinations of platforms and compilers. View the repository [Actions](https://github.com/oblivioncth/PxCrypt/actions) or [Releases](https://github.com/oblivioncth/PxCrypt/releases) to see examples

### Details
The source for this project is managed by a sensible CMake configuration that allows for straightforward compilation and consumption of its target(s), either as a sub-project or as an imported package. All required dependencies except for Qt6 and Doxygen are automatically acquired via CMake's FetchContent mechanism.

The configuration of this projects supports consumption both via find_package() and FetchContent.

See the *Packaging* and *Building From Source* sections of the [documentation](https://oblivioncth.github.io/PxCrypt/) for a detailed overview of the various CMake options, targets, install components, etc.