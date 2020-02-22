# sgs-remote
Remote control for smart-group-server.
## Introduction

This smart-group-server remote control application, `sgsremote` is based on Jonathan G4KLX's remotecontrold program. It was designed expressly for my Smart Group Server. This remote control program allows a user to connect with a smart-group-server using a specific UDP port and elicit commands.

### What's New

* **V# 200221** The remote control port has been moved from UDP to a much more secure TLS/TCP port. Using UDP to handle a remote control port is really a bad idea. Switch to TLS/TCP has greatly simplified the client (this program). TCP is a much better choice because data exchange between the server (sgs server) and the client guaranteed synchronus exchange of data and the quick transaction inherent to the single-action remote control command. The amount source code needed for this new remote client is a fraction of that required for the previous version. The code base for the server has also shrunk. Be sure to open up the appropriate TCP port for remote control on the server to allow for incoming requests from clients. Of course, the UDP port is no longer used. This client version is compatible with *smart-group-server* version 200220 or newer. A new command is available for halting a server. Combined with systemd's auto restart feature, this will allow a client to reboot a server.

* **V# 190630** `sgsremote` now compiles with the newer UDPReaderWriter used in the *smart-group-server* and should support IPv6 connections. The configuration file must use numeric addresses and not "dotted-name" values. If an IPv6 address is specified, an IPv6 connection will be made. It's okay to have some targets as IPv6 and some IPv4 in the `sgsremote.conf` file.

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
Install the needed support:
```
sudo apt-get install libconfig++-dev libssl-dev openssl
```
Change to the sgs-remote directory and type `make`. This should make the executable, `sgsremote` without errors or warnings.

## Configuring

Before you install the group server, you need to create a configuration file called `sgsremote.conf`. There is an example configuration file: `example.conf`. `sgsremote` is a command line program. You type a command and receive a response and then you are done. The remote can connect to any smart-group-server, you just need to configure a new section in the sgsremote.conf file. Copy example.conf to sgsremote.conf and then edit the new file.

## Installing and Uninstalling

To install and start the smart-group-server, type `sudo make install`. This will put all the executable in /usr/local/bin and the configuration file in /usr/local/etc. If sgsremote finds a sgsremote.conf in ~/.config/ it will use it instead of the one in /usr/local/etc. To uninstall it, type `sudo make uninstall`. This will remove /usr/local/bin/sgsremote and /usr/local/etc/sgsremote.conf.

73

Tom
n7tae (at) arrl (dot) net
