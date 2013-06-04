#ifndef CFILE_PARSE_H
#define CFILE_PARSE_H

#include <deque>
#include <string>
#include <iostream>
#include <sys/types.h>

typedef unsigned char uchar;

class CFile;

class CFileParse {
 public:
  CFileParse(CFile *file);

  void setStream(bool stream) { stream_ = stream; }
  bool getStream() const { return stream_; }

  bool skipSpace();
  bool skipNonSpace();
  bool skipToEnd();

  bool readSpace(std::string &text);
  bool readNonSpace(std::string &text);

  bool skipChar(uint num=1);

  bool readInteger(int *integer);
  bool readInteger(uint *integer);
  bool readBaseInteger(uint base, int *integer);
  bool readBaseInteger(uint base, uint *integer);
  bool readReal(double *real);
  bool readString(std::string &str);

  bool readToChar(char c, std::string &text);

  bool isIdentifier();
  bool readIdentifier(std::string &identifier);

  bool isSpace();
  bool isNonSpace();
  bool isAlpha();
  bool isAlnum();
  bool isDigit();
  bool isBaseChar(uint base);

  bool isChar(char c);
  bool isNextChar(char c);
  bool isString(const std::string &str);

  bool eol();
  bool eof();

  char readChar();
  bool readChar(char *c);
  bool readChar(uchar *c);

  char lookChar();
  bool lookChar(uchar *c);

  char lookNextChar();
  bool lookNextChar(uchar *c);

  void loadLine();

  void unread(const std::string &str);

  void dumpBuffer(std::ostream &os=std::cout);

  std::string getBuffer();

  bool rewind();

 private:
  bool eof1();

 private:
  typedef std::deque<char> CharBuffer;

  CFile      *file_;
  CharBuffer  buffer_;
  bool        stream_;
};

#endif
