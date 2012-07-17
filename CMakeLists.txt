cmake_minimum_required (VERSION 2.6)
project (CCV) 

FIND_PACKAGE( OpenCV REQUIRED )

#find_package( Boost 1.42
#    COMPONENTS 
#    system
#    REQUIRED )
#if(Boost_FOUND)
#    link_directories(${BOOST_LIBRARY_DIRS})
#    include_directories(${Boost_INCLUDE_DIRS})
#    add_definitions(${Boost_LIB_DIAGNOSTIC_DEFINITIONS})
#endif()

add_executable (ccv Main.cpp Texture.cpp ImageProcessor.cpp CCV.cpp) 

#TARGET_LINK_LIBRARIES( ccv ${OpenCV_LIBS} ${Boost_LIBS} )
TARGET_LINK_LIBRARIES( ccv ${OpenCV_LIBS} )

