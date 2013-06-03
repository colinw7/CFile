#include <CTempFile.h>
#include <COSFile.h>

CTempFile::
CTempFile(const std::string &dir)
{
  std::string filename;

  if (getTempFileName(dir, filename))
    file_ = new CFile(filename);
  else
    file_ = NULL;
}

CTempFile::
~CTempFile()
{
  delete file_;
}

bool
CTempFile::
getTempFileName(const std::string &dir, std::string &filename)
{
  if (! COSFile::getTempFileName(dir, filename))
    return false;

  return true;
}
