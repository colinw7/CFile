#ifndef CFile_H
#define CFile_H

#include <CFileBase.h>

class CFile : public CFileBase {
 private:
  FILE *fp_;

 public:
  CFile();
  explicit CFile(const std::string &filename);
  explicit CFile(const char *filename);
  explicit CFile(FILE *fp);

 ~CFile();

  FILE *getFP() const { return fp_; }

  bool isStdIn() const;
  bool isStdOut() const;
  bool isStdErr() const;

 protected:
  bool open_(const std::string &mode);
  bool close_();

  bool read_(uchar *data, size_t size, size_t *actual_size);

  bool write_(const uchar *data, size_t size);

  bool getc_(int &c);
  bool ungetc_(int c);
  bool putc_(int c);

  bool seek_(long pos);
  bool rseek_(long pos);
  bool tell_(long &pos);

  bool rewind_();

  bool eof_(bool &eof);

  bool error_(int &error);

  bool flush_();

  bool stat_ (struct stat *file_stat);
  bool lstat_(struct stat *file_stat);

  bool readlink_(std::string &linkname) const;

 private:
  // not allowed !
  CFile(const CFile &file);
  CFile &operator=(const CFile &file);

  void init();
};

#endif
