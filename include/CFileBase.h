#ifndef CFileBase_H
#define CFileBase_H

#include <sys/stat.h>
#include <sys/types.h>

#include <vector>
#include <string>
#include <list>

#include <CFileType.h>

typedef unsigned char uchar;

typedef bool CFileModeProc(int mode);

class CFileBase;

class CFileData {
 public:
  explicit CFileData(size_t size) :
   data_(nullptr), size_(0) {
    data_ = new uchar [size];
    size_ = size;
  }

 ~CFileData() {
    delete [] data_;
  }

  uchar  *getData() const { return data_; }
  uchar   getDataPos(int pos) const { return data_[pos]; }
  size_t  getSize() const { return size_; }

 private:
  CFileData(const CFileData &data);
  CFileData &operator=(const CFileData &data);

 private:
  uchar  *data_ { nullptr };
  size_t  size_ { 0 };
};

//------

class CFileLines {
 public:
  typedef std::vector<std::string>::iterator iterator;
  typedef std::vector<std::string>::const_iterator const_iterator;

  explicit CFileLines(CFileBase *file) : file_(file), lines_() { }

 ~CFileLines() { }

  int size() const {
    return lines_.size();
  }

  void addLine(const std::string &line);

  std::string getLine(int pos) const;

  iterator begin() {
    return lines_.begin();
  }

  const_iterator begin() const {
    return lines_.begin();
  }

  iterator end() {
    return lines_.end();
  }

  const_iterator end() const {
    return lines_.end();
  }

  std::string operator[](int pos) const {
    return getLine(pos);
  }

 private:
  CFileLines(const CFileLines &data);
  CFileLines &operator=(const CFileLines &data);

 private:
  typedef std::vector<std::string> Lines;

  CFileBase* file_ { nullptr };
  Lines      lines_;
};

//------

class CFileBase {
 public:
  enum class Mode {
    NONE       = 0,
    READ       = (1<<0),
    WRITE      = (1<<1),
    APPEND     = READ | WRITE,
    READ_WRITE = READ | WRITE
  };

 public:
  static std::string toString(const Mode &mode) {
    if (mode == Mode::READ      ) return "r";
    if (mode == Mode::WRITE     ) return "w";
    if (mode == Mode::READ_WRITE) return "rw";
    if (mode == Mode::APPEND    ) return "a";

    return "?";
  }

 public:
  CFileBase();
  explicit CFileBase(const std::string &filename);
  explicit CFileBase(const char *filename);

  virtual ~CFileBase();

  bool open(CFileBase::Mode mode);
  bool close();

  CFileBase::Mode getOpenMode() const { return mode_; }

         CFileLines *toLines();
  static CFileLines *toLines(const std::string &filename);
         bool        toLines(std::vector<std::string> &lines);
         bool        toLines(std::list<std::string> &lines);
  static bool        toLines(const std::string &filename, std::vector<std::string> &lines);
  static bool        toLines(const std::string &filename, std::list<std::string> &lines);

  bool readLine(std::string &line);

  std::string toString();

  bool readAll(uchar **data, size_t *len);

  CFileData *readAll();

  void printf(const char *format, ...);

  bool writeAll(const CFileData *file_data);

  bool read(CFileData *file_data);
  bool read(uchar *data, size_t size);

  bool read(uchar *data, size_t size, size_t *actual_size);

  template<typename T>
  bool readType(T &t) {
    return read((uchar *) &t, sizeof(T));
  }

  bool writef(const char *format, ...);

  bool write(const CFileData *file_data);
  bool write(const std::string &str);
  bool write(const char *str);
  bool write(const uchar *data, size_t size);

  bool write(const char *data, size_t size);

  bool write(uchar c);
  bool write(char c);

  int  getC();
  bool ungetC(int c);
  bool putC(int c);

  long getPos();
  bool setPos(long pos);
  bool setRPos(long pos);

  bool setPosStart();
  bool setPosEnd();

  bool rewind();

  bool eof();

  int error();

  bool flush();

         bool exists() const;
  static bool exists(const std::string &filename);

  uint getIDev ();
  uint getINode();

         CFileType getType();
  static CFileType getType(uint mode);
  static CFileType getType(const std::string &filename);

         char getTypeChar();
  static char getTypeChar(uint mode);
  static char getTypeChar(const std::string &filename);

