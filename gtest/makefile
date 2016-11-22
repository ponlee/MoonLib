#本项目相关变量
ROOT_DIR = ../..

include ${ROOT_DIR}/mk.inc

#编译器
CXX = g++

#目标文件
TARGET = MoonLibTest

#头文件包含目录，一行一个
INC_DIR += -I.
INC_DIR += -I${ROOT_DIR}
INC_DIR += -I${ROOT_DIR}/ext/include
INC_DIR += -I${ROOT_DIR}/ext/include/CppUtil
INC_DIR += -I${ROOT_DIR}/ext/include/mysql
INC_DIR += -I${ROOT_DIR}/ext/include/zookeeper
INC_DIR += -I${ROOT_DIR}/ext/include/zookeeper_generated
INC_DIR += -I${ROOT_DIR}/ext/include/zookeeper_src
INC_DIR += -I${ROOT_DIR}/ext/include/opencv/opencv
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/core/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/hal/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/imgproc/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/photo/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/video/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/features2d/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/flann/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/objdetect/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/calib3d/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/imgcodecs/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/videoio/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/highgui/include
INC_DIR += -I${ROOT_DIR}/ext/include/opencv_modules/ml/include

#静态库文件路径
STATIC_LIBS_DIR = ${ROOT_DIR}/ext/libs
#STATIC_LIBS = $(foreach n,$(STATIC_LIBS_DIR), $(wildcard $(n)/*.a)) 

#其他库文件，一行一个
STATIC_LIBS += ${STATIC_LIBS_DIR}/boost/libboost_filesystem.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/boost/libboost_system.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libCppUtil.a
#STATIC_LIBS += ${STATIC_LIBS_DIR}/libcrypto.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libgtest.a
#STATIC_LIBS += ${STATIC_LIBS_DIR}/libcurl.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libgtest_main.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libmysqlclient.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libpcre.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libprotobuf.a
#STATIC_LIBS += ${STATIC_LIBS_DIR}/libssl.a
STATIC_LIBS += $(STATIC_LIBS_DIR)/libmysqlclient.a
STATIC_LIBS += $(STATIC_LIBS_DIR)/libjsoncpp.a
STATIC_LIBS += $(STATIC_LIBS_DIR)/libmysqlpp.a
ifdef CYGWIN
STATIC_LIBS += $(STATIC_LIBS_DIR)/libiconv.a
else
STATIC_LIBS += $(STATIC_LIBS_DIR)/libzookeeper_mt.a
#STATIC_LIBS += /home/godmoon/study/project/work/narutomb/server/ext/zookeeper_mt/libzookeeper_ns_mt.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libsnappy.a
STATIC_LIBS += ${STATIC_LIBS_DIR}/libleveldb.a 
endif

#动态库文件路径
DYNAMIC_LIBS_DIR = ${ROOT_DIR}/ext/libs

#链接选项
LDFLAGS += -lrt
LDFLAGS += -fPIC
#LDFLAGS += -export-dynamic
LDFLAGS += -lrtmp
LDFLAGS += ${STATIC_LIBS}
ifdef CYGWIN
else
LDFLAGS += ${DYNAMIC_LIBS_DIR}/opencv/libopencv_calib3d.so
LDFLAGS += ${DYNAMIC_LIBS_DIR}/opencv/libopencv_imgproc.so
LDFLAGS += ${DYNAMIC_LIBS_DIR}/opencv/libopencv_highgui.so
LDFLAGS += ${DYNAMIC_LIBS_DIR}/opencv/libopencv_imgcodecs.so
LDFLAGS += ${DYNAMIC_LIBS_DIR}/opencv/libopencv_core.so
#LDFLAGS += -L/usr/lib/mysql/ -L/usr/lib64/mysql/ -lmysqlclient     #动态库
#LDFLAGS += -L${STATIC_LIBS_DIR} -Bstatic -lmysqlclient				#静态库
endif
LDFLAGS += -lz
LDFLAGS += -ldl
LDFLAGS += -lpthread
LDFLAGS += -lcurl
LDFLAGS += -lssl
LDFLAGS += -lcrypto
#LDFLAGS += -fprofile-arcs -ftest-coverage

#编译选项
CXXFLAGS += -g
CXXFLAGS += -MMD
CXXFLAGS += -O2
CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += -fPIC
CXXFLAGS += -std=gnu++11
#CXXFLAGS += -std=c++11
CXXFLAGS += -Wno-deprecated
CXXFLAGS += $(INC_DIR)
CXXFLAGS += -DUSE_CPP_LOG_MACRO
#CXXFLAGS += -fprofile-arcs -ftest-coverage

#自动搜寻，当前项目的目标文件
#SUBDIR可以指定多个目录，指定的目录下的所有cpp文件会加入编译，比如 SUBDIR = . src
PROTO_DIR = ./protocol
SUBDIR = ./src ${PROTO_DIR}
CXX_SOURCES =$(foreach n,$(SUBDIR), $(wildcard $(n)/*.cpp $(n)/*.proto)) 

#其他外部依赖cpp文件在此处加上
CXX_SOURCES +=

#Protocol文件
PROTO_SOURCE = $(foreach n, $(PROTO_DIR), $(wildcard $(n)/*.proto))
PROTO_H = $(patsubst %.proto, %.pb.h, $(PROTO_SOURCE))
PROTO_CC = $(patsubst %.proto, %.pb.cc, $(PROTO_SOURCE))

#生成对应的.o文件
CXX_OBJECTS_CPP = $(patsubst %.cpp, %.o, $(CXX_SOURCES))
CXX_OBJECTS = $(patsubst %.proto, %.pb.o, $(CXX_OBJECTS_CPP))

#添加自定义.o，这些.o不由clean删除，没有则无需填写
EXT_OBJECTS +=

#生成依赖关系
DEP_FILES = $(patsubst %.o, %.d, ${CXX_OBJECTS})

.PHONY:all 
all: ${TARGET}

$(TARGET): ${CXX_OBJECTS} ${EXT_OBJECTS} ${STATIC_LIBS}
	$(CXX) -o $@ $^ $(LDFLAGS)

#Protocol Buffer相关文件处理
%.pb.h: %.proto
	protoc $^ --cpp_out=.

%.pb.cc: %.proto
	protoc $^ --cpp_out=.

%.d: %.cc
	@set -e;${CXX} -MM $< $(CXXFLAGS) -MF $(patsubst %.cc, %.d, $<)

#cpp相关文件处理，此处会自动匹配到cc，c文件，隐含规则
%.o: %.cpp
	${CXX} -c $(CXXFLAGS) -o $@ $<

#为什么这一个依赖没有也可以？答：   它会自动匹配到.o的部分？貌似是这样的，使用%.c也可以匹配，并且会自动删除中间文件
#%.o: %.cc
#	${CXX} -c $(CXXFLAGS) -o $@ $<

%.d: %.cpp
	@set -e;${CXX} -MM $< $(CXXFLAGS) -MF $(patsubst %.cpp, %.d, $<)

-include ${DEP_FILES}

test:
	@echo "CXX_OBJECTS:"${CXX_OBJECTS}
	@echo "STATIC_LIBS:"${STATIC_LIBS}
	@echo "PROTO_CC:"${PROTO_CC}
	@echo "PROTO_H:"${PROTO_H}
	@echo "DEP_FILES:"${DEP_FILES}

.PHONY:clean 
clean:
	rm -rf ${TARGET} ${CXX_OBJECTS} ${DEP_FILES} ${PROTO_H} ${PROTO_CC} gcov

run:
	make
	./$(TARGET)
