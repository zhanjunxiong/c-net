PROJECT(test)
MESSAGE(STATUS "This is BINARY dir " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "This is SOURCE dir " ${PROJECT_SOURCE_DIR})
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
set(CMAKE_C_FLAGS
 -g
 # -DVALGRIND
 # -DMUDUO_STD_STRING
 # -Wall
 # -Wextra
 # -m32
 #-Werror
 #-Wconversion
 #-Wno-unused-parameter
 #-Wold-style-cast
 #-Woverloaded-virtual
 #-Wpointer-arith
 #-Wshadow
 #-Wwrite-strings
 #-march=native
 # -MMD
 # -std=c++0x
 #-rdynamic
 )
set(CMAKE_C_FLAGS_DEBUG -g -rdynamic)
INCLUDE_DIRECTORIES(/home/zjx/public/uiyiu_v0.4/src/net)
LINK_DIRECTORIES(/home/zjx/public/uiyiu_v0.4/src/net/build)
MESSAGE(STATUS "This is SOURCE dir " /home/zjx/public/uiyiu_v0.4/src/net)
AUX_SOURCE_DIRECTORY(. DIR_SRC)
AUX_SOURCE_DIRECTORY(net DIR_SRC)
ADD_EXECUTABLE(test ${DIR_SRC})
TARGET_LINK_LIBRARIES(test net.a)
TARGET_LINK_LIBRARIES(test -lpthread)
