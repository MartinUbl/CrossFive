
SET( TARGET_NAME        "server" )
SET( TARGET_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include/${TARGET_NAME}" )
SET( TARGET_SOURCE_DIR  "${PROJECT_SOURCE_DIR}/src/${TARGET_NAME}" )

SET( INCLUDE
     "${TARGET_INCLUDE_DIR}/global.h"
     "${TARGET_INCLUDE_DIR}/gameplay.h"
     "${TARGET_INCLUDE_DIR}/../shared.h"
     "${TARGET_INCLUDE_DIR}/../opcodes.h" )
SET( SOURCE
     "${TARGET_SOURCE_DIR}/gameplay.cpp"
     "${TARGET_SOURCE_DIR}/main.cpp" )

########################
# Setup the executable #
########################
INCLUDE_DIRECTORIES( "${TARGET_INCLUDE_DIR}/.."
                     "${TARGET_INCLUDE_DIR}" )

SOURCE_GROUP( "include" FILES ${INCLUDE} )
SOURCE_GROUP( "src"     FILES ${SOURCE} )

ADD_EXECUTABLE( "${TARGET_NAME}"
                ${INCLUDE} ${SOURCE} )

TARGET_LINK_LIBRARIES( "${TARGET_NAME}" )

SET( ${TARGET_NAME}_INCLUDE_DIR "${TARGET_INCLUDE_DIR}" PARENT_SCOPE )
