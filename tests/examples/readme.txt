--- Description of the examples ---

cube:
Show how to have a graphic FPS greater than the input video frame rate.
The source is the videotestsrc rgb.

cubeyuv:
Show how to have a graphic FPS greater than the input video frame rate.
The source is a dvix file. 
The colorspace conversion is maded by the glupload element.

doublecube:
The dvix source is displayed into two renderer. 
The first one is a normal 2D screen, the second is a 3D cube.

filterxoverlay:
Show how to use the xoverlay interface through GTK. 
The pipeline uses a glfilter.

mousexoverlay:
Show how to use the xoverlay interface through Qt (ThrollTech). 
The cube is rotating when moving the mouse (+ click maintained)

recordgraphic:
Show how to use the glfilterapp to define the draw callback in a gstreamer client code.
The scene is recorded into an avi file using mpeg4 encoder. 
The colorspace conversion is maded by the gldownload element.

videoxoverlay:
Show how to use the xoverlay interface through Qt (ThrollTech).
The video is displayed as normal 2D scene.

