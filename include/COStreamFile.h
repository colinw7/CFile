#ifndef COSTREAM_FILE_H
#define COSTREAM_FILE_H

// implementation of CFileBase methods for std::ostream class

#include <CFileBase.h>

class COStreamFile : public CFileBase {
 public:
  COStreamFile(std::ostream &os);

 ~COStreamFile();

  std::ostream &getStream() const { return os_; }

  bool isCOut() const;
  bool isCErr() const;

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

  bool stat_(struct stat *file_stat);
  bool lstat_(struct stat *file_stat);

  bool readlink_(std::string &linkname) const;

 private:
  std::ostream &os_;
};

#endif
