#ifndef CDir_H
#define CDir_H

#include <CFile.h>

class CDir;

#define CDIR_PROCESS_DATA \
  std::vector<CDir *> pdirs; \
  CDir *              dir; \
  std::string         filename;

struct CDirProcessData {
  CDIR_PROCESS_DATA
};

struct CDirProcessProc {
  CDIR_PROCESS_DATA

  CDirProcessProc() : dir(0) { }

  virtual ~CDirProcessProc() { }

  virtual void process() = 0;
};

class CDir {
 public:
  typedef bool (*CDirProcessPathListProc)(const std::string &path, void *clientData);

 public:
  CDir(const char *dirname=".");

  explicit CDir(const std::string &dirname);

 ~CDir();

  const std::string &getName() const;

  bool isValid() const { return valid_; }

  static std::string getParentName(const std::string &dirname);
  std::string getParentName() const;

  static bool getSortedFilenames(const std::string &dirname, std::vector<std::string> &files);
  static bool getFilenames(const std::string &dirname, std::vector<std::string> &files);

  bool getSortedFilenames(std::vector<std::string> &files);
  bool getFilenames(std::vector<std::string> &files);

  static bool isValid(const std::string &dirname);

  static bool processFiles(const std::string &dirname, CDirProcessProc &process);
         bool processFiles(CDirProcessProc &process);

  static bool processTree(const std::string &dirname, CDirProcessProc &process);
         bool processTree(CDirProcessProc &process);

  static bool enter(const std::string &dirname);
         bool enter() const;

  static bool leave();

  static bool makeDirs(CFile &file);
  static bool makeDirs(const std::string &dirname);
  static bool makeDir(const std::string &dirname);

  static bool changeDir(const std::string &dirname);

  static std::string getCurrent();

  static bool getCurrentPath(std::string &path);

  static std::string getHome();
  static std::string getHome(const std::string &name);

  static std::string &getErrorMsg() {
    static std::string error_msg;

    return error_msg;
  }

  static void processPathList(const std::string &name, CDirProcessPathListProc proc,
                              void *clientData);

 private:
  bool processSubTree(CDirProcessProc &process, CDir *pdir);

 private:
  CFile       file_;
  std::string dirname_;
  bool        valid_;
};

#endif
