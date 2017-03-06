# NAS2D: Core

*NAS2D* is an open source, object oriented 2D game development framework written in portable C++. It was designed to make the development of games easier by providing a high-level interface. It handles the low-level tasks of setting up the video display, input, sound mixing, file loading, etc.

One of the primary goals of *NAS2D* was to be cross-platform. A lot of effort has gone into the code to make it as platform independant as possible. The few areas that are different from platform to platform are completely hidden from the interface and is virtually transparent to the user.

## Why another 2D API?

*NAS2D* isn't just another 2D renderer. It's a complete set of classes and functions that let you jump into building a game right away.

We built *NAS2D* after we had looked at, considered and ruled out several other game frameworks. Either they were too low-level, were in a language that we didn't want to use or were lacking in features we really needed.

So we set about developing The Legend of Mazzeroth using a few low-level libraries. After awhile, we realized that the core code, once written, didn't change too much and that others could find it useful. And thus, *NAS2D* was born.

## What Platforms are Supported?

Officially, *NAS2D* is supported on Windows (Vista+) and MacOS X (10.8+). Binaries, source code and IDE Project files are provided and maintained for these platforms.

*NAS2D* has been tested and works on Linux and BSD but there are no official maintainers for these platforms.
