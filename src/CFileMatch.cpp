#include <CFileMatch.h>

#include <CDir.h>
#include <CStrUtil.h>
#include <CGlob.h>

CFileMatch::
CFileMatch()
{
}

CFileMatch::
~CFileMatch()
{
  for (const auto *glob : ignore_patterns_)
    delete glob;
}

void
CFileMatch::
addIgnorePattern(const std::string &pattern)
{
  auto *glob = new CGlob(pattern);

  ignore_patterns_.push_back(glob);
}

std::string
CFileMatch::
mostMatchPrefix(const std::string &prefix)
{
  StringArray files;

  if (! CFileMatch::matchPrefix(prefix, files))
    return prefix;

  return CStrUtil::mostMatch(files);
}

std::string
CFileMatch::
mostMatchPattern(const std::string &pattern)
{
  StringArray files;

  if (! CFileMatch::matchPattern(pattern, files))
    return pattern;

  return CStrUtil::mostMatch(files);
}

bool
CFileMatch::
matchPrefix(const std::string &prefix, StringArray &files)
{
  std::string pattern = prefix + "*";

  if (! matchPattern(pattern, files))
    return false;

  return true;
}

bool
CFileMatch::
matchPattern(const std::string &pattern, StringArray &files)
{
  auto words = CStrUtil::toFields(pattern, "/");

  auto num_words = words.size();
  if (num_words == 0) return false;

  StringArray words1;

  uint i = 0;

  for ( ; i < num_words; i++) {
    std::string word1 = words[int(i)].getWord();
    std::string word2;

    if (CFile::expandTilde(word1, word2))
      words1.push_back(word2);
    else
      words1.push_back(word1);
  }

  i = 0;

  while (i < num_words && words1[i].size() == 0)
    i++;

  if (i >= num_words)
    return false;

  if (pattern.size() > 0 && pattern[0] == '/')
    files.push_back("/");
  else
    files.push_back("");

  for ( ; i < num_words; i++) {
    if (words1[i].size() == 0)
      continue;

    std::vector< StringArray > files2_array;

    auto num_files = files.size();

    for (uint j = 0; j < num_files; j++) {
      if (files[j] != "") {
        if (! CFile::exists(files[j]) || ! CFile::isDirectory(files[j]))
          continue;

        CDir::enter(files[j]);
      }

      StringArray files1;

      matchCurrentDir(words1[i], files1);

      if (files1.size() > 0) {
        StringArray files2;

        auto num_files1 = files1.size();

        for (uint k = 0; k < num_files1; k++) {
          if (words1[i][0] != '.' &&
              (files1[k].size() > 0 && files1[k][0] == '.'))
            continue;

          std::string file = files[j];

          if (file.size() > 0 && file[file.size() - 1] != '/')
            file += "/";

          file += files1[k];

          files2.push_back(file);
        }

        files2_array.push_back(files2);
      }

      if (files[j] != "")
        CDir::leave();
    }

    files.clear();

    auto num_files2_array = files2_array.size();

    for (uint j = 0; j < num_files2_array; j++)
      copy(files2_array[j].begin(), files2_array[j].end(),
           back_inserter(files));
  }

  auto num_files = files.size();

  std::string word;

  for (i = 0; i < num_files; i++) {
    if (CFile::addTilde(files[i], word))
      files[i] = word;
  }

  return true;
}

bool
CFileMatch::
matchCurrentDir(const std::string &pattern, StringArray &files)
{
  if (pattern == "." || pattern == "..") {
    files.push_back(pattern);

    return true;
  }

  if (CFile::exists(pattern)) {
    files.push_back(pattern);

    return true;
  }

  CDir dir(".");

  CGlob glob(pattern);

  glob.setAllowOr(false);
  glob.setAllowNonPrintable(true);

  StringArray filenames;

  (void) dir.getFilenames(filenames);

  auto num_filenames = filenames.size();

  for (uint i = 0; i < num_filenames; i++) {
    const std::string &fileName = filenames[i];

    if (isIgnoreFile(fileName))
      continue;

    if (isOnlyExec() && ! CFile::isExecutable(fileName))
      continue;

    if (glob.compare(fileName))
      files.push_back(fileName);
  }

  return true;
}

bool
CFileMatch::
isIgnoreFile(const std::string &fileName)
{
  for (const auto *glob : ignore_patterns_)
    if (glob->compare(fileName))
      return true;

  return false;
}

bool
CFileMatch::
expandBraces(const std::string &str, StringArray &words)
{
  bool changed = false;

  //---

  uint i   = 0;
  uint len = uint(str.size());

  words.push_back("");

  std::string word;

  while (i < len) {
    if      (str[i] == '\"') {
      auto j = i;

      CStrUtil::skipDoubleQuotedString(str, &i, len);

      word += str.substr(j, i - j);
    }
    else if (str[i] == '\'') {
      auto j = i;

      CStrUtil::skipSingleQuotedString(str, &i, len);

      word += str.substr(j, i - j);
    }
    else if (str[i] == '{') {
      changed = true;

      //---

      if (word != "") {
        for (auto &w : words)
          w += word;

        word = "";
      }

      ++i;

      std::string word1;
      StringArray words1;

      int depth = 1;

      while (i < len) {
        if      (str[i] == ',' && depth == 1) {
          if (word1 != "") {
            StringArray words2;

            if (expandBraces(word1, words2)) {
              for (const auto &w2 : words2)
                words1.push_back(word + w2);
            }
            else
              words1.push_back(word + word1);

            word1 = "";
          }

          ++i;
        }
        else if (str[i] == '{') {
          ++depth;

          word1 += str[i++];
        }
        else if (str[i] == '}') {
          if (depth == 0)
            return false;

          --depth;

          if (depth == 0) {
            if (word1 != "") {
              StringArray words2;

              if (expandBraces(word1, words2)) {
                for (const auto &w2 : words2)
                  words1.push_back(word + w2);
              }
              else
                words1.push_back(word + word1);

              word1 = "";
            }

            ++i;

            break;
          }
          else
            word1 += str[i++];
        }
        else {
          word1 += str[i++];
        }
      }

      if (depth > 0)
        return false;

      StringArray words2;

      for (const auto &w : words) {
        for (const auto &w1 : words1) {
          words2.push_back(w + w1);
        }
      }

      words = words2;
    }
    else {
      word += str[i++];
    }
  }

  if (word != "") {
    for (auto &w : words)
      w += word;

    word = "";
  }

  return changed;
}
