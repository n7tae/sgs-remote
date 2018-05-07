/*
 *   Copyright (C) 2013,2014 by Jonathan Naylor G4KLX
 *   Copyright (c) 2018 by Thomas A. Early N7TAE
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
#include "Handler.h"
#include "SmartGroup.h"
#include "DStarDefines.h"
#include "SHA256.h"

void sendHash(CHandler *handler, const std::string &password, unsigned int rnd)
{
	assert(handler != NULL);

	unsigned int len = password.size() + sizeof(unsigned int);
	unsigned char  *in = new unsigned char[len];
	unsigned char *out = new unsigned char[32U];

	memcpy(in, &rnd, sizeof(unsigned int));
	for (unsigned int i = 0U; i < password.size(); i++)
		in[i + sizeof(unsigned int)] = password.at(i);

	CSHA256 sha256;
	sha256.buffer(in, len, out);

	handler->sendHash(out, 32U);

	delete[] in;
	delete[] out;
}

static void RepRes2Up(std::string &name)	//replace, resize, to upper
{
	for (unsigned int i=0; i<name.size(); i++) {
		if ('_' == name[i])
			name[i] = ' ';
		else if (islower(name[i]))
			name[i] = toupper(name[i]);
	}
	name.resize(LONG_CALLSIGN_LENGTH, ' ');
}

int main(int argc, char *argv[])
{
	if (argc < 4 || argc > 5) {
		fprintf(stderr, "%s Version 180507\n", argv[0]);
		fprintf(stderr, "Command line usage: %s <servername> <subscribe> list\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> drop <user>\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> drop all\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> unlink\n", argv[0]);
		fprintf(stderr, "                    %s <servername> <subscribe> link <reflector>\n", argv[0]);
		return 1;
	}

	std::string smartserver(argv[1]);
	std::string subscribe(argv[2]);
	RepRes2Up(subscribe);

	std::string action(argv[3]);

	std::string user, reflector;

	if (0 == action.compare("drop")) {
		if (5 != argc) {
			fprintf(stderr, "INVALID COMMAND LINE, usage: %s <servername> <subscribe> drop <user>\n", argv[0]);
			fprintf(stderr, "                             %s <servername> <subscribe> drop all\n", argv[0]);
			return 1;
		}

		user = std::string(argv[4]);
		RepRes2Up(user);
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
		reflector = std::string(argv[4]);
		RepRes2Up(reflector);
	} else {
		fprintf(stderr, "%s: invalid action value passed, only drop or list are allowed\n", argv[0]);
		return 1;
	}

	std::string cfgfile(getenv("HOME"));
	cfgfile.append("/.config/sgsremote.conf");
	struct stat buf;
	if (stat(cfgfile.c_str(), &buf)) {
		cfgfile.assign("/usr/local/etc/sgsremote.conf");
		if (stat(cfgfile.c_str(), &buf)) {
			fprintf(stderr, "ERROR: no configuration file found, exiting!\n");
			return 1;
		}
	}
	CConfig config(cfgfile);

	std::string address, password;
	unsigned int port;
	if (false == config.getConfig(smartserver, address, port, password)) {
		fprintf(stderr, "%s: '%s' smartserver not found in configuration file!\n", argv[0], smartserver.c_str());
		return 1;
	}

	if (0==address.size() || port == 0U || 0==password.size()) {
		fprintf(stderr, "%s: no address, port or password is set\n", argv[0]);
		return 1;
	}

	CHandler handler(address, port);

	if (false == handler.open()) {
		fprintf(stderr, "%s: uanble to open the UDP port %d\n", argv[0], port);
		return 1;
	}

	if (false == handler.login()) {
		handler.close();
		fprintf(stderr, "%s: uanble to login to the smart-group-server %s\n", argv[0], smartserver.c_str());
		return 1;
	}

	unsigned int count = 0U;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		RC_TYPE type = handler.readType();
		if (type == RCT_RANDOM)
			break;

		if (type == RCT_NONE)
			handler.retry();

		count++;
	}

	if (count >= 10U) {
		handler.close();
		fprintf(stderr, "%s: unable to get a response from the smart-group-server\n", argv[0]);
		return 1;
	}

	unsigned int rnd = handler.readRandom();
	sendHash(&handler, password, rnd);

	count = 0U;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		RC_TYPE type = handler.readType();
		if (type == RCT_ACK)
			break;

		if (type == RCT_NAK) {
			fprintf(stderr, "ERROR from %s: %s\n", smartserver.c_str(), handler.readNAK().c_str());
			handler.close();
			fprintf(stderr, "%s: invalid password sent to the smart-group-server\n", argv[0]);
			return 1;
		}

		if (type == RCT_NONE)
			handler.retry();

		count++;
	}

	if (count >= 10U) {
		handler.close();
		fprintf(stderr, "%s: unable to get a response from the smart-group-server\n", argv[0]);
		return 1;
	}

	handler.setLoggedIn(true);

	if (0 == action.compare("drop"))
		handler.logoff(subscribe, user);
	else if (0 == action.compare("unlink"))
		handler.unlink(subscribe);
	else if (0 == action.compare("link"))
		handler.link(subscribe, reflector);
	else
		handler.getSmartGroup(subscribe);

	count = 0U;
	CSmartGroup *smartGroup = NULL;
	while (count < 10U) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		RC_TYPE type = handler.readType();
		if (type == RCT_ACK)
			break;

		if (type == RCT_NAK) {
			fprintf(stderr, "ERROR from server %s: %s\n", smartserver.c_str(), handler.readNAK().c_str());
			handler.close();
			fprintf(stderr, "%s: drop/list/link/unlink command rejected by the smart-group-server\n", argv[0]);
			return 1;
		}

		if (type == RCT_NONE)
			handler.retry();

		if (type == RCT_STARNET) {
			smartGroup = handler.readSmartGroup();
			break;
		}

		count++;
	}

	if (count >= 10U) {
		handler.close();
		fprintf(stderr, "%s: unable to get a response from the smart-group-server\n", argv[0]);
		return 1;
	}

	if (smartGroup) {
		printf("Subscribe = %s\n", smartGroup->getCallsign().c_str());
		printf("Unsubscribe = %s\n", smartGroup->getLogoff().c_str());
		printf("Repeater = %s\n", smartGroup->getRepeater().c_str());
		printf("Description = %s\n", smartGroup->getInfoText().c_str());
		printf("Reflector = %s\nLink Status = ", smartGroup->getReflector().c_str());
		switch (smartGroup->getLinkStatus()) {
			case LS_LINKING_DCS:
			case LS_LINKING_DEXTRA:
				printf("Linking\n");
				break;
			case LS_LINKED_DCS:
			case LS_LINKED_DEXTRA:
				printf("Linked\n");
				break;
			default:
				printf("Unlinked\n");
				break;
		}
		printf("User Timeout = %d min\n", smartGroup->getUserTimeout());
		for (unsigned int i=0; i<smartGroup->getUserCount(); i++) {
			CSmartGroupUser user = smartGroup->getUser(i);
			printf("User = %s, timer = %u min, timeout = %u min\n", user.getCallsign().c_str(), user.getTimer()/60U, user.getTimeout()/60U);
		}
		delete smartGroup;
	} else
		fprintf(stdout, "The %s command accepted by the smart-group-server\n", action.c_str());

	handler.logout();
	handler.close();

	return 0;
}
