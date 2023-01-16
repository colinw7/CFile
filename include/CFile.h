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
  bool open_(const std::string &mode) override;
  bool close_() override;

  bool read_(uchar *data, size_t size, size_t *actual_size) override;

  bool write_(const uchar *data, size_t size) override;

  bool getc_(int &c) override;
  bool ungetc_(int c) override;
  bool putc_(int c) override;

  bool seek_(long pos) override;
  bool rseek_(long pos) override;
  bool tell_(long &pos) override;

  bool rewind_() override;

  bool eof_(bool &eof) override;

  bool error_(int &error) override;

  bool flush_() override;

  bool stat_ (struct stat *file_stat) override;
  bool lstat_(struct stat *file_stat) override;

  bool readlink_(std::string &linkname) const override;

 private:
  // not allowed !
  CFile(const CFile &file);
  CFile &operator=(const CFile &file);

  void init();
};

#endif
