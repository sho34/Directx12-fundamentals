#pragma once
#ifndef __timer__
#define __timer__


#include <windows.h>

class Timer
{
public:
	Timer();

	float Time() const;			// In seconds 
	float DeltaTime() const;	// In seconds 
	float TotalTime();			// In seconds 

	void Reset();				// call b4 message loop
	void Start();				// call when unpaused 
	void Stop();				// call when paused 
	void Tick();				// call Every frame 


private:
	double mSecondsPerCount	= 0.0f;
	double mDeltaTime		= 0.0f;

	__int64 mBaseTime		= 0;
	__int64 mPausedTime		= 0;
	__int64 mStopTime		= 0;
	__int64 mPrevTime		= 0;
	__int64 mCurrTime		= 0;

	bool mStopped			= false;
};

#endif // !__timer__

