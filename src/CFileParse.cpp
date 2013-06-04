#include <CFileParse.h>

#include <CFile.h>
#include <CStrUtil.h>

CFileParse::
CFileParse(CFile *file) :
 file_(file), stream_(false)
{
}

bool
CFileParse::
skipSpace()
{
  if (isSpace()) {
    while (isSpace())
      readChar();

    return true;
  }
  else
    return false;
}

bool
CFileParse::
skipNonSpace()
{
  if (isNonSpace()) {
    while (isNonSpace())
      readChar();

    return true;
  }
  else
    return false;
}

bool
CFileParse::
skipToEnd()
{
  if (! eof1()) {
    while (skipChar()) { }

    return true;
  }
  else
    return false;
}

bool
CFileParse::
readSpace(std::string &text)
{
  text = "";

  if (eof1() || isNonSpace())
    return false;

  while (isSpace())
    text += readChar();

  return true;
}

bool
CFileParse::
readNonSpace(std::string &text)
{
  text = "";

  if (eof1() || isSpace())
    return false;

  while (isNonSpace())
    text += readChar();

  return true;
}

bool
CFileParse::
readInteger(int *integer)
{
  std::string str;

  if (isChar('+') || isChar('-'))
    str += readChar();

  if (eof1() || ! isDigit()) {
    unread(str);
    return false;
  }

  str += readChar();

  while (isDigit())
    str += readChar();

  if (integer != NULL)
    *integer = CStrUtil::toInteger(str);

  return true;
}

bool
CFileParse::
readInteger(uint *integer)
{
  std::string str;

  if (eof1() || ! isDigit()) {
    unread(str);
    return false;
  }

  str += readChar();

  while (isDigit())
    str += readChar();

  if (integer != NULL)
    *integer = CStrUtil::toInteger(str);

  return true;
}

bool
CFileParse::
readBaseInteger(uint base, int *integer)
{
  std::string str;

  if (isChar('+') || isChar('-'))
    str += readChar();

  if (eof1() || ! isBaseChar(base)) {
    unread(str);
    return false;
  }

  str += readChar();

  while (isBaseChar(base))
    str += readChar();

  if (integer != NULL)
    *integer = CStrUtil::toBaseInteger(str, base);

  return true;
}

bool
CFileParse::
readBaseInteger(uint base, uint *integer)
{
  std::string str;

  if (eof1() || ! isBaseChar(base)) {
    unread(str);
    return false;
  }

  str += readChar();

  while (isBaseChar(base))
    str += readChar();

  if (integer != NULL)
    *integer = CStrUtil::toBaseInteger(str, base);

  return true;
}

bool
CFileParse::
readReal(double *real)
{
  std::string str;

  //------

  if (isChar('+') || isChar('-'))
    str += readChar();

  //------

  while (isDigit())
    str += readChar();

  //------

  if (isChar('.')) {
    str += readChar();

    while (isDigit())
      str += readChar();
  }

  //------

  if (isChar('e') || isChar('E')) {
    str += readChar();

    if (isChar('+') || isChar('-'))
      str += readChar();

    if (eof1() || ! isDigit()) {
      unread(str);
      return false;
    }

    while (isDigit())
      str += readChar();
  }

  //------

  if (str.size() == 0)
    return false;

  if (real != NULL)
    *real = CStrUtil::toReal(str);

  //------

  return true;
}

bool
CFileParse::
readString(std::string &str)
{
  str = "";

  if (eof1() || ! isChar('\"'))
    return false;

  str += readChar();

  while (! eof1 ()) {
    if      (isChar('\\')) {
      str += readChar();

      if (! eof1())
        str += readChar();
    }
    else if (isChar('\"'))
      break;
    else
      str += readChar();
  }

  if (eof1()) {
    unread(str);
    return false;
  }

  str += readChar();

  return true;
}

bool
CFileParse::
readToChar(char c, std::string &str)
{
  str = "";

  while (! eof1() && ! isChar(c))
    str += readChar();

  if (eof1()) {
    unread(str);
    return false;
  }

  return true;
}

bool
CFileParse::
isIdentifier()
{
  if (eof1())
    return false;

  if (! isChar('_') && ! isAlpha())
    return false;

  return true;
}

bool
CFileParse::
readIdentifier(std::string &identifier)
{
  if (isChar('_') || isAlpha()) {
    identifier = readChar();

    while (isChar('_') || isAlnum())
      identifier += readChar();

    return true;
  }
  else
    return false;
}

bool
CFileParse::
isSpace()
{
  return (! eof1() && isspace(lookChar()));
}

bool
CFileParse::
isNonSpace()
{
  return (! eof1() && ! isspace(lookChar()));
}

