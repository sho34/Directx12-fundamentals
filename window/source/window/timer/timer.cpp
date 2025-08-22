#include "timer.h"
/*
* 
* -> we gonna use this timing class for all sorts of things that require time mesurement 
*/



Timer::Timer()
	:
	mSecondsPerCount(0.0f),
	mDeltaTime(-1.0f),

	mBaseTime(0),
	mPausedTime(0),
	mPrevTime(0),
	mCurrTime(0)
{
	// query the frequency of the peformance counter
	__int64 countsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec);
	// number of seconds per count
	mSecondsPerCount = 1.0f / (double)countsPerSec;

}

float Timer::Time() const
{
	return 0.0f;
}

float Timer::DeltaTime() const
{
	return (float)mDeltaTime;
}

float Timer::TotalTime()
{
	if (mStopped)
	{
		return (float)(((mStopTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}
	else
	{
		return (float)(((mCurrTime - mPausedTime) - mBaseTime) * mSecondsPerCount);
	}


}

void Timer::Reset()
{
	__int64 currTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	mBaseTime = currTime;
	// There is no previous time when the application starts 
	// so we need to set this value to the current time b4 the message loop starts 
	mPrevTime = currTime;
	mStopTime = 0;
	mStopped  = false;
}

void Timer::Start()
{
	__int64 startTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	// if we are resuming the timer from the stopped state 
	if (mStopped)
	{
		// accumulate the the paused time 
		mPausedTime += (startTime - mStopTime);
		// since we are starting the timer from a paused state 
		// the previous time is not valid here 
		mPrevTime = startTime;
		// no longer stopped 
		mStopped = 0;
		mStopped = 0;
	}
}

void Timer::Stop()
{
	// if stopped don't do anything 
	if (!mStopped)
	{
		__int64 currTime = 0;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		mStopTime = currTime;
		mStopped = true;
	}
}

void Timer::Tick()
{
	if (mStopped)
	{
		mDeltaTime = 0.0f;
		return;
	}

	// Get the time for this frame
	__int64 currTime = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	mCurrTime = currTime;

	// time difference between this frame and the previous frame
	// in seconds
	mDeltaTime = (mCurrTime - mPrevTime) * mSecondsPerCount;

	//prepare for the next frame 
	mPrevTime = mCurrTime;

	// Force nonnegative. The DXSDK's CDXUTTimer mentions that if the
	// processor goes into a power save mode or we get shuffled to another
	// processor, then mDeltaTime can be negative.

	if (mDeltaTime < 0.0f)
	{
		mDeltaTime = 0.0f;
	}
}
