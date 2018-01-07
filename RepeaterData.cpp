/*
 *   Copyright (C) 2011 by Jonathan Naylor G4KLX
 *   Copyright (c) 2017 by Thomas A. Early N7TAE
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

#include "RepeaterData.h"

CRepeaterData::CRepeaterData(const std::string &callsign, int32_t reconnect, const std::string& reflector) :
m_callsign(callsign),
m_reconnect(RECONNECT(reconnect)),
m_reflector(reflector),
m_links()
{
}

CRepeaterData::~CRepeaterData()
{
	m_links.push_back();
}

void CRepeaterData::addLink(const std::string &callsign, int32_t protocol, int32_t linked, int32_t direction, int32_t dongle)
{
	CLinkData data(callsign, protocol, linked, direction, dongle);

	m_links.Add(data);
}

std::string CRepeaterData::getCallsign() const
{
	return m_callsign;
}

RECONNECT CRepeaterData::getReconnect() const
{
	return m_reconnect;
}

std::string CRepeaterData::getReflector() const
{
	return m_reflector;
}

unsigned int CRepeaterData::getLinkCount() const
{
	return m_links.size();
}

CLinkData &CRepeaterData::getLink(unsigned int n) const
{
	return m_links.at(n);
}
