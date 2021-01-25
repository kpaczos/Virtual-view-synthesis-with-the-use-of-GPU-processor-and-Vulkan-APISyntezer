#pragma once
#include "xCommon.h"
#include "xPlane.h"
#include "xSequence.h"
#include "xSynthesizerFast.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

namespace AVlib::Vulkan {

	//=====================================================================================================================================================================================
	struct Data {
		double c_PIP[4][4];
		int32  c_DstMinDepthValue2;
		int32  c_DstMaxDepthValue2;
		float c_DstZnear2;
		float c_DstZfar2;
		int32  c_SrcMinDepthValue2;
		int32  c_SrcMaxDepthValue2;
		float c_SrcZnear2;
		float c_SrcZfar2;
		int32  c_Width2;
		int32  c_Height2;
		int32  c_BitDepth2;
		int32 zero;
		int32      SrcDepthStride2;
		int32      DstDepthStride2;

	};
	class xVulkan2
	{
	protected:
		static const char c_ValidationLayerName[];

	protected:
		//setup
		bool m_EnableValidationLayers = false;

		//vulkan - query pool
		uint64            m_TotalTransferSrcTicks = 0;
		uint64            m_TotalComputrTicks = 0;
		uint64            m_TotalTransferDstTicks = 0;
		//vulkan - query pool
		int32                        m_NumTimeStamps;
		vk::QueryPool                m_QueryPool;

		//internal - Instance
		std::vector<const char*>   m_EnabledLayers;
		std::vector<const char*>   m_EnabledExtensions;
		vk::Instance               m_Instance;
		vk::DebugReportCallbackEXT m_DebugReportCallback = nullptr;
		vk::DispatchLoaderDynamic  m_DispatchLoaderDynamic;

		//internal - Device
		vk::PhysicalDevice m_PhysicalDevice;
		int32              m_QueueFamilyIdx = NOT_VALID;
		vk::Device         m_LogicalDevice;
		vk::Queue          m_Queue;

		//test
		vk::Buffer       m_HostUniformBuffer;
		vk::DeviceMemory m_HostUniformMemory;

		vk::Buffer       m_HostUniformBuffer2;
		vk::DeviceMemory m_HostUniformMemory2;

		xPlane<float>    m_SrcPlane;
		vk::Buffer       m_HostSrcBuffer;
		vk::DeviceMemory m_HostSrcMemory;

		vk::Buffer       m_HostSrcBuffer2;
		vk::DeviceMemory m_HostSrcMemory2;

		vk::Buffer       m_DeviceSrcBuffer;
		vk::DeviceMemory m_DeviceSrcMemory;

		vk::Buffer       m_DeviceSrcBuffer2;
		vk::DeviceMemory m_DeviceSrcMemory2;

		vk::Buffer       uni_DeviceSrcBuffer;
		vk::DeviceMemory uni_DeviceSrcMemory;

		vk::Buffer       uni_DeviceSrcBuffer2;
		vk::DeviceMemory uni_DeviceSrcMemory2;

		vk::Buffer       m_DeviceDstBuffer;
		vk::DeviceMemory m_DeviceDstMemory;

		vk::Buffer       m_DeviceDstBuffer2;
		vk::DeviceMemory m_DeviceDstMemory2;

		vk::Buffer       m_DeviceDstBuffer3;
		vk::DeviceMemory m_DeviceDstMemory3;

		vk::Buffer       m_DeviceDstBuffer4;
		vk::DeviceMemory m_DeviceDstMemory4;

		vk::Buffer       m_DeviceDstBuffer5;
		vk::DeviceMemory m_DeviceDstMemory5;

		vk::Buffer       m_HostDstBuffer;
		vk::DeviceMemory m_HostDstMemory;

		vk::Buffer       m_HostDstBuffer2;
		vk::DeviceMemory m_HostDstMemory2;

		vk::Buffer       m_HostDstBuffer3;
		vk::DeviceMemory m_HostDstMemory3;

		//TXT
		vk::Buffer			 m_HostSrcBufferTXT;
		vk::DeviceMemory m_HostSrcMemoryTXT;

		vk::Buffer			 m_DeviceSrcBufferTXT;
		vk::DeviceMemory m_DeviceSrcMemoryTXT;

		vk::Buffer			 m_HostSrcBufferTXT2;
		vk::DeviceMemory m_HostSrcMemoryTXT2;

		vk::Buffer			 m_DeviceSrcBufferTXT2;
		vk::DeviceMemory m_DeviceSrcMemoryTXT2;

		vk::Buffer			 m_HostUniformBuffer3;
		vk::DeviceMemory m_HostUniformMemory3;

		vk::Buffer			 uni_DeviceSrcBuffer3;
		vk::DeviceMemory uni_DeviceSrcMemory3;

