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

#include "SmartGroupUser.h"

CSmartGroupUser::CSmartGroupUser(const std::string &callsign, int32_t timer, int32_t timeout) :
m_callsign(callsign),
m_timer((unsigned int)timer),
m_timeout((unsigned int)timeout)
{
}

CSmartGroupUser::~CSmartGroupUser()
{
}

std::string CSmartGroupUser::getCallsign() const
{
	return m_callsign;
}

unsigned int CSmartGroupUser::getTimer() const
{
	return m_timer;
}

unsigned int CSmartGroupUser::getTimeout() const
{
	return m_timeout;
}
