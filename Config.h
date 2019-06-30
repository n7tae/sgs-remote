/*
 *   Copyright (c) 2018-2019 by Thomas A. Early
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
#include <map>
#include <libconfig.h++>

using namespace libconfig;

struct Sserver {
	std::string address;
	std::string password;
	unsigned short port;
};

class CConfig {
public:
	CConfig(const std::string &pathname);
	~CConfig();
	bool getConfig(const std::string name, std::string &address, unsigned short &port, std::string &password);
private:
	std::map<std::string, struct Sserver *> m_server;

	bool get_value(const Config &cfg, const char *path, int &value, int min, int max, int default_value);
	bool get_value(const Config &cfg, const char *path, std::string &value, int min, int max, const char *default_value);
};
