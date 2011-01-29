CMAKE_MINIMUM_REQUIRED( VERSION 2.6 FATAL_ERROR )

PROJECT( "crossfive" )

SET( CMAKE_MODULE_PATH
     "${PROJECT_SOURCE_DIR}/cmake"
     "${CMAKE_MODULE_PATH}" )

Find_Package ( SDL REQUIRED )
Find_Package ( SDL_image REQUIRED )
Find_Package ( SDL_net REQUIRED )

ADD_SUBDIRECTORY( "src/server" )
