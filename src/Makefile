CC = g++
AR = ar
RM = rm

CDEBUG = -g
LDEBUG = -g

INC_DIR = ../include
OBJ_DIR = ../obj
LIB_DIR = ../lib

all: $(LIB_DIR)/libCFile.a

SRC = \
CDir.cpp \
CDirFTW.cpp \
CFileBase.cpp \
CFile.cpp \
CFileDevice.cpp \
CTempFile.cpp \
COStreamFile.cpp \
CFileParse.cpp \
CFileMatch.cpp \

OBJS = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(SRC))

CPPFLAGS = \
-std=c++14 \
-I$(INC_DIR) \
-I../../COS/include \
-I../../CGlob/include \
-I../../CStrUtil/include \
-I.

clean:
	$(RM) -f $(OBJ_DIR)/*.o
	$(RM) -f $(LIB_DIR)/libCFile.a

$(OBJS): $(OBJ_DIR)/%.o: %.cpp
	$(CC) -c $< -o $(OBJ_DIR)/$*.o $(CPPFLAGS)

.SUFFIXES: .cpp

$(LIB_DIR)/libCFile.a: $(OBJS)
	$(AR) crv $(LIB_DIR)/libCFile.a $(OBJS)
