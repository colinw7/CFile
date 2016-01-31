#include <CDir.h>

#include <CStrUtil.h>
#include <COSFile.h>
#include <COSUser.h>

#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>

//------

namespace CDirStack {
  typedef std::pair<std::string,int> DirFd;
  typedef std::vector<DirFd> DirFdStack;

  DirFdStack stack;

  void push(const std::string &name, int fd) {
    stack.push_back(DirFd(name, fd));
  }

  bool empty() {
    return stack.empty();
  }

  int pop() {
    DirFd fd = stack.back();

    stack.pop_back();

    return fd.second;
  }
}

//------

class CDirScope {
 public:
  explicit CDirScope(const std::string &dirname) :
   dir_(0) {
    entered_ = CDir::enter(dirname);
  }

  explicit CDirScope(const CDir &dir) :
   dir_(&dir) {
    entered_ = dir_->enter();
  }

 ~CDirScope() {
    if (entered_) {
      if (dir_)
        dir_->leave();
      else
        CDir::leave();
    }
  }

  bool valid() const { return entered_; }

 private:
  const CDir *dir_;
  bool        entered_;
};

//------

CDir::
CDir(const char *dirname) :
 file_(dirname), dirname_(dirname)
{
  if (! file_.isDirectory()) {
    valid_ = false;

    getErrorMsg() = "File is not a directory";
  }
  else
    valid_ = true;
}

CDir::
CDir(const std::string &dirname) :
 file_(dirname), dirname_(dirname)
{
  if (! file_.isDirectory()) {
    valid_ = false;

    getErrorMsg() = "File is not a directory";
  }
  else
    valid_ = true;
}

CDir::
~CDir()
{
}

const std::string &
CDir::
getName() const
{
  return file_.getPath();
}

std::string
CDir::
getParentName(const std::string &dirName)
{
  CFile file(dirName + "/..");

  return file.getPath();
}

std::string
CDir::
getParentName() const
{
  CFile file(file_.getPath() + "/..");

  return file.getPath();
}

bool
CDir::
getSortedFilenames(const std::string &dirname, std::vector<std::string> &filenames)
{
  if (! getFilenames(dirname, filenames))
    return false;

  std::sort(filenames.begin(), filenames.end(), CStrCmpFunctor());

  return true;
}

bool
CDir::
getFilenames(const std::string &dirname, std::vector<std::string> &filenames)
{
  if (! enter(dirname))
    return false;

  void *handle = COSFile::openDir(".");

  std::string filename;

  while (handle && COSFile::readDir(handle, filename))
    filenames.push_back(filename);

  COSFile::closeDir(handle);

  leave();

  return true;
}

bool
CDir::
getSortedFilenames(std::vector<std::string> &filenames)
{
  if (! getFilenames(filenames))
    return false;

  std::sort(filenames.begin(), filenames.end(), CStrCmpFunctor());

  return true;
}

bool
CDir::
getFilenames(std::vector<std::string> &filenames)
{
  if (! enter(dirname_))
    return false;

  void *handle = COSFile::openDir(".");

  std::string filename;

  while (handle && COSFile::readDir(handle, filename))
    filenames.push_back(filename);

  COSFile::closeDir(handle);

  leave();

  return true;
}

bool
CDir::
processFiles(const std::string &dirname, CDirProcessProc &process)
{
  CDir dir(dirname);

  return dir.processFiles(process);
}

bool
CDir::
processFiles(CDirProcessProc &process)
{
  process.dir = this;

  if (! enter(dirname_))
    return false;

  void *handle = COSFile::openDir(".");

  while (handle && COSFile::readDir(handle, process.filename))
    process.process();

  COSFile::closeDir(handle);

  leave();

  return true;
}

bool
CDir::
processTree(const std::string &dirname, CDirProcessProc &process)
{
  CDir dir(dirname);

  return dir.processTree(process);
}

bool
CDir::
processTree(CDirProcessProc &process)
{
  return processSubTree(process, NULL);
}

bool
CDir::
processSubTree(CDirProcessProc &process, CDir *pdir)
{
  struct stat entry_stat;

  if (pdir != NULL)
    process.pdirs.push_back(pdir);

  process.dir = this;

  if (! enter(dirname_))
    return false;

  void *handle = COSFile::openDir(".");

  while (handle && COSFile::readDir(handle, process.filename)) {
    if (lstat(process.filename.c_str(), &entry_stat) < 0)
      continue;

    if (COSFile::stat_is_dir(&entry_stat)) {
      CDir dir(process.filename);

      CDir *save_dir = process.dir;

      dir.processSubTree(process, process.dir);

      process.dir = save_dir;
    }
    else
      process.process();
  }

  process.pdirs.pop_back();

  COSFile::closeDir(handle);

  leave();

  return true;
}

bool
CDir::
enter() const
{
  return enter(dirname_);
}

bool
CDir::
enter(const std::string &dirname)
{
  int dir_fd = open(".", O_RDONLY);

  if (dir_fd < 0) {
    getErrorMsg() = "Failed to open current dir";
    return false;
  }

  if (! changeDir(dirname))
    return false;

  CDirStack::push(dirname, dir_fd);

  return true;
}

