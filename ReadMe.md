# Realistic Soft Shadows

This project implements realistic shadows for 3D Graphics Rendering, using OpenGL.

![Output sample](https://github.com/aryaman-gupta/ScreenSpaceSoftShadows/blob/master/ShadowOutput.gif)

Realistic shadows consist of a hard shadow part (the umbra, which receives no direct light from the source), and soft shadow part (the penumbra, which receives partial light). The degree of softness of a shadow is dependent on several geometric parameters, which are:

1) Distance between obstructor and light source.
2) Distance between obstructor and screen.
3) Size of light source.

In this project, soft shadows are produced by applying Gaussian blurring to a hard shadow map. The size of the Gaussian kernel used for blurring is dependent on the aforementioned parameters, thus producing realistic shadows. The most computationally expensive step of the process is the Gaussian blurring. It is therefore performed in screen space [1].

Multiple lights are handled by dividing the light sources into layers of non-intersecting lights. Blurring shadows of each light individually is computationally prohibitive, while blurring all the shadows together produces incorrect results where shadows from different sources intersect. Therefore, light sources are assigned layers such that no two intersecting sources are assigned the same layer. This is done using a Graph Colouring approach [2]. The shadows from the lights in a layer can now safely be blurred together.

[1] Gumbau, J., Chover, M. and Sbert, M., 2010. Screen space soft shadows. GPU pro, pp.477-491.

[2] Tamás, M. and Heisenberger, V., 2015. Practical Screen-Space Soft Shadows. GPU Pro 6: Advanced Rendering Techniques, p.297.

The project can be treated as an implementation of the two publications mentioned above.
