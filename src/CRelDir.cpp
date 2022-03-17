#include <CRelDir.h>

#include <deque>

namespace CRelDir {

std::string
calc(const std::string &srcDir, const std::string &destDir)
{
  if (srcDir == "" || destDir == "")
    return "";

  if (srcDir[0] != '/')
    return ""; // use current dir

  if (destDir[0] != '/') {
    // use current dir

    if (destDir[0] != '.')
      return "./" + destDir;
    else
      return destDir;
   }

  auto splitPath = [](const std::string &str) {
    std::deque<std::string> parts;

    auto str1 = str;

    auto p = str1.rfind('/');

    while (p != std::string::npos) {
      parts.push_front(str1.substr(p + 1));

      str1 = str1.substr(0, p);

      p = str1.rfind('/');
    }

    if (str1 != "")
      parts.push_back(str1);

    return parts;
  };

  auto joinParts = [](const std::deque<std::string> &parts, bool relative=false) {
    std::string str;

    int i = 0;

    for (const auto &part : parts) {
      if (! relative || i > 0)
        str += "/";

      str += part;

      ++i;
    }

    return str;
  };

#if 0
  auto printParts = [&](const std::string &id, const std::deque<std::string> &parts) {
    std::cerr << id << " : " << joinParts(parts) << "\n";
  };
#endif

  auto srcParts  = splitPath(srcDir);
  auto destParts = splitPath(destDir);

//printParts("srcParts", srcParts);
//printParts("destParts", destParts);

//std::deque<std::string> srcPopParts;
  std::deque<std::string> destPopParts;

  std::deque<std::string> relParts;

  while (destParts.size() > srcParts.size()) {
    destPopParts.push_front(destParts.back());
    destParts.pop_back();
  }

  while (srcParts.size() > destParts.size()) {
    relParts.push_front("..");

//  srcPopParts.push_front(srcParts.back());
    srcParts.pop_back();
  }

  while (! srcParts.empty() && srcParts.back() != destParts.back()) {
    relParts.push_front("..");

//  srcPopParts .push_front(srcParts.back());
    destPopParts.push_front(destParts.back());

    srcParts .pop_back();
    destParts.pop_back();
  }

  for (const auto &part : destPopParts)
    relParts.push_back(part);

//printParts("srcParts", srcParts);
//printParts("destParts", destParts);
//printParts("relParts", relParts);
//printParts("srcPopParts", srcPopParts);
//printParts("destPopParts", destPopParts);

  auto res = joinParts(relParts, /*relative*/true);

  if      (res == "")
    res = ".";
  else if (res[0] != '.')
    res = "./" + res;

  return res;
}

}
