#include <geodesy/core/lgc/timer.h>

#include <cmath>
#include <thread>
#include <mutex>

//#define USE_PLATFORM_TIME

#ifdef USE_PLATFORM_TIME
#include <geodesy/bltn/obj/system_window.h>
#endif

namespace geodesy::core::lgc {

	//std::chrono::steady_clock::time_point timer::ApplicationStartTimePoint = std::chrono::high_resolution_clock::now();
	static std::chrono::steady_clock::time_point ApplicationStartTimePoint = std::chrono::steady_clock::now();

	timer::timer() {
		// this->Duration = std::chrono::duration<double>(0.0);
		// this->StartingTimePoint = std::chrono::high_resolution_clock::now();
		this->StartTime = get_time();
		this->Duration = 0.0;
	}

	timer::timer(double aDuration) : timer() {
		*this = aDuration;
	}

	timer& timer::operator=(double aDuration) {
		//this->Duration = std::chrono::duration<double>(aDuration);
		this->Duration = aDuration;
		return *this;
	}

	bool timer::check() {
		double CurrentTime = get_time();
		if (CurrentTime - StartTime >= this->Duration) {
			StartTime += this->Duration;
			return true;
		}
		else {
			return false;
		}
	}

	double timer::get_time() {
#ifndef USE_PLATFORM_TIME
		return std::chrono::duration<double>(std::chrono::steady_clock::now() - ApplicationStartTimePoint).count();
#else
		return bltn::obj::system_window::get_time();
#endif		
	}

	void timer::set_time(double aNewTime) {
#ifndef USE_PLATFORM_TIME
		//BaseTimePoint = std::chrono::high_resolution_clock::now();// - std::chrono::duration<double>(aNewTime);
#else
		bltn::obj::system_window::set_time(aNewTime);
#endif
	}

	void timer::wait(double aSeconds) {
#ifndef USE_PLATFORM_TIME
		std::this_thread::sleep_for(std::chrono::duration<double>(aSeconds));
#else
		bltn::obj::system_window::wait(aSeconds);
#endif
	}

}