bool
CFileParse::
isAlpha()
{
  return (! eof1() && isalpha(lookChar()));
}

bool
CFileParse::
isAlnum()
{
  return (! eof1() && isalnum(lookChar()));
}

bool
CFileParse::
isDigit()
{
  return (! eof1() && isdigit(lookChar()));
}

bool
CFileParse::
isBaseChar(uint base)
{
  return (! eof1() && CStrUtil::isBaseChar(lookChar(), base, NULL));
}

bool
CFileParse::
isChar(char c)
{
  return (! eof1() && lookChar() == c);
}

bool
CFileParse::
isNextChar(char c)
{
  return (! eof1() && lookNextChar() == c);
}

bool
CFileParse::
isString(const std::string &str)
{
  int pos = 0;
  int len = str.size();

  std::string str1;

  while (! eof1() && pos < len && isChar(str[pos])) {
    str1 += readChar();

    ++pos;
  }

  if (pos != len) {
    if (pos > 0)
      unread(str1);

    return false;
  }

  unread(str1);

  return true;
}

bool
CFileParse::
eof1()
{
  if (stream_)
    return eof();
  else
    return (eol() || eof());
}

bool
CFileParse::
eol()
{
  return buffer_.empty();
}

bool
CFileParse::
eof()
{
  return file_->eof();
}

bool
CFileParse::
skipChar(uint num)
{
  for (uint i = 0; i < num; ++i) {
    uchar c;

    if (! readChar(&c))
      return false;
  }

  return true;
}

char
CFileParse::
readChar()
{
  uchar c;

  readChar(&c);

  return c;
}

bool
CFileParse::
readChar(char *c)
{
  uchar c1;

  if (! readChar(&c1))
    return false;

  *c = c1;

  return true;
}

bool
CFileParse::
readChar(uchar *c)
{
  if (c != NULL)
    *c = '\0';

  if (! buffer_.empty()) {
    if (c != NULL)
      *c = buffer_[0];

    buffer_.pop_front();
  }
  else {
    if (eof1())
      return false;

    uchar c1;

    file_->read(&c1, 1);

    if (c != NULL)
      *c = c1;
  }

  return true;
}

char
CFileParse::
lookChar()
{
  uchar c;

  lookChar(&c);

  return c;
}

char
CFileParse::
lookNextChar()
{
  uchar c;

  lookNextChar(&c);

  return c;
}

bool
CFileParse::
lookChar(uchar *c)
{
  if (c != NULL)
    *c = '\0';

  if (! buffer_.empty()) {
    if (c != NULL)
      *c = buffer_[0];
  }
  else {
    if (eof1())
      return false;

    uchar c1;

    file_->read(&c1, 1);

    if (c != NULL)
      *c = c1;

    buffer_.push_back(c1);
  }

  return true;
}

bool
CFileParse::
lookNextChar(uchar *c)
{
  if (c != NULL)
    *c = '\0';

  if      (buffer_.size() > 1) {
    if (c != NULL)
      *c = buffer_[1];
  }
  else {
    if (eof1())
      return false;

    if (! buffer_.empty()) {
      uchar c1;

      file_->read(&c1, 1);

      if (c != NULL)
        *c = c1;

      buffer_.push_back(c1);
    }
    else {
      uchar c1[2];

      size_t num_read;

      file_->read(c1, 2, &num_read);

      if (num_read == 2) {
        if (c != NULL)
          *c = c1[1];

        buffer_.push_back(c1[0]);
        buffer_.push_back(c1[1]);
      }
      else {
        if (num_read == 1)
           buffer_.push_back(c1[0]);

        return false;
      }
    }
  }

  return true;
}

void
CFileParse::
loadLine()
{
  uchar c;

  buffer_.clear();

  while (! eof()) {
    file_->read(&c, 1);

    if (c == '\n')
      break;

    if (c != '\0')
      buffer_.push_back(c);
  }
}

void
CFileParse::
unread(const std::string &str)
{
  std::string::size_type len = str.size();

  std::string::size_type j = len - 1;

  for (std::string::size_type i = 0; i < len; ++i, --j)
    buffer_.push_front(str[j]);
}

void
CFileParse::
dumpBuffer(std::ostream &os)
{
  os << ">>" << getBuffer() << "<<" << std::endl;
}

std::string
CFileParse::
getBuffer()
{
  std::string str;

  CharBuffer::iterator p1 = buffer_.begin();
  CharBuffer::iterator p2 = buffer_.end  ();

  for ( ; p1 != p2; ++p1)
    str += *p1;

  return str;
}

bool
CFileParse::
rewind()
{
  if (! file_->rewind())
    return false;

  buffer_.clear();

  return true;
}
