#pragma once
#ifndef GEODESY_CORE_LGC_THREAD_CONTROLLER_H
#define GEODESY_CORE_LGC_THREAD_CONTROLLER_H

// thread_controller exists as a one size fits
// all thread contoller, for each thread to 
// impart control on neighboring threads by ID.
//

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>
#include <map>

namespace geodesy::core::lgc {

	class thread_controller {
	public:

		thread_controller();
		~thread_controller();

		// Controls the thread loop, and enforces a time step.
		bool cycle(double aTimeStep);

		// Thread Control Functions.
		double work_time();
		double halt_time();
		double total_time();

		void suspend();
		void suspend(std::thread::id aID);
		void suspend(std::vector<std::thread::id> aID);
		void suspend_all();

		void resume(std::thread::id aID);
		void resume(std::vector<std::thread::id> aID);
		void resume_all();

		void terminate();
		void terminate(std::thread::id aID);
		void terminate(std::vector<std::thread::id> aID);
		void terminate_all();

		bool exists();

	private:

		struct data {
			// Is it suspended, trapped, or need to be terminated?
			bool Suspend;
			bool Trapped;
			bool Terminate;
			//double SleepTime;

			// Time Control for the thread in question.
			double t1, t2;
			double wt, ht;
			double dt;
			double ts;

			data();
		};

		std::mutex Mutex;
		std::map<std::thread::id, data> ThreadData;

	};

}

#endif // !GEODESY_CORE_LGC_THREAD_CONTROLLER_H
