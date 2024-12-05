#include <geodesy/bltn/app/unit_test.h>

#include <memory>

#include <iostream>
#include <algorithm>
#include <complex>

namespace geodesy::bltn {

	using namespace core;
	using namespace ecs;

	using namespace gcl;
	using namespace obj;
	using namespace lgc;

	unit_test::unit_test(engine* aEngine) : ecs::app(aEngine, "geodesy-unit-test", { 1, 0, 0 }) {
		std::vector<uint> OperationList = {
			device::operation::TRANSFER,
			device::operation::COMPUTE,
			//device::operation::TRANSFER | device::operation::COMPUTE,
			device::operation::GRAPHICS_AND_COMPUTE,
			device::operation::PRESENT
		};
		TimeStep = 1.0 / 60.0;
		DeviceContext = Engine->create_device_context(Engine->PrimaryDevice, OperationList);
		Window = nullptr;
	}

	unit_test::~unit_test() {

	}

	math::vec<uchar, 4> complex_to_color(math::complex<float> aValue) {
		math::vec<uchar, 4> Color;
		math::vec<float, 3> ColorFloat;

		// Calculate phase (Hue)
		float Phase = phase(aValue); // Phase is in radians
		float Magnitude = abs(aValue);
		// Calculate magnitude (Brightness)
		float Brightness = std::min(Magnitude, 1.0f); // Cap the brightness to 1.0

		// Determine color by phase.
		ColorFloat[2] = std::sin(Phase);
		ColorFloat[1] = std::sin(Phase - (2.0f * math::constant::pi / 3.0f));
		ColorFloat[0] = std::sin(Phase + (2.0f * math::constant::pi / 3.0f));

		ColorFloat[2] = Brightness * std::clamp(ColorFloat[2], 0.0f, 1.0f);
		ColorFloat[1] = Brightness * std::clamp(ColorFloat[1], 0.0f, 1.0f);
		ColorFloat[0] = Brightness * std::clamp(ColorFloat[0], 0.0f, 1.0f);

		Color[3] = 255;  // Fully opaque alpha channel
		Color[2] = static_cast<uchar>(ColorFloat[2] * 255.0f);
		Color[1] = static_cast<uchar>(ColorFloat[1] * 255.0f);
		Color[0] = static_cast<uchar>(ColorFloat[0] * 255.0f);

		return Color;
	}

	void unit_test::run() {
		VkResult Result = VK_SUCCESS;
		std::cout << "Thread Count: " << omp_get_max_threads() << std::endl;
		omp_set_num_threads(omp_get_max_threads());

		this->math_test();

		timer PerformanceTimer(1.0);

		system_window::create_info WindowCreateInfo;
		math::vec<uint, 3> Resolution = { 640, 480, 1 };
		WindowCreateInfo.Swapchain.FrameRate = 0.333;
		WindowCreateInfo.Swapchain.ImageUsage = image::usage::COLOR_ATTACHMENT | image::usage::TRANSFER_DST | image::usage::TRANSFER_SRC;
		Window = std::make_shared<system_window>(DeviceContext, Engine->PrimaryDisplay, std::string("Triangle Demo with Texture Data"), WindowCreateInfo, math::vec<int, 2>(0, 0), math::vec<int, 2>(Resolution[0], Resolution[1]));

		std::shared_ptr<camera3d> Camera3D = std::make_shared<camera3d>(DeviceContext, nullptr, "Camera3D", Resolution, 0.333, 4);
		std::shared_ptr<object> Quad = std::dynamic_pointer_cast<object>(std::make_shared<triangle>(DeviceContext, nullptr, "Quad"));

		// Start main loop.
		float t = 0.0f;
		while (Engine->ThreadController.cycle(TimeStep)) {
			t += Engine->ThreadController.total_time() * 100.0f;

			system_window::poll_input();

			std::vector<gfx::draw_call> DrawCall = Quad->draw(Camera3D.get());

			///*
			// Acquire next image from swapchain.
			Result = Window->next_frame_now();

			Result = Window->present_frame_now();

			//*/

			if (PerformanceTimer.check()) {
				math::vec<float, 2> SamplePoint = { 1.0f, 0.75f };
				std::cout << "----- Performance Metrics -----" << std::endl;
				std::cout << "Current Time:\t" << timer::get_time() << " s" << std::endl;
				std::cout << "Time Step:\t" << TimeStep * 1000 << " ms" << std::endl;
				std::cout << "Work Time:\t" << Engine->ThreadController.work_time() * 1000.0 << " ms" << std::endl;
				std::cout << "Halt Time:\t" << Engine->ThreadController.halt_time() * 1000.0 << " ms" << std::endl;
				std::cout << "Total Time:\t" << Engine->ThreadController.total_time() * 1000.0 << " ms" << std::endl << std::endl;
				//std::cout << "Thread Over Time: " << Engine->ThreadController.work_time() - TimeStep << std::endl;
			}

		}
		
	}

