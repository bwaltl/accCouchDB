PROJECT(WinAccCouchDB)
cmake_minimum_required(VERSION 2.8)

SET( AccCouchDB_SRCS AccCouchDB.cpp)
SET (AccCouchDB_HDRS AccCouchDB.h )
SET (LIB_TYPE SHARED)


SET(EXTCODE_INCLUDE_PATH "C:\\Programme\\National Instruments\\LabVIEW 2011\\cintools")
INCLUDE_DIRECTORIES(${EXTCODE_INCLUDE_PATH})

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "c:\\projects\\libs")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "c:\\projects\\libs")

#--------------------------------Looking for pillowtalk---------------------------------------------

FIND_PATH(PILLOWTALK_INCLUDE_DIR pillowtalk.h)
FIND_LIBRARY(PILLOWTALK_LIBRARY NAMES pillowtalk libpillowtalk PATHS ${CMAKE_LIBRARY_PATH})

MESSAGE("-- Pillowtalk Include Dir:" ${PILLOWTALK_INCLUDE_DIR})
MESSAGE("-- Pillowtalk Library:" ${PILLOWTALK_LIBRARY})

IF (PILLOWTALK_INCLUDE_DIR AND PILLOWTALK_LIBRARY)
  SET(PILLOWTALK_FOUND TRUE)
ELSE (PILLOWTALK_INCLUDE_DIR AND PILLOWTALK_LIBRARY)
  MESSAGE(FATAL_ERROR "Could not find Pillowtalk")
ENDIF (PILLOWTALK_INCLUDE_DIR AND PILLOWTALK_LIBRARY)

INCLUDE_DIRECTORIES(${PILLOWTALK_INCLUDE_DIR})

#--------------------------------Looking for labview library---------------------------------------------

SET(EXTCODE_INCLUDE_PATH "C:\\Programme\\National Instruments\\LabVIEW 2011\\cintools")

FIND_PATH(EXTCODE_INCLUDE_DIR extcode.h)
FIND_LIBRARY(EXTCODE_LIBRARY NAMES labview liblabview PATHS ${CMAKE_LIBRARY_PATH})

MESSAGE("-- Extcode Include Dir:" ${EXTCODE_INCLUDE_DIR})
MESSAGE("-- Extcode Library:" ${EXTCODE_LIBRARY})

IF (EXTCODE_INCLUDE_DIR AND EXTCODE_LIBRARY)
  SET(PILLOWTALK_FOUND TRUE)
ELSE (EXTCODE_INCLUDE_DIR AND EXTCODE_LIBRARY)
  MESSAGE(FATAL_ERROR "Could not find Extcode")
ENDIF (EXTCODE_INCLUDE_DIR AND EXTCODE_LIBRARY)

INCLUDE_DIRECTORIES(${EXTCODE_INCLUDE_DIR})

#-------------------------------------------------------------------------------------------

# Output Paths
SET (output_include /../build)
FILE(MAKE_DIRECTORY ${output_include})

INCLUDE(GenerateExportHeader)

add_library(AccCouchDB ${LIB_TYPE} ${AccCouchDB_SRCS})
GENERATE_EXPORT_HEADER( AccCouchDB 
             BASE_NAME AccCouchDB
             EXPORT_MACRO_NAME AccCouchDB
             EXPORT_FILE_NAME AccCouchDB.h
             STATIC_DEFINE AccCouchDB_BUILT_AS_STATIC
)

target_link_libraries(AccCouchDB
general ${PILLOWTALK_LIBRARY}
general ${EXTCODE_LIBRARY}
) 

#SET_TARGET_PROPERTIES(AccCouchDB PROPERTIES LINKER_LANGUAGE C)
# Installation Step
IF(WIN32)
  INSTALL( TARGETS AccCouchDB LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
ELSE(WIN32)
  INSTALL( TARGETS AccCouchDB LIBRARY DESTINATION lib)
ENDIF(WIN32)
INSTALL( FILES AccCouchDB.h DESTINATION build)

