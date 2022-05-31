# QtMediaPlayer

A simple but powerful multimedia player **library** designed for Qt Quick.

## Features

- Full-featured multimedia player
- Cross-platform: support Windows, Linux, macOS, and mobile platforms **in theory**
- Easy to build and use

## Known limitations

Currently this library doesn't support reading from embeded resources or reading from `QIODevice`. It can only read from real local files or online streams. There's plan to implement this feature for this library, but it's not urgent.

## Why not just use QtMultimedia or own FFmpeg implementation?

Currently this project uses **MDK** and **MPV** as the player backends. They are world-famous multimedia frameworks with long time active development, they are known to have good code quality and especially outstanding performance, however, QtMultimedia is only a simple implementation based on the operating system's default multimedia framework, it has a friendly interface but it's not designed for performance, and I'm also not convinced that the Qt company has deep experience on the multimedia area. And I also don't think some custom FFmpeg implementation can be better than these impressive frameworks.

## Build

Since this project makes heavy usage of [Qt RHI](https://doc.qt.io/qt-6/qtquick-visualcanvas-adaptations.html), you'll need **at least Qt 5.14** to use this project. But Qt RHI in 5.14 is not quite stable, so **it's highly recommended to use Qt 5.15 or the latest version of Qt 6**.

```cmake
cmake .
cmake --build .
cmake --install .
```

Currently two player backends are available: [MDK](https://sourceforge.net/projects/mdk-sdk/files/) and [MPV](https://sourceforge.net/projects/mpv-player-windows/files/libmpv/). [FFmpeg](https://ffmpeg.org/) is on plan. All backends will be loaded dynamically at run-time.

**Notes for using the MDK backend**: you need to download a separate FFmpeg package yourself and put them into the application directory due to MDK doesn't link against FFmpeg statically.

**Notes for using the MPV backend**: libmpv needs [ANGLE](https://github.com/google/angle) (libEGL.dll & libGLESv2.dll) and Microsoft's shader compiler (d3dcompiler_XX.dll), you need to put them into the application directory. The latter is shipped with Windows SDK, the former is shipped with Google Chrome/Mozilla Firefox/Visual Studio Code/etc. If you want to build ANGLE yourself, [vcpkg](https://github.com/microsoft/vcpkg) is a good choice. If you want to play online media streams, you need to download [youtube-dl](https://github.com/yt-dlp/yt-dlp/releases/latest)'s executable and place it into your application's directory.

About the [VLC](https://artifacts.videolan.org/vlc/nightly-win64-llvm/) backend: There has been quite some work on the initial porting of the VLC backend, but it comes to an dead end. libVLC recommends to use a separate window to do the rendering to get the best performance and experience. It also supports get the decoded data and do the rendering ourself, but after some testing, I found there is significant performance lost, just like libVLC's official documentation said. And we want to use QtRHI to catch up Qt's latest design, however, libVLC's design is not suitable for doing so. It supports custom rendering through Direct3D and OpenGL, but it works quite differently with QtRHI's design. In one word: the VLC backend is abandoned due to significant performance lost and incompatible design with QtRHI.

Recommended 3rd-party pre-built FFmpeg binaries:
- **RECOMMENDED** [avbuild](https://sourceforge.net/projects/avbuild/files/) (desktop & mobile & embeded platforms, full featured & lite builds, shared & static libraries, LGPL & GPL)
- [FFmpeg-Builds](https://github.com/BtbN/FFmpeg-Builds/releases/latest) (Win64 & Linux64, full featured builds, shared & static libraries, LGPL & GPL)
- [Mile.FFmpeg](https://github.com/ProjectMile/Mile.FFmpeg/releases/latest) (based on vcpkg, currently Windows only, full featured builds, shared libraries only, LGPL)

## Usage

Please refer to my another project: <https://github.com/wangwenx190/QPlayer309/>.

## License

```text
MIT License

Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)

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
