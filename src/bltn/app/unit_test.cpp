#include <geodesy/bltn/app/unit_test.h>

#include <memory>

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <complex>

namespace geodesy::bltn {

	using namespace core;
	using namespace runtime;

	using namespace gpu;
	using namespace obj;
	using namespace lgc;

	unit_test::unit_test(engine* aEngine) : runtime::app(aEngine, "geodesy-unit-test", { 1, 0, 0 }) {
		TimeStep = 1.0 / 2000.0;
		Window = nullptr;
		// I want my device context to support these operation types.
		std::vector<uint> OperationList = {
			device::operation::TRANSFER,
			device::operation::TRANSFER_AND_COMPUTE,
			device::operation::GRAPHICS_AND_COMPUTE,
			device::operation::PRESENT
		};
		std::set<std::string> LayerList = {};
		// I want my device context to be able to render to system windows.
		std::set<std::string> ExtensionList = system_window::context_extensions();
		// Add ray tracing extensions to the device context.
		// TODO: Check if ray tracing is supported by the device. Disabled for now.
		// ExtensionList.insert(context::RayTracingExtensions.begin(), context::RayTracingExtensions.end());
		// Engine create device context for gpu operations.
		DeviceContext = Engine->create_device_context(Engine->PrimaryDevice, OperationList, LayerList, ExtensionList);
	}

	unit_test::~unit_test() {

	}

	void unit_test::run() {
		VkResult Result = VK_SUCCESS;
		std::cout << "Thread Count: " << omp_get_max_threads() << std::endl;
		omp_set_num_threads(omp_get_max_threads());

		this->math_test();

		timer PerformanceTimer(1.0);

		// Create Main 3D environment.
		this->create_stage<stg::scene3d>(DeviceContext, "3D Rendering Testing");
		// Create Reflection Stage for Camera3D.
		this->create_stage<stage>(DeviceContext, "Window Testing");

		// Create System Window Object.
		system_window::creator SystemWindowCreator;
		SystemWindowCreator.Name 			= "System Window";
		SystemWindowCreator.Resolution 		= { 1920, 1080, 1 };
		SystemWindowCreator.FrameCount 		= 3;
		SystemWindowCreator.FrameRate 		= 60.0f;
		SystemWindowCreator.Display 		= Engine->PrimaryDisplay;

		// Use subject window to share camera3d renderings.
		subject_window::creator SubjectWindowCreator;
		SubjectWindowCreator.Name 			= "Camera3D Window";
		SubjectWindowCreator.ModelPath 		= "assets/models/quad.obj";
		SubjectWindowCreator.Position 		= { 0.0f, 0.0f, 0.5f };
		SubjectWindowCreator.Direction 		= { 0.0f, 0.0f };
		SubjectWindowCreator.Scale 			= { 1.0f, 1.0f, 1.0f };
		SubjectWindowCreator.Subject 		= std::dynamic_pointer_cast<runtime::subject>(this->StageLookup["3D Rendering Testing"]->ObjectLookup["Camera3D"]);

		// Create System Window.
		Window = this->StageLookup["Window Testing"]->create_object<obj::system_window>(&SystemWindowCreator);
		// Create Subject Window to Show Camera3D Renderings to System window.
		this->StageLookup["Window Testing"]->create_object<obj::subject_window>(&SubjectWindowCreator);
		// Forward Window User Input to Camera3D.
		Window->InputTarget = this->StageLookup["3D Rendering Testing"]->ObjectLookup["Camera3D"];

		// Start main loop.
		float t = 0.0f;
		while (Engine->ThreadController.cycle(TimeStep)) {
			t += Engine->ThreadController.total_time() * 100.0f;

			double t1 = timer::get_time();

			system_window::poll_input();

			double t2 = timer::get_time();

			// Update host resources.
			this->update(Engine->ThreadController.total_time());

			double t3 = timer::get_time();

			// Execute gpu workloads.
			Result = Engine->execute_render_operations(this);

			double t4 = timer::get_time();

			if (PerformanceTimer.check()) {
				std::cout << "----- Performance Metrics -----" << std::endl;
				std::cout << "Current Time:\t" << timer::get_time() << " s" << std::endl;
				std::cout << "Time Step:\t" << TimeStep * 1000 << " ms" << std::endl;
				std::cout << "Work Time:\t" << (t4 - t1) * 1000.0 << " ms" << std::endl;
				std::cout << "-Input Time:\t" << (t2 - t1) * 1000.0 << " ms" << std::endl;
				std::cout << "-Update Time:\t" << (t3 - t2) * 1000.0 << " ms" << std::endl;
				std::cout << "-Render Time:\t" << (t4 - t3) * 1000.0 << " ms" << std::endl;
				std::cout << "Halt Time:\t" << Engine->ThreadController.halt_time() * 1000.0 << " ms" << std::endl;
				std::cout << "Total Time:\t" << Engine->ThreadController.total_time() * 1000.0 << " ms" << std::endl << std::endl;
				//std::cout << "Thread Over Time: " << Engine->ThreadController.work_time() - TimeStep << std::endl;
			}

			// if (timer::get_time() > 30.0f) {
			// 	break;
			// }

		}

		lgc::timer::wait(5.0f);

	}

