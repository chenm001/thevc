# the SOURCE definiton lets you move your makefile to another position
CONFIG 				= CONSOLE

# set directories to your wanted values
SRC_DIR				= ../../../../source/App/TAppDecoder
INC_DIR				= ../../../../source/App/TAppDecoder
LIB_DIR				= ../../../../lib
BIN_DIR				= ../../../../bin

SRC_DIR1		=
SRC_DIR2		=
SRC_DIR3		=
SRC_DIR4		=

USER_INC_DIRS	= -I$(SRC_DIR) 
USER_LIB_DIRS	=

# intermediate directory for object files
OBJ_DIR				= ./objects

# set executable name
PRJ_NAME			= TAppDecoder

# defines to set
DEFS				= -DMSYS_LINUX -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -DMSYS_UNIX_LARGEFILE

# set objects
OBJS          		= 	\
					$(OBJ_DIR)/decmain.o \
					$(OBJ_DIR)/TAppDecCfg.o \
					$(OBJ_DIR)/TAppDecOption.o \
					$(OBJ_DIR)/TAppDecTop.o \

# set libs to link with
LIBS				= -ldl

DEBUG_LIBS			=
RELEASE_LIBS		=

STAT_LIBS			= -lpthread
DYN_LIBS			=


DYN_DEBUG_LIBS		= -lTLibDecoderd -lTLibCommond -lTLibVideoIOd
DYN_DEBUG_PREREQS		= $(LIB_DIR)/libTLibDecoderd.a $(LIB_DIR)/libTLibCommond.a $(LIB_DIR)/libTLibVideoIOd.a
STAT_DEBUG_LIBS		= -lTLibDecoderStaticd -lTLibCommonStaticd -lTLibVideoIOStaticd
STAT_DEBUG_PREREQS		= $(LIB_DIR)/libTLibDecoderStaticd.a $(LIB_DIR)/libTLibCommonStaticd.a $(LIB_DIR)/libTLibVideoIOStaticd.a

DYN_RELEASE_LIBS	= -lTLibDecoder -lTLibCommon -lTLibVideoIO
DYN_RELEASE_PREREQS	= $(LIB_DIR)/libTLibDecoder.a $(LIB_DIR)/libTLibCommon.a $(LIB_DIR)/libTLibVideoIO.a
STAT_RELEASE_LIBS	= -lTLibDecoderStatic -lTLibCommonStatic -lTLibVideoIOStatic
STAT_RELEASE_PREREQS	= $(LIB_DIR)/libTLibDecoderStatic.a $(LIB_DIR)/libTLibCommonStatic.a $(LIB_DIR)/libTLibVideoIOStatic.a


# name of the base makefile
MAKE_FILE_NAME		= ../../common/makefile.base

# include the base makefile
include $(MAKE_FILE_NAME)
