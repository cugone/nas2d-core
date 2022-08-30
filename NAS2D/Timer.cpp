// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2020 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgment of your use of NAS2D is appreciated but is not required.
// ==================================================================================

#include "Timer.h"

#include <SDL2/SDL.h>


using namespace NAS2D;


unsigned int Timer::tick()
{
	return SDL_GetTicks();
}


Timer::Timer() :
	Timer{Timer::tick()}
{}


Timer::Timer(unsigned int startTick) :
	mStartTick{startTick}
{}


unsigned int Timer::elapsedTicks() const
{
	return tick() - mStartTick;
}


void Timer::adjustStartTick(unsigned int ticksForward)
{
	mStartTick += ticksForward;
}


unsigned int Timer::accumulator() const
{
	return elapsedTicks();
}


void Timer::adjust_accumulator(unsigned int ticksForward)
{
	adjustStartTick(ticksForward);
}


void Timer::reset()
{
	mStartTick = tick();
}
