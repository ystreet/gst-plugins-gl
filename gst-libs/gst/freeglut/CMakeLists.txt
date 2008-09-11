add_library (gstfreeglut STATIC
  freeglut_callbacks.c
  freeglut_display.c
  freeglut_init.c
  freeglut_main.c
  freeglut_state.c
  freeglut_structure.c
  freeglut_window.c)

target_link_libraries(gstfreeglut
  general glu32
  general opengl32
  general gdi32
  general winmm)
  