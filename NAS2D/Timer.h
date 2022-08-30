// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2020 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgment of your use of NAS2D is appreciated but is not required.
// ==================================================================================

#pragma once

namespace NAS2D
{

	/**
	 * A timing class that provides high-resolution, millisecond-precision timing services.
	 *
	 * The Timer class provides elapsed time from the Timer start/initialization time.
	 *
	 * The reference start time can be moved forward relative by a specified number of ticks
	 * in case a new event needs to be measured relative to a previous event.
	 * This relative adjustment can be used to avoid issues with jitter,
	 * caused by the processing time for the old event before the new event is setup.
	 *
	 * \section Raw Tick
	 *
	 * A static method is provided for the raw time in ticks.
	 *
	 * With the current implementation, raw ticks are since app startup, and wrap back to 0 after about 49 days.
	 */
	class Timer
	{
	public:
		static unsigned int tick();

		Timer();
		Timer(unsigned int startTick);

		Timer(const Timer&) = default;
		Timer& operator=(const Timer&) = default;

		unsigned int elapsedTicks() const;
		void adjustStartTick(unsigned int ticksForward);

		[[deprecated("Replaced by `elapsedTicks`")]]
		unsigned int accumulator() const;
		[[deprecated("Replaced by `adjustStartTick`")]]
		void adjust_accumulator(unsigned int ticksForward);

		void reset();

	private:
		unsigned int mStartTick;
	};

} // namespace
