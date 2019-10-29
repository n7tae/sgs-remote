/*
 *   Copyright (C) 2011,2013 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early
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

#include <cassert>
#include <cstring>
#include <cstdint>

#include "Handler.h"
#include "DStarDefines.h"

const unsigned int BUFFER_LENGTH = 2000U;

const unsigned int MAX_RETRIES = 3U;

CHandler::CHandler(const std::string &address, unsigned short port) :
m_socket((address.find(':')==std::string::npos) ? AF_INET : AF_INET6, (unsigned short)0),	// use ephemeral port
m_loggedIn(false),
m_retryCount(0U),
m_type(RCT_NONE),
m_inBuffer(NULL),
m_inLength(0U),
m_outBuffer(NULL),
m_outLength(0U)
{
	assert(!address.empty());
	assert(port > 0U);

	int family = (std::string::npos == address.find(':')) ? AF_INET : AF_INET6;
	m_addr.Initialize(family, port, address.c_str());

	m_inBuffer  = new unsigned char[BUFFER_LENGTH];
	m_outBuffer = new unsigned char[BUFFER_LENGTH];

}

CHandler::~CHandler()
{
	delete[] m_inBuffer;
	delete[] m_outBuffer;
}

bool CHandler::open()
{
	return m_socket.Open();
}

RC_TYPE CHandler::readType()
{
	m_type = RCT_NONE;

	CSockAddress addr;

	int length = m_socket.Read(m_inBuffer, BUFFER_LENGTH, addr);
	if (length <= 0)
		return m_type;

	m_inLength = length;

	if (memcmp(m_inBuffer, "ACK", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_ACK;
		return m_type;
	} else if (memcmp(m_inBuffer, "NAK", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_NAK;
		return m_type;
	} else if (memcmp(m_inBuffer, "RND", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_RANDOM;
		return m_type;
	} else if (memcmp(m_inBuffer, "CAL", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_CALLSIGNS;
		return m_type;
	} else if (memcmp(m_inBuffer, "RPT", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_REPEATER;
		return m_type;
	} else if (memcmp(m_inBuffer, "SNT", 3U) == 0) {
		m_retryCount = 0U;
		m_type = RCT_STARNET;
		return m_type;
	}

	return m_type;
}

std::string CHandler::readNAK()
{
	if (m_type != RCT_NAK)
		return std::string("");

	std::string text((char*)(m_inBuffer + 3U));

	return text;
}

unsigned int CHandler::readRandom()
{
	if (m_type != RCT_RANDOM)
		return 0U;

	int32_t random;
	memcpy(&random, m_inBuffer + 3U, sizeof(int32_t));

	return random;
}

CCallsignData *CHandler::readCallsigns()
{
	if (m_type != RCT_CALLSIGNS)
		return NULL;

	CCallsignData* data = new CCallsignData;

	unsigned char* p = m_inBuffer + 3U;
	unsigned int pos = 3U;

	while (pos < m_inLength) {
		unsigned char type = *p;
		pos += 1U;
		p += 1U;

		std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
		pos += LONG_CALLSIGN_LENGTH;
		p += LONG_CALLSIGN_LENGTH;

		switch (type) {
			case 'R':
				data->addRepeater(callsign);
				break;
			case 'S':
				data->addStarNet(callsign);
				break;
			default:		// ????
				break;
		}
	}

	return data;
}

CSmartGroup *CHandler::readSmartGroup()
{
	if (m_type != RCT_STARNET)
		return NULL;

	if (m_inLength < 23 + 4 * LONG_CALLSIGN_LENGTH + sizeof(unsigned int) + sizeof(enum LINK_STATUS))
		return NULL;

	unsigned char* p = m_inBuffer + 3U;
	unsigned int pos = 3U;

	std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	std::string logoff((char*)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	std::string repeater((char *)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	std::string infoText((char *)p, 20);
	pos += 20;
	p += 20;

	std::string reflector((char *)p, LONG_CALLSIGN_LENGTH);
	pos += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	LINK_STATUS ls;
	memcpy(&ls, p, sizeof(enum LINK_STATUS));
	pos += sizeof(enum LINK_STATUS);
	p += sizeof(enum LINK_STATUS);

	unsigned int ut;
	memcpy(&ut, p, sizeof(unsigned int));
	pos += sizeof(unsigned int);
	p += sizeof(unsigned int);

	CSmartGroup *group = new CSmartGroup(callsign, logoff, repeater, infoText, reflector, ls, ut);

	while (pos + LONG_CALLSIGN_LENGTH + 2U * sizeof(int32_t) <= m_inLength) {
		std::string callsign((char*)p, LONG_CALLSIGN_LENGTH);
		pos += LONG_CALLSIGN_LENGTH;
		p += LONG_CALLSIGN_LENGTH;

		int32_t timer;
		memcpy(&timer, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		int32_t timeout;
		memcpy(&timeout, p, sizeof(int32_t));
		pos += sizeof(int32_t);
		p += sizeof(int32_t);

		group->addUser(callsign, timer, timeout);
	}

	return group;
}

bool CHandler::login()
{
	if (m_loggedIn)
		return false;

	memcpy(m_outBuffer, "LIN", 3U);
	m_outLength = 3U;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

void CHandler::setLoggedIn(bool set)
{
	m_loggedIn = set;
}

bool CHandler::getCallsigns()
{
	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	memcpy(m_outBuffer, "GCS", 3U);
	m_outLength = 3U;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::sendHash(const unsigned char *hash, unsigned int length)
{
	assert(hash != NULL);
	assert(length > 0U);

	if (m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	memcpy(p, "SHA", 3U);
	m_outLength += 3U;
	p += 3U;

	memcpy(p, hash, length);
	m_outLength += length;
	p += length;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::getSmartGroup(const std::string &callsign)
{
	assert(callsign.size());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	memcpy(p, "GSN", 3U);
	m_outLength += 3U;
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.size(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::link(const std::string &callsign, const std::string &reflector)
{
	assert(callsign.size());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	memcpy(p, "LNK", 3U);
	m_outLength += 3U;
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.size(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < reflector.size(); i++)
		p[i] = reflector.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::unlink(const std::string &callsign)
{
	assert(callsign.size());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	memcpy(p, "UNL", 3U);
	m_outLength += 3U;
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.size(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::logoff(const std::string &callsign, const std::string &user)
{
	assert(callsign.size());
	assert(user.size());

	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	unsigned char* p = m_outBuffer;
	m_outLength = 0U;

	memcpy(p, "LGO", 3U);
	m_outLength += 3U;
	p += 3U;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < callsign.size(); i++)
		p[i] = callsign.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	memset(p, ' ', LONG_CALLSIGN_LENGTH);
	for (unsigned int i = 0U; i < user.size(); i++)
		p[i] = user.at(i);

	m_outLength += LONG_CALLSIGN_LENGTH;
	p += LONG_CALLSIGN_LENGTH;

	bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
	if (!ret) {
		m_retryCount = 0U;
		return false;
	} else {
		m_retryCount = 1U;
		return true;
	}
}

bool CHandler::logout()
{
	if (!m_loggedIn || m_retryCount > 0U)
		return false;

	memcpy(m_outBuffer, "LOG", 3U);
	m_outLength = 3U;

	for (unsigned int i = 0U; i < 5U; i++) {
		bool ret = m_socket.Write(m_outBuffer, m_outLength, m_addr);
		if (!ret) {
			m_retryCount = 0U;
			return false;
		}
	}

	m_retryCount = 1U;

	return true;
}

bool CHandler::retry()
{
	if (m_retryCount > 0U) {
		m_retryCount++;
		if (m_retryCount >= MAX_RETRIES) {
			m_retryCount = 0U;
			return false;
		}

		m_socket.Write(m_outBuffer, m_outLength, m_addr);
	}

	return true;
}

void CHandler::close()
{
	m_socket.Close();
}
