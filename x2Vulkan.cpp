#include "x2Vulkan.h"
#include "xString.h"
#include "xReprojection.h"
#include <iostream>
#include <fstream>



namespace AVlib::Vulkan {

  //=====================================================================================================================================================================================
  std::vector<uint32_t> computeInput(4);
  std::vector<uint32_t> computeInput2(4);
  std::vector<uint32_t> computeOutput(4);
  std::vector<uint32_t> computeOutput2(4);
  vk::DeviceSize SizeOfBuffer0;
  vk::DeviceSize SizeOfBuffer1;
  vk::DeviceSize SizeOfBuffer2;
  const char xVulkan2::c_ValidationLayerName[] = "VK_LAYER_LUNARG_standard_validation";

  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  void xVulkan2::printInstanceLayers()
  {
    auto LayerProperties = vk::enumerateInstanceLayerProperties();
    if (LayerProperties.result != vk::Result::eSuccess) { fmt::printf("enumerateInstanceLayerProperties ERROR\n"); return; }
    fmt::printf("InstanceLayerProperties:\n");
    for (const vk::LayerProperties& Prop : LayerProperties.value) { fmt::printf("  Name=%s\n", Prop.layerName); }
  }
  void xVulkan2::printInstanceExtensions()
  {
    auto ExtensionProperties = vk::enumerateInstanceExtensionProperties();
    if (ExtensionProperties.result != vk::Result::eSuccess) { fmt::printf("enumerateInstanceExtensionProperties ERROR\n"); return; }
    fmt::printf("InstanceExtensionProperties:\n");
    for (const vk::ExtensionProperties& Prop : ExtensionProperties.value) { fmt::printf("  Name=%s\n", Prop.extensionName); }
  }
  bool xVulkan2::createInstance()
  {
    if (m_EnableValidationLayers)
    {
      //validation layer
      auto LayerProperties = vk::enumerateInstanceLayerProperties();
      if (LayerProperties.result != vk::Result::eSuccess) { fmt::printf("enumerateInstanceLayerProperties ERROR\n"); return false; }

      bool FoundLayer = false;
      for (const vk::LayerProperties& Prop : LayerProperties.value)
      {
        if (std::string_view(c_ValidationLayerName) == std::string_view(Prop.layerName)) { FoundLayer = true; break; }
      }
      if (!FoundLayer) { fmt::printf("Layer %s not supported\n", c_ValidationLayerName); return false; }
      m_EnabledLayers.push_back(c_ValidationLayerName);

      //debug export
      auto ExtensionProperties = vk::enumerateInstanceExtensionProperties();
      if (ExtensionProperties.result != vk::Result::eSuccess) { fmt::printf("enumerateInstanceExtensionProperties ERROR\n"); return false; }

      bool FoundExtension = false;
      for (const vk::ExtensionProperties& Prop : ExtensionProperties.value)
      {
        if (std::string_view(VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == std::string_view(Prop.extensionName)) { FoundExtension = true; break; }
      }

      if (!FoundExtension) { fmt::printf("Extension %s not supported\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME); return false; }
      m_EnabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    vk::ApplicationInfo ApplicationInfo;
    ApplicationInfo.pApplicationName = "AVlib";
    ApplicationInfo.applicationVersion = 0;
    ApplicationInfo.pEngineName = "AVlib";
    ApplicationInfo.engineVersion = 0;
    ApplicationInfo.apiVersion = VK_API_VERSION_1_0;;

    vk::InstanceCreateInfo InstanceCreateInfo;
    InstanceCreateInfo.pApplicationInfo = &ApplicationInfo;
    InstanceCreateInfo.enabledLayerCount = (uint32)m_EnabledLayers.size();
    InstanceCreateInfo.ppEnabledLayerNames = m_EnabledLayers.data();
    InstanceCreateInfo.enabledExtensionCount = (uint32)m_EnabledExtensions.size();
    InstanceCreateInfo.ppEnabledExtensionNames = m_EnabledExtensions.data();

    auto Instance = vk::createInstance(InstanceCreateInfo);
    if (Instance.result != vk::Result::eSuccess) { fmt::printf("createInstance ERROR\n"); return false; }
    m_Instance = Instance.value;

    m_DispatchLoaderDynamic.init(m_Instance, vkGetInstanceProcAddr);

    if (m_EnableValidationLayers)
    {
      vk::DebugReportCallbackCreateInfoEXT CreateInfo;
      CreateInfo.flags = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning;
      CreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)xDebugReportCallback;

      auto Callback = m_Instance.createDebugReportCallbackEXT(CreateInfo, nullptr, m_DispatchLoaderDynamic);
      if (Callback.result != vk::Result::eSuccess) { fmt::printf("createDebugReportCallbackEXT ERROR\n"); return false; }
      m_DebugReportCallback = Callback.value;
    }
    return true;
  }
  void xVulkan2::printPhysicalDevices()
  {
    auto PhysicalDevices = m_Instance.enumeratePhysicalDevices();
    if (PhysicalDevices.result != vk::Result::eSuccess) { fmt::printf("enumeratePhysicalDevices ERROR\n"); return; }
    if (PhysicalDevices.value.empty()) { fmt::printf("Could not find a device with vulkan support\n"); return; }
    fmt::printf("PhysicalDevices\n");
    for (const vk::PhysicalDevice& Device : PhysicalDevices.value) { fmt::printf("  Name=%s\n", Device.getProperties().deviceName); }
  }
  bool xVulkan2::findPhysicalDevice(vk::PhysicalDeviceType PreferedPhysicalDeviceType)
  {
    auto PhysicalDevices = m_Instance.enumeratePhysicalDevices();
    if (PhysicalDevices.result != vk::Result::eSuccess) { fmt::printf("enumeratePhysicalDevices ERROR\n"); return false; }
    if (PhysicalDevices.value.empty()) { fmt::printf("Could not find a device with vulkan support\n"); return false; }
    //first loop - find prefered
    for (const vk::PhysicalDevice& Device : PhysicalDevices.value)
    {
      vk::PhysicalDeviceProperties DeviceProperties = Device.getProperties();
      if (DeviceProperties.deviceType == PreferedPhysicalDeviceType) { m_PhysicalDevice = Device; return true; }
    }
    //second loop - find any
    for (const vk::PhysicalDevice& Device : PhysicalDevices.value) { m_PhysicalDevice = Device; return true; }
    return false;
  }
  void xVulkan2::printPhysicalDeviceProperties()
  {
    if (!m_PhysicalDevice) { fmt::printf("NO DEVICE\n"); return; }
    vk::PhysicalDeviceProperties DeviceProperties = m_PhysicalDevice.getProperties();
    fmt::printf("DEVICE Name = %s\n", DeviceProperties.deviceName);
  }
  void xVulkan2::printPhysicalDeviceExtensions()
  {
    if (!m_PhysicalDevice) { fmt::printf("NO DEVICE\n"); return; }
    auto ExtensionProperties = m_PhysicalDevice.enumerateDeviceExtensionProperties();
    fmt::printf("DeviceExtensionProperties\n");
    for (const vk::ExtensionProperties& Prop : ExtensionProperties.value) { fmt::printf("  Name=%s\n", Prop.extensionName); }
  }
  bool xVulkan2::createDevice()
  {
    const float DefaultQueuePriorities = 0.0;  // we only have one queue, so this is not that imporant. 
    m_QueueFamilyIdx = xGetComputeQueueFamilyIndex(); // find queue family with compute capability.

    // Request a single compute queue
    vk::DeviceQueueCreateInfo DeviceQueueCreateInfo;
    DeviceQueueCreateInfo.queueFamilyIndex = m_QueueFamilyIdx;
    DeviceQueueCreateInfo.queueCount = 1; // create one queue in this family. We don't need more.  
    DeviceQueueCreateInfo.pQueuePriorities = &DefaultQueuePriorities;

    // Create logical device
    vk::DeviceCreateInfo DeviceCreateInfo;
    DeviceCreateInfo.enabledLayerCount = m_EnabledLayers.size();  // need to specify validation layers here as well.
    DeviceCreateInfo.ppEnabledLayerNames = m_EnabledLayers.data();
    DeviceCreateInfo.pQueueCreateInfos = &DeviceQueueCreateInfo; // when creating the logical device, we also specify what queues it has.
    DeviceCreateInfo.queueCreateInfoCount = 1;
    auto LogicalDevice = m_PhysicalDevice.createDevice(DeviceCreateInfo);
    if (LogicalDevice.result != vk::Result::eSuccess) { fmt::printf("createDevice ERROR\n"); return false; }
    m_LogicalDevice = LogicalDevice.value;

    // Get one compute queue
    m_Queue = m_LogicalDevice.getQueue(m_QueueFamilyIdx, 0);

    return true;
  }


  bool xVulkan2::createTestStorageBuffers()
  {
    //input sequences
    const char* c_FileNameSrcDpth0 = "_gopro_hala_depth_cam14_1920x1080_cf420_16bps.yuvbf.yuvcut.yuv";
    const char* c_FileNameSrcDpth1 = "_gopro_hala_depth_cam16_1920x1080_cf420_16bps.yuvbf.yuvcut.yuv";
    const char* c_FileNameSrcText0 = "basketball_cam14_1920x1080.yuvdist.yuv_600_10bps.yuvcut.yuv";
    const char* c_FileNameSrcText1 = "basketball_cam16_1920x1080.yuvdist.yuv_600_10bps.yuvcut.yuv";

    //intermediate
    const char* c_FileNameDst = "CAM14NEW_depth_cam14_1920x1080_cf420_16bps.yuv";
    const char* c_FileNameDst2 = "CAM16NEW_depth_cam16_1920x1080_cf420_16bps.yuv";
    const char* c_FileNameDst3 = "Combine_2_depths_1920x1080_cf420_16bps.yuv";

    const char* c_FileNameDst4 = "Filtr_depths_1920x1080_cf420_16bps.yuv";

    const char* c_FileNameDst5 = "TXT_CAM14_1920x1080_cf420_10bps.yuv";

    const char* c_FileNameDst6 = "TXT_CAM16_1920x1080_cf420_10bps.yuv";

    const char* c_FileNameDst7 = "TXT_MERGE_1920x1080_cf420_10bps.yuv";

    const char* c_FileNameDst8 = "TXT_FILTR_1920x1080_cf420_10bps.yuv";

    const char* c_FileNameDst9 = "TXT_INPAINT_1920x1080_cf420_10bps.yuv";




    const int32  c_Width2 = 1920;
    const int32  c_Height2 = 1080;
    const int32  c_BitDepthD = 16;
    const int32  c_BitDepthT = 10;

    const int32  c_SrcMinDepthValue2 = 1;
    const int32  c_SrcMaxDepthValue2 = 65535;
    const float  c_SrcZnear2 = 3;
    const float  c_SrcZfar2 = 25;

    const int32  c_DstMinDepthValue2 = 1;
    const int32  c_DstMaxDepthValue2 = 65535;
    const float  c_DstZnear2 = 2.8321037923759862;
    const float  c_DstZfar2 = 25.855157835437637;

    m_SrcDpthSeq[0].createRead(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcDpth0, 0, nullptr);
    m_SrcDpthSeq[1].createRead(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcDpth1, 0, nullptr);
    m_SrcTextSeq[0].createRead(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcText0, 0, nullptr);
    m_SrcTextSeq[1].createRead(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcText1, 0, nullptr);

    m_SrcDpthPlane[0].create(c_Width2, c_Height2, 0, c_BitDepthD);
    m_SrcDpthPlane[1].create(c_Width2, c_Height2, 0, c_BitDepthD);

    m_SrcTextPicOrg[0].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);
    m_SrcTextPicOrg[1].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);

    m_SrcTextPic444[0].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_SrcTextPic444[1].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);

    m_SrcTextPicI[0].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_SrcTextPicI[1].create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);


