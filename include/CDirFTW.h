#ifndef CDIR_FTW_H
#define CDIR_FTW_H

#include <string>

#include <CFileType.h>

class CDirFTW {
 public:
  CDirFTW(const std::string &dirname);

  virtual ~CDirFTW() { }

  void setFollowLinks(bool follow) { follow_links_ = follow; }
  bool getFollowLinks() const { return follow_links_; }

  void setChangeDir(bool cdir) { change_dir_ = cdir; }
  bool getChangeDir() const { return change_dir_; }

  void setDebug(bool debug) { debug_ = debug; }
  bool getDebug() const { return debug_; }

  bool walk();

  virtual void process() = 0;

 protected:
  const std::string &getFileName() const { return filename_; }
  const struct stat *getStat    () const { return stat_    ; }
  CFileType          getType    () const { return type_    ; }

 private:
  void callProcess(const char *filename, struct stat *stat, CFileType type);

#if defined(sun) || defined(__linux__)
  static int processCB(const char *filename, struct stat *stat, int type);
#else
  static int processCB(const char *filename, struct stat *stat, int type, struct FTW *ftw);
#endif

 private:
  static CDirFTW *walk_;

  std::string  dirname_;
  std::string  filename_;
  bool         follow_links_;
  bool         change_dir_;
  bool         debug_;
  struct stat *stat_;
  CFileType    type_;
  std::string  link_dirname_;
};

#endif
