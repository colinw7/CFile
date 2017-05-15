#include <CFileBase.h>
#include <CDir.h>
#include <CFileDevice.h>

#include <COSFile.h>
#include <CStrUtil.h>

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cassert>
#include <unistd.h>

bool        CFileBase::use_lstat_;
std::string CFileBase::last_error_;

CFileBase::
CFileBase() :
 mode_(Mode::NONE)
{
}

CFileBase::
CFileBase(const std::string &filename) :
 mode_(Mode::NONE)
{
  init(filename);
}

CFileBase::
CFileBase(const char *filename) :
 mode_(Mode::NONE)
{
  init(filename);
}

CFileBase::
~CFileBase()
{
}

void
CFileBase::
init(const std::string &filename)
{
  std::string::size_type pos = 0;

  std::string::size_type pos1 = filename.find(':');

  if (pos1 != std::string::npos) {
    device_ = filename.substr(pos, pos1 - pos);

    pos = pos1 + 1;
  }
  else
    device_ = CFileDevice::getCurrent();

  bool fix_dir = false;

  if (filename[pos] == '/') {
    pos1 = filename.rfind('/');

    dir_ = filename.substr(pos, pos1 - pos);

    pos = pos1 + 1;

    fix_dir = true;
  }
  else {
    pos1 = filename.rfind('/');

    if (pos1 != std::string::npos) {
      dir_ = filename.substr(pos, pos1 - pos);

      pos = pos1 + 1;

      fix_dir = true;
    }
    else
      dir_ = CDir::getCurrent();
  }

  if (dir_.empty()) {
    dir_ = "/";

    fix_dir = false;
  }

  if (fix_dir) {
    if (exists(dir_) && isDirectory(dir_)) {
      std::string current_dir = CDir::getCurrent();

      CDir::changeDir(dir_);

      dir_ = CDir::getCurrent();

      CDir::changeDir(current_dir);
    }
  }

  name_ = filename.substr(pos);

  pos1 = name_.rfind('.');

  if (pos1 != std::string::npos) {
    base_   = name_.substr(0, pos1);
    suffix_ = name_.substr(pos1 + 1);
  }
  else {
    base_   = name_;
    suffix_ = "";
  }

  path_ = dir_ + "/" + name_;

  opened_ = false;

  owner_ = true;

  last_error_ = "";
}

bool
CFileBase::
open(CFileBase::Mode mode)
{
  if (opened_) {
    if (! close())
      return false;
  }

  bool flag;

  if      (mode == Mode::READ)
    flag = open_("rb");
  else if (mode == Mode::WRITE)
    flag = open_("wb");
  else if (mode == Mode::APPEND) {
    if (! exists())
      flag = open_("wb+");
    else
      flag = open_("rb+");
  }
  else {
    last_error_ = "Invalid mode '" + toString(mode) + "'";
    return false;
  }

  if (! flag) {
    last_error_ = "Failed to open " + path_;
    return false;
  }

  opened_ = true;
  mode_   = mode;

  return true;
}

bool
CFileBase::
close()
{
  if (opened_) {
    if (! owner_) {
      last_error_ = "File not Owned";
      return false;
    }

    close_();
  }

  opened_ = false;

  return true;
}

CFileLines *
CFileBase::
toLines()
{
  CFileLines *lines = new CFileLines(this);

  if (! openCheck(Mode::READ))
    return nullptr;

  if (! rewind())
    return nullptr;

  std::string line;

  while (readLine(line))
    lines->addLine(line);

  close();

  return lines;
}

CFileLines *
CFileBase::
toLines(const std::string &filename)
{
  CFile file(filename);

  return file.toLines();
}

bool
CFileBase::
toLines(std::vector<std::string> &lines)
{
  if (! openCheck(Mode::READ))
    return false;

  if (! rewind())
    return false;

  std::string line;

  while (readLine(line))
    lines.push_back(line);

  close();

  return true;
}

bool
CFileBase::
toLines(std::list<std::string> &lines)
{
  if (! openCheck(Mode::READ))
    return false;

  if (! rewind())
    return false;

  std::string line;

  while (readLine(line))
    lines.push_back(line);

  close();

  return true;
}

