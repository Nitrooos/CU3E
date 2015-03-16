CU3E
====

This app was written as a project for Human-Computer Interaction at my university. It uses an integrated or external
camera to catch pictures and, frame after frame, tries to detect a cube, holding by a user in front of camera.

User should use a special cube, with:
   * all walls painted to green
   * 6 of the 8 vertexes should be proper colored

When program has detected cube on a frame (thanks to OpenCV library), it tries to compute the rotate angles
of cube in each of three axis: x, y and z, using POSIT algorithm. After that it displays the cube on the monitor,
properly rotated and with some custom texture which user can change.

To display a 3D cube program uses OpenGL library.

####**Used technologies:**
   * C++
   * OpenGL
   * OpenCV
   * POSIT algorithm
