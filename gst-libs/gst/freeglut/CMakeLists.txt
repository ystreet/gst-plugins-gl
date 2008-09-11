include_directories(AFTER
  ${OPENGL_INCLUDE_DIR})

add_library (gstfreeglut STATIC
  freeglut_callbacks.c
  freeglut_display.c
  freeglut_init.c
  freeglut_main.c
  freeglut_state.c
  freeglut_structure.c
  freeglut_window.c)

  #FIXME: get ride of this condition
if (WIN32)
target_link_libraries(gstfreeglut
  general ${OPENGL_LIBRARIES}
  gdi32
  winmm)
else (WIN32)
target_link_libraries(gstfreeglut
  general ${OPENGL_LIBRARIES})
endif (WIN32)
  