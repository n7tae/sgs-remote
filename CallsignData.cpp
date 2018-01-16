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

#include "CallsignData.h"

CCallsignData::CCallsignData() :
m_repeaters(),
m_smartGroups()
{
}

CCallsignData::~CCallsignData()
{
}

void CCallsignData::addRepeater(const std::string &callsign)
{
	m_repeaters.push_back(callsign);
}

void CCallsignData::addStarNet(const std::string &callsign)
{
	m_smartGroups.push_back(callsign);
}

unsigned int CCallsignData::getRepeaterCount() const
{
	return m_repeaters.size();
}

std::string CCallsignData::getRepeater(unsigned int n) const
{
	return m_repeaters.at(n);
}

unsigned int CCallsignData::getSmartGroupCount() const
{
	return m_smartGroups.size();
}

std::string CCallsignData::getSmartGroup(unsigned int n) const
{
	return m_smartGroups.at(n);
}
