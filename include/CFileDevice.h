#ifndef CFileDevice_H
#define CFileDevice_H

#include <string>

class CFileDevice {
 private:
  static std::string device_;

 public:
  static const std::string &getCurrent();
};

#endif
