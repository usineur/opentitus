cmake_minimum_required(VERSION 2.8)

# VitaSDK defines
if( NOT DEFINED CMAKE_TOOLCHAIN_FILE )
  if( DEFINED ENV{VITASDK} )
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    unset(TITUS CACHE)
    unset(MOKTAR CACHE)
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

# Game selection
if ((NOT DEFINED TITUS AND NOT DEFINED MOKTAR) OR (DEFINED TITUS AND DEFINED MOKTAR))
  unset(TITUS CACHE)
  unset(MOKTAR CACHE)
  message(FATAL_ERROR "Use cmake with -DTITUS=1 or -DMOKTAR=1 (exclusive).")
endif()

# Project start
set(VITA_APPNAME OpenTitus)
set(VITA_TITLEID "OPENTITUS")
set(VITA_VERSION "01.00")

project(${VITA_APPNAME})
include("${VITASDK}/share/vita.cmake" REQUIRED)

# Flags and includes
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -D__PSP2__ -fno-exceptions")

include_directories(
  include
  opl
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/opl
)

# Builds
file(GLOB_RECURSE SRCS src/*.c)
add_executable(${VITA_APPNAME}
  ${SRCS}
  opentitus.c
)

target_link_libraries(${VITA_APPNAME}
  SDL_mixer
  SDL
  vorbisfile
  vorbis
  ogg
  opl_vita
  mikmod
  pthread
  kbdvita
  vita2d
  z
  m
  SceAppUtil_stub
  SceAudio_stub
  SceCommonDialog_stub
  SceCtrl_stub
  SceDisplay_stub
  SceGxm_stub
  SceSysmodule_stub
)

# Create Vita artifacts
vita_create_self(eboot.bin ${VITA_APPNAME} UNSAFE)
if (TITUS)
  vita_create_vpk(Titus.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APPNAME}
    FILE vita/icon0.png sce_sys/icon0.png
    FILE vita/bg_titus.png sce_sys/livearea/contents/bg.png
    FILE vita/startup_titus.png sce_sys/livearea/contents/startup.png
    FILE vita/template.xml sce_sys/livearea/contents/template.xml
    FILE vita/vita_titus.conf titus_vita.conf
  )
elseif (MOKTAR)
  vita_create_vpk(Moktar.vpk ${VITA_TITLEID} eboot.bin
    VERSION ${VITA_VERSION}
    NAME ${VITA_APPNAME}
    FILE vita/icon0.png sce_sys/icon0.png
    FILE vita/bg_moktar.png sce_sys/livearea/contents/bg.png
    FILE vita/startup_moktar.png sce_sys/livearea/contents/startup.png
    FILE vita/template.xml sce_sys/livearea/contents/template.xml
    FILE vita/vita_moktar.conf titus_vita.conf
  )
endif()

unset(TITUS CACHE)
unset(MOKTAR CACHE)
