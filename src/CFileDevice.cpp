#include <CFileDevice.h>

std::string CFileDevice::device_ = "";

const std::string &
CFileDevice::
getCurrent()
{
  return device_;
}