bool
CDir::
leave()
{
  if (CDirStack::empty()) {
    getErrorMsg() = "enter/leave mismatch";
    return false;
  }

  int dir_fd = CDirStack::pop();

  if (fchdir(dir_fd) < 0) {
    getErrorMsg() = "failed to change to original dir";

    close(dir_fd);

    return false;
  }

  close(dir_fd);

  return true;
}

bool
CDir::
makeDirs(CFile &file)
{
  return makeDirs(file.getDir());
}

bool
CDir::
makeDirs(const std::string &dirname)
{
  if (CFile::exists(dirname))
    return true;

  std::string::size_type pos = dirname.rfind('/');

  if (pos != std::string::npos) {
    if (pos == 0)
      return true;

    std::string dirl = dirname.substr(0, pos);

    if (! makeDirs(dirl))
      return false;
  }

  if (! makeDir(dirname))
    return false;

  return true;
}

bool
CDir::
makeDir(const std::string &dirname)
{
  if (CFile::exists(dirname))
    return true;

  if (! COSFile::makeDir(dirname)) {
    getErrorMsg() = COSFile::getErrorMsg();

    return false;
  }

  return true;
}

#if 0
bool
CDir::
makeDirHier(const std::string &dirname)
{
  CDir dir(".");

  CDirScope dir_scope(dir);

  if (! dir_scope.valid())
    return false;

  int level = 0;

  std::string dirname1 = dirname;

  if (dirname1[0] == '/') {
    if (! changeDir("/"))
      getErrorMsg() = "Unable to Change to Root Directory";
      dir.leave();
      return false;
    }

    dirname1 = dirname1.substr(1);

    level++;
  }

  std::string::size_type pos = dirname1.find('/');

  while (pos != std::string::npos) {
    std::string prefix = dirname1.substr(0, pos);

    if (prefix != "") {
      if (! makeDir(prefix)) {
        getErrorMsg() = "Unable to Make Directory " + prefix;
        dir.leave();
        return false;
      }

      if (! changeDir(prefix)) {
        getErrorMsg() = "Unable to Change to Directory " + prefix;
        dir.leave();
        return false;
      }
    }

    dirname1 = dirname1.substr(pos + 1);

    level++;
  }

  if (dirname1 == "") {
    dir.leave();
    return true;
  }

  if (! makeDir(dirname1)) {
    getErrorMsg() = "Unable to Make Directory " + dirname1;
    dir.leave();
    return false;
  }

  if (! changeDir(dirname1)) {
    getErrorMsg() = "Unable to Change to Directory " + dirname1;
    dir.leave();
    return false;
  }

  dir.leave();

  return true;
}
#endif

bool
CDir::
isValid(const std::string &dirname)
{
  return CFile::isDirectory(dirname);
}

bool
CDir::
changeDir(const std::string &dirname)
{
  if (! COSFile::changeDir(dirname)) {
    getErrorMsg() = COSFile::getErrorMsg();

    return false;
  }

  return true;
}

std::string
CDir::
getCurrent()
{
  std::string dirname;

  if (! COSFile::getCurrentDir(dirname)) {
    getErrorMsg() = COSFile::getErrorMsg();

    return "";
  }

  return dirname;
}

#define CDIR_SAME_INODE(s1,s2) \
  ((s1).st_dev == (s2).st_dev && (s1).st_ino == (s2).st_ino)

bool
CDir::
getCurrentPath(std::string &path)
{
  struct stat stat_child, stat_parent, stat_entry;

  path = "";

  std::string dirname1(".");

  CDirScope scope(dirname1);

  if (lstat(".", &stat_child) < 0)
    return false;

  while (true) {
    if (lstat("..", &stat_parent) < 0)
      return false;

    int rc = COSFile::changeDir("..");

    if ((rc == -1 && errno == ENOENT) ||
        CDIR_SAME_INODE(stat_child, stat_parent)) {
      if (path == "")
        path = "/";

      return true;
    }

    if (rc < 0)
      return false;

    void *handle = COSFile::openDir(".");

    bool found = false;

    std::string filename;

    while (handle && COSFile::readDir(handle, filename)) {
      if (lstat(filename.c_str(), &stat_entry) < 0)
        continue;

      if (CDIR_SAME_INODE(stat_child, stat_entry)) {
        path = "/" + filename + path;
        found = true;
        break;
      }
    }

    COSFile::closeDir(handle);

    if (! found)
      return false;

    stat_child = stat_parent;
  }

  leave();

  return true;
}

std::string
CDir::
getHome()
{
  return COSUser::getUserHome();
}

std::string
CDir::
getHome(const std::string &name)
{
  std::string home_dir;

  if (! COSUser::getUserHome(name, home_dir)) {
    getErrorMsg() = COSUser::getErrorMsg();

    return "";
  }

  return home_dir;
}

//------------

void
CDir::
processPathList(const std::string &name, CDirProcessPathListProc proc, void *clientData)
{
  char *env = getenv((char *) name.c_str());

  if (env == NULL)
    return;

  std::string value = env;

  std::vector<std::string> paths;

  CStrUtil::addFields(value, paths, ":");

  uint num_paths = paths.size();

  for (uint i = 0; i < num_paths; ++i) {
    paths[i] = CStrUtil::stripSpaces(paths[i]);

    if (! CFile::exists(paths[i]) || ! CFile::isDirectory(paths[i]))
      continue;

    bool cont = (*proc)(paths[i], clientData);

    if (! cont)
      break;
  }
}
