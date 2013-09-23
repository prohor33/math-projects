#sample CMakeLists.txt

cmake_minimum_required(VERSION 2.6)

  PROJECT(fractals)

  FIND_PACKAGE(OpenThreads)
  FIND_PACKAGE(osg)
  FIND_PACKAGE(osgDB)
  FIND_PACKAGE(osgViewer)
  FIND_PACKAGE(osgGA)

  SET(SOURCES
          fractal.cpp
          two_dim_manipulator.cpp
          )

  INCLUDE_DIRECTORIES(${OPENTHREADS_INCLUDE_DIR} ${OSG_INCLUDE_DIR})

  LINK_DIRECTORIES(${OSG_LIB_DIR})

  ADD_EXECUTABLE(myapp ${SOURCES})

  TARGET_LINK_LIBRARIES(myapp ${OSG_LIBRARY} ${OSGUTIL_LIBRARY}
                        ${OSGDB_LIBRARY} ${OSGVIEWER_LIBRARY}
                        ${OPENTHREADS_LIBRARY} ${OSGGA_LIBRARY})