	void unit_test::math_test() {

		{
			// This section of the code will test the various methods and function calls
			// of the base class vec.h.
			float Scalar = 3.14159f;
			math::vec<float, 3> A = { 2.0f, 1.0f, -3.0 }, B = { 0.0f, -2.09f, 9.9870f }, C;
			//C = -A;
			C += B;
			C -= B;
			C *= Scalar;
			C /= Scalar;
			C /= Scalar;
			std::cout << "C = " << C << std::endl;
		}

		{
			// Test Complex Class
			float S = 3.14159f;
			float Out = 0.0f;
			math::complex<float> A(1.0f, 2.0f), B(3.0f, 4.0f), C;
			//C = -A;
			C = ~A;
			C += B;
			C -= B;
			C *= B;
			C /= B;
			Out = abs(A);
			Out = phase(A);
			//std::cout << "C = " << C << std::endl;
		}

		{
			// Test Quaternion Class
			float S = 3.14159f;
			float Out = 0.0f;
			math::quaternion<float> A(1.0f, 2.0f, 3.0f, 4.0f), B(5.0f, 6.0f, 7.0f, 8.0f), C;
			//C = -A;
			C = ~A;
			C += B;
			C -= B;
			C *= B;
			C /= B;
			Out = abs(A);
			//Out = phase(A);
			//std::cout << "C = " << C << std::endl;
		}

		{
			// Test matrix class
			math::mat<float, 4, 4> A, B, C;
			A = math::mat<float, 4, 4>(
				0.0f, 0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				1.0f, 0.0f, 0.0f, 0.0f
			);
			B = math::mat<float, 4, 4>(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
			C += B;
			C -= B;
			C = A * B;
			std::cout << "C = " << C << std::endl;
			C = A * B;
		}


		{
			// math::field<float, 1, float> R1(-5.0f, 5.0f, 100);
			// math::field<float, 2, float> R2({-5.0f, -5.0f }, { 5.0f, 5.0f }, { 100, 100 });
			// math::field<float, 3, float> R3({-5.0f, -5.0f, -5.0f }, { 5.0f, 5.0f, 5.0f }, { 100, 100, 100 });
			// R3({ 0u, 0u, 0u }) = 1.0f;

			math::field<float, 2, float> x({-5.0f, -5.0f }, { 2.0f, 2.0f }, { 50, 50 }, 1);
			math::field<float, 2, float> y({-2.0f, -3.0f }, { 4.0f, 5.0f }, { 50, 50 }, 2);
			math::field<float, 3, float> z({-5.0f, -5.0f, -5.0f }, { 5.0f, 5.0f, 5.0f }, { 50, 50, 50 }, 3);
			math::field<float, 2, float> f({-5.0f, -5.0f }, { 5.0f, 5.0f }, { 50, 50 });
			f = x + y;
			math::vec<float, 2> SamplePoint = { 1.9f, 1.3f };
			math::vec<float, 3> SamplePoint1 = { 2.2f, 1.3f, 3.4f };
			std::cout << "x(" << SamplePoint << ") = " << x(SamplePoint) << std::endl;
			std::cout << "y(" << SamplePoint << ") = " << y(SamplePoint) << std::endl;
			std::cout << "f(" << SamplePoint << ") = " << f(SamplePoint) << std::endl;
			std::cout << z(SamplePoint1) << std::endl;
		}

	}

}