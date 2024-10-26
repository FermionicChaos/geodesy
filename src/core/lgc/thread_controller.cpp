#include <geodesy/core/lgc/thread_controller.h>
#include <geodesy/core/lgc/timer.h>

// C++ STD
#include <thread>
#include <mutex>

#define GEODESY_CORE_LGC_THREAD_SLEEP_TIME 0.001

namespace geodesy::core::lgc {

	thread_controller::thread_controller() {}

	thread_controller::~thread_controller() {}

	bool thread_controller::cycle(double aTimeStep) {
		std::thread::id ID = std::this_thread::get_id();
		data td;

		// If thread does not exist, create it. Load data on stack.
		this->Mutex.lock();
		if (this->ThreadData.count(ID) == 0) {
			this->ThreadData[ID] = data();
			this->ThreadData[ID].t1 = timer::get_time();
		}
		td = this->ThreadData[ID];
		this->Mutex.unlock();

		// Lock thread if requested.
		td.Trapped = true;
		this->Mutex.lock();
		this->ThreadData[ID] = td;
		this->Mutex.unlock();
		while (td.Suspend) {
			timer::wait(0.001);
			this->Mutex.lock();
			td = this->ThreadData[ID];
			this->Mutex.unlock();
		}
		td.Trapped = false;

		// Set new timestep.
		td.ts = aTimeStep;

		// End of Time Cycle
		td.t2 = timer::get_time();

		// Wait if finished early.
		td.wt = td.t2 - td.t1;
		if (td.wt < td.ts) {
			td.ht = td.ts - td.wt;
			timer::wait(td.ht);
		}
		else {
			td.ht = 0.0;
		}
		td.dt = td.wt + td.ht;

		// Start of new cycle
		td.t1 = timer::get_time();

		// Store modified data, if thread is not terminated.
		this->Mutex.lock();
		this->ThreadData[ID] = td;
		if ((this->ThreadData.count(ID) > 0) && (td.Terminate)) {
			this->ThreadData.erase(ID);
		}
		this->Mutex.unlock();

		return !td.Terminate;
	}

	// Thread Control Functions.
	double thread_controller::work_time() {
		double WorkTime = 0.0;
		this->Mutex.lock();
		WorkTime = ThreadData[std::this_thread::get_id()].wt;
		this->Mutex.unlock();
		return WorkTime;
	}

	double thread_controller::halt_time() {
		double HaltTime = 0.0;
		this->Mutex.lock();
		HaltTime = ThreadData[std::this_thread::get_id()].ht;
		this->Mutex.unlock();
		return HaltTime;
	}

	double thread_controller::total_time() {
		double TotalTime = 0.0;
		this->Mutex.lock();
		TotalTime = ThreadData[std::this_thread::get_id()].dt;
		this->Mutex.unlock();
		return TotalTime;
	}

	void thread_controller::suspend() {
		this->suspend(std::this_thread::get_id());
	}

	void thread_controller::suspend(std::thread::id aID) {
		std::vector<std::thread::id> ID = { aID };
		this->suspend(ID);
	}

	void thread_controller::suspend(std::vector<std::thread::id> aID) {
		bool AllThreadsTrapped = false;
		while (!AllThreadsTrapped) {
			AllThreadsTrapped = true;
			this->Mutex.lock();

			// Flag all threads to suspend.
			for (size_t i = 0; i < aID.size(); i++) {
				// Self suspension is not allowed.
				if (std::this_thread::get_id() == aID[i]) continue;
				// Check if thread exists, then suspend.
				if (this->ThreadData.count(aID[i]) > 0) {
					this->ThreadData[aID[i]].Suspend = true;
				}
			}

			// Check if all threads are trapped.
			for (size_t i = 0; i < aID.size(); i++) {
				// Skip, do not wait for current thread to be suspended.
				if (std::this_thread::get_id() == aID[i]) continue;
				// Check if non-current threads have been trapped.
				if (this->ThreadData.count(aID[i]) > 0) {
					AllThreadsTrapped &= this->ThreadData[aID[i]].Trapped;
				}
			}

			this->Mutex.unlock();
			// Hand off back to other threads and wait for update.
			timer::wait(0.001);
		}
	}

	void thread_controller::suspend_all() {
		std::vector<std::thread::id> SuspendList;
		this->Mutex.lock();
		for (auto& [ID, Data] : this->ThreadData) {
			SuspendList.push_back(ID);
		}
		this->Mutex.unlock();
		this->suspend(SuspendList);
	}

	void thread_controller::resume(std::thread::id aID) {
		std::vector<std::thread::id> ID = { aID };
		this->resume(ID);
	}

	void thread_controller::resume(std::vector<std::thread::id> aID) {
		this->Mutex.lock();
		for (std::thread::id ID : aID) {
			if (this->ThreadData.count(ID) > 0) {
				this->ThreadData[ID].Suspend = false;
			}
		}
		this->Mutex.unlock();
	}

	void thread_controller::resume_all() {
		std::vector<std::thread::id> ResumeList;
		this->Mutex.lock();
		for (auto& [ID, Data] : this->ThreadData) {
			ResumeList.push_back(ID);
		}
		this->Mutex.unlock();
		this->resume(ResumeList);
	}

	void thread_controller::terminate() {
		this->terminate(std::this_thread::get_id());
	}

	void thread_controller::terminate(std::thread::id aID) {
		std::vector<std::thread::id> ID = {aID};
		this->terminate(ID);
	}

	void thread_controller::terminate(std::vector<std::thread::id> aID) {
		this->Mutex.lock();
		for (std::thread::id ID : aID) {
			if (this->ThreadData.count(ID) > 0) {
				this->ThreadData[ID].Terminate = true;
			}
		}
		this->Mutex.unlock();
	}

	void thread_controller::terminate_all() {
		std::vector<std::thread::id> TerminateList;
		this->Mutex.lock();
		for (auto& [ID, Data] : this->ThreadData) {
			TerminateList.push_back(ID);
		}
		this->Mutex.unlock();
		this->terminate(TerminateList);
	}

	bool thread_controller::exists() {
		return (this->ThreadData.count(std::this_thread::get_id()) > 0);
	}

	thread_controller::data::data() {
		this->Suspend = false;
		this->Trapped = false;
		this->Terminate = false;
		//this->SleepTime = 0.001;
		this->t1 = 0.0; this->t2 = 0.0;
		this->wt = 0.0; this->ht = 0.0;
		this->dt = 0.0;
		this->ts = 0.0;
	}

}
