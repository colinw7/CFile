#ifndef CTempFile_H
#define CTempFile_H

#include <CFile.h>

class CTempFile {
 private:
  CFile *file_;

 public:
  explicit CTempFile(const std::string &dir="");
 ~CTempFile();

  CFile *getFile() const { return file_; }

  static bool getTempFileName(const std::string &dir, std::string &filename);

 private:
  CTempFile(const CTempFile &file);
  CTempFile &operator=(const CTempFile &file);
};

#endif
