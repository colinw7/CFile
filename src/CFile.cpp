#include <CFile.h>

#include <cstdio>
#include <cerrno>
#include <cassert>
#include <unistd.h>
#include <set>
#include <map>
#include <iostream>
#include <sys/param.h>

#define CFileCtrlInst CFileCtrl::getInstance()

class CFileCtrl {
 private:
  typedef std::set<CFile *>      FileList;
  typedef std::map<int,FileList> FileMap;

  FileMap file_map_;

 public:
  static CFileCtrl *getInstance();

  void open (CFile *file);
  void close(CFile *file);

 private:
  CFileCtrl();
 ~CFileCtrl();
};

CFileCtrl *
CFileCtrl::
getInstance()
{
  static CFileCtrl *instance;

  if (! instance)
    instance = new CFileCtrl;

  return instance;
}

CFileCtrl::
CFileCtrl()
{
}

CFileCtrl::
~CFileCtrl()
{
}

void
CFileCtrl::
open(CFile *file)
{
  int num = fileno(file->getFP());

  file_map_[num].insert(file);

  //std:cerr << "Open " << file->getPath() << ":" << num << std::endl;
}

void
CFileCtrl::
close(CFile *file)
{
  static bool inside;

  if (inside) return;

  inside = true;

  int num = fileno(file->getFP());

  file_map_[num].erase(file);

  //std:cerr << "Close " << file->getPath() << ":" << num << std::endl;

  FileList::iterator p1 = file_map_[num].begin();
  FileList::iterator p2 = file_map_[num].end  ();

  while (p1 != p2) {
    CFile *file = *p1;

    file_map_[num].erase(file);

    file->close();

    p1 = file_map_[num].begin();
    p2 = file_map_[num].end  ();
  }

  inside = false;
}

//-------------

CFile::
CFile()
{
  init();
}

CFile::
CFile(const std::string &filename) :
 CFileBase(filename)
{
  init();
}

CFile::
CFile(const char *filename) :
 CFileBase(filename)
{
  init();
}

CFile::
CFile(FILE *fp) :
 CFileBase("")
{
  fp_     = fp;
  opened_ = true;

  if      (isStdIn())
    mode_ = Mode::READ;
  else if (isStdOut())
    mode_ = Mode::WRITE;
  else if (isStdErr())
    mode_ = Mode::WRITE;
  else
    assert(false);

  owner_ = false;
}

CFile::
~CFile()
{
  if (owner_)
    close();
}

void
CFile::
init()
{
  fp_     = NULL;
  opened_ = false;
}

bool
CFile::
open_(const std::string &mode)
{
  if (isFIFO())
    return false;

  fp_ = fopen(path_.c_str(), mode.c_str());

  if (fp_ == NULL) {
    if      (errno == ENOENT)
      last_error_ = "File does not exist";
    else if (errno == EACCES)
      last_error_ = "No read permission for file";
    else
      last_error_ = "Open Failed";

    std::cerr << path_ << ":" << last_error_ << std::endl;

    return false;
  }

  //CFileCtrlInst->open(this);

  opened_ = true;

  return true;
}

bool
CFile::
close_()
{
  if (opened_) {
    opened_ = false;

    //CFileCtrlInst->close(this);

    fclose(fp_);

    fp_ = NULL;
  }

  return true;
}

bool
CFile::
read_(uchar *data, size_t size, size_t *actual_size)
{
  size_t actual_size1 = fread(data, 1, size, fp_);

  if (actual_size != NULL)
    *actual_size = actual_size1;

  if (actual_size1 != size)
    return false;

  return true;
}

bool
CFile::
write_(const uchar *data, size_t size)
{
  size_t num = fwrite(data, 1, size, fp_);

  if (num != size)
    return false;

  return true;
}

bool
CFile::
getc_(int &c)
{
  c = fgetc(fp_);

  return true;
}

bool
CFile::
ungetc_(int c)
{
  ungetc(c, fp_);

  return true;
}

bool
CFile::
putc_(int c)
{
  fputc(c, fp_);

  return true;
}

bool
CFile::
seek_(long pos)
{
  int error;

  if (pos >= 0)
    error = fseek(fp_, pos, SEEK_SET);
  else
    error = fseek(fp_, -pos - 1, SEEK_END);

  if (error != 0)
    return false;

  return true;
}

bool
CFile::
rseek_(long pos)
{
  int error = fseek(fp_, pos, SEEK_CUR);

  if (error != 0)
    return false;

  return true;
}

bool
CFile::
tell_(long &pos)
{
  pos = ftell(fp_);

  return true;
}

bool
CFile::
rewind_()
{
  ::rewind(fp_);

  return true;
}

bool
CFile::
eof_(bool &eof)
{
  eof = feof(fp_);

  return true;
}

bool
CFile::
error_(int &error)
{
  error = ferror(fp_);

  return true;
}

bool
CFile::
flush_()
{
  fflush(fp_);

  return true;
}

bool
CFile::
stat_(struct stat *file_stat)
{
  int error_code = fstat(fileno(fp_), file_stat);

  if (error_code != 0) {
    if      (errno == ENOENT)
      last_error_ = "File does not exist";
    else if (errno == EACCES)
      last_error_ = "No read permission for file";
    else
      last_error_ = "Stat Failed";

    return false;
  }

  return true;
}

bool
CFile::
lstat_(struct stat *file_stat)
{
  int error_code = lstat(getPath().c_str(), file_stat);

  if (error_code != 0) {
    if      (errno == ENOENT)
      last_error_ = "File does not exist";
    else if (errno == EACCES)
      last_error_ = "No read permission for file";
    else
      last_error_ = "Stat Failed";

    return false;
  }

  return true;
}

bool
CFile::
readlink_(std::string &linkname) const
{
  static char link_name[MAXPATHLEN + 1];

  int len = readlink(getPath().c_str(), link_name, MAXPATHLEN);

  if (len <= 0)
    return false;

  link_name[len] = '\0';

  linkname = link_name;

  return true;
}

bool
CFile::
isStdIn() const
{
  return (fp_ == stdin);
}

bool
CFile::
isStdOut() const
{
  return (fp_ == stdout);
}

bool
CFile::
isStdErr() const
{
  return (fp_ == stderr);
}
