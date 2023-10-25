#include <CDirFTW.h>

#include <ftw.h>

#include <iostream>

using FtwProc = int (*)(const char *file, const struct stat *sb, int flag, struct FTW *ftw);

CDirFTW *CDirFTW::walk_ = NULL;

CDirFTW::
CDirFTW(const std::string &dirname) :
 dirname_(dirname)
{
}

bool
CDirFTW::
walk()
{
  walk_ = this;

#if defined(sun) || defined(__linux__)
  int flags = 0;

  if (! getFollowLinks()) flags |= FTW_PHYS;
  if (! getChangeDir  ()) flags |= FTW_CHDIR;

  nftw(dirname_.c_str(), reinterpret_cast<FtwProc>(CDirFTW::processCB), 10, flags);
#else
  ftw (dirname_.c_str(), reinterpret_cast<FtwProc>(CDirFTW::processCB), 10);
#endif

  return true;
}

int
CDirFTW::
#if defined(sun) || defined(__linux__)
processCB(const char *filename, struct stat *stat, int type, struct FTW *)
#else
processCB(const char *filename, struct stat *stat, int type, struct FTW *)
#endif
{
  if (walk_->getDebug()) {
    switch (type) {
      case FTW_D  : std::cerr << "Dir : " << filename << "\n"; break;
      case FTW_DNR: std::cerr << "DirE: " << filename << "\n"; break;
      case FTW_DP : std::cerr << "DirP: " << filename << "\n"; break;
      case FTW_F  : std::cerr << "File: " << filename << "\n"; break;
      case FTW_NS : std::cerr << "LnkE: " << filename << "\n"; break;
      case FTW_SL : std::cerr << "Lnk : " << filename << "\n"; break;
      case FTW_SLN: std::cerr << "LnkN: " << filename << "\n"; break;
      default     : std::cerr << "??  : " << filename << "\n"; break;
    }
  }

  if (type == FTW_NS)
    return 0;

  CFileType fileType = CFILE_TYPE_INODE_REG;

  if (type == FTW_D || type == FTW_DNR || type == FTW_DP)
    fileType = CFILE_TYPE_INODE_DIR;

  auto rc = walk_->callProcess(filename, stat, fileType);

  return (rc ? 0 : 1);
}

bool
CDirFTW::
callProcess(const char *filename, struct stat *stat, CFileType type)
{
  filename_ = filename;
  stat_     = stat;
  type_     = type;

  return process();
}

std::string
CDirFTW::
getBasename() const
{
  auto p = filename_.rfind('/');

  if (p != std::string::npos)
    return filename_.substr(p + 1);
  else
    return filename_;
}
