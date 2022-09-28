#include <CFileParse.h>
#include <CFile.h>
#include <CStrUtil.h>
#include <cassert>

CFileParse::
CFileParse(const std::string &fileName) :
 fileName_(fileName)
{
  file_ = new CFile(fileName_);

  file_->open(CFile::Mode::READ);
}

CFileParse::
CFileParse(CFile *file) :
 file_(file), remove_(false)
{
}

CFileParse::
~CFileParse()
{
  if (remove_)
    delete file_;
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

  if (integer != nullptr)
    *integer = int(CStrUtil::toInteger(str));

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

  if (integer != nullptr)
    *integer = uint(CStrUtil::toInteger(str));

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

  if (integer != nullptr)
    *integer = int(CStrUtil::toBaseInteger(str, base));

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

  if (integer != nullptr)
    *integer = uint(CStrUtil::toBaseInteger(str, base));

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

  if (real != nullptr)
    *real = CStrUtil::toReal(str);

  //------

  return true;
}

bool
CFileParse::
readString(std::string &str, bool stripQuotes)
{
  str = "";

  if (eof1() || (! isChar('\"') && ! isChar('\'')))
    return false;

  char strChar = readChar();

  if (! stripQuotes)
    str += strChar;

  while (! eof1()) {
    if      (isChar('\\')) {
      str += readChar();

      if (! eof1())
        str += readChar();
    }
    else if (isChar(strChar))
      break;
    else
      str += readChar();
  }

  if (eof1()) {
    unread(str);
    return false;
  }

  strChar = readChar();

  if (! stripQuotes)
    str += strChar;

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
isOneOf(const std::string &str)
{
  auto isCharStr = [&](char c) { return (str.find(c) != std::string::npos); };

  return (! eof1() && isCharStr(lookChar()));
}

bool
CFileParse::
isBaseChar(uint base)
{
  return (! eof1() && CStrUtil::isBaseChar(lookChar(), base, nullptr));
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
  uint pos = 0;
  auto len = str.size();

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

  return eol() || eof();
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

std::string
CFileParse::
readChars(int n)
{
  std::string str;

  for (int i = 0; i < n; ++i) {
    char c;

    if (! readChar(&c))
      break;

    str += c;
  }

  return str;
}

char
CFileParse::
readChar()
{
  uchar c;

  (void) readChar(&c);

  return char(c);
}

bool
CFileParse::
readChar(char *c)
{
  uchar c1;

  if (! readChar(&c1))
    return false;

  *c = char(c1);

  return true;
}

bool
CFileParse::
readChar(uchar *c)
{
  if (c != nullptr)
    *c = '\0';

  uchar c1;

  if (! buffer_.empty()) {
    c1 = uchar(buffer_[0]);

    buffer_.pop_front();
  }
  else {
    if (eof1())
      return false;

    if (! readOneChar(c1))
      return false;
  }

  if (c != nullptr)
    *c = c1;

  if (c1 == '\n') {
    ++lineNum_;

    charNum_ = 0;
  }
  else
    ++charNum_;

  return true;
}

char
CFileParse::
lookChar()
{
  uchar c;

  lookChar(&c);

  return char(c);
}

char
CFileParse::
lookNextChar()
{
  uchar c;

  lookNextChar(&c);

  return char(c);
}

bool
CFileParse::
lookChar(uchar *c)
{
  if (c)
    *c = '\0';

  if (! buffer_.empty()) {
    if (c)
      *c = uchar(buffer_[0]);
  }
  else {
    if (eof1())
      return false;

    uchar c1;

    if (! readOneChar(c1))
      return false;

    if (c)
      *c = c1;

    buffer_.push_back(char(c1));
  }

  return true;
}

bool
CFileParse::
lookNextChar(uchar *c)
{
  if (c != nullptr)
    *c = '\0';

  if      (buffer_.size() > 1) {
    if (c != nullptr)
      *c = uchar(buffer_[1]);
  }
  else {
    if (eof1())
      return false;

    if (! buffer_.empty()) {
      uchar c1;

      if (! readOneChar(c1))
        return false;

      if (c != nullptr)
        *c = c1;

      buffer_.push_back(char(c1));
    }
    else {
      uchar c1[2];

      size_t num_read;

      file_->read(c1, 2, &num_read);

      if (num_read == 2) {
        if (c != nullptr)
          *c = c1[1];

        buffer_.push_back(char(c1[0]));
        buffer_.push_back(char(c1[1]));
      }
      else {
        if (num_read == 1)
           buffer_.push_back(char(c1[0]));

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
    if (! readOneChar(c))
      break;

    if (c == '\n')
      break;

    if (c != '\0')
      buffer_.push_back(char(c));
  }
}

void
CFileParse::
unread(const std::string &str)
{
  auto len = str.size();

  std::string::size_type j = len - 1;

  for (std::string::size_type i = 0; i < len; ++i, --j)
    buffer_.push_front(str[j]);
}

bool
CFileParse::
readOneChar(uchar &c)
{
  size_t num_read;

  file_->read(&c, 1, &num_read);

  return (num_read == 1);
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

  lineNum_ = 1;
  charNum_ = 0;

  return true;
}
