#ifndef CFileMatch_H
#define CFileMatch_H

#include <string>
#include <vector>
#include <list>

class CGlob;

class CFileMatch {
 public:
  using StringArray = std::vector<std::string>;

 public:
  CFileMatch();
 ~CFileMatch();

  void addIgnorePattern(const std::string &pattern);

  bool isOnlyExec() const { return only_exec_; }
  void setOnlyExec(bool exec=true) { only_exec_ = exec; }

  std::string mostMatchPrefix(const std::string &prefix);
  std::string mostMatchPattern(const std::string &pattern);

  bool matchPrefix(const std::string &prefix, StringArray &files);
  bool matchPattern(const std::string &pattern, StringArray &files);

  bool matchCurrentDir(const std::string &pattern, StringArray &files);

  bool expandBraces(const std::string &str, StringArray &words);

 private:
  bool isIgnoreFile(const std::string &fileName);

 private:
  using GlobList = std::list<CGlob *>;

  GlobList ignore_patterns_;
  bool     only_exec_ { false };
};

#endif
