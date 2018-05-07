/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
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

#pragma once

#include <string>

#include "SmartGroup.h"
#include "CallsignData.h"
#include "UDPReaderWriter.h"

enum RC_TYPE {
	RCT_NONE,
	RCT_ACK,
	RCT_NAK,
	RCT_RANDOM,
	RCT_CALLSIGNS,
	RCT_REPEATER,
	RCT_STARNET
};

class CHandler {
public:
	CHandler(const std::string &address, unsigned int port);
	~CHandler();

	bool open();

	RC_TYPE readType();

	std::string    readNAK();
	unsigned int   readRandom();
	CCallsignData *readCallsigns();
	CSmartGroup   *readSmartGroup();

	bool login();
	bool sendHash(const unsigned char *hash, unsigned int length);

	void setLoggedIn(bool set);

	bool getCallsigns();
	bool getSmartGroup(const std::string &callsign);

	bool link(const std::string &callsign, const std::string &reflector);
	bool unlink(const std::string &callsign);
	bool logoff(const std::string &callsign, const std::string &user);

	bool logout();

	bool retry();

	void close();

private:
	CUDPReaderWriter  m_socket;
	in_addr           m_address;
	unsigned int      m_port;
	bool              m_loggedIn;
	unsigned int      m_retryCount;
	RC_TYPE           m_type;
	unsigned char    *m_inBuffer;
	unsigned int      m_inLength;
	unsigned char    *m_outBuffer;
	unsigned int      m_outLength;
};