         bool isDirectory() const;
  static bool isDirectory(const std::string &filename);
         bool isRegular() const;
  static bool isRegular(const std::string &filename);
         bool isChar() const;
  static bool isChar(const std::string &filename);
         bool isBlock() const;
  static bool isBlock(const std::string &filename);
         bool isFIFO() const;
  static bool isFIFO(const std::string &filename);
         bool isLink() const;
  static bool isLink(const std::string &filename);
         bool isSocket() const;
  static bool isSocket(const std::string &filename);
         bool isHidden() const;
  static bool isHidden(const std::string &filename);

         bool isReadable() const;
  static bool isReadable(const std::string &filename);
         bool isWritable() const;
  static bool isWritable(const std::string &filename);
         bool isExecutable() const;
  static bool isExecutable(const std::string &filename);

         bool isOwner() const;
  static bool isOwner(const std::string &filename);

         size_t getSize();
  static size_t getSize(const std::string &filename);

  // <path> = <device>:<dir>/<name>
  // <name> = <base>.<suffix>

  const std::string &getPath() const;
  const std::string &getDevice() const;
  const std::string &getDir() const;
  const std::string &getName() const;
  const std::string &getBase() const;

         const std::string &getSuffix() const;
  static       std::string  getSuffix(const std::string &filename);

  static std::string getTail(const std::string &filename);

         int getMode() const;
  static int getMode(const std::string &filename);

  int getUMode() const;
  int getGMode() const;
  int getOMode() const;

         int getUID() const;
  static int getUID(const std::string &filename);
         int getGID() const;
  static int getGID(const std::string &filename);

         int getMTime() const;
  static int getMTime(const std::string &filename);
         int getCTime() const;
  static int getCTime(const std::string &filename);
         int getATime() const;
  static int getATime(const std::string &filename);

  bool getLinkName(std::string &linkname) const;

  bool isStdIn() const;
  bool isStdOut() const;
  bool isStdErr() const;

  static bool expandTilde(const std::string &str, std::string &str1);
  static bool addTilde(const std::string &str, std::string &str1);

  bool remove();

  static bool remove(const std::string &str);

  bool copy(CFileBase &file);

  std::string getModeString();
  std::string getUModeString();
  std::string getGModeString();
  std::string getOModeString();

  static bool getStat(const std::string &filename, struct stat *file_stat);
  static bool getFStat(const std::string &filename, struct stat *file_stat);
  static bool getLStat(const std::string &filename, struct stat *file_stat);

  void skipSpace();

  static bool getUseLStat() { return use_lstat_; }

  static bool setUseLStat(bool lstat) {
    std::swap(use_lstat_, lstat); return lstat;
  }

 protected:
  virtual bool open_(const std::string &mode) = 0;
  virtual bool close_() = 0;

  virtual bool read_(uchar *data, size_t size, size_t *actual_size) = 0;

  virtual bool write_(const uchar *data, size_t size) = 0;

  virtual bool getc_(int &c) = 0;
  virtual bool ungetc_(int c) = 0;
  virtual bool putc_(int c) = 0;

  virtual bool seek_(long pos) = 0;
  virtual bool rseek_(long pos) = 0;
  virtual bool tell_(long &pos) = 0;

  virtual bool rewind_() = 0;

  virtual bool eof_(bool &eof) = 0;

  virtual bool error_(int &error) = 0;

  virtual bool flush_() = 0;

  virtual bool stat_ (struct stat *file_stat) = 0;
  virtual bool lstat_(struct stat *file_stat) = 0;

  virtual bool readlink_(std::string &linkname) const = 0;

 private:
  void init(const std::string &filename);

  bool openCheck(CFileBase::Mode mode);
  bool openCheck(CFileBase::Mode mode, bool &opened);

  bool isType(CFileModeProc proc) const;

  static bool isType(CFileModeProc proc, const std::string &filename);

  static std::string getModeString1(int mode, int type);

  static bool isDirMode(int mode);
  static bool isRegMode(int mode);
  static bool isCharMode(int mode);
  static bool isBlockMode(int mode);
  static bool isFIFOMode(int mode);
  static bool isLinkMode(int mode);
  static bool isSocketMode(int mode);

  bool copyBytes(CFileBase &file);

  bool getStat() const;
  bool getFStat() const;
  bool getLStat() const;

 protected:
  static bool        use_lstat_;
  static std::string last_error_;

  std::string device_;
  std::string dir_;
  std::string name_;
  std::string base_;
  std::string suffix_;
  std::string path_;
  bool        opened_ { false };
  Mode        mode_ { Mode::NONE };
  bool        owner_ { false };
  struct stat file_stat_;
};

#endif
