<div align="center">

![PMMA logo](https://media.githubusercontent.com/media/Project-PMMA/PMMA-Documentation/main/docs/_static/logo.png)

</div>

# PMMA (Python Multi-Media API)

![PyPI - Wheel](https://img.shields.io/pypi/wheel/pmma) ![Python 3.8](https://img.shields.io/badge/python-3.8-blue) ![Python 3.9](https://img.shields.io/badge/python-3.9-blue) ![Python 3.10](https://img.shields.io/badge/python-3.10-blue) ![Python 3.11](https://img.shields.io/badge/python-3.11-blue) ![Windows](https://img.shields.io/badge/platform-Windows-blue?logo=windows) ![Linux](https://img.shields.io/badge/platform-Linux-yellow?logo=linux) ![GitHub commit activity](https://img.shields.io/github/commit-activity/y/PycraftDeveloper/pmma) ![GitHub commits since latest release](https://img.shields.io/github/commits-since/PycraftDeveloper/pmma/latest)

<p align="center">
    <a href="https://github.com/Project-PMMA/PMMA/blob/main/repository/BuildGuides/intro.md#pmma-build-guide">Build Guide</a> •
    <a href="https://github.com/Project-PMMA/PMMA/blob/main/repository/Troubleshooting/intro.md#pmma-troubleshooting">Troubleshooting</a> •
    <a href="https://pmma.readthedocs.io/en/latest/">Documentation</a> •
    <a href="https://discord.gg/88duynCX57">Discord</a> •
    <a href="https://github.com/Project-PMMA/PMMA-Benchmarks">Benchmarking</a> •
    <a href="https://github.com/Project-PMMA/PMMA-Tests">Tests</a>
</p>

PMMA is a Python module targeted at helping you build applications in the Python programming language. It does this by providing its own tools covering areas like 2D graphics, noise generation, audio and video playback, event handling, text rendering and much more. The API has two fundamental goals; to make application development in Python easier, whilst also focusing on improving the performance and efficiency of the end result. The API is also being engineered with compatibility with other python modules, like [Pygame](https://github.com/pygame/pygame), [PIL](https://github.com/python-pillow/Pillow) and [Numpy](https://github.com/numpy/numpy) and is ideal for prototyping, application development, simulations, graphics intensive tasks and game development.

## Contents

- [Installation](https://github.com/Project-PMMA/PMMA/blob/main/README.md#installation)
- [Development Progress](https://github.com/Project-PMMA/PMMA?tab=readme-ov-file#development-progress)
- [Credits](https://github.com/Project-PMMA/PMMA/blob/main/repository/Troubleshooting/into.md#pmma-credits)
- [About](https://github.com/Project-PMMA/PMMA/blob/main/repository/Troubleshooting/into.md#pmma-about)

## Installation

You can install the latest version of PMMA from PyPi using the command: `pip install pmma` or you can head over to the website here: [PMMA on PyPi](https://pypi.org/project/pmma/) to select a custom version to install. Alternatively all versions of the API are also available [here, on the releases page](https://github.com/Project-PMMA/PMMA/releases) of this GitHub repository!

### Requirements

In order to install PMMA 5 and newer, you must ensure you meet the following criteria:

| Category         |                                                                        Requirement                                                                         |
| :--------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------: |
| Operating System | `Windows`, `RHEL 9+`, `Debian 12+`, `Fedora 36+`, `Ubuntu 22.04+`, `AlmaLinux 9+`, `Rocky Linux 9+`, `openSUSE Leap 15.5` **Architecture**: `64-bit (x64)` |
| Python Version   |                                                            `3.8.x`, `3.9.x`, `3.10.x`, `3.11.x`                                                            |
| `pip` Version    |                                                                      `20.3 or newer`                                                                       |

_Note: If your platform is not listed here then you can attempt to build your own version of PMMA using our [build guide](https://github.com/Project-PMMA/PMMA/blob/main/repository/BuildGuides/intro.md)!_
_Note: If you do not see your operating system listed above, please check the 'Additional Technical Requirements' section below to see if your operating system is supported._

<details><summary>Additional Technical Requirements</summary>

_Please note, these requirements are only needed by users installing PMMA onto Linux machines and in most cases the operating systems listed above should be compatible._

- In order for PMMA to work as expected, you must be using either X-Lib, or Wayland. This means that Ubuntu 22.04 DESKTOP will work, but Ubuntu 22.04 SERVER is unlikely to.
- Additionally, you will need `glibc 2.31` or newer, this can be checked on linux using the command `ldd --version` (root not required). The result should be on the first line as shown in the image below:

![Example output](https://github.com/user-attachments/assets/45c4e860-e044-4c3e-96ad-1a9c3258b200)

</details>
</br>

> For older versions of PMMA, there are no hardware requirements.

_If you encounter any issues or problems then check out our [troubleshooting page](https://github.com/Project-PMMA/PMMA/blob/main/repository/Troubleshooting/into.md#pmma-troubleshooting)._

## Development Progress

![Progress on PMMA 5.1](https://geps.dev/progress/55)

We are currently working on the next minor update to PMMA, version 5.1.x. This update is targeted at transitioning away from the OpenGL graphics API and into BGFX to better secure the future of the API with the gradual deprecation of the OpenGL API. This update will also bring numerous bug fixes, a major overhaul to the build system for the C++ side of PMMA, and an expansion ans general refinement of the Text and Animation portions of the API. If you want to check out our current progress list, you can find it here: [Progress on PMMA 5.1](https://github.com/Project-PMMA/PMMA/blob/main/repository/DevelopmentProgress.md#progress-on-pmma-5)

## Credits

PMMA is made possible thanks to the following third party components:

C/C++ projects:

- LIBZ - 1.3.1 - https://github.com/madler/zlib
- Libpng - 1.6.49 - https://github.com/glennrp/libpng
- GLFW - 3.4 - https://github.com/glfw/glfw
- FreeType - 2-13-3 - https://gitlab.freedesktop.org/freetype/freetype
- HarfBuzz - 11.2.1 - https://github.com/harfbuzz/harfbuzz
- STB - N/A - https://github.com/nothings/stb
- Bc7enc - N/A - https://github.com/richgel999/bc7enc_rdo
- CPR - 1.14.2 - https://github.com/libcpr/cpr
- BGFX.CMake - v1.143.9248-540 - https://github.com/bkaradzic/bgfx.cmake
- ZSTD - v1.5.7 - https://github.com/facebook/zstd
- JSON - 3.12.0 - https://github.com/nlohmann/json

> _Note: When downloading official compiled versions of PMMA 5 and newer these C++ projects are included by default for an easier installation process._

Python projects:

- Cython - latest - https://github.com/cython/cython
- Numpy - >= 2.0 - https://github.com/numpy/numpy
- Pedalboard - latest - https://github.com/spotify/pedalboard
- Pprofile - latest - https://github.com/vpelletier/pprofile
- Requests - latest - https://github.com/psf/requests
- Send2Trash - latest - https://github.com/arsenetar/send2trash
- SoundDevice - latest - https://github.com/spatialaudio/python-sounddevice/
- SoundFile - latest - https://github.com/bastibe/python-soundfile
- PyWin32 - latest - https://github.com/mhammond/pywin32
- WMI - latest - https://timgolden.me.uk/python/wmi/

> _Note: These requirements are refined in the `requirements.txt` file for your convenience._

None of the projects mentioned above are owned or maintained by PycraftDeveloper the maker of this repository, who would also like to say a big thank you to all the teams working on these projects!

You can check out our licenses and the licenses of all the C/C++ projects PMMA uses as standard [here](https://github.com/Project-PMMA/PMMA/tree/main/pmma/licenses) or on your installed version of PMMA (version 5 or later) under `pmma/licenses`.

_Note: If you spot a problem in our licensing or distribution of third party dependencies please raise the problem as [an issue here](https://github.com/Project-PMMA/PMMA/issues) with the title-prefix: 'LICENSING: ' and we will respond to these problems as soon as possible. Thanks!_

## About

We have worked on numerous large applications using the Python programming language. Most notably [Pycraft](https://github.com/PycraftDeveloper/Pycraft) which is our flagship project, an OpenGL based game using Python. Every time we write these large projects, we often find ourselves writing the same utility programs - which are small programs that help to eliminate complexity in our larger program files. So we decided to combine all of these utility programs into this project, PMMA which is intended then to make writing these larger applications easier as we don't need to keep re-using the same utility programs. The benefits don't stop there though as we are also ensuring this API is as fast and efficient as possible.

Please note that PMMA is currently in a developmental state, meaning that the API is subject to change - we are hoping to remove this warning and improve backwards compatibility in PMMA 6.
