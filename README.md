#Quick Curver
This is an OpenGL accelerated Qt/C++ implementation of the famous game "Achtung die Kurve".

![alt tag](http://i.imgur.com/IGa3NwA.png)

Please note that this project is yet in development and hasn't reached a stable version for now. This means that you could experience bugs, when playing this game.

#Features
* Material Design
* Local Multiplayer
* Online Multiplayer (still in development)
* Bots
* Items


#Installation
##Compiling from source
###Dependencies
First make sure, that you have the required dependencies of QuickCurver installed. These are:

* g++ (You may use an alternative C++ compiler, but things will get slightly more difficult)
* make
* At least Qt 5.7
* The following Qt Modules (in the parantheses there is an example how the package could be called for your distro (this depends on the distro!)):
	- Qt Core (qt5-base)
	- Qt GUI (qt5-base)
	- Qt Quick (qt5-declarative)
	- Qt QML (qt5-declarative)
	- Qt SVG (qt5-svg)
	- Qt Network (qt5-base)
	- Qt Quick Controls (qt5-quickcontrols)
	- Qt Quick Controls 2 (qt5-quickcontrols2)
	- Qt Graphical Effects (qt5-graphicaleffects)

###Build instructions
####Cloning using SSH
This is the preferred way, simply execute:
```{r, engine='bash', count_lines}
git clone --recursive git@github.com:magnus-gross/quickcurver.git
cd quickcurver
./build.sh
```

####Cloning using HTTPS
If you do not have setup SSH keys on your machine, you can alternatively run the following commands:
```{r, engine='bash', count_lines}
git clone https://github.com/magnus-gross/quickcurver.git
cd quickcurver
./httpsClonePatch.sh
./build.sh
```

When the application has been successfully compiled, it can usually be found in the build subdirectory with the name `./QuickCurver`.

In general prefer to use the script `./build.sh`. However, if the script fails due to an **unexplainable reason**, you may want to run `./build.sh -f`. If this still fails, try the following manually instead of `./build.sh`:
```{r, engine='bash', count_lines}
mkdir build; cd build
qmake ..
make
```

###Updating
If you built QuickCurver from source you can simply update by running the following script:
```{r, engine='bash', count_lines}
./update.sh # You may have to fix the permissions first with chmod +x ./update.sh
```

##Installing compiled binaries
Coming soon!

#Multiplayer
To play multiplayer, the host has to make at least one player an "Online Player" by clicking on that player in the Playerselector screen and using the drop down menu. Once a slot is open to join, the host has to publish his IP to the client. Note that the program automatically prints out the IP, assuming you have started it CLI. If you are not in the same local network, the host has to use [Port Forwarding](https://en.wikipedia.org/wiki/Port_forwarding) to make his device available to the internet. When the client receives the host's IP, he has to start the program, click on the "Join" Icon on the top right and enter the IP. If the join request was successfull, you will see "Joined, waiting for host to start...", otherwise the connection will timeout after a few seconds.

#Troubleshooting
##The drawing does not get updated, or only rarely
Use one of the following commands to start QuickCurver instead:
```{r, engine='bash', count_lines}
QSG_RENDER_LOOP=basic ./QuickCurver #prefer this one
QSG_RENDER_LOOP=windows ./QuickCurver #alternatively use this command
```

#Acknowledgements
QuickCurver uses [qml-material](https://github.com/papyros/qml-material), which is a nice QML Material Design library.
