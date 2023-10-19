/*
 *   Copyright (C) 2013,2014 by Jonathan Naylor G4KLX
 *   Copyright (c) 2018,2020, 2023 by Thomas A. Early N7TAE
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

#ifndef CFG_DIR
#define CFG_DIR "/tmp"
#endif

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
	char * home = getenv("HOME");
	std::string cfgfile(home != NULL ? home : "");
	cfgfile.append("/.config/sgsremote.conf");
	struct stat sbuf;
	if (stat(cfgfile.c_str(), &sbuf)) {
		cfgfile.assign(CFG_DIR);
		cfgfile.append("/sgsremote.conf");
		if (stat(cfgfile.c_str(), &sbuf)) {
			fprintf(stderr, "ERROR: no configuration file found, exiting!\n");
			return 1;
		}
	}

	CConfig config(cfgfile);

	if (argc < 3 || argc > 5) {
		printf("%s Version 231019\n", argv[0]);
		printf("Command line usage: %s <servername> halt\n", argv[0]);
		printf("                    %s <servername> list all\n", argv[0]);
		printf("                    %s <servername> list <subscribe>\n", argv[0]);
		printf("                    %s <servername> drop <subscribe> <user>\n", argv[0]);
		printf("                    %s <servername> drop <subscribe> all\n", argv[0]);
		printf("                    %s <servername> link <subscribe> <reflector>\n", argv[0]);
		printf("                    %s <servername> unlink <subscribe>\n", argv[0]);
		printf("\nWhere <server> is the sgs server callsign and <subscribe> is the smart-group callsign.\n\n");
		printf("Configured server name are:\n");
		auto servers = config.getServers();
		for (auto it=servers.begin(); it!=servers.end(); it++)
			printf("%s\n", it->c_str());
		return 0;
	}

	std::string sgsname(argv[1]);
	std::string action(argv[2]);
	std::string subscribe, object;
	if (3 == argc) {
		if (0 == action.compare("halt")) {
		} else {
			fprintf(stderr, "Malformed command. Did you mean %s %s halt?\n", argv[0], argv[2]);
			return 1;
		}
	} else {
		subscribe.assign(argv[3]);
		Res2Up(subscribe);
		if (0 == action.compare("drop")) {
			if (5 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> drop <subscribe> <user>\n", argv[0]);
				fprintf(stderr, "                             %s <servername> drop <subscribe> all\n", argv[0]);
				return 1;
			}

			object.assign(argv[4]);
			Res2Up(object);
		} else if (0 == action.compare("list")) {
			if (4 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> list <subscribe>\n", argv[0]);
				return 1;
			}
		} else if (0 == action.compare("unlink")) {
			if (4 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> unlink <subscribe>\n", argv[0]);
				return 1;
			}
		} else if (0 == action.compare("link")) {
			if (5 != argc) {
				fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> link <subscribe> <reflector>\n", argv[0]);
				return 1;
			}
			object.assign(argv[4]);
			Res2Up(object);
		} else {
			fprintf(stderr, "%s: invalid action value passed, only halt, drop, link or unlink or list are allowed\n", argv[0]);
			return 1;
		}
	}

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

	std::string command(action);
	if (action.compare("halt")) {
		command += " " + subscribe;
		if (0==action.compare("drop") || 0==action.compare("link"))
			command += " " + object;
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
