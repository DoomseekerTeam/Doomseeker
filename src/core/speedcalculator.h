//------------------------------------------------------------------------------
// speedcalculator.h
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
#ifndef __SPEEDCALCULATOR_H__
#define __SPEEDCALCULATOR_H__

#include <QElapsedTimer>
#include <QVector>

class SpeedCalculator
{
public:
	SpeedCalculator();

	/**
	 * @brief In seconds.
	 *
	 * @return A negative value is returned if time cannot be calculated
	 * properly.
	 */
	long double estimatedTimeUntilArrival() const;

	qint64 expectedDataSize() const { return dataSizeExpected; }

	/**
	 * @brief In bytes per second.
	 *
	 * @return Non-negative value if speed was calculated properly.
	 *         A negative value if it was impossible to calculate speed.
	 */
	long double getSpeed() const;

	/**
	 * @brief Last amount of data registered through registerDataAmount()
	 *        method.
	 *
	 * If SpeedCalculator is used properly this will equal to
	 * total amount of downloaded data.
	 */
	qint64 lastRegisteredDataAmount() const;

	/**
	 * @brief Last amount of data that was passed to registerDataAmount()
	 *
	 * It doesn't matter whether the data was actually registered for the
	 * speed calculation or not.
	 */
	qint64 lastRegisterAttemptedDataAmount() const
	{
		return lastRegisterAttemptDataSize;
	}

	/**
	 * @brief Register new total amount of data.
	 *
	 * This will be used to calculate speed and ETA. Internally this will
	 * store data only once per second. It is not a problem to call
	 * this method more often as in such case it will change no data
	 * inside the object of this class.
	 */
	void registerDataAmount(qint64 totalAmountOfArrivedData);

	/**
	 * @brief Maximum expected size of the data.
	 *
	 * This value is used to calculate ETA.
	 */
	void setExpectedDataSize(qint64 size);

	/**
	 * @brief Clears all values. Prepares SpeedCalculator for new speed
	 * measure.
	 *
	 * This will clear arrivalData and dataSizeExpected variables and
	 * restart the clock.
	 */
	void start();

private:
	class DataArrivalInfo
	{
	public:
		/**
		 * @brief In bytes.
		 */
		qint64 totalAmountOfArrivedData;

		/**
		 * @brief In miliseconds.
		 */
		qint64 timeOfArrival;

		DataArrivalInfo()
		{
			this->totalAmountOfArrivedData = 0;
			this->timeOfArrival = 0;
		}

		DataArrivalInfo(qint64 totalAmountOfArrivedData, qint64 timeOfArrival)
		{
			this->totalAmountOfArrivedData = totalAmountOfArrivedData;
			this->timeOfArrival = timeOfArrival;
		}
	};

	static const int NUM_ARRIVAL_DATA = 2;

	QVector<DataArrivalInfo> arrivalData;

	QElapsedTimer clock;
	qint64 dataSizeExpected;

	/**
	 * @brief Holds last amount of data that was passed to
	 *        registerDataAmount() method.
	 */
	qint64 lastRegisterAttemptDataSize;
};

#endif
