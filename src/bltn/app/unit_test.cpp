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

		// std::shared_ptr<camera3d> Camera3D = std::make_shared<camera3d>(DeviceContext, nullptr, "Camera3D", Resolution, 0.333, 4);
		// std::shared_ptr<object> Quad = std::dynamic_pointer_cast<object>(std::make_shared<triangle>(DeviceContext, nullptr, "Quad"));

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

		// Start main loop.
		float t = 0.0f;
		while (Engine->ThreadController.cycle(TimeStep)) {
			t += Engine->ThreadController.total_time() * 100.0f;

			system_window::poll_input();

			// // Update host resources.
			// Result = Engine->update_resources(this);

			// // Execute render operations.
			// Result = Engine->execute_render_operations(this);

			///*
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