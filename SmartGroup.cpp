/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
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

#include "SmartGroup.h"

CSmartGroup::CSmartGroup(const std::string &callsign, const std::string &logoff, const std::string &repeater, const std::string &infoText, const std::string &reflector,
			LINK_STATUS linkStatus, unsigned int userTimeout) :
m_callsign(callsign),
m_logoff(logoff),
m_repeater(repeater),
m_infoText(infoText),
m_linkReflector(reflector),
m_linkStatus(linkStatus),
m_userTimeout(userTimeout),
m_users()
{
	if (0 == m_logoff.compare("        "))
		m_logoff.erase();
}

CSmartGroup::~CSmartGroup()
{
	m_users.clear();
}

void CSmartGroup::addUser(const std::string &callsign, int32_t timer, int32_t timeout)
{
	CSmartGroupUser user(callsign, timer, timeout);

	m_users.push_back(user);
}

std::string CSmartGroup::getCallsign() const
{
	return m_callsign;
}

std::string CSmartGroup::getLogoff() const
{
	return m_logoff;
}

std::string CSmartGroup::getRepeater() const
{
	return m_repeater;
}

std::string CSmartGroup::getInfoText() const
{
	return m_infoText;
}

std::string CSmartGroup::getReflector() const
{
	return m_linkReflector;
}

LINK_STATUS CSmartGroup::getLinkStatus()
{
	return m_linkStatus;
}

unsigned int CSmartGroup::getUserTimeout()
{
	return m_userTimeout;
}

unsigned int CSmartGroup::getUserCount() const
{
	return m_users.size();
}

const CSmartGroupUser &CSmartGroup::getUser(unsigned int n) const
{
	return m_users.at(n);
}