		vk::Buffer       m_DeviceDstBufferTXT;
		vk::DeviceMemory m_DeviceDstMemoryTXT;

		vk::Buffer       m_HostDstBufferTXT;
		vk::DeviceMemory m_HostDstMemoryTXT;

		vk::Buffer       m_DeviceDstBufferTXT2;
		vk::DeviceMemory m_DeviceDstMemoryTXT2;

		vk::Buffer       m_HostDstBufferTXT2;
		vk::DeviceMemory m_HostDstMemoryTXT2;

		vk::Buffer       m_HostDstBufferTXT3;
		vk::DeviceMemory m_HostDstMemoryTXT3;

		vk::Buffer       m_DeviceDstBufferTXT3;
		vk::DeviceMemory m_DeviceDstMemoryTXT3;

		vk::Buffer       m_DeviceDstBufferTXT4;
		vk::DeviceMemory m_DeviceDstMemoryTXT4;

		vk::Buffer       m_HostDstBufferTXT4;
		vk::DeviceMemory m_HostDstMemoryTXT4;
//INPAINTG
		vk::Buffer       m_DeviceDstBufferINPAINT1;
		vk::DeviceMemory m_DeviceDstMemoryINPAINT1;

		vk::Buffer       m_DeviceDstBufferINPAINT2;
		vk::DeviceMemory m_DeviceDstMemoryINPAINT2;

		vk::Buffer       m_DeviceDstBufferINPAINT3;
		vk::DeviceMemory m_DeviceDstMemoryINPAINT3;

		vk::Buffer       m_DeviceDstBufferINPAINT4;
		vk::DeviceMemory m_DeviceDstMemoryINPAINT4;
			
		vk::Buffer       m_DeviceDstBufferINPAINT5;
		vk::DeviceMemory m_DeviceDstMemoryINPAINT5;

		vk::Buffer       m_HostDstBufferINPAINT1;
		vk::DeviceMemory m_HostDstMemoryINPAINT1;
		//<<<<<<< HEAD
		vk::Buffer       m_HostDstBuffer4;
		vk::DeviceMemory m_HostDstMemory4;

		vk::Buffer       m_HostDstBuffer5;
		vk::DeviceMemory m_HostDstMemory5;


		xPlane<float>    m_DstPlane;
		xSeqRAW m_SeqSrc;
		xSeqRAW m_SeqDst;
		xSeqRAW m_SeqSrc2;
		xSeqRAW m_SeqDst2;
		xSeqRAW m_SeqDst3;
		xSeqRAW m_SeqDst4;
		xSeqRAW m_SeqDst5;

		//=======
				//src sequences
		xSeqRAW m_SrcDpthSeq[2];
		xSeqRAW m_SrcTextSeq[2];
		xSeqRAW m_DstTextSeq;
		xSeqRAW m_DstTextSeq2;
		xSeqRAW m_DstTextSeq3;
		xSeqRAW m_DstTextSeq4;
		xSeqRAW m_DstTextSeq5;
		//>>>>>>> 0124c6b0ade636ab9eff2450951ea7917638932a

				//src buffers
		xPlane<uint32> m_SrcDpthPlane[2];
		xPic  <uint16> m_SrcTextPicOrg[2];
		xPic  <uint16> m_SrcTextPic444[2];
		xPicI <uint16> m_SrcTextPicI[2]; //interleaved

		xPicI <uint16> m_DstTextPicI;
		xPic  <uint16> m_DstTextPic444;
		xPic  <uint16> m_DstTextPic420;
		xPicI <uint16> m_DstTextPicI_2;
		xPic  <uint16> m_DstTextPic444_2;
		xPic  <uint16> m_DstTextPic420_2;
		xPicI <uint16> m_DstTextPicI_3;
		xPic  <uint16> m_DstTextPic444_3;
		xPic  <uint16> m_DstTextPic420_3;
		xPicI <uint16> m_DstTextPicI_4;
		xPic  <uint16> m_DstTextPic444_4;
		xPic  <uint16> m_DstTextPic420_4;
		xPicI <uint16> m_DstTextPicI_5;
		xPic  <uint16> m_DstTextPic444_5;
		xPic  <uint16> m_DstTextPic420_5;


		//dst buffers - TODO
		xPlane<uint32> m_PlaneDst;
		xPlane<uint32> m_PlaneDst2;
		xPlane<uint32> m_PlaneDst3;
		xPlane<uint32> m_PlaneDst4;
		xPlane<uint32> m_PlaneDst5;
		xPlane<uint32> m_PlaneDst6;

		//	xPlane<float>    m_DstPlane;
			//xSeqRAW m_SeqDst;
			//xSeqRAW m_SeqDst2;
			//xSeqRAW m_SeqDst3;

