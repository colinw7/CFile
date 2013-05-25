#ifndef DEVICE_H
#define DEVICE_H

#include <string>

class CFileDevice {
 private:
  static std::string device_;

 public:
  static const std::string &getCurrent();
};

#endif
