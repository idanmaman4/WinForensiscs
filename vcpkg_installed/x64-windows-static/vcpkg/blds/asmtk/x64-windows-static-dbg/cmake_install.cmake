# Install script for directory: C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/pkgs/asmtk_x64-windows-static/debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "OFF")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/x64-windows-static-dbg/asmtk.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk/asmtk-config.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk/asmtk-config.cmake"
         "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/x64-windows-static-dbg/CMakeFiles/Export/d82acc61b1b534f95d5d81dc840124af/asmtk-config.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk/asmtk-config-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk/asmtk-config.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/x64-windows-static-dbg/CMakeFiles/Export/d82acc61b1b534f95d5d81dc840124af/asmtk-config.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/x64-windows-static-dbg/CMakeFiles/Export/d82acc61b1b534f95d5d81dc840124af/asmtk-config-debug.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/asmtk.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/asmparser.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/asmtokenizer.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/elfdefs.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/globals.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/parserutils.h")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/asmtk" TYPE FILE FILES "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/src/ae29381422-4259e57375.clean/src/asmtk/strtod.h")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/Idang/source/repos/WinForensiscs/vcpkg_installed/x64-windows-static/vcpkg/blds/asmtk/x64-windows-static-dbg/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