		vk::DescriptorSetLayout m_DescriptorSetLayout;
		vk::DescriptorSetLayout m_DescriptorSetLayout2;
		vk::DescriptorSetLayout m_DescriptorSetLayout3;
		vk::DescriptorSetLayout m_DescriptorSetLayout4;
		vk::DescriptorSetLayout m_DescriptorSetLayout5;
		vk::DescriptorSetLayout m_DescriptorSetLayout6;
		vk::DescriptorSetLayout m_DescriptorSetLayout7;
		vk::DescriptorSetLayout m_DescriptorSetLayout8;
		vk::DescriptorSetLayout m_DescriptorSetLayout9;
		vk::DescriptorPool      m_DescriptorPool;
		vk::DescriptorSet       m_DescriptorSet;
		vk::DescriptorSet       m_DescriptorSet2;
		vk::DescriptorSet       m_DescriptorSet3;
		vk::DescriptorSet       m_DescriptorSet4;
		vk::DescriptorSet       m_DescriptorSet5;
		vk::DescriptorSet       m_DescriptorSet6;
		vk::DescriptorSet       m_DescriptorSet7;
		vk::DescriptorSet       m_DescriptorSet8;
		vk::DescriptorSet       m_DescriptorSet9;

		vk::PipelineLayout      m_PipelineLayout;
		vk::Pipeline            m_Pipeline;
		vk::PipelineLayout      m_PipelineLayout2;
		vk::Pipeline            m_Pipeline2;
		vk::PipelineLayout      m_PipelineLayout3;
		vk::Pipeline            m_Pipeline3;
		vk::PipelineLayout      m_PipelineLayout4;
		vk::Pipeline            m_Pipeline4;
		vk::PipelineLayout      m_PipelineLayout5;
		vk::Pipeline            m_Pipeline5;
		vk::PipelineLayout      m_PipelineLayout6;
		vk::Pipeline            m_Pipeline6;
		vk::PipelineLayout      m_PipelineLayout7;
		vk::PipelineLayout      m_PipelineLayout8;
		vk::PipelineLayout      m_PipelineLayout9;
		vk::Pipeline            m_Pipeline7;
		vk::Pipeline            m_Pipeline8;
		vk::Pipeline            m_Pipeline9;

		vk::CommandBuffer       m_CommandBuffer;
		int32 m_LocalGroupSizeX = 1;
		int32 m_LocalGroupSizeY = 1;

	public:
		static constexpr int32 c_SizeX = 1920;
		static constexpr int32 c_SizeY = 1080;


		//static void xCombineDepthSTD();


		void  printInstanceLayers();
		void  printInstanceExtensions();
		bool  createInstance();
		void  printPhysicalDevices();
		bool  findPhysicalDevice(vk::PhysicalDeviceType PreferedPhysicalDeviceType = vk::PhysicalDeviceType::eDiscreteGpu);
		void  printPhysicalDeviceProperties();
		void  printPhysicalDeviceExtensions();
		bool  createDevice();

		bool  readQueryPool();
		void  printTimeStats(int32 NumFrames);

		bool  createTestStorageBuffers();
		void copyToStorageBuffer();
		bool  createQueryPool(int32 QueryCount);
		bool  createTestDescriptorSetLayout();
		bool  createTestDescriptorSetLayout2();
		bool	createTestDescriptorSetLayout3();
		bool  createTestDescriptorSetLayout4();
		bool  createTestDescriptorSetLayout5();
		bool  createTestDescriptorPool();
		bool  createTestDescriptorSet();

		bool  createTestPipeline();
		bool  createTestPipeline2();
		bool  createTestCommandBuffer();
		bool  executeTestCommandBuffer();


		void xRun();
		void printBuffers();
		bool copyBackTestStorageBuffers();



		bool  createBuffer(vk::Buffer& Buffer, vk::DeviceMemory& Memory, vk::BufferUsageFlags BufferUsageFlags, vk::MemoryPropertyFlags MemoryPropertyFlags, vk::DeviceSize Size);
		bool  copyToHostMemory(vk::DeviceMemory& Memory, vk::DeviceSize Offset, vk::DeviceSize Size, const void* Data, bool HostCoherent);
		bool readFromHostMemory(vk::DeviceMemory& Memory, vk::DeviceSize Offset, vk::DeviceSize Size, void* Data, bool HostCoherent);


	protected:
		static VKAPI_ATTR vk::Bool32 VKAPI_CALL xDebugReportCallback(vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);

		int32 xGetComputeQueueFamilyIndex(vk::QueueFlags RequiredQueueFlags = vk::QueueFlagBits::eCompute);
		int32 xDetermineMemoryTypeIdx(vk::MemoryRequirements MemoryRequirements, vk::MemoryPropertyFlags MemoryPropertyFlags);

		std::vector<uint32> xReadShader(const std::string& FilePath);


	};

	//=====================================================================================================================================================================================

} //end of namespace AVLib::Vulkan
