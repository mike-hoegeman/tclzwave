#
# Makefile for tclzwave (owzsh) 
#

# GNU make only

.SUFFIXES:	.c .cpp .o .a

CC     := gcc
CXX    := g++
LD     := g++
AR     := ar rc
RANLIB := ranlib

OZW := ../../open-zwave-read-only

DEBUG_CFLAGS    := -Wall -Wno-format -g -DDEBUG
RELEASE_CFLAGS  := -Wall -Wno-unknown-pragmas -Wno-format -O3

DEBUG_LDFLAGS	:= -g


# cribbed fron output of tcl core make for osx. probabaly way more 
# explicit than is necessary but there you have it.
TCL_CFLAGS := \
 -g -pipe -fvisibility=hidden  -Wall -fno-common -DBUILD_tcl \
 -I"." -I/Users/michaelhoegeman/tcl/unix \
 -I/Users/michaelhoegeman/tcl/generic \
 -I/Users/michaelhoegeman/tcl/libtommath -DPACKAGE_NAME=\"tcl\" \
 -DPACKAGE_TARNAME=\"tcl\" \
 -DPACKAGE_VERSION=\"8.6\" \
 -DPACKAGE_STRING=\"tcl\8.6\" \
 -DPACKAGE_BUGREPORT=\"\" -DSTDC_HEADERS=1 -DHAVE_SYS_TYPES_H=1 \
 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 \
 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 \
 -DNO_VALUES_H=1 -DHAVE_LIMITS_H=1 -DHAVE_SYS_PARAM_H=1 -DUSE_THREAD_ALLOC=1 \
 -D_REENTRANT=1 -D_THREAD_SAFE=1 -DHAVE_PTHREAD_ATTR_SETSTACKSIZE=1 \
 -DTCL_THREADS=1 -DTCL_CFGVAL_ENCODING=\"iso8859-1\" -DHAVE_ZLIB=1 \
 -DMODULE_SCOPE=extern -DMAC_OSX_TCL=1 -DHAVE_COREFOUNDATION=1 \
 -DHAVE_CAST_TO_UNION=1 -DTCL_SHLIB_EXT=\".dylib\" -DTCL_TOMMATH=1 \
 -DMP_PREC=4 -DTCL_WIDE_INT_IS_LONG=1 -DHAVE_GETCWD=1 -DHAVE_MKSTEMP=1 \
 -DHAVE_OPENDIR=1 -DHAVE_STRTOL=1 -DHAVE_WAITPID=1 -DHAVE_GETNAMEINFO=1 \
 -DHAVE_GETADDRINFO=1 -DHAVE_FREEADDRINFO=1 -DHAVE_GAI_STRERROR=1 \
 -DHAVE_STRUCT_ADDRINFO=1 -DHAVE_STRUCT_IN6_ADDR=1 \
 -DHAVE_STRUCT_SOCKADDR_IN6=1 -DHAVE_STRUCT_SOCKADDR_STORAGE=1 \
 -DHAVE_GETPWUID_R_5=1 -DHAVE_GETPWUID_R=1 -DHAVE_GETPWNAM_R_5=1 \
 -DHAVE_GETPWNAM_R=1 -DHAVE_GETGRGID_R_5=1 -DHAVE_GETGRGID_R=1 \
 -DHAVE_GETGRNAM_R_5=1 -DHAVE_GETGRNAM_R=1 -DHAVE_MTSAFE_GETHOSTBYNAME=1 \
 -DHAVE_MTSAFE_GETHOSTBYADDR=1 -DUSE_TERMIOS=1 -DHAVE_SYS_TIME_H=1 \
 -DTIME_WITH_SYS_TIME=1 -DHAVE_GMTIME_R=1 -DHAVE_LOCALTIME_R=1 \
 -DHAVE_MKTIME=1 -DHAVE_TM_GMTOFF=1 -DHAVE_TIMEZONE_VAR=1 \
 -DHAVE_STRUCT_STAT_ST_BLOCKS=1 -DHAVE_STRUCT_STAT_ST_BLKSIZE=1 \
 -DHAVE_BLKCNT_T=1 -DHAVE_INTPTR_T=1 -DHAVE_UINTPTR_T=1 -DHAVE_SIGNED_CHAR=1 \
 -DHAVE_LANGINFO=1 -DHAVE_CHFLAGS=1 -DHAVE_MKSTEMPS=1 -DHAVE_GETATTRLIST=1 \
 -DHAVE_COPYFILE_H=1 -DHAVE_COPYFILE=1 -DHAVE_LIBKERN_OSATOMIC_H=1 \
 -DHAVE_OSSPINLOCKLOCK=1 -DHAVE_PTHREAD_ATFORK=1 -DUSE_VFORK=1 \
 -DTCL_DEFAULT_ENCODING=\"utf-8\" -DTCL_LOAD_FROM_MEMORY=1 \
 -DTCL_WIDE_CLICKS=1 -DHAVE_AVAILABILITYMACROS_H=1 -DHAVE_WEAK_IMPORT=1 \
 -D_DARWIN_C_SOURCE=1 -DHAVE_FTS=1 -DHAVE_SYS_IOCTL_H=1 -DHAVE_SYS_FILIO_H=1 \
 -DTCL_UNLOAD_DLLS=1 -DUSE_DTRACE=1 -DHAVE_CPUID=1 -DTCL_FRAMEWORK=1 \
 -DTCL_FRAMEWORK_VERSION=\"8.6\"  -mdynamic-no-pic

# /Users/michaelhoegeman/tcl/unix/tclAppInit.c
 
TCL_LDFLAGS :=  \
 -g -pipe -fvisibility=hidden      \
 -prebind -headerpad_max_install_names \
 -Wl,-search_paths_first \
 -F/Users/michaelhoegeman/build/tcl/Development \
 -framework Tcl -lz  -lpthread -framework CoreFoundation \
 -sectcreate __TEXT __info_plist Ozwsh-Info.plist

# Change for DEBUG or RELEASE
CFLAGS	:= -c -DDARWIN $(DEBUG_CFLAGS) $(TCL_CFLAGS)
LDFLAGS	:= $(DEBUG_LDFLAGS) $(TCL_LDFLAGS)

INCLUDES := \
 -I $(OZW)/cpp/src \
 -I $(OZW)/cpp/src/command_classes/ \
 -I $(OZW)/cpp/src/value_classes/ \
 -I $(OZW)/cpp/src/platform/ \
 -I $(OZW)/cpp/h/platform/unix \
 -I $(OZW)/cpp/tinyxml/ \
 -I $(OZW)/cpp/hidapi/hidapi/

LIBS = $(OZW)/cpp/lib/mac/libopenzwave.a $(TCL_LIBS)

%.o : %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

%.o : %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $<

all: ozwsh

lib:
	$(MAKE) -C $(OZW)/cpp/build/mac

$(OBJS): ozw.h

OBJS = ozwsh.o init.o options.o
ozwsh:	$(OBJS) lib
	$(LD) $(LDFLAGS) $(OBJS) $(LIBS) \
          -framework IOKit -framework CoreFoundation -o ozwsh

clean:
	rm -f ozwsh $(OBJS) 

XMLLINT := $(shell whereis xmllint)

ifeq ($(XMLLINT),)
xmltest:	$(XMLLINT)
	$(error xmllint command not found.)
else
xmltest:	$(XMLLINT)
	@$(XMLLINT) --noout --schema ../../../../config/zwcfg.xsd zwcfg_*.xml
	@$(XMLLINT) --noout --schema ../../../../config/zwscene.xsd zwscene.xml
endif
