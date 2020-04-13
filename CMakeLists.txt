cmake_minimum_required (VERSION 2.6)
project (LayoutTest)
set (CMAKE_CXX_STANDARD 17)
add_executable(LayoutTest test/CppKatLayoutTest.cpp test/TestWidget.h src/Message.h)