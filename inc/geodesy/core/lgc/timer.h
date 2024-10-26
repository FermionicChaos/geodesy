#pragma once
#ifndef GEODESY_CORE_LGC_TIMER_H
#define GEODESY_CORE_LGC_TIMER_H

#include <mutex>

namespace geodesy::core::lgc {

	class timer {
	public:

		// Enter in a desired duration.
		timer();
		timer(double aDuration);

		timer& operator=(double aDuration);

		bool check();

		static double get_time();
		static void set_time(double aNewTime);
		static void wait(double aSeconds);

	private:
	
		double StartTime;
		double Duration;

	};

}

#endif // !GEODESY_CORE_LGC_TIMER_H
