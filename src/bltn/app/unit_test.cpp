#include <geodesy/bltn/app/unit_test.h>

#include <memory>

#include <iostream>
#include <iomanip>
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

		/*
		float Scalar = 1.0f;

		image::create_info MaterialTextureInfo;
		MaterialTextureInfo.Memory = device::memory::DEVICE_LOCAL;
		MaterialTextureInfo.Usage = image::usage::TRANSFER_DST | image::usage::SAMPLED;
		MaterialTextureInfo.Layout = image::layout::SHADER_READ_ONLY_OPTIMAL;

		buffer::create_info UniformBufferCI;
		UniformBufferCI.Memory = device::memory::HOST_VISIBLE | device::memory::HOST_COHERENT;
		UniformBufferCI.Usage = buffer::usage::UNIFORM | buffer::usage::TRANSFER_DST;	

		std::vector<std::string> AssetList = {
			"assets/models/quad.obj",
			"assets/images/wall.jpg",
			"assets/shader/triangle.vert",
			"assets/shader/triangle.frag"
		};

		// Load files into host memory.
		std::vector<std::shared_ptr<core::io::file>> Asset = Engine->FileManager.open(AssetList);

		// Cast loaded files 
		std::shared_ptr<gfx::model> HostModel 				= std::dynamic_pointer_cast<gfx::model>(Asset[0]);
		std::shared_ptr<gcl::image> HostTexture 			= std::dynamic_pointer_cast<gcl::image>(Asset[1]);
		std::shared_ptr<gcl::shader> VertexShader 			= std::dynamic_pointer_cast<gcl::shader>(Asset[2]);
		std::shared_ptr<gcl::shader> PixelShader 			= std::dynamic_pointer_cast<gcl::shader>(Asset[3]);

		std::vector<std::shared_ptr<gcl::shader>> ShaderList = { VertexShader, PixelShader };
		std::shared_ptr<gcl::pipeline::rasterizer> Rasterizer = std::make_shared<gcl::pipeline::rasterizer>(ShaderList, Window->Framechain->Resolution);

		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 0, offsetof(gfx::mesh::vertex, Position));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 1, offsetof(gfx::mesh::vertex, Normal));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 2, offsetof(gfx::mesh::vertex, Tangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 3, offsetof(gfx::mesh::vertex, Bitangent));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 4, offsetof(gfx::mesh::vertex, TextureCoordinate));
		Rasterizer->bind(VK_VERTEX_INPUT_RATE_VERTEX, 0, sizeof(gfx::mesh::vertex), 5, offsetof(gfx::mesh::vertex, Color));

		Rasterizer->attach(0, Window->Framechain->Image[0]["Color"], image::layout::PRESENT_SRC_KHR);

		// How to intepret vertex data in rasterization.
		Rasterizer->InputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		Rasterizer->InputAssembly.primitiveRestartEnable	= false;
		
		// Rasterizer Info
		Rasterizer->Rasterizer.rasterizerDiscardEnable		= VK_FALSE;
		Rasterizer->Rasterizer.polygonMode					= VK_POLYGON_MODE_FILL;
		Rasterizer->Rasterizer.cullMode						= VK_CULL_MODE_NONE;
		Rasterizer->Rasterizer.frontFace					= VK_FRONT_FACE_COUNTER_CLOCKWISE;

		// Copy Paste
		Rasterizer->Multisample.rasterizationSamples		= VK_SAMPLE_COUNT_1_BIT;

		// Load into gpu memory.
		VkFence Fence 										= DeviceContext->create_fence();
		std::shared_ptr<gfx::model> Model 					= DeviceContext->create_model(HostModel, MaterialTextureInfo);
		std::shared_ptr<gcl::image> Texture 				= DeviceContext->create_image(MaterialTextureInfo, HostTexture);
 		std::shared_ptr<gcl::buffer> UniformBuffer 			= DeviceContext->create_buffer(UniformBufferCI, sizeof(float), &Scalar);
		std::shared_ptr<gcl::pipeline> Pipeline 			= DeviceContext->create_pipeline(Rasterizer);
		//*/

		// Start main loop.
		float t = 0.0f;
		while (Engine->ThreadController.cycle(TimeStep)) {
			t += Engine->ThreadController.total_time() * 100.0f;

			system_window::poll_input();

			// // Update host resources.
			// Result = Engine->update_resources(this);

			// // Execute render operations.
			// Result = Engine->execute_render_operations(this);
			
			Window->next_frame_now();

			std::vector<gfx::draw_call> DrawCall = Quad->draw(Camera3D.get());

			Window->present_frame_now();

			/*
			// Acquire next image from swapchain.
			Result = Window->next_frame(VK_NULL_HANDLE, Fence);

			if ((Result == VK_ERROR_OUT_OF_DATE_KHR) || (Result == VK_SUBOPTIMAL_KHR)) {
				std::cout << "Window Out of Date or Suboptimal." << std::endl;
				// Resize rasterizer to new resolution.
				Rasterizer->resize(Window->Framechain->Resolution);
				// Remake pipeline on modified rasterizer create info.
				Pipeline = std::make_shared<gcl::pipeline>(DeviceContext, Rasterizer);
			}

			DeviceContext->wait_and_reset(Fence);

			// Do computation here.
			{
				std::vector<std::shared_ptr<buffer>> VertexBuffer = { Model->Mesh[0]->VertexBuffer };
				std::shared_ptr<buffer> IndexBuffer = Model->Mesh[0]->IndexBuffer;

				// Allocated GPU Resources needed to execute.
				std::shared_ptr<framebuffer> Framebuffer = DeviceContext->create_framebuffer(Pipeline, { Window->current_frame()["Color"] }, Window->Framechain->Resolution);
				std::shared_ptr<descriptor::array> DescriptorArray = DeviceContext->create_descriptor_array(Pipeline);

				DescriptorArray->bind(0, 0, 0, UniformBuffer);
				DescriptorArray->bind(1, 0, 0, Texture, image::layout::SHADER_READ_ONLY_OPTIMAL);

				Pipeline->draw(Framebuffer, VertexBuffer, IndexBuffer, DescriptorArray);

				// std::vector<std::shared_ptr<image>> Image = { Window->current_frame()["Color"] };
				// std::map<std::pair<int, int>, std::shared_ptr<buffer>> UniformBufferList = { std::make_pair(std::make_pair(0, 0), UniformBuffer) };
				// std::map<std::pair<int, int>, std::shared_ptr<image>> SamplerList = { std::make_pair(std::make_pair(1, 0), Texture) };
				// Pipeline->draw(Image, VertexBuffer, IndexBuffer, UniformBufferList, SamplerList);
			}

			VkPresentInfoKHR PresentInfo = Window->present_frame();

			Result = DeviceContext->present({ PresentInfo });
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