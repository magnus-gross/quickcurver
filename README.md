# Quick Curver
This is a Qt/C++ implementation of the famous game "Achtung die Kurve", accelerated using OpenGL.

[![travis build status](https://travis-ci.org/magnus-gross/quickcurver.svg?branch=develop)](https://travis-ci.org/magnus-gross/quickcurver)
[![Appveyor build status](https://ci.appveyor.com/api/projects/status/v0yfey5sbceddbd8?svg=true)](https://ci.appveyor.com/project/magnus-gross/quickcurver)
[![Total alerts](https://img.shields.io/lgtm/alerts/g/magnus-gross/quickcurver.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/magnus-gross/quickcurver/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/magnus-gross/quickcurver.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/magnus-gross/quickcurver/context:cpp)

![screenshot](https://raw.githubusercontent.com/sam9032/quickcurver/master/Screenshots/2020-04-18%2014_16_25-Quick%20Curver.png)

# Screenshots
Please have a look into the "Screenshots" folder for more impressions!

# Features
* Material Design
* Local Multiplayer
* Online Multiplayer
* Bots
* Items


# Installation
## Compiling from source

Note: Arch Linux users can simply install [quickcurver-git](https://aur.archlinux.org/packages/quickcurver-git) from the AUR:

```bash
yay -S quickcurver-git
quickcurver
```

### Dependencies
First make sure, that you have the required dependencies of QuickCurver installed. These are:

* A C++ compiler with C++14 support
* make
* At least Qt 5.12
* The following Qt Modules (in the parantheses there is an example how the package could be called for your distro (this depends on the distro!)):
	- Qt Core (qt5-base)
	- Qt GUI (qt5-base)
	- Qt Quick (qt5-declarative)
	- Qt QML (qt5-declarative)
	- Qt SVG (qt5-svg)
	- Qt Network (qt5-base)
	- Qt Quick Controls 2 (qt5-quickcontrols2)
	- Qt Graphical Effects (qt5-graphicaleffects)

### Build instructions
Run the following commands:
```bash
git clone --recursive git@github.com:magnus-gross/quickcurver.git
# Alternatively clone with HTTPS, if you don't have SSH keys
# If you forgot to clone with --recursive, just run git submodule update --init
cd quickcurver
mkdir build
cd build
qmake .. -config release
make
```

To start QuickCurver you need to run the built executable in the build/src directory, for example on Linux run: `src/QuickCurver`

Note: You can compile the game with Mumble Positional Audio support, by defining `MUMBLE_SUPPORT=true` at compile time.

## Installing compiled binaries

### Windows
When you click on the AppVeyor icon or go to https://ci.appveyor.com/project/magnus-gross/quickcurver directly, you can download and see the automated AppVeyor build.
If you are not interested in the technology behind it and you finally want to play, just click here:
https://ci.appveyor.com/api/buildjobs/j8ptwckm84f1tn23/artifacts/build%2FQuickCurver.zip

Extract all files and run `QuickCurver.exe` in the `release` directory.

# Multiplayer
To play multiplayer, the host starts an instance and shares the port that QuickCurver is running on. The client then just has to connect to this port on the host's ip address.
If you are not in the same local network, the host most likely has to use [Port Forwarding](https://en.wikipedia.org/wiki/Port_forwarding) to make his device available to the internet.
If a firewall is the problem, you might also want to take a look at [Hole Punching](https://en.wikipedia.org/wiki/Hole_punching_(networking)).

If network performance isn't good, the Server can tweak the "Network update rate" value in the settings, which causes data to be sent less frequently which may improve the network performance at the cost of update frequency. (A higher value means worse quality, but better network performance)

If you want to host Quickcurver cleanly on a separate server and do not need the GUI, you can start it with the CLI parameter `-platform offscreen`.
