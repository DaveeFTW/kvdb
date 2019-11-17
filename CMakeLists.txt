cmake_minimum_required(VERSION 3.3)

set(CMAKE_CXX_COMPILER_FORCED True)

project(kvdb CXX ASM)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99 -fno-strict-aliasing")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-q -Wall -flto -O3 -std=c++11 -static -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit -nostdlib")

add_executable(kvdb
  src/main.cpp
  src/exception.cpp
  src/exception_asm.S
  src/debugger.cpp
  src/serial.cpp
  src/rsp.cpp
  src/hex.cpp
  src/uart.cpp
  src/pipe.cpp
  src/launch.cpp
  src/cache.cpp
  src/services.cpp
  src/eventflag.cpp
  src/pipecache.cpp
  src/commands/qsupported.cpp
  src/commands/haltreason.cpp
  src/commands/readregister.cpp
  src/commands/readregisters.cpp
  src/commands/readmemory.cpp
  src/commands/writememory.cpp
  src/commands/setthread.cpp
  src/commands/getthread.cpp
  src/commands/resume.cpp
  src/commands/kill.cpp
  src/commands/qxfer.cpp
  src/commands/qoffsets.cpp
  src/commands/qsymbol.cpp
  src/commands/qattached.cpp
  src/commands/threadinfo.cpp
  src/commands/step.cpp
)

target_link_libraries(kvdb
  c
  gcc
  SceSysmemForDriver_stub
  SceSysmemForKernel_stub
  SceAppMgrForDriver_stub
  SceExcpmgrForKernel_stub
  SceThreadmgrForKernel_stub
  SceUartForKernel_stub
  SceThreadmgrForDriver_stub
  SceProcessmgrForDriver_stub
  SceProcessmgrForKernel_stub
  SceModulemgrForKernel_stub
  ScePervasiveForDriver_stub
  SceCpuForDriver_stub
  SceCpuForKernel_stub
  SceIofilemgrForDriver_stub
  SceSysrootForDriver_stub
  SceDebugForDriver_stub
  SceDebugForKernel_stub
)

target_include_directories(kvdb PRIVATE src)

add_custom_target(exports SOURCES exports.yml)
vita_create_self(kvdb.skprx kvdb CONFIG exports.yml UNSAFE)

set(KERNEL_STUBS
  imports/kvdb_vdb_kernel_vdb_serial_uart.S
  imports/kvdb_vdb_kernel_vdb_serial_pipe.S
)

set(USER_STUBS
  imports/kvdb_vdb_user_vdb_serial_uart.S
  imports/kvdb_vdb_user_vdb_serial_pipe.S
  imports/kvdb_vdb_user_vdb_send_serial_pipe.S
  imports/kvdb_vdb_user_vdb_recv_serial_pipe.S
  imports/kvdb_vdb_user_vdb_launch_debug.S
)

add_custom_target(imports ALL
  DEPENDS exports.yml
  DEPENDS kvdb.skprx
  COMMAND ${VITASDK}/bin/vita-elf-export k kvdb.velf ${CMAKE_CURRENT_SOURCE_DIR}/exports.yml imports.yml
  COMMAND ${VITASDK}/bin/vita-libs-gen imports.yml imports
  BYPRODUCTS ${KERNEL_STUBS} ${USER_STUBS}
)

add_library(vdb_kernel_stub STATIC ${KERNEL_STUBS})
target_compile_definitions(vdb_kernel_stub PRIVATE -DGEN_WEAK_EXPORTS=0)

add_library(vdb_user_stub STATIC ${USER_STUBS})
target_compile_definitions(vdb_user_stub PRIVATE -DGEN_WEAK_EXPORTS=0)

install (TARGETS vdb_kernel_stub vdb_user_stub
  ARCHIVE DESTINATION lib
)

install (FILES ${CMAKE_SOURCE_DIR}/include/vdb.h
  DESTINATION include
)
