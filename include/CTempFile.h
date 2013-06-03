#ifndef CTEMP_FILE_H
#define CTEMP_FILE_H

#include <CFile.h>

class CTempFile {
 private:
  CFile *file_;

 public:
  CTempFile(const std::string &dir="");
 ~CTempFile();

  CFile *getFile() const { return file_; }

 private:
  static bool getTempFileName(const std::string &dir, std::string &filename);

 private:
  CTempFile(const CTempFile &file);
  CTempFile &operator=(const CTempFile &file);
};

#endif
