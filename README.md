# QtMediaPlayer

A simple but powerful multimedia player **library** designed for Qt Quick.

## Features

- Full-featured multimedia player
- Cross-platform: support Windows, Linux, macOS, and mobile platforms in theory
- Easy to build and use

## Build

Since this project makes heavy usage of [Qt RHI](https://doc.qt.io/qt-6/qtquick-visualcanvas-adaptations.html), you'll need **at least Qt 5.14** to use this project. It's recommended to use Qt 6.

```cmake
cmake .
cmake --build .
cmake --install .
```

Currently two backends are available: [MDK](https://sourceforge.net/projects/mdk-sdk/files/) and [MPV](https://sourceforge.net/projects/mpv-player-windows/files/). [VLC](http://download.videolan.org/pub/videolan/vlc/last/) is on plan. All backends will be loaded dynamically at run-time.

## Usage

See [example](/example).

## License

```text
MIT License

Copyright (C) 2021 by wangwenx190 (Yuhang Zhao)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```
