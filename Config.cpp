/*
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

#include <string>

#include "Config.h"


CConfig::CConfig(const std::string &pathname)
{
	if (pathname.size() < 2) {
		fprintf(stderr, "Configuration filename too short!\n");
		return;
	}

	Config cfg;
	try {
		cfg.readFile(pathname.c_str());
	}
	catch(const FileIOException &fioex) {
		fprintf(stderr, "Can't read %s\n", pathname.c_str());
		return;
	}
	catch(const ParseException &pex) {
		fprintf(stderr, "Parse error at %s:%d - %s\n", pex.getFile(), pex.getLine(), pex.getError());
		return;
	}

	// module parameters
	for (int i=0; i<cfg.lookup("server").getLength(); i++) {
		char key[32];
		std::string name;
		snprintf(key, 32, "server.[%d].name", i);
		get_value(cfg, key, name, 1, 16, "");
		if (name.size()) {
			if (m_server.end() == m_server.find(name)) {
				std::string address, password;
				unsigned int port;
				snprintf(key, 32, "server.[%d].address", i);
				get_value(cfg, key, address, 7, 128, "127.0.0.1");
				snprintf(key, 32, "server.[%d].password", i);
				get_value(cfg, key, password, 8, 128, "changeme");
				int iport;
				snprintf(key, 32, "server.[%d].port", i);
				get_value(cfg, key, iport, 100, 66635, 39999);
				port = (unsigned int)iport;
				struct Sserver *pserver = new struct Sserver;
				pserver->address = address;
				pserver->password = password;
				pserver->port = port;
				m_server[name] = pserver;
			} else
				fprintf(stderr, "servername \"%s\" already defined!\n", name.c_str());
		}
	}
}

CConfig::~CConfig()
{
	for (auto it=m_server.begin(); it!=m_server.end(); it++) {
		struct Sserver *pserver = it->second;
		delete pserver;
	}
	m_server.empty();
}

bool CConfig::get_value(const Config &cfg, const char *path, int &value, int min, int max, int default_value)
{
	if (cfg.lookupValue(path, value)) {
		if (value < min || value > max)
			value = default_value;
	} else
		value = default_value;
	return true;
}

bool CConfig::get_value(const Config &cfg, const char *path, std::string &value, int min, int max, const char *default_value)
{
	if (cfg.lookupValue(path, value)) {
		int l = value.length();
		if (l<min || l>max) {
			fprintf(stderr, "%s=%s has  invalid length\n", path, value.c_str());
			value = default_value;
			return false;
		}
	} else
		value = default_value;
	return true;
}

bool CConfig::getConfig(const std::string name, std::string &address, unsigned int &port, std::string &password)
{
	auto it = m_server.find(name);
	if (it == m_server.end())
		return false;
	struct Sserver *pserver = it->second;
	address = pserver->address;
	port = pserver->port;
	password = pserver->password;
	return true;
}