    m_DstTextPicI.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_DstTextPic444.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_DstTextPic420.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);

    m_DstTextPicI_2.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_DstTextPic444_2.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_DstTextPic420_2.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);

    m_DstTextPicI_3.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_DstTextPic444_3.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_DstTextPic420_3.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);

    m_DstTextPicI_4.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_DstTextPic444_4.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_DstTextPic420_4.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);


    m_DstTextPicI_5.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV);
    m_DstTextPic444_5.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_444);
    m_DstTextPic420_5.create(c_Width2, c_Height2, 0, c_BitDepthT, eImgTp::YUV, eCrF::CrF_420);


    m_PlaneDst.create(c_Width2, c_Height2, 0, c_BitDepthD);
    m_PlaneDst2.create(c_Width2, c_Height2, 0, c_BitDepthD);
    m_PlaneDst3.create(c_Width2, c_Height2, 0, c_BitDepthD);
    m_PlaneDst4.create(c_Width2, c_Height2, 0, c_BitDepthD);

    m_PlaneDst5.create(c_Width2, c_Height2, 0, c_BitDepthD);


    m_SeqDst.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst, 0, nullptr);
    m_SeqDst2.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst2, 0, nullptr);
    m_SeqDst3.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst3, 0, nullptr);
    m_SeqDst4.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst4, 0, nullptr);
    m_DstTextSeq.createWrite(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst5, 0, nullptr);
    m_DstTextSeq2.createWrite(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst6, 0, nullptr);
    m_DstTextSeq3.createWrite(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst7, 0, nullptr);
    m_DstTextSeq4.createWrite(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst8, 0, nullptr);
    m_DstTextSeq5.createWrite(c_Width2, c_Height2, c_BitDepthT, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst9, 0, nullptr);






    //m_PlaneDst.create(c_Width2, c_Height2, 0, c_BitDepthD);
   // m_PlaneDst2.create(c_Width2, c_Height2, 0, c_BitDepthD);

    const int32      SrcDpthStride0 = m_SrcDpthPlane[0].getStride();
    const int32      SrcDpthStride1 = m_SrcDpthPlane[1].getStride();
    const int32      DstDpthStride = m_PlaneDst.getStride();
    const int32      DstDepthStride2 = m_PlaneDst2.getStride();


    const float c_PIP[16] =
    { 1.0010150855297955    ,
      -0.082387506664965582  ,
      62.692083139187616 ,
      802.08799930999703 ,
      0.032573031899749839  ,
      0.96302277533907743    ,
      15.303790643484831 ,
      -5.7150271402154544  ,
      7.4183068440710149e-06,
      -5.5324011958637084e-05,
      1.0221853157048744 ,
      -0.055368328028528868,
      0.00000000000000000  ,
      0.00000000000000000    ,
      0.00000000000000000,
      1.0000000000000002
    };



    float DataToShader[112] =
    {
      c_PIP[0]  ,
      c_PIP[1]   ,
      c_PIP[2]    ,
      c_PIP[3]  ,
      c_PIP[4]  ,
      c_PIP[5]  ,
      c_PIP[6]  ,
      c_PIP[7] ,
      c_PIP[8]  ,
      c_PIP[9]   ,
      c_PIP[10] ,
      c_PIP[11]  ,
      c_PIP[12]  ,
      c_PIP[13]   ,
      c_PIP[14]   ,
      c_PIP[15]  ,
      c_DstMinDepthValue2,
      c_DstMaxDepthValue2 ,
      c_DstZnear2 ,
      c_DstZfar2  ,
      c_SrcMinDepthValue2,
      c_SrcMaxDepthValue2 ,
      c_SrcZnear2 ,
      c_SrcZfar2  ,
      c_Width2    ,
      c_Height2   ,
      c_BitDepthD  ,
      0            ,
      SrcDpthStride0,
      DstDpthStride
    };


    const float c_PIP2[16] =
    {
      0.99762997090804550  , -0.014998058162516315   , 15.050814434681911 , -875.62258466292042 ,
      -0.016551252126842771, 0.97438437250392851     , 40.838965121062138 , -12.753434978281376  ,
      4.5915655905241399e-06, -3.4289612520733634e-05, 1.0139559976129453 , -0.013643813329712595,
      0.00000000000000000   , 0.00000000000000000    , 0.00000000000000000, 1.0000000000000000
    };


    float DataToShader2[112] =
    {
      c_PIP2[0]  ,
      c_PIP2[1]   ,
      c_PIP2[2]    ,
      c_PIP2[3]  ,
      c_PIP2[4]  ,
      c_PIP2[5]  ,
      c_PIP2[6]  ,
      c_PIP2[7] ,
      c_PIP2[8]  ,
      c_PIP2[9]   ,
      c_PIP2[10] ,
      c_PIP2[11]  ,
      c_PIP2[12]  ,
      c_PIP2[13]   ,
      c_PIP2[14]   ,
      c_PIP2[15]  ,
      c_DstMinDepthValue2,
      c_DstMaxDepthValue2 ,
      c_DstZnear2 ,
      c_DstZfar2  ,
      c_SrcMinDepthValue2,
      c_SrcMaxDepthValue2 ,
      c_SrcZnear2 ,
      c_SrcZfar2  ,
      c_Width2    ,
      c_Height2   ,
      c_BitDepthD  ,
      0            ,
      SrcDpthStride0,
      DstDpthStride
    };

    //<<<<<<< HEAD
    m_SeqSrc.createRead(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcDpth0, 0, nullptr);
    //m_SeqDst.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst, 0, nullptr);

    m_SeqSrc2.createRead(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameSrcDpth1, 0, nullptr);
    // m_SeqDst2.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst2, 0, nullptr);

    // m_SeqDst3.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst3, 0, nullptr);
    // m_SeqDst4.createWrite(c_Width2, c_Height2, c_BitDepthD, eImgTp::YUV, ePckTp::Planar, eCrF::CrF_420, eCmpO::YUV, c_FileNameDst4, 0, nullptr);

     // m_PlaneSrc.create(c_Width2, c_Height2, 0, c_BitDepthD);
     //m_PlaneDst.create(c_Width2, c_Height2, 0, c_BitDepthD);

     // m_PlaneSrc2.create(c_Width2, c_Height2, 0, c_BitDepthD);

    // m_PlaneDst2.create(c_Width2, c_Height2, 0, c_BitDepthD);
     //m_PlaneDst3.create(c_Width2, c_Height2, 0, c_BitDepthD);
     //m_PlaneDst4.create(c_Width2, c_Height2, 0, c_BitDepthD);
     // m_SeqSrc.readPlane(&m_PlaneSrc);
     // m_SeqSrc2.readPlane(&m_PlaneSrc2);
  //=======


     //>>>>>>> 0124c6b0ade636ab9eff2450951ea7917638932a

    vk::DeviceSize BufferSize0 = m_SrcDpthPlane[0].getBufferSize();
    vk::DeviceSize BufferSize1 = m_SrcDpthPlane[1].getBufferSize();
    vk::DeviceSize BufferSize2 = m_SrcTextPicI[0].getBuffSize();


    SizeOfBuffer0 = BufferSize0;
    SizeOfBuffer1 = BufferSize1;
    SizeOfBuffer2 = BufferSize2;
    //vulkan host uniform buffer
    createBuffer(m_HostUniformBuffer, m_HostUniformMemory, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);
    //vulkan host src buffer
    createBuffer(m_HostSrcBuffer, m_HostSrcMemory, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize0);

    //vulkan device src buffer
    createBuffer(m_DeviceSrcBuffer, m_DeviceSrcMemory, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize0);

    //vulkan device uniform buffer
    createBuffer(uni_DeviceSrcBuffer, uni_DeviceSrcMemory, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);

    //vulkan device dst buffer
    createBuffer(m_DeviceDstBuffer, m_DeviceDstMemory, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize0);

    //vulkan host dst buffer
    createBuffer(m_HostDstBuffer, m_HostDstMemory, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize0);


    //2 obraz
    //vulkan host uniform buffer
    createBuffer(m_HostUniformBuffer2, m_HostUniformMemory2, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);
    //vulkan host src buffer
    createBuffer(m_HostSrcBuffer2, m_HostSrcMemory2, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize1);

    //vulkan device src buffer
    createBuffer(m_DeviceSrcBuffer2, m_DeviceSrcMemory2, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize1);

    //vulkan device uniform buffer
    createBuffer(uni_DeviceSrcBuffer2, uni_DeviceSrcMemory2, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);

    //vulkan device dst buffer
    createBuffer(m_DeviceDstBuffer2, m_DeviceDstMemory2, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize1);

    //vulkan host dst buffer
    createBuffer(m_HostDstBuffer2, m_HostDstMemory2, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize1);

    //<<<<<<< HEAD
    createBuffer(m_DeviceDstBuffer3, m_DeviceDstMemory3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize1);
    createBuffer(m_DeviceDstBuffer4, m_DeviceDstMemory4, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize1);
    //=======
    //createBuffer(m_DeviceDstBuffer3, m_DeviceDstMemory3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize1);
    //>>>>>>> 0124c6b0ade636ab9eff2450951ea7917638932a

    createBuffer(m_HostDstBuffer3, m_HostDstMemory3, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize1);

    createBuffer(m_HostDstBuffer4, m_HostDstMemory4, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize1);
    //TXT
    //vulkan host src buffer TXT
    createBuffer(m_HostSrcBufferTXT, m_HostSrcMemoryTXT, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);
    //vulkan device src buffer TXT
    createBuffer(m_DeviceSrcBufferTXT, m_DeviceSrcMemoryTXT, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan host uniform buffer
    createBuffer(m_HostUniformBuffer3, m_HostUniformMemory3, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);
    //vulkan device uniform buffer
    createBuffer(uni_DeviceSrcBuffer3, uni_DeviceSrcMemory3, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, 1000);
    //vulkan host src buffer TXT2
    createBuffer(m_HostSrcBufferTXT2, m_HostSrcMemoryTXT2, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);
    //vulkan device src buffer TXT2
    createBuffer(m_DeviceSrcBufferTXT2, m_DeviceSrcMemoryTXT2, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan device dst buffer
    createBuffer(m_DeviceDstBufferTXT, m_DeviceDstMemoryTXT, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan host dst buffer
    createBuffer(m_HostDstBufferTXT, m_HostDstMemoryTXT, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);
    //vulkan device dst buffer
    createBuffer(m_DeviceDstBufferTXT2, m_DeviceDstMemoryTXT2, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan host dst buffer
    createBuffer(m_HostDstBufferTXT2, m_HostDstMemoryTXT2, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);

    createBuffer(m_DeviceDstBufferTXT3, m_DeviceDstMemoryTXT3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan host dst buffer
    createBuffer(m_HostDstBufferTXT3, m_HostDstMemoryTXT3, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);

    createBuffer(m_DeviceDstBufferTXT4, m_DeviceDstMemoryTXT4, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    //vulkan host dst buffer
    createBuffer(m_HostDstBufferTXT4, m_HostDstMemoryTXT4, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);

    //INPAINTING
    createBuffer(m_DeviceDstBufferINPAINT1, m_DeviceDstMemoryINPAINT1, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    createBuffer(m_DeviceDstBufferINPAINT2, m_DeviceDstMemoryINPAINT2, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    createBuffer(m_DeviceDstBufferINPAINT3, m_DeviceDstMemoryINPAINT3, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    createBuffer(m_DeviceDstBufferINPAINT4, m_DeviceDstMemoryINPAINT4, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);
    createBuffer(m_DeviceDstBufferINPAINT5, m_DeviceDstMemoryINPAINT5, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, BufferSize2);

    createBuffer(m_HostDstBufferINPAINT1, m_HostDstMemoryINPAINT1, vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, BufferSize2);


    copyToHostMemory(uni_DeviceSrcMemory, 0, 1000, DataToShader, true);
    copyToHostMemory(uni_DeviceSrcMemory2, 0, 1000, DataToShader2, true);

    copyToHostMemory(m_HostSrcMemory, 0, SizeOfBuffer0, m_SrcDpthPlane[0].getBuffer(), true);
    copyToHostMemory(m_HostSrcMemory2, 0, SizeOfBuffer1, m_SrcDpthPlane[1].getBuffer(), true);
    copyToHostMemory(m_HostSrcMemoryTXT, 0, SizeOfBuffer2, m_SrcTextPicI[0].getBuffer(), true);
    copyToHostMemory(m_HostSrcMemoryTXT2, 0, SizeOfBuffer2, m_SrcTextPicI[1].getBuffer(), true);

    return true;
  }
  bool xVulkan2::createTestDescriptorSetLayout()
  {
    vk::DescriptorSetLayoutBinding DescriptorSetLayoutBinding[5];
    DescriptorSetLayoutBinding[0].binding = 0;
    DescriptorSetLayoutBinding[0].descriptorType = vk::DescriptorType::eUniformBuffer;
    DescriptorSetLayoutBinding[0].descriptorCount = 1;
    DescriptorSetLayoutBinding[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding[1].binding = 1;
    DescriptorSetLayoutBinding[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding[1].descriptorCount = 1;
    DescriptorSetLayoutBinding[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding[2].binding = 2;
    DescriptorSetLayoutBinding[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding[2].descriptorCount = 1;
    DescriptorSetLayoutBinding[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding[3].binding = 3;
    DescriptorSetLayoutBinding[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding[3].descriptorCount = 1;
    DescriptorSetLayoutBinding[3].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding[4].binding = 4;
    DescriptorSetLayoutBinding[4].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding[4].descriptorCount = 1;
    DescriptorSetLayoutBinding[4].stageFlags = vk::ShaderStageFlagBits::eCompute;



    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
    DescriptorSetLayoutCreateInfo.bindingCount = 5;
    DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBinding;

    // Create the descriptor set layout. 
    auto CeatedDescriptorSetLayout = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo);
    if (CeatedDescriptorSetLayout.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout = CeatedDescriptorSetLayout.value;

    auto CeatedDescriptorSetLayout2 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo);
    if (CeatedDescriptorSetLayout2.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout2 = CeatedDescriptorSetLayout2.value;

    return true;
  }
  bool xVulkan2::createTestDescriptorSetLayout2()
  {
    vk::DescriptorSetLayoutBinding DescriptorSetLayoutBinding2[6];
    DescriptorSetLayoutBinding2[0].binding = 0;
    DescriptorSetLayoutBinding2[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[0].descriptorCount = 1;
    DescriptorSetLayoutBinding2[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding2[1].binding = 1;
    DescriptorSetLayoutBinding2[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[1].descriptorCount = 1;
    DescriptorSetLayoutBinding2[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding2[2].binding = 2;
    DescriptorSetLayoutBinding2[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[2].descriptorCount = 1;
    DescriptorSetLayoutBinding2[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding2[3].binding = 3;
    DescriptorSetLayoutBinding2[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[3].descriptorCount = 1;
    DescriptorSetLayoutBinding2[3].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding2[4].binding = 4;
    DescriptorSetLayoutBinding2[4].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[4].descriptorCount = 1;
    DescriptorSetLayoutBinding2[4].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding2[5].binding = 5;
    DescriptorSetLayoutBinding2[5].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding2[5].descriptorCount = 1;
    DescriptorSetLayoutBinding2[5].stageFlags = vk::ShaderStageFlagBits::eCompute;


    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo2;
    DescriptorSetLayoutCreateInfo2.bindingCount = 6;
    DescriptorSetLayoutCreateInfo2.pBindings = DescriptorSetLayoutBinding2;

    // Create the descriptor set layout. 
    auto CeatedDescriptorSetLayout3 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo2);
    if (CeatedDescriptorSetLayout3.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout3 = CeatedDescriptorSetLayout3.value;



    return true;
  }

  bool xVulkan2::createTestDescriptorSetLayout3()
  {
    vk::DescriptorSetLayoutBinding DescriptorSetLayoutBinding3[4];
    DescriptorSetLayoutBinding3[0].binding = 0;
    DescriptorSetLayoutBinding3[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding3[0].descriptorCount = 1;
    DescriptorSetLayoutBinding3[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding3[1].binding = 1;
    DescriptorSetLayoutBinding3[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding3[1].descriptorCount = 1;
    DescriptorSetLayoutBinding3[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding3[2].binding = 2;
    DescriptorSetLayoutBinding3[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding3[2].descriptorCount = 1;
    DescriptorSetLayoutBinding3[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding3[3].binding = 3;
    DescriptorSetLayoutBinding3[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding3[3].descriptorCount = 1;
    DescriptorSetLayoutBinding3[3].stageFlags = vk::ShaderStageFlagBits::eCompute;


    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo3;
    DescriptorSetLayoutCreateInfo3.bindingCount = 4;
    DescriptorSetLayoutCreateInfo3.pBindings = DescriptorSetLayoutBinding3;

    // Create the descriptor set layout. 
    auto CeatedDescriptorSetLayout4 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo3);
    if (CeatedDescriptorSetLayout4.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout4 = CeatedDescriptorSetLayout4.value;



    return true;
  }

  bool xVulkan2::createTestDescriptorSetLayout4()
  {
    vk::DescriptorSetLayoutBinding DescriptorSetLayoutBinding4[2];
    DescriptorSetLayoutBinding4[0].binding = 0;
    DescriptorSetLayoutBinding4[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding4[0].descriptorCount = 1;
    DescriptorSetLayoutBinding4[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding4[1].binding = 1;
    DescriptorSetLayoutBinding4[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding4[1].descriptorCount = 1;
    DescriptorSetLayoutBinding4[1].stageFlags = vk::ShaderStageFlagBits::eCompute;



    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo4;
    DescriptorSetLayoutCreateInfo4.bindingCount = 2;
    DescriptorSetLayoutCreateInfo4.pBindings = DescriptorSetLayoutBinding4;

    // Create the descriptor set layout. 
    auto CeatedDescriptorSetLayout5 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo4);
    if (CeatedDescriptorSetLayout5.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout5 = CeatedDescriptorSetLayout5.value;

    auto CeatedDescriptorSetLayout7 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo4);
    if (CeatedDescriptorSetLayout7.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout7 = CeatedDescriptorSetLayout7.value;

    auto CeatedDescriptorSetLayout8 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo4);
    if (CeatedDescriptorSetLayout8.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout8 = CeatedDescriptorSetLayout8.value;

    auto CeatedDescriptorSetLayout9 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo4);
    if (CeatedDescriptorSetLayout9.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout9 = CeatedDescriptorSetLayout9.value;




    return true;
  }


  bool xVulkan2::createTestDescriptorSetLayout5()
  {
    vk::DescriptorSetLayoutBinding DescriptorSetLayoutBinding5[7];
    DescriptorSetLayoutBinding5[0].binding = 0;
    DescriptorSetLayoutBinding5[0].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[0].descriptorCount = 1;
    DescriptorSetLayoutBinding5[0].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[1].binding = 1;
    DescriptorSetLayoutBinding5[1].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[1].descriptorCount = 1;
    DescriptorSetLayoutBinding5[1].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[2].binding = 2;
    DescriptorSetLayoutBinding5[2].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[2].descriptorCount = 1;
    DescriptorSetLayoutBinding5[2].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[3].binding = 3;
    DescriptorSetLayoutBinding5[3].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[3].descriptorCount = 1;
    DescriptorSetLayoutBinding5[3].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[4].binding = 4;
    DescriptorSetLayoutBinding5[4].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[4].descriptorCount = 1;
    DescriptorSetLayoutBinding5[4].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[5].binding = 5;
    DescriptorSetLayoutBinding5[5].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[5].descriptorCount = 1;
    DescriptorSetLayoutBinding5[5].stageFlags = vk::ShaderStageFlagBits::eCompute;
    DescriptorSetLayoutBinding5[6].binding = 6;
    DescriptorSetLayoutBinding5[6].descriptorType = vk::DescriptorType::eStorageBuffer;
    DescriptorSetLayoutBinding5[6].descriptorCount = 1;
    DescriptorSetLayoutBinding5[6].stageFlags = vk::ShaderStageFlagBits::eCompute;


    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo5;
    DescriptorSetLayoutCreateInfo5.bindingCount = 7;
    DescriptorSetLayoutCreateInfo5.pBindings = DescriptorSetLayoutBinding5;

    // Create the descriptor set layout. 
    auto CeatedDescriptorSetLayout6 = m_LogicalDevice.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo5);
    if (CeatedDescriptorSetLayout6.result != vk::Result::eSuccess) { fmt::printf("createDescriptorSetLayout ERROR\n"); return false; }
    m_DescriptorSetLayout6 = CeatedDescriptorSetLayout6.value;



    return true;
  }


  bool xVulkan2::createTestDescriptorPool()
  {
    vk::DescriptorPoolSize DescriptorPoolSize;
    DescriptorPoolSize.type = vk::DescriptorType::eStorageBuffer;
    DescriptorPoolSize.descriptorCount = 9;

    vk::DescriptorPoolCreateInfo DescriptorPoolCreateInfo;
    DescriptorPoolCreateInfo.maxSets = 9; // we only need to allocate one descriptor set from the pool.
    DescriptorPoolCreateInfo.poolSizeCount = 1;
    DescriptorPoolCreateInfo.pPoolSizes = &DescriptorPoolSize;

    auto CreatedDescriptorPool = m_LogicalDevice.createDescriptorPool(DescriptorPoolCreateInfo);
    if (CreatedDescriptorPool.result != vk::Result::eSuccess) { fmt::printf("createDescriptorPool ERROR\n"); return false; }
    m_DescriptorPool = CreatedDescriptorPool.value;

    return true;
  }
  bool xVulkan2::createTestDescriptorSet()
  {
    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo;
    DescriptorSetAllocateInfo.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayout;
    auto ResultDescriptorSets = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo);
    if (ResultDescriptorSets.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet = ResultDescriptorSets.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo2;
    DescriptorSetAllocateInfo2.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo2.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo2.pSetLayouts = &m_DescriptorSetLayout2;
    auto ResultDescriptorSets2 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo2);
    if (ResultDescriptorSets2.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet2 = ResultDescriptorSets2.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo3;
    DescriptorSetAllocateInfo3.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo3.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo3.pSetLayouts = &m_DescriptorSetLayout3;
    auto ResultDescriptorSets3 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo3);
    if (ResultDescriptorSets3.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet3 = ResultDescriptorSets3.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo4;
    DescriptorSetAllocateInfo4.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo4.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo4.pSetLayouts = &m_DescriptorSetLayout4;
    auto ResultDescriptorSets4 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo4);
    if (ResultDescriptorSets4.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet4 = ResultDescriptorSets4.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo5;
    DescriptorSetAllocateInfo5.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo5.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo5.pSetLayouts = &m_DescriptorSetLayout5;
    auto ResultDescriptorSets5 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo5);
    if (ResultDescriptorSets5.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet5 = ResultDescriptorSets5.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo6;
    DescriptorSetAllocateInfo6.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo6.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo6.pSetLayouts = &m_DescriptorSetLayout6;
    auto ResultDescriptorSets6 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo6);
    if (ResultDescriptorSets6.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet6 = ResultDescriptorSets6.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo7;
    DescriptorSetAllocateInfo7.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo7.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo7.pSetLayouts = &m_DescriptorSetLayout7;
    auto ResultDescriptorSets7 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo7);
    if (ResultDescriptorSets7.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet7 = ResultDescriptorSets7.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo8;
    DescriptorSetAllocateInfo8.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo8.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo8.pSetLayouts = &m_DescriptorSetLayout8;
    auto ResultDescriptorSets8 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo8);
    if (ResultDescriptorSets8.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet8 = ResultDescriptorSets8.value[0];

    vk::DescriptorSetAllocateInfo DescriptorSetAllocateInfo9;
    DescriptorSetAllocateInfo9.descriptorPool = m_DescriptorPool; // pool to allocate from.
    DescriptorSetAllocateInfo9.descriptorSetCount = 1; // allocate a single descriptor set.
    DescriptorSetAllocateInfo9.pSetLayouts = &m_DescriptorSetLayout9;
    auto ResultDescriptorSets9 = m_LogicalDevice.allocateDescriptorSets(DescriptorSetAllocateInfo9);
    if (ResultDescriptorSets9.result != vk::Result::eSuccess) { fmt::printf("allocateDescriptorSets ERROR\n"); return false; }
    m_DescriptorSet9 = ResultDescriptorSets9.value[0];


    // Specify the buffer to bind to the descriptor.
    vk::DescriptorBufferInfo DescriptorBufferInfo[35];
    DescriptorBufferInfo[0].buffer = uni_DeviceSrcBuffer;
    DescriptorBufferInfo[0].offset = 0;
    DescriptorBufferInfo[0].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[1].buffer = m_DeviceSrcBuffer;
    DescriptorBufferInfo[1].offset = 0;
    DescriptorBufferInfo[1].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[2].buffer = m_DeviceDstBuffer;
    DescriptorBufferInfo[2].offset = 0;
    DescriptorBufferInfo[2].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[3].buffer = m_DeviceSrcBufferTXT;
    DescriptorBufferInfo[3].offset = 0;
    DescriptorBufferInfo[3].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[4].buffer = m_DeviceDstBufferTXT;
    DescriptorBufferInfo[4].offset = 0;
    DescriptorBufferInfo[4].range = VK_WHOLE_SIZE;

    DescriptorBufferInfo[5].buffer = uni_DeviceSrcBuffer2;
    DescriptorBufferInfo[5].offset = 0;
    DescriptorBufferInfo[5].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[6].buffer = m_DeviceSrcBuffer2;
    DescriptorBufferInfo[6].offset = 0;
    DescriptorBufferInfo[6].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[7].buffer = m_DeviceDstBuffer2;
    DescriptorBufferInfo[7].offset = 0;
    DescriptorBufferInfo[7].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[8].buffer = m_DeviceSrcBufferTXT2;
    DescriptorBufferInfo[8].offset = 0;
    DescriptorBufferInfo[8].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[9].buffer = m_DeviceDstBufferTXT2;
    DescriptorBufferInfo[9].offset = 0;
    DescriptorBufferInfo[9].range = VK_WHOLE_SIZE;

    //merge 1,2 - depth wej, 3 - depth wyj, 4,5 - txt wej, 6 - -txt wyj 
    DescriptorBufferInfo[10].buffer = m_DeviceDstBuffer;
    DescriptorBufferInfo[10].offset = 0;
    DescriptorBufferInfo[10].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[11].buffer = m_DeviceDstBuffer2;
    DescriptorBufferInfo[11].offset = 0;
    DescriptorBufferInfo[11].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[12].buffer = m_DeviceDstBuffer3;
    DescriptorBufferInfo[12].offset = 0;
    DescriptorBufferInfo[12].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[13].buffer = m_DeviceDstBufferTXT;
    DescriptorBufferInfo[13].offset = 0;
    DescriptorBufferInfo[13].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[14].buffer = m_DeviceDstBufferTXT2;
    DescriptorBufferInfo[14].offset = 0;
    DescriptorBufferInfo[14].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[15].buffer = m_DeviceDstBufferTXT3;
    DescriptorBufferInfo[15].offset = 0;
    DescriptorBufferInfo[15].range = VK_WHOLE_SIZE;
    //FILTR
    DescriptorBufferInfo[16].buffer = m_DeviceDstBuffer3;
    DescriptorBufferInfo[16].offset = 0;
    DescriptorBufferInfo[16].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[17].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[17].offset = 0;
    DescriptorBufferInfo[17].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[18].buffer = m_DeviceDstBufferTXT3;
    DescriptorBufferInfo[18].offset = 0;
    DescriptorBufferInfo[18].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[19].buffer = m_DeviceDstBufferTXT4;
    DescriptorBufferInfo[19].offset = 0;
    DescriptorBufferInfo[19].range = VK_WHOLE_SIZE;
    //INPAINTING
    DescriptorBufferInfo[20].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[20].offset = 0;
    DescriptorBufferInfo[20].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[21].buffer = m_DeviceDstBufferINPAINT1;
    DescriptorBufferInfo[21].offset = 0;
    DescriptorBufferInfo[21].range = VK_WHOLE_SIZE;

    DescriptorBufferInfo[22].buffer = m_DeviceDstBufferINPAINT1;
    DescriptorBufferInfo[22].offset = 0;
    DescriptorBufferInfo[22].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[23].buffer = m_DeviceDstBufferINPAINT2;
    DescriptorBufferInfo[23].offset = 0;
    DescriptorBufferInfo[23].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[24].buffer = m_DeviceDstBufferINPAINT3;
    DescriptorBufferInfo[24].offset = 0;
    DescriptorBufferInfo[24].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[25].buffer = m_DeviceDstBufferINPAINT4;
    DescriptorBufferInfo[25].offset = 0;
    DescriptorBufferInfo[25].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[26].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[26].offset = 0;
    DescriptorBufferInfo[26].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[27].buffer = m_DeviceDstBufferTXT4;
    DescriptorBufferInfo[27].offset = 0;
    DescriptorBufferInfo[27].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[28].buffer = m_DeviceDstBufferINPAINT5;
    DescriptorBufferInfo[28].offset = 0;
    DescriptorBufferInfo[28].range = VK_WHOLE_SIZE;

    DescriptorBufferInfo[29].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[29].offset = 0;
    DescriptorBufferInfo[29].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[30].buffer = m_DeviceDstBufferINPAINT3;
    DescriptorBufferInfo[30].offset = 0;
    DescriptorBufferInfo[30].range = VK_WHOLE_SIZE;

    DescriptorBufferInfo[31].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[31].offset = 0;
    DescriptorBufferInfo[31].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[32].buffer = m_DeviceDstBufferINPAINT2;
    DescriptorBufferInfo[32].offset = 0;
    DescriptorBufferInfo[32].range = VK_WHOLE_SIZE;

    DescriptorBufferInfo[33].buffer = m_DeviceDstBuffer4;
    DescriptorBufferInfo[33].offset = 0;
    DescriptorBufferInfo[33].range = VK_WHOLE_SIZE;
    DescriptorBufferInfo[34].buffer = m_DeviceDstBufferINPAINT4;
    DescriptorBufferInfo[34].offset = 0;
    DescriptorBufferInfo[34].range = VK_WHOLE_SIZE;


    vk::WriteDescriptorSet WriteDescriptorSet[35];
    WriteDescriptorSet[0].dstSet = m_DescriptorSet; // write to this descriptor set.
    WriteDescriptorSet[0].dstBinding = 0; // write to the first, and only binding.
    WriteDescriptorSet[0].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[0].descriptorType = vk::DescriptorType::eUniformBuffer; // storage buffer.
    WriteDescriptorSet[0].pBufferInfo = &DescriptorBufferInfo[0];
    WriteDescriptorSet[1].dstSet = m_DescriptorSet; // write to this descriptor set.
    WriteDescriptorSet[1].dstBinding = 1; // write to the firjst, and only binding.
    WriteDescriptorSet[1].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[1].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[1].pBufferInfo = &DescriptorBufferInfo[1];
    WriteDescriptorSet[2].dstSet = m_DescriptorSet; // write to this descriptor set.
    WriteDescriptorSet[2].dstBinding = 2; // write to the first, and only binding.
    WriteDescriptorSet[2].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[2].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[2].pBufferInfo = &DescriptorBufferInfo[2];
    WriteDescriptorSet[3].dstSet = m_DescriptorSet; // write to this descriptor set.
    WriteDescriptorSet[3].dstBinding = 3; // write to the firjst, and only binding.
    WriteDescriptorSet[3].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[3].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[3].pBufferInfo = &DescriptorBufferInfo[3];
    WriteDescriptorSet[4].dstSet = m_DescriptorSet; // write to this descriptor set.
    WriteDescriptorSet[4].dstBinding = 4; // write to the first, and only binding.
    WriteDescriptorSet[4].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[4].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[4].pBufferInfo = &DescriptorBufferInfo[4];

    WriteDescriptorSet[5].dstSet = m_DescriptorSet2; // write to this descriptor set.
    WriteDescriptorSet[5].dstBinding = 0; // write to the first, and only binding.
    WriteDescriptorSet[5].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[5].descriptorType = vk::DescriptorType::eUniformBuffer; // storage buffer.
    WriteDescriptorSet[5].pBufferInfo = &DescriptorBufferInfo[5];
    WriteDescriptorSet[6].dstSet = m_DescriptorSet2; // write to this descriptor set.
    WriteDescriptorSet[6].dstBinding = 1; // write to the firjst, and only binding.
    WriteDescriptorSet[6].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[6].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[6].pBufferInfo = &DescriptorBufferInfo[6];
    WriteDescriptorSet[7].dstSet = m_DescriptorSet2; // write to this descriptor set.
    WriteDescriptorSet[7].dstBinding = 2; // write to the first, and only binding.
    WriteDescriptorSet[7].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[7].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[7].pBufferInfo = &DescriptorBufferInfo[7];
    WriteDescriptorSet[8].dstSet = m_DescriptorSet2; // write to this descriptor set.
    WriteDescriptorSet[8].dstBinding = 3; // write to the firjst, and only binding.
    WriteDescriptorSet[8].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[8].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[8].pBufferInfo = &DescriptorBufferInfo[8];
    WriteDescriptorSet[9].dstSet = m_DescriptorSet2; // write to this descriptor set.
    WriteDescriptorSet[9].dstBinding = 4; // write to the first, and only binding.
    WriteDescriptorSet[9].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[9].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[9].pBufferInfo = &DescriptorBufferInfo[9];

    //MERGE
    WriteDescriptorSet[10].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[10].dstBinding = 0; // write to the first, and only binding.
    WriteDescriptorSet[10].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[10].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[10].pBufferInfo = &DescriptorBufferInfo[10];
    WriteDescriptorSet[11].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[11].dstBinding = 1; // write to the firjst, and only binding.
    WriteDescriptorSet[11].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[11].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[11].pBufferInfo = &DescriptorBufferInfo[11];
    WriteDescriptorSet[12].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[12].dstBinding = 2; // write to the first, and only binding.
    WriteDescriptorSet[12].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[12].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[12].pBufferInfo = &DescriptorBufferInfo[12];
    WriteDescriptorSet[13].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[13].dstBinding = 3; // write to the first, and only binding.
    WriteDescriptorSet[13].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[13].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[13].pBufferInfo = &DescriptorBufferInfo[13];
    WriteDescriptorSet[14].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[14].dstBinding = 4; // write to the firjst, and only binding.
    WriteDescriptorSet[14].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[14].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[14].pBufferInfo = &DescriptorBufferInfo[14];
    WriteDescriptorSet[15].dstSet = m_DescriptorSet3; // write to this descriptor set.
    WriteDescriptorSet[15].dstBinding = 5; // write to the first, and only binding.
    WriteDescriptorSet[15].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[15].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[15].pBufferInfo = &DescriptorBufferInfo[15];

    //FILTR

    WriteDescriptorSet[16].dstSet = m_DescriptorSet4; // write to this descriptor set.
    WriteDescriptorSet[16].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[16].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[16].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[16].pBufferInfo = &DescriptorBufferInfo[16];
    WriteDescriptorSet[17].dstSet = m_DescriptorSet4; // write to this descriptor set.
    WriteDescriptorSet[17].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[17].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[17].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[17].pBufferInfo = &DescriptorBufferInfo[17];
    WriteDescriptorSet[18].dstSet = m_DescriptorSet4; // write to this descriptor set.
    WriteDescriptorSet[18].dstBinding = 2; // write to the firjst, and only binding.
    WriteDescriptorSet[18].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[18].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[18].pBufferInfo = &DescriptorBufferInfo[18];
    WriteDescriptorSet[19].dstSet = m_DescriptorSet4; // write to this descriptor set.
    WriteDescriptorSet[19].dstBinding = 3; // write to the first, and only binding.
    WriteDescriptorSet[19].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[19].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[19].pBufferInfo = &DescriptorBufferInfo[19];
    //INPAINTING
    WriteDescriptorSet[20].dstSet = m_DescriptorSet5; // write to this descriptor set.
    WriteDescriptorSet[20].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[20].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[20].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[20].pBufferInfo = &DescriptorBufferInfo[20];
    WriteDescriptorSet[21].dstSet = m_DescriptorSet5; // write to this descriptor set.
    WriteDescriptorSet[21].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[21].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[21].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[21].pBufferInfo = &DescriptorBufferInfo[21];

    WriteDescriptorSet[22].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[22].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[22].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[22].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[22].pBufferInfo = &DescriptorBufferInfo[22];
    WriteDescriptorSet[23].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[23].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[23].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[23].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[23].pBufferInfo = &DescriptorBufferInfo[23];
    WriteDescriptorSet[24].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[24].dstBinding = 2; // write to the firjst, and only binding.
    WriteDescriptorSet[24].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[24].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[24].pBufferInfo = &DescriptorBufferInfo[24];
    WriteDescriptorSet[25].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[25].dstBinding = 3; // write to the first, and only binding.
    WriteDescriptorSet[25].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[25].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[25].pBufferInfo = &DescriptorBufferInfo[25];
    WriteDescriptorSet[26].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[26].dstBinding = 4; // write to the firjst, and only binding.
    WriteDescriptorSet[26].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[26].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[26].pBufferInfo = &DescriptorBufferInfo[26];
    WriteDescriptorSet[27].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[27].dstBinding = 5; // write to the firjst, and only binding.
    WriteDescriptorSet[27].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[27].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[27].pBufferInfo = &DescriptorBufferInfo[27];
    WriteDescriptorSet[28].dstSet = m_DescriptorSet6; // write to this descriptor set.
    WriteDescriptorSet[28].dstBinding = 6; // write to the firjst, and only binding.
    WriteDescriptorSet[28].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[28].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[28].pBufferInfo = &DescriptorBufferInfo[28];

    WriteDescriptorSet[29].dstSet = m_DescriptorSet7; // write to this descriptor set.
    WriteDescriptorSet[29].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[29].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[29].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[29].pBufferInfo = &DescriptorBufferInfo[29];
    WriteDescriptorSet[30].dstSet = m_DescriptorSet7; // write to this descriptor set.
    WriteDescriptorSet[30].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[30].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[30].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[30].pBufferInfo = &DescriptorBufferInfo[30];

    WriteDescriptorSet[31].dstSet = m_DescriptorSet8; // write to this descriptor set.
    WriteDescriptorSet[31].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[31].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[31].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[31].pBufferInfo = &DescriptorBufferInfo[31];
    WriteDescriptorSet[32].dstSet = m_DescriptorSet8; // write to this descriptor set.
    WriteDescriptorSet[32].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[32].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[32].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[32].pBufferInfo = &DescriptorBufferInfo[32];

    WriteDescriptorSet[33].dstSet = m_DescriptorSet9; // write to this descriptor set.
    WriteDescriptorSet[33].dstBinding = 0; // write to the firjst, and only binding.
    WriteDescriptorSet[33].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[33].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[33].pBufferInfo = &DescriptorBufferInfo[33];
    WriteDescriptorSet[34].dstSet = m_DescriptorSet9; // write to this descriptor set.
    WriteDescriptorSet[34].dstBinding = 1; // write to the first, and only binding.
    WriteDescriptorSet[34].descriptorCount = 1; // update a single descriptor.
    WriteDescriptorSet[34].descriptorType = vk::DescriptorType::eStorageBuffer; // storage buffer.
    WriteDescriptorSet[34].pBufferInfo = &DescriptorBufferInfo[34];




    // perform the update of the descriptor set.
    m_LogicalDevice.updateDescriptorSets(35, WriteDescriptorSet, 0, nullptr);

    return true;
  }

  bool xVulkan2::createTestPipeline()
  {
    vk::SpecializationMapEntry SpecializationMapEntries[2];
    SpecializationMapEntries[0].constantID = 0;
    SpecializationMapEntries[0].offset = 0;
    SpecializationMapEntries[0].size = 4;
    SpecializationMapEntries[1].constantID = 1;
    SpecializationMapEntries[1].offset = 4;
    SpecializationMapEntries[1].size = 4;

    uint32_t SpecializationData[] = { m_LocalGroupSizeX, m_LocalGroupSizeY };

    vk::SpecializationInfo SpecializationInfo;
    SpecializationInfo.mapEntryCount = 2;
    SpecializationInfo.pMapEntries = SpecializationMapEntries;
    SpecializationInfo.dataSize = 8;
    SpecializationInfo.pData = SpecializationData;

    std::vector<uint32> ShaderIR = xReadShader("../../projects/vulkan_experiment/shaders/computedouble.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo;
    ShaderModuleCreateInfo.pCode = ShaderIR.data();
    ShaderModuleCreateInfo.codeSize = ShaderIR.size() * sizeof(uint32);

    auto ResultShaderModule = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo);
    if (ResultShaderModule.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule = ResultShaderModule.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo;
    PipelineShaderStageCreateInfo.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo.module = ShaderModule;
    PipelineShaderStageCreateInfo.pName = "main";

    std::vector<uint32> ShaderIR2 = xReadShader("../../projects/vulkan_experiment/shaders/combinedouble.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo2;
    ShaderModuleCreateInfo2.pCode = ShaderIR2.data();
    ShaderModuleCreateInfo2.codeSize = ShaderIR2.size() * sizeof(uint32);

    auto ResultShaderModule2 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo2);
    if (ResultShaderModule2.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule2 = ResultShaderModule2.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo2;
    PipelineShaderStageCreateInfo2.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo2.module = ShaderModule2;
    PipelineShaderStageCreateInfo2.pName = "main";


    std::vector<uint32> ShaderIR3 = xReadShader("../../projects/vulkan_experiment/shaders/filtr.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo3;
    ShaderModuleCreateInfo3.pCode = ShaderIR3.data();
    ShaderModuleCreateInfo3.codeSize = ShaderIR3.size() * sizeof(uint32);

    auto ResultShaderModule3 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo3);
    if (ResultShaderModule3.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule3 = ResultShaderModule3.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo3;
    PipelineShaderStageCreateInfo3.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo3.module = ShaderModule3;
    PipelineShaderStageCreateInfo3.pName = "main";

    std::vector<uint32> ShaderIR4 = xReadShader("../../projects/vulkan_experiment/shaders/analyzeL.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo4;
    ShaderModuleCreateInfo4.pCode = ShaderIR4.data();
    ShaderModuleCreateInfo4.codeSize = ShaderIR4.size() * sizeof(uint32);

    auto ResultShaderModule4 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo4);
    if (ResultShaderModule4.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule4 = ResultShaderModule4.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo4;
    PipelineShaderStageCreateInfo4.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo4.module = ShaderModule4;
    PipelineShaderStageCreateInfo4.pName = "main";

    std::vector<uint32> ShaderIR5 = xReadShader("../../projects/vulkan_experiment/shaders/comp.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo5;
    ShaderModuleCreateInfo5.pCode = ShaderIR5.data();
    ShaderModuleCreateInfo5.codeSize = ShaderIR5.size() * sizeof(uint32);

    auto ResultShaderModule5 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo5);
    if (ResultShaderModule5.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule5 = ResultShaderModule5.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo5;
    PipelineShaderStageCreateInfo5.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo5.module = ShaderModule5;
    PipelineShaderStageCreateInfo5.pName = "main";

    std::vector<uint32> ShaderIR6 = xReadShader("../../projects/vulkan_experiment/shaders/analyzeA.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo6;
    ShaderModuleCreateInfo6.pCode = ShaderIR6.data();
    ShaderModuleCreateInfo6.codeSize = ShaderIR6.size() * sizeof(uint32);

    auto ResultShaderModule6 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo6);
    if (ResultShaderModule6.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule6 = ResultShaderModule6.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo6;
    PipelineShaderStageCreateInfo6.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo6.module = ShaderModule6;
    PipelineShaderStageCreateInfo6.pName = "main";

    std::vector<uint32> ShaderIR7 = xReadShader("../../projects/vulkan_experiment/shaders/analyzeR.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo7;
    ShaderModuleCreateInfo7.pCode = ShaderIR7.data();
    ShaderModuleCreateInfo7.codeSize = ShaderIR7.size() * sizeof(uint32);

    auto ResultShaderModule7 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo7);
    if (ResultShaderModule7.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule7 = ResultShaderModule7.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo7;
    PipelineShaderStageCreateInfo7.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo7.module = ShaderModule7;
    PipelineShaderStageCreateInfo7.pName = "main";

    std::vector<uint32> ShaderIR8 = xReadShader("../../projects/vulkan_experiment/shaders/analyzeB.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo8;
    ShaderModuleCreateInfo8.pCode = ShaderIR8.data();
    ShaderModuleCreateInfo8.codeSize = ShaderIR8.size() * sizeof(uint32);

    auto ResultShaderModule8 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo8);
    if (ResultShaderModule8.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule8 = ResultShaderModule8.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo8;
    PipelineShaderStageCreateInfo8.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo8.module = ShaderModule8;
    PipelineShaderStageCreateInfo8.pName = "main";



    //TXT
    /*
    std::vector<uint32> ShaderIR4 = xReadShader("../../projects/vulkan_experiment/shaders/ComputeTXT.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo4;
    ShaderModuleCreateInfo4.pCode = ShaderIR4.data();
    ShaderModuleCreateInfo4.codeSize = ShaderIR4.size() * sizeof(uint32);

    auto ResultShaderModule4 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo4);
    if (ResultShaderModule4.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule4 = ResultShaderModule4.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo4;
    PipelineShaderStageCreateInfo4.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo4.module = ShaderModule4;
    PipelineShaderStageCreateInfo4.pName = "main";

    std::vector<uint32> ShaderIR5 = xReadShader("../../projects/vulkan_experiment/shaders/CombineTXT.spv");
    vk::ShaderModuleCreateInfo ShaderModuleCreateInfo5;
    ShaderModuleCreateInfo5.pCode = ShaderIR5.data();
    ShaderModuleCreateInfo5.codeSize = ShaderIR5.size() * sizeof(uint32);

    auto ResultShaderModule5 = m_LogicalDevice.createShaderModule(ShaderModuleCreateInfo5);
    if (ResultShaderModule5.result != vk::Result::eSuccess) { fmt::printf("createShaderModule ERROR\n"); return false; }
    vk::ShaderModule ShaderModule5 = ResultShaderModule5.value;

    vk::PipelineShaderStageCreateInfo PipelineShaderStageCreateInfo5;
    PipelineShaderStageCreateInfo5.stage = vk::ShaderStageFlagBits::eCompute;;
    PipelineShaderStageCreateInfo5.module = ShaderModule5;
    PipelineShaderStageCreateInfo5.pName = "main";
    */


    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo;
    PipelineLayoutCreateInfo.setLayoutCount = 1;
    PipelineLayoutCreateInfo.pSetLayouts = &m_DescriptorSetLayout;
    auto ResultPipelineLayout = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo);
    if (ResultPipelineLayout.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout = ResultPipelineLayout.value;

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo2;
    PipelineLayoutCreateInfo2.setLayoutCount = 1;
    PipelineLayoutCreateInfo2.pSetLayouts = &m_DescriptorSetLayout;
    auto ResultPipelineLayout2 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo2);
    if (ResultPipelineLayout2.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout2 = ResultPipelineLayout2.value;
    //MERGE
    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo3;
    PipelineLayoutCreateInfo3.setLayoutCount = 1;
    PipelineLayoutCreateInfo3.pSetLayouts = &m_DescriptorSetLayout3;
    auto ResultPipelineLayout3 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo3);
    if (ResultPipelineLayout3.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout3 = ResultPipelineLayout3.value;
    //FILTR
    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo4;
    PipelineLayoutCreateInfo4.setLayoutCount = 1;
    PipelineLayoutCreateInfo4.pSetLayouts = &m_DescriptorSetLayout4;
    auto ResultPipelineLayout4 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo4);
    if (ResultPipelineLayout4.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout4 = ResultPipelineLayout4.value;
    //INPAINT
    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo5;
    PipelineLayoutCreateInfo5.setLayoutCount = 1;
    PipelineLayoutCreateInfo5.pSetLayouts = &m_DescriptorSetLayout5;
    auto ResultPipelineLayout5 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo5);
    if (ResultPipelineLayout5.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout5 = ResultPipelineLayout5.value;

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo6;
    PipelineLayoutCreateInfo6.setLayoutCount = 1;
    PipelineLayoutCreateInfo6.pSetLayouts = &m_DescriptorSetLayout6;
    auto ResultPipelineLayout6 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo6);
    if (ResultPipelineLayout6.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout6 = ResultPipelineLayout6.value;

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo7;
    PipelineLayoutCreateInfo7.setLayoutCount = 1;
    PipelineLayoutCreateInfo7.pSetLayouts = &m_DescriptorSetLayout7;
    auto ResultPipelineLayout7 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo7);
    if (ResultPipelineLayout7.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout7 = ResultPipelineLayout7.value;

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo8;
    PipelineLayoutCreateInfo8.setLayoutCount = 1;
    PipelineLayoutCreateInfo8.pSetLayouts = &m_DescriptorSetLayout8;
    auto ResultPipelineLayout8 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo8);
    if (ResultPipelineLayout8.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout8 = ResultPipelineLayout8.value;

    vk::PipelineLayoutCreateInfo PipelineLayoutCreateInfo9;
    PipelineLayoutCreateInfo9.setLayoutCount = 1;
    PipelineLayoutCreateInfo9.pSetLayouts = &m_DescriptorSetLayout9;
    auto ResultPipelineLayout9 = m_LogicalDevice.createPipelineLayout(PipelineLayoutCreateInfo9);
    if (ResultPipelineLayout9.result != vk::Result::eSuccess) { fmt::printf("createPipelineLayout ERROR\n"); return false; }
    m_PipelineLayout9 = ResultPipelineLayout9.value;


    vk::PipelineCache PipelineCache;

    vk::PipelineCache PipelineCache2;

    vk::PipelineCache PipelineCache3;

    vk::PipelineCache PipelineCache4;

    vk::PipelineCache PipelineCache5;
    vk::PipelineCache PipelineCache6;
    vk::PipelineCache PipelineCache7;
    vk::PipelineCache PipelineCache8;
    vk::PipelineCache PipelineCache9;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo;
    ComputePipelineCreateInfo.stage = PipelineShaderStageCreateInfo;
    ComputePipelineCreateInfo.layout = m_PipelineLayout;
    auto ResultComputePipelines = m_LogicalDevice.createComputePipeline(PipelineCache, ComputePipelineCreateInfo);
    if (ResultComputePipelines.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline = ResultComputePipelines.value;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo2;
    ComputePipelineCreateInfo2.stage = PipelineShaderStageCreateInfo;
    ComputePipelineCreateInfo2.layout = m_PipelineLayout2;
    auto ResultComputePipelines2 = m_LogicalDevice.createComputePipeline(PipelineCache2, ComputePipelineCreateInfo2);
    if (ResultComputePipelines2.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline2 = ResultComputePipelines2.value;
    //MERGE
    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo3;
    ComputePipelineCreateInfo3.stage = PipelineShaderStageCreateInfo2;
    ComputePipelineCreateInfo3.layout = m_PipelineLayout3;
    auto ResultComputePipelines3 = m_LogicalDevice.createComputePipeline(PipelineCache3, ComputePipelineCreateInfo3);
    if (ResultComputePipelines3.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline3 = ResultComputePipelines3.value;
    //FITLR
    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo4;
    ComputePipelineCreateInfo4.stage = PipelineShaderStageCreateInfo3;
    ComputePipelineCreateInfo4.layout = m_PipelineLayout4;
    auto ResultComputePipelines4 = m_LogicalDevice.createComputePipeline(PipelineCache4, ComputePipelineCreateInfo4);
    if (ResultComputePipelines4.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline4 = ResultComputePipelines4.value;
    //INPAINT
    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo5;
    ComputePipelineCreateInfo5.stage = PipelineShaderStageCreateInfo4;
    ComputePipelineCreateInfo5.layout = m_PipelineLayout5;
    auto ResultComputePipelines5 = m_LogicalDevice.createComputePipeline(PipelineCache5, ComputePipelineCreateInfo5);
    if (ResultComputePipelines5.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline5 = ResultComputePipelines5.value;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo6;
    ComputePipelineCreateInfo6.stage = PipelineShaderStageCreateInfo5;
    ComputePipelineCreateInfo6.layout = m_PipelineLayout6;
    auto ResultComputePipelines6 = m_LogicalDevice.createComputePipeline(PipelineCache6, ComputePipelineCreateInfo6);
    if (ResultComputePipelines6.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline6 = ResultComputePipelines6.value;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo7;
    ComputePipelineCreateInfo7.stage = PipelineShaderStageCreateInfo6;
    ComputePipelineCreateInfo7.layout = m_PipelineLayout7;
    auto ResultComputePipelines7 = m_LogicalDevice.createComputePipeline(PipelineCache7, ComputePipelineCreateInfo7);
    if (ResultComputePipelines7.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline7 = ResultComputePipelines7.value;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo8;
    ComputePipelineCreateInfo8.stage = PipelineShaderStageCreateInfo7;
    ComputePipelineCreateInfo8.layout = m_PipelineLayout8;
    auto ResultComputePipelines8 = m_LogicalDevice.createComputePipeline(PipelineCache8, ComputePipelineCreateInfo8);
    if (ResultComputePipelines8.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline8 = ResultComputePipelines8.value;

    vk::ComputePipelineCreateInfo ComputePipelineCreateInfo9;
    ComputePipelineCreateInfo9.stage = PipelineShaderStageCreateInfo8;
    ComputePipelineCreateInfo9.layout = m_PipelineLayout9;
    auto ResultComputePipelines9 = m_LogicalDevice.createComputePipeline(PipelineCache9, ComputePipelineCreateInfo9);
    if (ResultComputePipelines9.result != vk::Result::eSuccess) { fmt::printf("createComputePipeline ERROR\n"); return false; }
    m_Pipeline9 = ResultComputePipelines9.value;

    return true;
  }

  bool xVulkan2::createTestCommandBuffer()
  {
    vk::CommandPoolCreateInfo CommandPoolCreateInfo = {};
    CommandPoolCreateInfo.queueFamilyIndex = m_QueueFamilyIdx;
    auto ResultCommandPool = m_LogicalDevice.createCommandPool(CommandPoolCreateInfo);
    if (ResultCommandPool.result != vk::Result::eSuccess) { fmt::printf("createCommandPool ERROR\n"); return false; }
    vk::CommandPool CommandPool = ResultCommandPool.value;

    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo = {};
    CommandBufferAllocateInfo.commandPool = CommandPool;
    CommandBufferAllocateInfo.level = vk::CommandBufferLevel::ePrimary;
    CommandBufferAllocateInfo.commandBufferCount = 25; // allocate a single command buffer. 
    auto ResultCommandBuffers = m_LogicalDevice.allocateCommandBuffers(CommandBufferAllocateInfo);
    if (ResultCommandBuffers.result != vk::Result::eSuccess) { fmt::printf("allocateCommandBuffers ERROR\n"); return false; }
    m_CommandBuffer = ResultCommandBuffers.value[0];

    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
    auto ResultBegin = m_CommandBuffer.begin(CommandBufferBeginInfo);
    if (ResultBegin != vk::Result::eSuccess) { fmt::printf("begin ERROR\n"); return false; }

    m_CommandBuffer.fillBuffer(m_DeviceDstBuffer, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBuffer2, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBuffer3, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBuffer4, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferTXT, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferTXT2, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferTXT3, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferTXT4, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferINPAINT1, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferINPAINT2, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferINPAINT3, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferINPAINT4, 0, VK_WHOLE_SIZE, 0);
    m_CommandBuffer.fillBuffer(m_DeviceDstBufferINPAINT5, 0, VK_WHOLE_SIZE, 0);




    //SHADER TEST
    vk::BufferCopy BufferCopySrc;
    vk::BufferCopy BufferCopySrc2;
    vk::BufferCopy BufferCopySrc3;
    vk::BufferCopy BufferCopySrc4;
    vk::BufferCopy BufferCopySrc5;

    BufferCopySrc.size = m_SrcDpthPlane[0].getBufferSize();
    BufferCopySrc2.size = m_SrcDpthPlane[1].getBufferSize();
    BufferCopySrc4.size = m_SrcTextPicI[0].getBuffSize();
    BufferCopySrc5.size = m_SrcTextPicI[0].getBuffSize();

    m_CommandBuffer.copyBuffer(m_HostSrcBuffer, m_DeviceSrcBuffer, BufferCopySrc);
    m_CommandBuffer.copyBuffer(m_HostSrcBuffer2, m_DeviceSrcBuffer2, BufferCopySrc2);
    m_CommandBuffer.copyBuffer(m_HostSrcBufferTXT, m_DeviceSrcBufferTXT, BufferCopySrc4);
    m_CommandBuffer.copyBuffer(m_HostSrcBufferTXT2, m_DeviceSrcBufferTXT2, BufferCopySrc4);



    vk::MemoryBarrier MemoryBarrierSrc;
    MemoryBarrierSrc.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    MemoryBarrierSrc.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eComputeShader, vk::DependencyFlags(), MemoryBarrierSrc, nullptr, nullptr);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout, 0, m_DescriptorSet, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, c_SizeY, 1);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline2);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout2, 0, m_DescriptorSet2, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, c_SizeY, 1);
    //MERGE
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline3);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout3, 0, m_DescriptorSet3, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, c_SizeY, 1);
    //FILTR
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline4);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout4, 0, m_DescriptorSet4, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, c_SizeY, 1);
    //INPAINT
    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline5);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout5, 0, m_DescriptorSet5, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, 1, 1);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline7);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout7, 0, m_DescriptorSet7, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, 1, 1);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline8);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout8, 0, m_DescriptorSet8, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, 1, 1);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline9);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout9, 0, m_DescriptorSet9, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, 1, 1);

    m_CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_Pipeline6);
    m_CommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_PipelineLayout6, 0, m_DescriptorSet6, nullptr);
    m_CommandBuffer.dispatch(c_SizeX, c_SizeY, 1);




    vk::MemoryBarrier MemoryBarrierDst;
    MemoryBarrierDst.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
    MemoryBarrierDst.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    m_CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eComputeShader, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), MemoryBarrierDst, nullptr, nullptr);

    vk::BufferCopy BufferCopyDst;
    vk::BufferCopy BufferCopyDst2;
    vk::BufferCopy BufferCopyDst3;
    BufferCopyDst.size = m_SrcDpthPlane[0].getBufferSize();
    BufferCopyDst2.size = m_SrcDpthPlane[1].getBufferSize();
    BufferCopyDst3.size = m_SrcTextPicI[0].getBuffSize();
    m_CommandBuffer.copyBuffer(m_DeviceDstBuffer, m_HostDstBuffer, BufferCopyDst);
    m_CommandBuffer.copyBuffer(m_DeviceDstBuffer2, m_HostDstBuffer2, BufferCopyDst2);
    m_CommandBuffer.copyBuffer(m_DeviceDstBuffer3, m_HostDstBuffer3, BufferCopyDst2);
    m_CommandBuffer.copyBuffer(m_DeviceDstBuffer4, m_HostDstBuffer4, BufferCopyDst2);
    m_CommandBuffer.copyBuffer(m_DeviceDstBufferTXT, m_HostDstBufferTXT, BufferCopyDst3);
    m_CommandBuffer.copyBuffer(m_DeviceDstBufferTXT2, m_HostDstBufferTXT2, BufferCopyDst3);
    m_CommandBuffer.copyBuffer(m_DeviceDstBufferTXT3, m_HostDstBufferTXT3, BufferCopyDst3);
    m_CommandBuffer.copyBuffer(m_DeviceDstBufferTXT4, m_HostDstBufferTXT4, BufferCopyDst3);
    m_CommandBuffer.copyBuffer(m_DeviceDstBufferINPAINT5, m_HostDstBufferINPAINT1, BufferCopyDst3);



    m_CommandBuffer.end();
    return true;
  }
  bool xVulkan2::executeTestCommandBuffer()
  {
    vk::SubmitInfo SubmitInfo;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &m_CommandBuffer;

    vk::FenceCreateInfo FenceCreateInfo;
    auto ResultFence = m_LogicalDevice.createFence(FenceCreateInfo);
    if (ResultFence.result != vk::Result::eSuccess) { fmt::print("createFence ERROR\n"); return false; }
    vk::Fence Fence = ResultFence.value;

    auto ResultSubmit = m_Queue.submit(SubmitInfo, Fence);
    if (ResultSubmit != vk::Result::eSuccess) { fmt::print("submit ERROR\n"); return false; }

    m_LogicalDevice.waitForFences(Fence, VK_TRUE, 100000000000);
    m_LogicalDevice.destroyFence(Fence);
    return true;
  }

  bool xVulkan2::copyBackTestStorageBuffers()
  {
    //copy data to from dst
//<<<<<<< HEAD
    readFromHostMemory(m_HostDstMemory, 0, SizeOfBuffer0, m_PlaneDst.getBuffer(), true);
    readFromHostMemory(m_HostDstMemory2, 0, SizeOfBuffer1, m_PlaneDst2.getBuffer(), true);
    readFromHostMemory(m_HostDstMemory3, 0, SizeOfBuffer1, m_PlaneDst3.getBuffer(), true);
    readFromHostMemory(m_HostDstMemory4, 0, SizeOfBuffer1, m_PlaneDst4.getBuffer(), true);
    readFromHostMemory(m_HostDstMemoryTXT, 0, SizeOfBuffer2, m_DstTextPicI.getBuffer(), true);
    readFromHostMemory(m_HostDstMemoryTXT2, 0, SizeOfBuffer2, m_DstTextPicI_2.getBuffer(), true);
    readFromHostMemory(m_HostDstMemoryTXT3, 0, SizeOfBuffer2, m_DstTextPicI_3.getBuffer(), true);
    readFromHostMemory(m_HostDstMemoryTXT4, 0, SizeOfBuffer2, m_DstTextPicI_4.getBuffer(), true);
    readFromHostMemory(m_HostDstMemoryINPAINT1, 0, SizeOfBuffer2, m_DstTextPicI_5.getBuffer(), true);
    //=======

    //>>>>>>> 0124c6b0ade636ab9eff2450951ea7917638932a
    return true;
  }


  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  bool xVulkan2::createBuffer(vk::Buffer& Buffer, vk::DeviceMemory& Memory, vk::BufferUsageFlags BufferUsageFlags, vk::MemoryPropertyFlags MemoryPropertyFlags, vk::DeviceSize Size)
  {
    // Create the buffer handle
    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo.usage = BufferUsageFlags;
    BufferCreateInfo.size = Size;
    BufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    auto CreatedBuffer = m_LogicalDevice.createBuffer(BufferCreateInfo);
    if (CreatedBuffer.result != vk::Result::eSuccess) { fmt::printf("createBuffer ERROR\n"); return false; }
    Buffer = CreatedBuffer.value;

    // Create the memory backing up the buffer handle
    vk::PhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties = m_PhysicalDevice.getMemoryProperties();
    vk::MemoryRequirements MemoryRequirements = m_LogicalDevice.getBufferMemoryRequirements(Buffer);

    int32 MemoryTypeIdx = xDetermineMemoryTypeIdx(MemoryRequirements, MemoryPropertyFlags);
    if (MemoryTypeIdx == NOT_VALID) { fmt::printf("Memory type index not found\n"); return false; }
    vk::MemoryAllocateInfo MemoryAllocateInfo;
    MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
    MemoryAllocateInfo.memoryTypeIndex = MemoryTypeIdx;

    auto MemoryAllocation = m_LogicalDevice.allocateMemory(MemoryAllocateInfo);
    if (MemoryAllocation.result != vk::Result::eSuccess) { fmt::printf("createBuffer ERROR\n"); return false; }
    Memory = MemoryAllocation.value;

    vk::Result BindResult = m_LogicalDevice.bindBufferMemory(Buffer, Memory, 0);
    if (BindResult != vk::Result::eSuccess) { fmt::printf("bindBufferMemory ERROR\n"); return false; }

    return true;
  }
  bool xVulkan2::copyToHostMemory(vk::DeviceMemory& HostMemory, vk::DeviceSize Offset, vk::DeviceSize Size, const void* Data, bool HostCoherent)
  {
    auto MappedMemory = m_LogicalDevice.mapMemory(HostMemory, Offset, Size);
    if (MappedMemory.result != vk::Result::eSuccess) { fmt::printf("mapMemory ERROR\n"); return false; }

    memcpy(MappedMemory.value, Data, Size);

    if (!HostCoherent)
    {
      vk::MappedMemoryRange MappedMemoryRange;
      MappedMemoryRange.memory = HostMemory;
      MappedMemoryRange.offset = Offset;
      MappedMemoryRange.size = Size; // VK_WHOLE_SIZE;
      m_LogicalDevice.flushMappedMemoryRanges(28, &MappedMemoryRange);
    }

    m_LogicalDevice.unmapMemory(HostMemory);


    return true;
  }
  bool xVulkan2::readFromHostMemory(vk::DeviceMemory& HostMemory, vk::DeviceSize Offset, vk::DeviceSize Size, void* Data, bool HostCoherent)
  {
    auto MappedMemory = m_LogicalDevice.mapMemory(HostMemory, Offset, Size);
    if (MappedMemory.result != vk::Result::eSuccess) { fmt::printf("mapMemory ERROR\n"); return false; }

    if (!HostCoherent)
    {
      vk::MappedMemoryRange MappedMemoryRange;
      MappedMemoryRange.memory = HostMemory;
      MappedMemoryRange.offset = Offset;
      MappedMemoryRange.size = Size; // VK_WHOLE_SIZE;
      m_LogicalDevice.invalidateMappedMemoryRanges(1, &MappedMemoryRange);
    }

    memcpy(Data, MappedMemory.value, Size);

    m_LogicalDevice.unmapMemory(HostMemory);

    return true;
  }


  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  VKAPI_ATTR vk::Bool32 VKAPI_CALL xVulkan2::xDebugReportCallback(vk::DebugReportFlagsEXT flags, vk::DebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
  {
    fmt::printf("Debug Report: %s: %s\n", pLayerPrefix, pMessage);
    return VK_FALSE;
  }

  int32 xVulkan2::xGetComputeQueueFamilyIndex(vk::QueueFlags RequiredQueueFlags)
  {
    auto QueueFamilies = m_PhysicalDevice.getQueueFamilyProperties();

    // Now find a family that supports compute.
    for (int32 i = 0; i < QueueFamilies.size(); ++i)
    {
      const vk::QueueFamilyProperties& QueueFamilyProps = QueueFamilies[i];
      if (QueueFamilyProps.queueCount > 0 && (QueueFamilyProps.queueFlags & RequiredQueueFlags)) { return i; }
    }
    fmt::printf("Could not find a queue family that supports operations\n");
    return NOT_VALID;
  }

  int32 xVulkan2::xDetermineMemoryTypeIdx(vk::MemoryRequirements MemoryRequirements, vk::MemoryPropertyFlags MemoryPropertyFlags)
  {
    vk::PhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties = m_PhysicalDevice.getMemoryProperties();
    // Find a memory type index that fits the properties of the buffer
    //See the documentation of VkPhysicalDeviceMemoryProperties for a detailed description.
    for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
      if (((MemoryRequirements.memoryTypeBits & (1 << i)) && ((PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & MemoryPropertyFlags) == MemoryPropertyFlags)))
      {
        return i;
      }
    }
    return NOT_VALID;
  }

  std::vector<uint32> xVulkan2::xReadShader(const std::string& FilePath)
  {
    std::vector<uint32> Shader;

    FILE* File = fopen(FilePath.c_str(), "rb");
    if (File == NULL) { fmt::printf("Could not find or open file: %s\n", FilePath); return Shader; }

    // get file size.
    fseek(File, 0, SEEK_END);
    uint64 FileSize = ftell(File);
    fseek(File, 0, SEEK_SET);

    uint64 NumWordsInFile = uint64(ceil(FileSize / 4.0));
    Shader.resize(NumWordsInFile, 0);

    // read file contents.
    fread(Shader.data(), 1, FileSize, File);
    fclose(File);

    return Shader;
  }

  void xVulkan2::printBuffers()
  {
    m_SrcDpthSeq[0].destroy();
    m_SrcDpthSeq[1].destroy();

    m_SrcTextSeq[0].destroy();
    m_SrcTextSeq[1].destroy();
    m_SrcTextPic444[0].destroy();
    m_SrcTextPic444[1].destroy();
    m_SrcTextPicI[0].destroy();
    m_SrcTextPicI[1].destroy();

    //m_PlaneDst.destroy();
    //m_PlaneSrc.destroy();

    //m_SeqDst.destroy();
    //m_SeqSrc.destroy();

    //m_PlaneDst2.destroy();
    //m_PlaneSrc2.destroy();

    //m_SeqDst2.destroy();
    //m_SeqSrc2.destroy();

  }



  double roznica;
  void xVulkan2::xRun()
  {
    std::cout.setf(std::ios::fixed); //notacja zwykla, czyli nie wywali wyniku typu 1.175000e+003	
    std::cout.precision(5); //liczba miejsc po przecinku, dokladnosc naszego wyniku	
    clock_t start, koniec; //inicjacja zmiennych zegarowych	
    start = clock(); //zapisanie czasu startu mierzenia	

    createInstance();
    findPhysicalDevice();
    createDevice();
    createTestStorageBuffers();
    createTestDescriptorSetLayout();
    createTestDescriptorSetLayout2();
    createTestDescriptorSetLayout3();
    createTestDescriptorSetLayout4();
    createTestDescriptorSetLayout5();
    createTestDescriptorPool();
    createTestDescriptorSet();
    createTestPipeline();

    koniec = clock();//zapisanie konca mierzenia    
    roznica = (koniec - start) / (double)CLOCKS_PER_SEC;//obliczenie roznicy, czyli czasu wykonania    
    std::cout << "Czas wykonania po pipeline: " << roznica << std::endl;

    for (uint32_t i = 0; i < 17; i++)
    {
      //read depth
      m_SrcDpthSeq[0].readPlane(&m_SrcDpthPlane[0]);
      m_SrcDpthSeq[1].readPlane(&m_SrcDpthPlane[1]);

      //read and cvt texture
      m_SrcTextSeq[0].readPicture(&m_SrcTextPicOrg[0]);
      m_SrcTextSeq[1].readPicture(&m_SrcTextPicOrg[1]);
      m_SrcTextPic444[0].rescaleChroma(&m_SrcTextPicOrg[0], true);
      m_SrcTextPic444[1].rescaleChroma(&m_SrcTextPicOrg[1], true);
      m_SrcTextPicI[0].rearrangeConvertFromPlanar(&m_SrcTextPic444[0]);
      m_SrcTextPicI[1].rearrangeConvertFromPlanar(&m_SrcTextPic444[1]);

      //copy to GPU
      copyToHostMemory(m_HostSrcMemory, 0, SizeOfBuffer0, m_SrcDpthPlane[0].getBuffer(), true);
      copyToHostMemory(m_HostSrcMemory2, 0, SizeOfBuffer1, m_SrcDpthPlane[1].getBuffer(), true);

      copyToHostMemory(m_HostSrcMemoryTXT, 0, SizeOfBuffer2, m_SrcTextPicI[0].getBuffer(), true);
      copyToHostMemory(m_HostSrcMemoryTXT2, 0, SizeOfBuffer2, m_SrcTextPicI[1].getBuffer(), true);


      //do synth
      createTestCommandBuffer();
      executeTestCommandBuffer();

      //copy from GPU
      copyBackTestStorageBuffers();
      m_SeqDst.writePlane(&m_PlaneDst);
      m_SeqDst2.writePlane(&m_PlaneDst2);
      m_SeqDst3.writePlane(&m_PlaneDst3);
      m_SeqDst4.writePlane(&m_PlaneDst4);
      m_DstTextPicI.rearrangeConvertToPlanar(&m_DstTextPic444);
      m_DstTextPic420.rescaleChroma(&m_DstTextPic444, true);
      m_DstTextSeq.writePicture(&m_DstTextPic420);
      m_DstTextPicI_2.rearrangeConvertToPlanar(&m_DstTextPic444_2);
      m_DstTextPic420_2.rescaleChroma(&m_DstTextPic444_2, true);
      m_DstTextSeq2.writePicture(&m_DstTextPic420_2);
      m_DstTextPicI_3.rearrangeConvertToPlanar(&m_DstTextPic444_3);
      m_DstTextPic420_3.rescaleChroma(&m_DstTextPic444_3, true);
      m_DstTextSeq3.writePicture(&m_DstTextPic420_3);
      m_DstTextPicI_4.rearrangeConvertToPlanar(&m_DstTextPic444_4);
      m_DstTextPic420_4.rescaleChroma(&m_DstTextPic444_4, true);
      m_DstTextSeq4.writePicture(&m_DstTextPic420_4);
      m_DstTextPicI_5.rearrangeConvertToPlanar(&m_DstTextPic444_5);
      m_DstTextPic420_5.rescaleChroma(&m_DstTextPic444_5, true);
      m_DstTextSeq5.writePicture(&m_DstTextPic420_5);

      koniec = clock();//zapisanie konca mierzenia    
      roznica = (koniec - start) / (double)CLOCKS_PER_SEC;//obliczenie roznicy, czyli czasu wykonania    
      std::cout << "Czas wykonania po " << i << "klatce : " << roznica << std::endl;
    }




  }


  //=====================================================================================================================================================================================

} //end of namespace AVLib::Vulkan



