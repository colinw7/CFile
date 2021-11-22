#ifndef CFileParse_H
#define CFileParse_H

#include <deque>
#include <string>
#include <iostream>
#include <sys/types.h>

typedef unsigned char uchar;

class CFile;

/*!
 * Parser for file
 */
class CFileParse {
 public:
  CFileParse(const std::string &fileName);
  CFileParse(CFile *file);

 ~CFileParse();

  //! get/set is stream (stream is file based, non-stream is line based)
  void setStream(bool stream) { stream_ = stream; }
  bool getStream() const { return stream_; }

  //! get/set filename
  const std::string &fileName() const { return fileName_; }
  void setFileName(const std::string &v) { fileName_ = v; }

  //! get/set current line number
  int lineNum() const { return lineNum_; }
  void setLineNum(int i) { lineNum_ = i; }

  //! get/set current char number
  int charNum() const { return charNum_; }
  void setCharNum(int i) { charNum_ = i; }

  //---

  //! skip space/non-space (for space separated words)
  bool skipSpace();
  bool skipNonSpace();

  //! skip to end
  bool skipToEnd();

  //! read space/non-space text (for space separated words)
  bool readSpace(std::string &text);
  bool readNonSpace(std::string &text);

  //! skip one or more characters
  bool skipChar(uint num=1);

  //! read number, real, string
  bool readInteger(int *integer);
  bool readInteger(uint *integer);

  bool readBaseInteger(uint base, int *integer);
  bool readBaseInteger(uint base, uint *integer);

  bool readReal(double *real);

  bool readString(std::string &str, bool stripQuotes=false);

  //! read to specified character (return if found)
  bool readToChar(char c, std::string &text);

  //! check and read identifier (underscore/alpha + underscore/alpnum)
  bool isIdentifier();
  bool readIdentifier(std::string &identifier);

  //! check for space/non-space
  bool isSpace();
  bool isNonSpace();

  //! check for character types
  bool isAlpha();
  bool isAlnum();
  bool isDigit();
  bool isBaseChar(uint base);

  //! check for character/string
  bool isOneOf(const std::string &str);
  bool isChar(char c);
  bool isNextChar(char c);
  bool isString(const std::string &str);

  //! check eol (non-stream) or eof (stream)
  bool eol();
  bool eof();

  //! read character(s)
  std::string readChars(int n);

  char readChar();
  bool readChar(char *c);
  bool readChar(uchar *c);

  //! check character (not consumed)
  char lookChar();
  bool lookChar(uchar *c);

  //! check next character (not consumed)
  char lookNextChar();
  bool lookNextChar(uchar *c);

  //! read a line into buffer (non-stream)
  void loadLine();

  //! unread string
  void unread(const std::string &str);

  //! print buffer
  void dumpBuffer(std::ostream &os=std::cout);

  //! get buffer
  std::string getBuffer();

  //! rewind to start
  bool rewind();

 private:
  //! check eol/eof based on stream type
  bool eof1();

  bool readOneChar(uchar &c);

 private:
  typedef std::deque<char> CharBuffer;

  CFile      *file_    { nullptr }; //!< file pointer
  CharBuffer  buffer_;              //!< buffer
  bool        stream_  { false };   //!< is stream based (not line based)
  bool        remove_  { true };    //!< is file created by class (deleted by destructor)
  std::string fileName_;            //!< file name (if known)
  int         lineNum_ { 1 };       //!< current line number
  int         charNum_ { 0 };       //!< current char number
};

#endif
