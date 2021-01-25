//#include "xVulkan.h"
//#include "xVulkanTestSimple.h"
//#include "xVulkanTestOneDepth.h"
//#include "xVulkanTestOneText.h"
#include "x2Vulkan.h"

using namespace AVlib;

const int32 Verbose = 2;
/*
int main_TestSimple()
{
  Vulkan::xVulkanTestSimple Vulkan;
  Vulkan.runTest(Verbose);
  return 0;
}
int main_TestOneDepth()
{  
  Vulkan::xVulkanTestOneDepth Vulkan;
  Vulkan.runTest(Verbose);
  return 0;
}
int main_TestOneText()
{
  Vulkan::xVulkanTestOneText Vulkan;
  Vulkan.runTest(Verbose);
  return 0;
}
<<<<<<< HEAD
*/


//>>>>>>> e26c696147446b2b5cfd6bf90a7b39ee42f6db23

int main_2()
{
  Vulkan::xVulkan2 app;
  app.xRun();
  return 0;

}

int main()
{
  main_2();
  return 0;
}