bool
CFileBase::
toLines(const std::string &filename, std::vector<std::string> &lines)
{
  CFile file(filename);

  return file.toLines(lines);
}

bool
CFileBase::
toLines(const std::string &filename, std::list<std::string> &lines)
{
  CFile file(filename);

  return file.toLines(lines);
}

bool
CFileBase::
readLine(std::string &line)
{
  if (! openCheck(Mode::READ))
    return false;

  int c;

  if (! getc_(c))
    return false;

  if (c == EOF)
    return false;

  line = "";

  while (c != EOF && c != '\n') {
    line += (char) c;

    if (! getc_(c))
      return false;
  }

  return true;
}

std::string
CFileBase::
toString()
{
  CFileData *data = readAll();

  if (! data)
    return "";

  std::string str((const char *) data->getData(), data->getSize());

  delete data;

  return str;
}

bool
CFileBase::
readAll(uchar **data, size_t *len)
{
  if (! openCheck(Mode::READ))
    return false;

  if (! rewind())
    return false;

  *len = getSize();

  *data = new uchar [*len + 1];

  bool flag = read_(*data, *len, nullptr);

  (*data)[*len] = 0;

  close();

  if (! flag) {
    last_error_ = "Read Failed";
    return false;
  }

  return true;
}

CFileData *
CFileBase::
readAll()
{
  if (! openCheck(Mode::READ))
    return nullptr;

  if (! rewind())
    return nullptr;

  size_t size = getSize();

  CFileData *file_data = new CFileData(size);

  bool flag = read_(file_data->getData(), size, nullptr);

  close();

  if (! flag) {
    last_error_ = "Read Failed";
    return nullptr;
  }

  return file_data;
}

void
CFileBase::
printf(const char *format, ...)
{
  va_list vargs;

  va_start(vargs, format);

  std::string str;

  CStrUtil::vsprintf(str, format, &vargs);

  write(str);

  va_end(vargs);
}

