cmake_minimum_required (VERSION 2.6)
project (CCV) 

FIND_PACKAGE( OpenCV REQUIRED )

add_executable (ccv Main.cpp) 

TARGET_LINK_LIBRARIES( ccv ${OpenCV_LIBS} )

