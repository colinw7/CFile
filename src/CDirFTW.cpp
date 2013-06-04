#include <CDirFTW.h>

#include <ftw.h>

#include <iostream>

typedef int (*FtwProc)(const char *file, const struct stat *sb, int flag, struct FTW *ftw);

CDirFTW *CDirFTW::walk_ = NULL;

CDirFTW::
CDirFTW(const std::string &dirname) :
 dirname_(dirname), follow_links_(false), change_dir_(false), debug_(false)
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

  nftw(dirname_.c_str(), (FtwProc) CDirFTW::processCB, 10, flags);
#else
  ftw (dirname_.c_str(), (FtwProc) CDirFTW::processCB, 10);
#endif

  return true;
}

int
CDirFTW::
#if defined(sun) || defined(__linux__)
processCB(const char *filename, struct stat *stat, int type)
#else
processCB(const char *filename, struct stat *stat, int type, struct FTW *)
#endif
{
  if (walk_->getDebug()) {
    switch (type) {
      case FTW_D  : std::cerr << "Dir : " << filename << std::endl; break;
      case FTW_DNR: std::cerr << "DirE: " << filename << std::endl; break;
      case FTW_DP : std::cerr << "DirP: " << filename << std::endl; break;
      case FTW_F  : std::cerr << "File: " << filename << std::endl; break;
      case FTW_NS : std::cerr << "LnkE: " << filename << std::endl; break;
      case FTW_SL : std::cerr << "Lnk : " << filename << std::endl; break;
      case FTW_SLN: std::cerr << "LnkN: " << filename << std::endl; break;
      default     : std::cerr << "??  : " << filename << std::endl; break;
    }
  }

  if (type == FTW_NS)
    return 0;

  CFileType file_type = CFILE_TYPE_INODE_REG;

  if (type == FTW_D || type == FTW_DNR || type == FTW_DP)
    file_type = CFILE_TYPE_INODE_DIR;

  walk_->callProcess(filename, stat, file_type);

  return 0;
}

void
CDirFTW::
callProcess(const char *filename, struct stat *stat, CFileType type)
{
  filename_ = filename;
  stat_     = stat;
  type_     = type;

  process();
}
