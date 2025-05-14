# Gravi

## Introduction
Gravi is a non-linear raytracing engine built in C++. Using the featured USD Hydra Delegate, HdGravi, Scenes can be rendered with interesting gravitational effects.

The aim of this project was to combine my knowledge of C++, Qt, and Hydra USD all into a single application. I believe that the VFX and Animation industry is heading more towards a more USD centric pipeline, and I felt that creating Gravi would prepare me well. I'm also a huge fan of Interstellar, hence the non-linear raytracing.

Gravi was made for *Computing For Graphics And Animation* at Bournemouth University.

## Features
Gravi has multiple features:
- **USD Renderer Qt Widget**: Gravi is one of the few publicly available applications that combines Qt with USD inside of C++.
- **Custom USD Schemas**: Gravi has it's own USD Schemas for rendering, which can be accessed by any DCC.
- **USD Hydra Render Delegate**: hdGravi can be loaded into any DCC.
- **Non-Linear Raytracing**: Gravi is a working non-linear raytracer for simulating gravitational lensing.

## Implementation
This document goes over the implementation of Gravi. It will *not* address all the techniques and systems used, only the important ones. For those more curious into building their own kind of application, I strongly recommend reading the commented source code.

### Overview
In order to keep in line with the CY2025 [VFX Reference Platform](https://vfxplatform.com/), I used:

| DCC    | Version |
|--------|---------| 
| C++    | 17      |
| Python | 3.11.8  |
| Qt     | 6.5     | 
| USD    | 25.02   |

I decided to stick with a newer version of USD to future-proof it. However, this mean't that I was unable to test the renderer in Houdini 20.5 as they are on USD 24.xx.

### Project Structure
Gravi is structured in three parts: usdGravi, hdGravi, and Gravi. 
- usdGravi contains all the USD Schemas for the delegate and the app.
- hdGravi is the actual render delegate is used by Gravi as well as other DCCs.
- Gravi is the full graphical application which users interact with to set-up and render their projects. 
<br></br>
![Gravi Pipeline](doc/GRAVI_pipe.jpg)

Above is a diagram explaining the application's structure. Ideally, usdGravi would also feed into external DCCs so that you can work directly within them. This will happen in a future release.

### UsdGravi
Gravi has a specific USD Schema object called "GravityWell".

A GravityWell is essentially a black hole. 
It has a center position, and a force factor. This force factor can either repel or attract passing light rays.

```usda
class GravityWell "GravityWell" (
    doc = """A Gravity Well, used to bend light in hdGravi renders."""
    inherits = </Gprim>
    customData = {
        string className = "GravityWell"
    }
    )
{
    float force = -0.98 (
        doc = "The amount of gravitional force. Negative values attract light particles, positive values repel them."
    )
}
```

usdGravi also contains /Render/GraviRenderSettings which is how Gravi stores settings such as max-samples and light-path-stepping.


### HdGravi
HdGravi is a USD Hydra render delegate. 
USD delegates are an entire process in themselves and I won't go over it in this report, but all that needs to be known is that a render delegate reads in a USD file and renders it.

What's special about HdGravi is that it is one of the first non-linear Hydra render delegates. Although previous renderers exist, hdGravi's ability to work directly with USD makes it much easier to develop, optimize, and use in large VFX and Animation productions.

#### Gravitational Rendering (Non-Linear Raytracing)
Light bends! Although we don't see it on Earth, if light undergoes enough gravitational force it will begin to bend. This is because (Say it with me physics nerds) light is both a **particle** and a wave!

Sadly, calculating light rays is not something that can be numerically solved (at least not for multiple gravitational bodies), therefore the way I went about doing the Non-linear raytracer was to fire a particle out from each pixel, and advect it through the scene. Between each particle step, I'm doing a line segment -> object intersection test to see if the light "ray" has hit anything.

For actually *bending* the light rays, I apply a gravitation force using

<div style="display: flex; justify-content:center">

$Fg = |f|/d^2$ 

</div>

Where f is force and d is distance.

Since we're doing intersection tests *every* step, the renderer is extremly slow. To combat this I implement a couple optimization tricks.

1. Parallel Processing: I used tbb to pararrelize my rendering so that it would do multiple pixels at the same time.

2. BVH Scene Reconstruction: I'm using a BVH tree for all my objects (and my triangle meshes) for localized intersection testing.

3. Adaptive Particle Stepping & Influence Switching *(See Below)*

#### Adaptive Particle Stepping & Influence Switching

Originally, every particle would step the same distance throughout the scene. This was extremely inefficient for particles that had very little gravitational force on them. My first approach was to use adaptive particle stepping where the step size of each particle is inversely proportional to the amount of gravitational force acting on it. (more gravity -> smaller steps).

I then had the idea to do something which I have called "Influence Switching".

It's a known fact that you are *never* out of range from the gravitational pull of an object. *However*, at a certain distance this pull becomes negilible and can be ignored. Influence Switching is based on this idea. First, I restructured my Graviational Force equation so that:

<div style="display: flex; justify-content:center">

$d = \sqrt{|f|}/\alpha$

</div>

In which $\alpha$ is the gravitational cuttoff threshold. This calculated distance, $d$ was then used to create an "Influence Sphere" around each of my Gravity Wells.

Now, when raytracing I first check if the particle is inside an influence sphere. If it is, it undergoes an adaptive particle step.

If it isnt, then it does a normal ray intersection test (no segmentation). If the ray collides with an influence sphere. It stops the particle at the sphere's edge and then begins adaptive particle stepping. Otherwise, it will return an object hit or nothing.

<div style="text-align: center;">

<div style="display:flex; justify-content:center;">
    <img src="doc/influence_switching.png" style="margin: 1em; width: 50%; height: auto;"/>
    <img src="doc/influence2d.png" style="margin: 1em; width: 35%; height: auto;"/>
</div>

*Left: A diagram showing Influence Switching with adaptive particle stepping. Right: A chart showing the gravitational cutoff at $\alpha=0.1$.*

</div>

There is still one problem, in some cases the light-ray will continute on forever (inside the black hole influence) without hitting anything. I set a "max distance" variable so that the rays will eventually die.

These techniques (while resulting in a loss of accuracy), help speed up rendering dramatically.

#### Other Ideas & Limitations

Although I didn't have the time (or technical ability) to do so, here are some further ideas for more accurate and faster renders:

1. Use real world values. Then the renderer can be used for scientific visualization (and it would be *very* accurate).

2. Give particles a velocity. Since we're dealing with forces. It would be useful to give the light particles a velocity. More advanced renders could look at going forward/backwards in *time* to intersect the scene. (You could get some pretty funky time travelling light rays).

3. Use Bezier Curves instead of line segments for rendering. Using the given velocity direction, the segments between particles could be smoothed out for less "stepping" and more accurate intersections.

4. I'm currently using a linear technique for sampling lights and shadows. This is not ideal. Finding a way to quickly (and accurate) sample lights would help improve hdGravi's accuracy.

### Gravi
Although Gravi is not very complex (It's mostly graphical frontend code), there are a few important features worth noticing.

#### HdWidget
The HdWidget is an extension of the QOpenGLWidget that uses UsdImagingGLEngine to paint whatever a Hydra delegate outputs onto the screen.

Being able to use USD's prebuilt tools made life alot easier for me as I didn't have to create an OpenGL delegate for the viewport. I did, however, have to manually work some OpenGL code to display the grid and gravitywell.

One notable problem I had whilst writing the widget was that it would only show parts of the render. I had to solve this by creating a draw timer that re-painted the widget at 60 FPS. There were also some issues with USD upAxis and unit scale which I had to sort out to maintain a cohesive viewport.

#### Tabs
Tabs was the other widget I designed that allowed users to customize their Gravity Wells and also render out animations, There was a fun battle in trying to read and write USD primvars and attribtues. The code generated in usdGravi was extremelty useful in getting it to work.

## Final Results

<div style="display:flex;width:100%;gap: 1em;">
<div style="display:flex;flex-direction:column;align-items:center;">
    <h2> Kitchen Set </h2>
    <p> A video example of a gravity well inside of the USD kitchen scene. </p>

https://github.com/user-attachments/assets/f8b9bf07-0ea1-4f0e-9d16-bb68e565d046
    
</div>

<div style="display:flex;flex-direction:column;align-items:center;">
    <h2> Interstellar </h2>
    <p> An attempted re-creation of a shot from Interstellar (2014) </p>
    
https://github.com/user-attachments/assets/eb25930b-0167-466f-8d57-9037a90ac457

</div>
</div>

## Demo Video

A demo of Gravi in use. Gravi is available for download from the [Release Page](https://github.com/cjhosken/gravi/releases).

<div style="display:flex; align-items:center; justify-content:center;">

https://github.com/user-attachments/assets/12013a84-0334-4b61-a554-2fad2d615dff

</div>

## Critical Evaluation

Overall, I am very happy with how Gravi turned out. Given more time (and a team), Gravi could become a pretty usable render delegate for production. 

Here were a few of things I wasn't able to get implemented (although some of code is in this project):

- Faster Rendering (CUDA + OptiX)
- Better light sampling techniques
- Textures, Volumes, Instances & USD Geom Prims + Complete Primvar Support (st (uv), subdiv, etc.)
- Spinning Gravity Wells
- Animated Gravity Wells 
- A more multi-purpose interface for editing USD files

## Reference List

1. alex-v-dev, 2021. GitHub - alex-v-dev/hdcycles: Cycles Hydra Delegate [online]. GitHub. Available from: https://github.com/alex-v-dev/hdcycles [Accessed 26 Mar 2025].
2. DreamWorks Animation, 2024. GitHub - dreamworksanimation/openmoonray [online]. GitHub. Available from: https://github.com/dreamworksanimation/openmoonray [Accessed 25 Mar 2025].
3. Moore, N. J., Zhao, Y., Schmidt, G. and Moore, D. T., 2021. Ray-tracing simulation of gravitational lensing using a gradient-index model. Optical Engineering, 60 (01).
4. Nvidia, 2025. CUDA Toolkit Documentation [online]. docs.nvidia.com. Available from: https://docs.nvidia.com/cuda/index.html [Accessed 17 Mar 2025].
5. ospray, 2024. GitHub - ospray/hdospray: Rendering plugin for Pixar’s USD Hydra [online]. GitHub. Available from: https://github.com/ospray/hdospray/tree/master [Accessed 26 Mar 2025].
6. Pixar, 2025. USD Home — Universal Scene Description Documentation [online]. openusd.org. Available from: https://openusd.org/release/index.html [Accessed 17 Mar 2025].
7. Working, P., 2025. VFX Reference Platform: Home Page [online]. Vfxplatform.com. Available from: https://vfxplatform.com/ [Accessed 26 Mar 2025].
