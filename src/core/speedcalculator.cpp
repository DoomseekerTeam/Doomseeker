//------------------------------------------------------------------------------
// speedcalculator.cpp
//------------------------------------------------------------------------------
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//
//------------------------------------------------------------------------------
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "speedcalculator.h"

SpeedCalculator::SpeedCalculator()
{
	dataSizeExpected = 0;
	lastRegisterAttemptDataSize = 0;
}

long double SpeedCalculator::estimatedTimeUntilArrival() const
{
	long double speed = getSpeed();
	if (speed == 0.0)
	{
		return -1.0;
	}

	const DataArrivalInfo &endInfo = arrivalData.last();
	unsigned currentData = endInfo.totalAmountOfArrivedData;

	// This happens if dataSizeExpected isn't set properly.
	if (dataSizeExpected < currentData)
	{
		return -1.0;
	}

	unsigned remainingData = dataSizeExpected - currentData;
	return (double)remainingData / speed;
}

long double SpeedCalculator::getSpeed() const
{
	if (arrivalData.size() < 2)
	{
		return -1.0;
	}

	// Let's calculate weighted mean.
	long double numerator = 0.0;
	long double denominator = 0.0;

	numerator = arrivalData.last().totalAmountOfArrivedData - arrivalData.first().totalAmountOfArrivedData;
	denominator = arrivalData.last().timeOfArrival - arrivalData.first().timeOfArrival;

	long double speed = numerator / denominator;

	// Scale speed to 1 second (multiply by 1000ms).
	speed *= 1000.0;

	return speed;
}

qint64 SpeedCalculator::lastRegisteredDataAmount() const
{
	if (arrivalData.isEmpty())
	{
		return 0;
	}

	return arrivalData.last().totalAmountOfArrivedData;
}

void SpeedCalculator::registerDataAmount(qint64 totalAmountOfArrivedData)
{
	DataArrivalInfo dataArrivalInfo(totalAmountOfArrivedData, clock.elapsed());
	this->lastRegisterAttemptDataSize = totalAmountOfArrivedData;

	if (arrivalData.isEmpty())
	{
		arrivalData << dataArrivalInfo;
	}
	else
	{
		DataArrivalInfo &lastPacket = arrivalData.last();
		if (lastPacket.timeOfArrival + 1000 < dataArrivalInfo.timeOfArrival)
		{
			// Circulate the DataArrivalInfo objects.
			arrivalData << dataArrivalInfo;
		}
	}

	if (arrivalData.size() > NUM_ARRIVAL_DATA)
	{
		arrivalData.pop_front();
	}
}

void SpeedCalculator::setExpectedDataSize(qint64 size)
{
	dataSizeExpected = size;
}

void SpeedCalculator::start()
{
	arrivalData.clear();
	dataSizeExpected = 0;
	lastRegisterAttemptDataSize = 0;
	clock.start();
}