bool
CFileBase::
writeAll(const CFileData *file_data)
{
  close();

  if (! openCheck(Mode::WRITE))
    return false;

  bool flag = write_(file_data->getData(), file_data->getSize());

  close();

  if (! flag) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
read(CFileData *file_data)
{
  if (! openCheck(Mode::READ))
    return false;

  if (! read_(file_data->getData(), file_data->getSize(), nullptr)) {
    last_error_ = "Read Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
read(uchar *data, size_t size)
{
  if (! openCheck(Mode::READ))
    return false;

  if (! read_(data, size, nullptr)) {
    last_error_ = "Read Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
read(uchar *data, size_t size, size_t *actual_size)
{
  if (! openCheck(Mode::READ))
    return false;

  return read_(data, size, actual_size);
}

bool
CFileBase::
writef(const char *format, ...)
{
  std::string str;

  va_list vargs;

  va_start(vargs, format);

  CStrUtil::vsprintf(str, format, &vargs);

  va_end(vargs);

  return write(str);
}

bool
CFileBase::
write(const CFileData *file_data)
{
  if (! openCheck(Mode::WRITE))
    return false;

  if (! write_(file_data->getData(), file_data->getSize())) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
write(const std::string &str)
{
  return write(str.c_str(), str.size());
}

bool
CFileBase::
write(const char *str)
{
  return write((uchar *) str, strlen(str));
}

bool
CFileBase::
write(const uchar *data, size_t size)
{
  if (! openCheck(Mode::WRITE))
    return false;

  if (! write_(data, size)) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
write(const char *data, size_t size)
{
  if (! openCheck(Mode::WRITE))
    return false;

  if (! write_((const uchar *) data, size)) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
write(uchar c)
{
  if (! openCheck(Mode::WRITE))
    return false;

  if (! write_(&c, 1)) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

bool
CFileBase::
write(char c)
{
  if (! openCheck(Mode::WRITE))
    return false;

  if (! write_((uchar *) &c, 1)) {
    last_error_ = "Write Failed";
    return false;
  }

  return true;
}

int
CFileBase::
getC()
{
  if (! openCheck(Mode::READ))
    return EOF;

  int c;

  if (! getc_(c))
    return EOF;

  return c;
}

bool
CFileBase::
ungetC(int c)
{
  if (! openCheck(Mode::READ))
    return EOF;

  if (! ungetc_(c))
    return false;

  return true;
}

bool
CFileBase::
putC(int c)
{
  if (! openCheck(Mode::WRITE))
    return false;

  putc_(c);

  return true;
}

bool
CFileBase::
rewind()
{
  if (! openCheck(Mode::READ))
    return false;

  rewind_();

  return true;
}

long
CFileBase::
getPos()
{
  if (opened_) {
    long pos;

    if (! tell_(pos))
      return 0;

    return pos;
  }
  else
    return 0;
}

bool
CFileBase::
setPos(long pos)
{
  if (! openCheck(Mode::READ_WRITE))
    return false;

  if (! seek_(pos)) {
    last_error_ = "setPos failed";
    return false;
  }

  return true;
}

bool
CFileBase::
setRPos(long pos)
{
  if (! openCheck(Mode::READ_WRITE))
    return false;

  if (! rseek_(pos)) {
    last_error_ = "setRPos failed";
    return false;
  }

  return true;
}

bool
CFileBase::
setPosStart()
{
  if (! openCheck(Mode::READ_WRITE))
    return false;

  if (! seek_(0)) {
    last_error_ = "setPosStart failed";
    return false;
  }

  return true;
}

bool
CFileBase::
setPosEnd()
{
  if (! openCheck(Mode::READ_WRITE))
    return false;

  if (! seek_(-1)) {
    last_error_ = "setPosEnd failed";
    return false;
  }

  return true;
}

bool
CFileBase::
eof()
{
  if (! openCheck(Mode::READ))
    return true;

  bool flag;

  if (! eof_(flag))
    return true;

  return flag;
}

int
CFileBase::
error()
{
  if (! openCheck(Mode::READ))
    return true;

  int flag;

  if (! error_(flag))
    return true;

  return flag;
}

bool
CFileBase::
flush()
{
  if (! openCheck(Mode::READ))
    return true;

  return flush_();
}

bool
CFileBase::
exists() const
{
  if (opened_)
    return true;
  else
    return exists(path_);
}

bool
CFileBase::
exists(const std::string &filename)
{
  struct stat file_stat;

  int error_code = stat(filename.c_str(), &file_stat);

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

uint
CFileBase::
getIDev()
{
  if (! openCheck(Mode::READ))
    return 0;

  if (! getStat())
    return 0;

  return file_stat_.st_dev;
}

uint
CFileBase::
getINode()
{
  if (! openCheck(Mode::READ))
    return 0;

  if (! getStat())
    return 0;

  return file_stat_.st_ino;
}

CFileType
CFileBase::
getType()
{
  if (opened_) {
    if (! getStat())
      return CFILE_TYPE_NONE;

    return getType(file_stat_.st_mode);

    return CFILE_TYPE_NONE;
  }
  else
    return getType(path_);
}

CFileType
CFileBase::
getType(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return CFILE_TYPE_NONE;

  return getType(file_stat.st_mode);
}

CFileType
CFileBase::
getType(uint mode)
{
  if (isFIFOMode  (mode)) return CFILE_TYPE_INODE_FIFO;
  if (isCharMode  (mode)) return CFILE_TYPE_INODE_CHR ;
  if (isDirMode   (mode)) return CFILE_TYPE_INODE_DIR ;
  if (isBlockMode (mode)) return CFILE_TYPE_INODE_BLK ;
  if (isRegMode   (mode)) return CFILE_TYPE_INODE_REG ;
  if (isLinkMode  (mode)) return CFILE_TYPE_INODE_LNK ;
  if (isSocketMode(mode)) return CFILE_TYPE_INODE_SOCK;

  return CFILE_TYPE_NONE;
}

char
CFileBase::
getTypeChar()
{
  if (opened_) {
    if (! getStat())
      return '?';

    return getTypeChar(file_stat_.st_mode);

    return '?';
  }
  else
    return getTypeChar(path_);
}

char
CFileBase::
getTypeChar(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return '?';

  return getTypeChar(file_stat.st_mode);
}

char
CFileBase::
getTypeChar(uint mode)
{
  if (isFIFOMode  (mode)) return 'p';
  if (isCharMode  (mode)) return 'c' ;
  if (isDirMode   (mode)) return 'd' ;
  if (isBlockMode (mode)) return 'b' ;
  if (isRegMode   (mode)) return ' ' ;
  if (isLinkMode  (mode)) return 'l' ;
  if (isSocketMode(mode)) return 's';

  return '?';
}

bool
CFileBase::
isDirectory() const
{
  return isType(&CFileBase::isDirMode);
}

bool
CFileBase::
isDirectory(const std::string &filename)
{
  return isType(&CFileBase::isDirMode, filename);
}

bool
CFileBase::
isRegular() const
{
  return isType(&CFileBase::isRegMode);
}

bool
CFileBase::
isRegular(const std::string &filename)
{
  return isType(&CFileBase::isRegMode, filename);
}

bool
CFileBase::
isChar() const
{
  return isType(&CFileBase::isCharMode);
}

bool
CFileBase::
isChar(const std::string &filename)
{
  return isType(&CFileBase::isCharMode, filename);
}

bool
CFileBase::
isBlock() const
{
  return isType(&CFileBase::isBlockMode);
}

bool
CFileBase::
isBlock(const std::string &filename)
{
  return isType(&CFileBase::isBlockMode, filename);
}

bool
CFileBase::
isFIFO() const
{
  return isType(&CFileBase::isFIFOMode);
}

bool
CFileBase::
isFIFO(const std::string &filename)
{
  return isType(&CFileBase::isFIFOMode, filename);
}

bool
CFileBase::
isLink() const
{
  return isType(&CFileBase::isLinkMode);
}

bool
CFileBase::
isLink(const std::string &filename)
{
  return isType(&CFileBase::isLinkMode, filename);
}

bool
CFileBase::
isSocket() const
{
  return isType(&CFileBase::isSocketMode);
}

bool
CFileBase::
isSocket(const std::string &filename)
{
  return isType(&CFileBase::isSocketMode, filename);
}

bool
CFileBase::
isHidden() const
{
  const std::string &name = getName();

  return (name.size() > 0 && name[0] == '.');
}

bool
CFileBase::
isHidden(const std::string &filename)
{
  CFile file(filename);

  return file.isHidden();
}

bool
CFileBase::
isType(CFileModeProc proc) const
{
  if (opened_) {
    if (! getStat())
      return false;

    if (! proc(file_stat_.st_mode))
      return false;

    return true;
  }
  else
    return isType(proc, path_);
}

bool
CFileBase::
isType(CFileModeProc proc, const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return false;

  if (! proc(file_stat.st_mode))
    return false;

  return true;
}

bool
CFileBase::
isDirMode(int mode)
{
  return COSFile::stat_mode_is_dir(mode);
}

bool
CFileBase::
isRegMode(int mode)
{
  return COSFile::stat_mode_is_reg(mode);
}

bool
CFileBase::
isCharMode(int mode)
{
  return COSFile::stat_mode_is_char(mode);
}

bool
CFileBase::
isBlockMode(int mode)
{
  return COSFile::stat_mode_is_block(mode);
}

bool
CFileBase::
isFIFOMode(int mode)
{
  return COSFile::stat_mode_is_fifo(mode);
}

bool
CFileBase::
isLinkMode(int mode)
{
  return COSFile::stat_mode_is_link(mode);
}

bool
CFileBase::
isSocketMode(int mode)
{
  return COSFile::stat_mode_is_socket(mode);
}

bool
CFileBase::
isReadable() const
{
  if (! exists() || isDirectory())
    return false;

  int rc = access(path_.c_str(), R_OK);

  return (rc == 0);
}

bool
CFileBase::
isReadable(const std::string &filename)
{
  if (! exists(filename) || isDirectory(filename))
    return false;

  int rc = access(filename.c_str(), R_OK);

  return (rc == 0);
}

bool
CFileBase::
isWritable() const
{
  if (! exists() || isDirectory())
    return false;

  int rc = access(path_.c_str(), W_OK);

  return (rc == 0);
}

bool
CFileBase::
isWritable(const std::string &filename)
{
  if (! exists(filename) || isDirectory(filename))
    return false;

  int rc = access(filename.c_str(), W_OK);

  return (rc == 0);
}

bool
CFileBase::
isExecutable() const
{
  if (! exists() || isDirectory())
    return false;

  int rc = access(path_.c_str(), X_OK);

  return (rc == 0);
}

bool
CFileBase::
isExecutable(const std::string &filename)
{
  if (! exists(filename) || isDirectory(filename))
    return false;

  int rc = access(filename.c_str(), X_OK);

  return (rc == 0);
}

bool
CFileBase::
isOwner() const
{
  if (opened_) {
    if (! getStat())
      return false;

    if (! COSFile::stat_is_effective_owner(&file_stat_))
      return false;

    return true;
  }
  else
    return isOwner(path_);
}

bool
CFileBase::
isOwner(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return false;

  if (! COSFile::stat_is_effective_owner(&file_stat))
    return false;

  return true;
}

size_t
CFileBase::
getSize()
{
#if 0
  bool opened;

  if (! openCheck(Mode::READ, opened) && ! openCheck(Mode::WRITE, opened))
    return false;

  long pos;

  if (! tell_(pos))
    return 0;

  if (! seek_(-1))
    return 0;

  long size = 0;

  if (! tell_(size))
    return 0;

  if (! seek_(pos))
    return 0;

  if (opened)
    close();

  return (size_t) size;
#else
  if (! getStat())
    return false;

  return file_stat_.st_size;
#endif
}

size_t
CFileBase::
getSize(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_size;
}

const std::string &
CFileBase::
getPath() const
{
  return path_;
}

const std::string &
CFileBase::
getDevice() const
{
  return device_;
}

const std::string &
CFileBase::
getDir() const
{
  return dir_;
}

const std::string &
CFileBase::
getName() const
{
  return name_;
}

const std::string &
CFileBase::
getBase() const
{
  return base_;
}

const std::string &
CFileBase::
getSuffix() const
{
  return suffix_;
}

std::string
CFileBase::
getSuffix(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('.');

  if (pos != std::string::npos)
    return filename.substr(pos + 1);
  else
    return "";
}

std::string
CFileBase::
getTail(const std::string &filename)
{
  std::string::size_type pos = filename.rfind('/');

  if (pos != std::string::npos)
    return filename.substr(pos + 1);

  return filename;
}

int
CFileBase::
getMode() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_mode;
}

int
CFileBase::
getMode(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_mode;
}

int
CFileBase::
getUMode() const
{
  return ((getMode() & S_IRWXU) >> 6);
}

int
CFileBase::
getGMode() const
{
  return ((getMode() & S_IRWXG) >> 3);
}

int
CFileBase::
getOMode() const
{
  return ((getMode() & S_IRWXO) >> 0);
}

int
CFileBase::
getUID() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_uid;
}

int
CFileBase::
getUID(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_uid;
}

int
CFileBase::
getGID() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_gid;
}

int
CFileBase::
getGID(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_gid;
}

int
CFileBase::
getMTime() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_mtime;
}

int
CFileBase::
getMTime(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_mtime;
}

int
CFileBase::
getCTime() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_ctime;
}

int
CFileBase::
getCTime(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_ctime;
}

int
CFileBase::
getATime() const
{
  if (! getStat())
    return 0;

  return file_stat_.st_atime;
}

int
CFileBase::
getATime(const std::string &filename)
{
  struct stat file_stat;

  if (! getStat(filename, &file_stat))
    return 0;

  return file_stat.st_atime;
}

bool
CFileBase::
getLinkName(std::string &linkname) const
{
  return readlink_(linkname);
}

bool
CFileBase::
openCheck(CFileBase::Mode mode)
{
  bool opened;

  return openCheck(mode, opened);
}

bool
CFileBase::
openCheck(CFileBase::Mode mode, bool &opened)
{
  opened = false;

  if (opened_) {
    if (! (uint(mode_) & uint(mode)))
      close();
  }

  if (! opened_) {
    if (! open(mode))
      return false;

    opened = true;
  }

  return true;
}

bool
CFileBase::
expandTilde(const std::string &str, std::string &str1)
{
  int len = str.size();

  if (len == 0 || str[0] != '~')
    return false;

  int i = 1;

  while (i < len && str[i] != '/')
    i++;

  std::string name;

  if (i > 1)
    name = str.substr(1, i - 1);

  if (name != "")
    str1 = CDir::getHome(name);
  else
    str1 = CDir::getHome();

  str1 += str.substr(i);

  return true;
}

bool
CFileBase::
addTilde(const std::string &str, std::string &str1)
{
  std::string home = CDir::getHome();

  std::string::size_type pos = str.find(home);

  if (pos != 0)
    return false;

  if (pos + home.size() > str.size() && str[pos + home.size()] != '/')
    return false;

  str1 = "~" + str.substr(pos + home.size());

  return true;
}

bool
CFileBase::
remove()
{
  close();

  return remove(path_.c_str());
}

bool
CFileBase::
remove(const std::string &path)
{
  int error = ::unlink(path.c_str());

  return (error == 0);
}

bool
CFileBase::
copy(CFileBase &file)
{
  close();

  file.close();

  return copyBytes(file);
}

bool
CFileBase::
copyBytes(CFileBase &file)
{
  if (! open(Mode::READ))
    return false;

  if (! file.open(Mode::WRITE))
    return false;

  int c;

  while (getc_(c) && c != EOF)
    file.putc_(c);

  close();

  file.close();

  return true;
}

bool
CFileBase::
getStat() const
{
  if (getUseLStat())
    return getLStat();
  else
    return getFStat();
}

bool
CFileBase::
getFStat() const
{
  CFileBase *th = const_cast<CFileBase *>(this);

  if (! th->openCheck(Mode::READ) && ! th->openCheck(Mode::WRITE))
    return false;

  if (! th->stat_(&th->file_stat_))
    return false;

  return true;
}

bool
CFileBase::
getLStat() const
{
  CFileBase *th = const_cast<CFileBase *>(this);

  if (! th->openCheck(Mode::READ) && ! th->openCheck(Mode::WRITE))
    return false;

  if (! th->lstat_(&th->file_stat_))
    return false;

  return true;
}

bool
CFileBase::
getStat(const std::string &filename, struct stat *file_stat)
{
  if (getUseLStat())
    return getLStat(filename, file_stat);
  else
    return getFStat(filename, file_stat);
}

bool
CFileBase::
getFStat(const std::string &filename, struct stat *file_stat)
{
  int error_code = ::stat(filename.c_str(), file_stat);

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
CFileBase::
getLStat(const std::string &filename, struct stat *file_stat)
{
  int error_code = ::lstat(filename.c_str(), file_stat);

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

std::string
CFileBase::
getModeString()
{
  return getUModeString() + getGModeString() + getOModeString();
}

std::string
CFileBase::
getUModeString()
{
  return getModeString1(getUMode(), S_IRUSR);
}

std::string
CFileBase::
getGModeString()
{
  return getModeString1(getGMode(), S_IRGRP);
}

std::string
CFileBase::
getOModeString()
{
  return getModeString1(getOMode(), S_IROTH);
}

std::string
CFileBase::
getModeString1(int mode, int type)
{
  std::string str = "";

  if (mode & S_IROTH)
    str += 'r';
  else
    str += '-';

  if (mode & S_IWOTH)
    str += 'w';
  else
    str += '-';

  if (mode & S_IXOTH)
    str += 'x';
  else
    str += '-';

  if (type == S_IRUSR && COSFile::stat_mode_is_uid_on_exec(mode)) {
    if (mode & S_IXOTH)
      str[2] = 's';
    else
      str[2] = 'S';
  }

  if (type == S_IRGRP && COSFile::stat_mode_is_gid_on_exec(mode)) {
    if (mode & S_IXOTH)
      str[2] = 's';
    else
      str[2] = 'S';
  }

  if (type == S_IROTH && COSFile::stat_mode_is_restrict_delete(mode)) {
    if (mode & S_IXOTH)
      str[2] = 't';
    else
      str[2] = 'T';
  }

  return str;
}

void
CFileBase::
skipSpace()
{
  int c;

  while ((c = getC()) != EOF) {
    if (! isspace(c)) {
      ungetC(c);
      break;
    }
  }
}

std::string
CFileLines::
getLine(int pos) const
{
  assert(pos >= 0 && pos < size());

  return lines_[pos];
}

void
CFileLines::
addLine(const std::string &line)
{
  lines_.push_back(line);
}
