cmake_minimum_required(VERSION "2.6.0")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

project (GST-PLUGINS-GL)

find_package(Glib2 REQUIRED)
find_package(Libpng REQUIRED)
find_package(GTK2 REQUIRED)
find_package(ATK REQUIRED)
find_package(Cairo REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(LibIconv REQUIRED)
find_package(Gstreamer REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Glew REQUIRED)

function(check_package name LEVEL)
  if (${name}_FOUND)
    message(STATUS "Found ${name}... YES")
  else (${name}_FOUND)
    message(STATUS "Found ${name}... NO")
    if (LEVEL)
      if (WIN32)
        message(FATAL_ERROR "You should set the env variable ${name}_DIR")
      else (WIN32)
        message(FATAL_ERROR "You should install ${name} dev package")
      endif (WIN32)
    endif (LEVEL)
  endif (${name}_FOUND)
endfunction(check_package)

# required packages
check_package(GLIB2 1)
check_package(LIBXML2 1)
check_package(LIBICONV 1)
check_package(GSTREAMER 1)
check_package(OPENGL 1)
check_package(GLEW 1)

# optionnal packages
check_package(PNG 0)
check_package(GTK2 0)
check_package(ATK 0)
check_package(CAIRO 0)

# debug
#message("****" ${GLIB2_INCLUDE_DIR})
#message("****" ${GLIB2_LIBRARIES})
#message("****" ${LIBXML2_INCLUDE_DIR})
#message("****" ${LIBICONV_INCLUDE_DIR})
#message("****" ${GSTREAMER_INCLUDE_DIR})
#message("****" ${GSTREAMER_LIBRARIES})
#message("****" ${OPENGL_INCLUDE_DIR})
#message("****" ${OPENGL_LIBRARIES})
#message("****" ${GLEW_INCLUDE_DIR})
#message("****" ${GLEW_LIBRARY})
#message("****" ${PNG_INCLUDE_DIR})
#message("****" ${PNG_LIBRARY})
#message("****" ${GTK2_INCLUDE_DIR})
#message("****" ${GTK2_LIBRARIES})
#message("****" ${ATK_INCLUDE_DIR})
#message("****" ${CAIRO_INCLUDE_DIR})

add_subdirectory (gst-libs)
add_subdirectory (gst)
add_subdirectory (tests)

#add_dependencies(gstgl)

#FIXME: generate config.h
add_definitions(-DHAVE_CONFIG_H)
