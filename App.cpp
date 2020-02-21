/*
 *   Copyright (C) 2013,2014 by Jonathan Naylor G4KLX
 *   Copyright (c) 2018,2020 by Thomas A. Early N7TAE
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <sys/stat.h>
#include <string>
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>

#include "Config.h"
#include "TLSClient.h"

static void Res2Up(std::string &name)	//resize, to upper
{
	for (unsigned int i=0; i<name.size(); i++) {
		if (islower(name[i]))
			name[i] = toupper(name[i]);
	}
	name.resize(8, ' ');
}

int main(int argc, char *argv[])
{
	if (argc < 3 || argc > 5) {
		fprintf(stderr, "%s Version 200221\n", argv[0]);
		fprintf(stderr, "Command line usage: %s <servername> <subscribe> list\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> drop <user>\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> drop all\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> unlink\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> link <reflector>\n", argv[0]);
		fprintf(stderr, "                    %s <servername> halt\n", argv[0]);
		return 1;
	}

	std::string sgsname(argv[1]);
	std::string subscribe(argv[2]);
	std::string action;
	if (3 == argc) {
		if (0 == subscribe.compare("halt")) {
			action.assign("halt");
			subscribe.clear();
		} else {
			fprintf(stderr, "Malformed command\n");
			return 1;
		}
	} else {
		Res2Up(subscribe);
		action.assign(argv[3]);
	}

	std::string user, reflector;

	if (action.compare("halt")) {
		if (0 == action.compare("drop")) {
			if (5 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> <subscribe> drop <user>\n", argv[0]);
				fprintf(stderr, "                             %s <servername> <subscribe> drop all\n", argv[0]);
				return 1;
			}

			user.assign(argv[4]);
			Res2Up(user);
		} else if (0 == action.compare("list")) {
			if (4 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> <subscribe> list\n", argv[0]);
				return 1;
			}
		} else if (0 == action.compare("unlink")) {
			if (4 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> <subscribe> unlink\n", argv[0]);
				return 1;
			}
		} else if (0 == action.compare("link")) {
			if (5 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> <subscribe> link <reflector>\n", argv[0]);
				return 1;
			}
			reflector.assign(argv[4]);
			Res2Up(reflector);
		} else {
			fprintf(stderr, "%s: invalid action value passed, only drop or list are allowed\n", argv[0]);
			return 1;
		}
	}

	std::string cfgfile(getenv("HOME"));
	cfgfile.append("/.config/sgsremote.conf");
	struct stat sbuf;
	if (stat(cfgfile.c_str(), &sbuf)) {
		cfgfile.assign("/usr/local/etc/sgsremote.conf");
		if (stat(cfgfile.c_str(), &sbuf)) {
			fprintf(stderr, "ERROR: no configuration file found, exiting!\n");
			return 1;
		}
	}
	CConfig config(cfgfile);

	std::string address, password;
	unsigned short port;
	if (false == config.getConfig(sgsname, address, port, password)) {
		fprintf(stderr, "%s: '%s' sgsname not found in configuration file!\n", argv[0], sgsname.c_str());
		return 1;
	}

	if (0==address.size() || port == 0U || 0==password.size()) {
		fprintf(stderr, "%s: no address, port or password is set\n", argv[0]);
		return 1;
	}

	CTLSClient tlsclient;

	if (tlsclient.Open(address.c_str(), port)) {
		fprintf(stderr, "Could not open TLS port\n");
		return 1;
	}

	std::string command;
	if (0 == action.compare("halt"))
		command.assign(action);
	else {
		command.assign(subscribe);
		command += " " + action;
		if (0 == action.compare("drop"))
			command += " " + user;
		else if (0 == action.compare("link"))
			command += " " + reflector;
	}

	//printf("sending command \"%s\" to %s\n", command.c_str(), sgsname.c_str());

	tlsclient.SendCommand(password.c_str(), command.c_str());

	int ret;
	do {
		char buf[128] = { 0 };
		ret = tlsclient.Read(buf, 128);
		if (ret > 0)
			printf("%s\n", buf);
	} while (ret > 0);

	return 0;
}
