cmake_minimum_required(VERSION 2.8)
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
AUX_SOURCE_DIRECTORY(. DIR_SERVER_SRC)
ADD_LIBRARY(net ${DIR_SERVER_SRC})