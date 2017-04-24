#include <COStreamFile.h>
#include <iostream>

COStreamFile::
COStreamFile(std::ostream &os) :
 CFileBase(""), os_(os)
{
  mode_   = Mode::WRITE;
  opened_ = true;
  owner_  = false;
}

COStreamFile::
~COStreamFile()
{
}

bool
COStreamFile::
open_(const std::string &)
{
  return false;
}

bool
COStreamFile::
close_()
{
  return false;
}

bool
COStreamFile::
read_(uchar *, size_t, size_t *)
{
  return false;
}

bool
COStreamFile::
write_(const uchar *data, size_t size)
{
  for (size_t i = 0; i < size; ++i)
    os_ << data[i];

  return true;
}

bool
COStreamFile::
getc_(int &)
{
  return false;
}

bool
COStreamFile::
ungetc_(int)
{
  return false;
}

bool
COStreamFile::
putc_(int c)
{
  os_ << (char) c;

  return true;
}

bool
COStreamFile::
seek_(long)
{
  return false;
}

bool
COStreamFile::
rseek_(long)
{
  return false;
}

bool
COStreamFile::
tell_(long &)
{
  return false;
}

bool
COStreamFile::
rewind_()
{
  return false;
}

bool
COStreamFile::
eof_(bool &)
{
  return false;
}

bool
COStreamFile::
error_(int &)
{
  return false;
}

bool
COStreamFile::
flush_()
{
  os_.flush();

  return true;
}

bool
COStreamFile::
stat_(struct stat *)
{
  return false;
}

bool
COStreamFile::
lstat_(struct stat *)
{
  return false;
}

bool
COStreamFile::
isCOut() const
{
  return (&os_ == &std::cout);
}

bool
COStreamFile::
isCErr() const
{
  return (&os_ == &std::cerr);
}

bool
COStreamFile::
readlink_(std::string &) const
{
  return false;
}