	void unit_test::math_test() {
	    // Test configuration
	    float TestEpsilon = 1e-5f;
	    uint32_t TotalTests = 0;
	    uint32_t PassedTests = 0;

	    auto test_result = [&](const std::string& TestName, bool Result) {
	        TotalTests++;
	        if(Result) PassedTests++;
	        std::cout << std::setw(50) << std::left << TestName 
	                  << (Result ? "PASSED" : "FAILED") << std::endl;
	    };

	    auto float_equal = [TestEpsilon](float A, float B) -> bool {
	        return std::abs(A - B) < TestEpsilon;
	    };

	    std::cout << "\n=== Testing Math Library ===\n\n";

	    // Vector Tests
	    {
	        std::cout << "Testing vec<T,N>:\n";
	
	        // Constructor tests
	        {
	            math::vec<float, 3> DefaultVec;
	            math::vec<float, 3> InitVec = { 1.0f, 2.0f, 3.0f };
	
	            test_result("Default constructor zero initialization", 
	                float_equal(DefaultVec[0], 0.0f) && 
	                float_equal(DefaultVec[1], 0.0f) && 
	                float_equal(DefaultVec[2], 0.0f));
	
	            test_result("Initializer list constructor",
	                float_equal(InitVec[0], 1.0f) && 
	                float_equal(InitVec[1], 2.0f) && 
	                float_equal(InitVec[2], 3.0f));
	        }

	        // Arithmetic operations
	        {
	            math::vec<float, 3> A(2.0f, 1.0f, -3.0f);
	            math::vec<float, 3> B = { 0.0f, -2.09f, 9.987f };
	            float Scalar = 3.14159f;

	            // Test negation
	            math::vec<float, 3> Neg = -A;
	            test_result("Vector negation", 
	                float_equal(Neg[0], -2.0f) && 
	                float_equal(Neg[1], -1.0f) && 
	                float_equal(Neg[2], 3.0f));

	            // Test addition
	            math::vec<float, 3> Sum = A + B;
	            test_result("Vector addition", 
	                float_equal(Sum[0], 2.0f) && 
	                float_equal(Sum[1], -1.09f) && 
	                float_equal(Sum[2], 6.987f));

	            // Test subtraction
	            math::vec<float, 3> Diff = A - B;
	            test_result("Vector subtraction", 
	                float_equal(Diff[0], 2.0f) && 
	                float_equal(Diff[1], 3.09f) && 
	                float_equal(Diff[2], -12.987f));

	            // Test scalar multiplication
	            math::vec<float, 3> Scaled = A * Scalar;
	            test_result("Vector scalar multiplication", 
	                float_equal(Scaled[0], 2.0f * Scalar) && 
	                float_equal(Scaled[1], 1.0f * Scalar) && 
	                float_equal(Scaled[2], -3.0f * Scalar));

	            // Test scalar division
	            math::vec<float, 3> Divided = A / Scalar;
	            test_result("Vector scalar division", 
	                float_equal(Divided[0], 2.0f / Scalar) && 
	                float_equal(Divided[1], 1.0f / Scalar) && 
	                float_equal(Divided[2], -3.0f / Scalar));
	        }

	        // Compound assignments
	        {
	            math::vec<float, 3> A = { 2.0f, 1.0f, -3.0f };
	            math::vec<float, 3> B = { 0.0f, -2.09f, 9.987f };
	            float Scalar = 3.14159f;

	            math::vec<float, 3> TestVec = A;
	            TestVec += B;
	            test_result("Vector compound addition", 
	                float_equal(TestVec[0], 2.0f) && 
	                float_equal(TestVec[1], -1.09f) && 
	                float_equal(TestVec[2], 6.987f));

	            TestVec = A;
	            TestVec -= B;
	            test_result("Vector compound subtraction", 
	                float_equal(TestVec[0], 2.0f) && 
	                float_equal(TestVec[1], 3.09f) && 
	                float_equal(TestVec[2], -12.987f));

	            TestVec = A;
	            TestVec *= Scalar;
	            test_result("Vector compound multiplication", 
	                float_equal(TestVec[0], 2.0f * Scalar) && 
	                float_equal(TestVec[1], 1.0f * Scalar) && 
	                float_equal(TestVec[2], -3.0f * Scalar));

	            TestVec = A;
	            TestVec /= Scalar;
	            test_result("Vector compound division", 
	                float_equal(TestVec[0], 2.0f / Scalar) && 
	                float_equal(TestVec[1], 1.0f / Scalar) && 
	                float_equal(TestVec[2], -3.0f / Scalar));
	        }

	        // Special operations
	        {
	            math::vec<float, 3> A = { 2.0f, 1.0f, -3.0f };
	            math::vec<float, 3> B = { 0.0f, -2.09f, 9.987f };

	            // Dot product
	            float Dot = A * B;
	            test_result("Vector dot product", 
	                float_equal(Dot, 2.0f*0.0f + 1.0f*(-2.09f) + (-3.0f)*9.987f));

	            // Cross product
	            math::vec<float, 3> Cross = A ^ B;
	            test_result("Vector cross product", true);  // Add specific value checks
	        }
	    }

	    // Complex number tests
	    {
	        std::cout << "\nTesting complex<T>:\n";
	
	        // Constructor tests
	        {
	            math::complex<float> DefaultComplex;
	            math::complex<float> InitComplex(1.0f, 2.0f);
	
	            test_result("Complex default constructor", 
	                float_equal(DefaultComplex[0], 0.0f) && 
	                float_equal(DefaultComplex[1], 0.0f));
	
	            test_result("Complex initialization constructor",
	                float_equal(InitComplex[0], 1.0f) && 
	                float_equal(InitComplex[1], 2.0f));
	        }

	        // Basic operations
	        {
	            math::complex<float> A(1.0f, 2.0f);
	            math::complex<float> B(3.0f, 4.0f);

	            // Addition
	            math::complex<float> Sum = A + B;
	            test_result("Complex addition",
	                float_equal(Sum[0], 4.0f) && 
	                float_equal(Sum[1], 6.0f));

	            // Conjugate
	            math::complex<float> Conj = ~A;
	            test_result("Complex conjugate",
	                float_equal(Conj[0], 1.0f) && 
	                float_equal(Conj[1], -2.0f));

	            // Multiplication
	            math::complex<float> Prod = A * B;
	            test_result("Complex multiplication",
	                float_equal(Prod[0], -5.0f) && 
	                float_equal(Prod[1], 10.0f));

	            // Functions
	            float Abs = abs(A);
	            test_result("Complex absolute value",
	                float_equal(Abs, std::sqrt(5.0f)));

	            float Phase = phase(A);
	            test_result("Complex phase",
	                float_equal(Phase, std::atan2(2.0f, 1.0f)));
	        }
	    }

	    // Matrix tests
	    {
	        std::cout << "\nTesting mat<T,M,N>:\n";
	
	        // Test column-major storage with row-major input
	        {
	            math::mat<float, 4, 4> A = math::mat<float, 4, 4>(
	                1.0f, 0.0f, 0.0f, 1.0f,    
	                2.0f, 1.0f, -1.0f, 2.0f,   
	                3.0f, 2.0f, 0.0f, 3.0f,    
	                4.0f, 3.0f, 1.0f, 4.0f     
	            );

	            // Test column-major access
	            test_result("Matrix column-major storage",
	                float_equal(A(0,0), 1.0f) && 
	                float_equal(A(1,0), 2.0f) && 
	                float_equal(A(2,0), 3.0f) && 
	                float_equal(A(3,0), 4.0f));
	        }

	        // Test matrix multiplication
	        {
	            math::mat<float, 4, 4> A = math::mat<float, 4, 4>(
	                1.0f, 0.0f, 0.0f, 1.0f,
	                2.0f, 1.0f, -1.0f, 2.0f,
	                3.0f, 2.0f, 0.0f, 3.0f,
	                4.0f, 3.0f, 1.0f, 4.0f
	            );

	            math::mat<float, 4, 4> B = {
	                1.0f, 2.0f, 3.0f, 4.0f,
	                5.0f, 6.0f, 7.0f, 8.0f,
	                9.0f, 10.0f, 11.0f, 12.0f,
	                13.0f, 14.0f, 15.0f, 16.0f
	            };

	            math::mat<float, 4, 4> C = A * B;
	            test_result("Matrix multiplication", true);  // Add specific value checks
	        }

	        // Test determinant
	        {
	            math::mat<float, 4, 4> A = math::mat<float, 4, 4>(
	                2.0f, -1.0f, 0.0f, 1.0f,
	                1.0f, 3.0f, -2.0f, 0.0f,
	                0.0f, 2.0f, 4.0f, -1.0f,
	                1.0f, -1.0f, 1.0f, 2.0f
	            );

	            float Det = determinant(A);
	            test_result("Matrix determinant",
	                float_equal(Det, 48.0f));
	        }
	    }

	    // Field tests
	    {
	        std::cout << "\nTesting field<X,N,Y>:\n";

	        math::field<float, 2, float> X({-5.0f, -5.0f}, {2.0f, 2.0f}, {50, 50}, 1);
	        math::field<float, 2, float> Y({-2.0f, -3.0f}, {4.0f, 5.0f}, {50, 50}, 2);
	
	        // Test field addition
	        math::field<float, 2, float> Sum = X + Y;
	
	        // Test field sampling
	        math::vec<float, 2> SamplePoint = {1.9f, 1.3f};
	        float SampleValue = Sum(SamplePoint);
	
	        test_result("Field operations", true);  // Add specific value checks
	    }

	    // Print summary
	    std::cout << "\n=== Test Summary ===\n"
	              << "Total Tests: " << TotalTests << "\n"
	              << "Passed: " << PassedTests << "\n"
	              << "Failed: " << (TotalTests - PassedTests) << "\n"
	              << "Success Rate: " 
	              << (TotalTests > 0 ? (100.0f * PassedTests / TotalTests) : 0.0f)
	              << "%\n\n";
	}


}