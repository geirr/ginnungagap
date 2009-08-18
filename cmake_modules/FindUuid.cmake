FIND_PATH(Uuid_INCLUDE_DIR uuid.h /usr/include/uuid /usr/local/include/uuid)

set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
FIND_LIBRARY(Uuid_LIBRARY NAMES uuid PATH /usr/lib /usr/local/lib) 

IF (Uuid_INCLUDE_DIR AND Uuid_LIBRARY)
   SET(Uuid_FOUND TRUE)
ENDIF (Uuid_INCLUDE_DIR AND Uuid_LIBRARY)


IF (Uuid_FOUND)
   IF (NOT Uuid_FIND_QUIETLY)
      MESSAGE(STATUS "Found uuid library: ${Uuid_LIBRARY}")
   ENDIF (NOT Uuid_FIND_QUIETLY)
ELSE (Uuid_FOUND)
   IF (Uuid_FIND_REQUIRED)
      MESSAGE(FATAL_ERROR "Could not find uuid library")
   ENDIF (Uuid_FIND_REQUIRED)
ENDIF (Uuid_FOUND)

