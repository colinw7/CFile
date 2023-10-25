#ifndef CDirFTW_H
#define CDirFTW_H

#include <string>

#include <CFileType.h>

class CDirFTW {
 public:
  explicit CDirFTW(const std::string &dirname);

  virtual ~CDirFTW() = default;

  bool getFollowLinks() const { return followLinks_; }
  void setFollowLinks(bool follow) { followLinks_ = follow; }

  bool getChangeDir() const { return changeDir_; }
  void setChangeDir(bool cdir) { changeDir_ = cdir; }

  bool getDebug() const { return debug_; }
  void setDebug(bool debug) { debug_ = debug; }

  bool walk();

  virtual bool process() = 0;

 protected:
  const std::string &getDirName () const { return dirname_ ; }
  const std::string &getFileName() const { return filename_; }
  const struct stat *getStat    () const { return stat_    ; }
  CFileType          getType    () const { return type_    ; }

  std::string getBasename() const;

 private:
  bool callProcess(const char *filename, struct stat *stat, CFileType type);

#if defined(sun) || defined(__linux__)
  static int processCB(const char *filename, struct stat *stat, int type, struct FTW *ftw);
#else
  static int processCB(const char *filename, struct stat *stat, int type, struct FTW *ftw);
#endif

 private:
  static CDirFTW *walk_;

  std::string  dirname_;
  std::string  filename_;
  bool         followLinks_ { false };
  bool         changeDir_   { false };
  bool         debug_       { false };
  struct stat *stat_        { nullptr };
  CFileType    type_        { CFILE_TYPE_NONE };
};

#endif
