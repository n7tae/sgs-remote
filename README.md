# sgs-remote
Remote control for smart-group-server.
## Introduction

This smart-group-server remote control application, `sgsremote` is based on Jonathan G4KLX's remotecontrold program. It was designed expressly for my Smart Group Server. This remote control program allows a user to connect with a smart-group-server using a specific UDP port and elicit commands.

### What's New

* **V# 180118** Linking and unlinking are now supported: `link` and `unlink`. A group that is linked to an XRF reflector can be unlinked and linked to a DCS reflector and *vis versa*.

* **V# 180111** This first version only supports two actions: `drop` is used to unsubscribe users from a smart-group. `list` is used to print information about a smart-group.

## Server OS Requirements

The smart-group-server remote program requires a modern OS to compile and run. At least Debian 8 or Ubuntu 16.10, or equivilent. The command
```
g++ --version
```
must return at least Version 4.9. The latest Debian and Ubuntu will be far above ths. Unlike the remotecontrold, sgsremote does not use wxWidgits. Modern C++ calls to the standard library (c++11 standard) are used instead of wxWidgets: std::string replaces wxString, std::list, std::queue and std::vector replace the older wx containers. The only external library used is libconfig++.

#
## Building

These instructions are for a Debian-based OS. Begin by downloading this git repository:
```
git clone git://github.com/n7tae/sgs-remote.git
```
Install the only needed development library:
```
sudo apt-get install libconfig++-dev
```
Change to the sgs-remote directory and type `make`. This should make the executable, `sgsremote` without errors or warnings.

## Configuring

Before you install the group server, you need to create a configuration file called `sgsremote.conf`. There is an example configuration file: `example.conf`. `sgsremote` is a command line program. You type a command and receive a response and then you are done. The remote can connect to any smart-group-server, you just need to configure a new section in the sgsremote.conf file. Copy example.conf to sgsremote.conf and then edit the new file.

## Installing and Uninstalling

To install and start the smart-group-server, type `sudo make install`. This will put all the executable in /usr/local/bin and the configuration file in /usr/local/etc. If sgsremote finds a sgsremote.conf in ~/.config/ it will use it instead of the one in /usr/local/etc. To uninstall it, type `sudo make uninstall`. This will remove /usr/local/bin/sgsremote and /usr/local/etc/sgsremote.conf.

73

Tom
n7tae (at) arrl (dot) net
