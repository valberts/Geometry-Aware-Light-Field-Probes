﻿# 3dcg-final-project

## Controls

`W - move forward`

`A - move left`

`S - move back`

`D - move right`

`Click and drag mouse - Pan camera`

## User Interface

### Window
- Camera mode: 0 freecam, 1 top down, 2 third-person
- Normal mapping: ON/OFF to toggle normal mapping
- Skybox: ON/OFF to toggle environment mapping
- Minimap: ON/OFF to toggle displaying the minimap
- Night time: ON/OFF to toggle between daytime and nighttime lighting conditions
- Light a flame: ON/OFF to toggle flame particle effects
- Snake length: determines the number of segments for the snake
- Animate snake: ON/OFF render the snake and animate it with hierarchical transformations
- Animate texture: ON/OFF animate the texture of the player
- Robot arm position: X/Y/Z change the target position of the robot arm
- Start camera motion: starts the bezier camera motion

### Light controls
- Light position - X/Y/Z controls the position of the point light
- Light color - R/G/B controls the color of the point light

### PBR
- Albedo: R/G/B controls the color of the albedo material property of the player
- Metallic: 0.0-1.0 controls the metallic material property of the player
- Roughness: 0.0-1.0 controls the roughenss material property of the player

### Views

### Animate Snake
Check "animate snake" to render a snake with selected length, can use UP DOWN RIGHT LEFT to control/move the snake's position.

### Animate Camera

press R mouse to add points, then click "Start Camera Motion" to animate camera along the bezier line (once).

## Minimal features:

- [x] (Vincent) Multiple viewpoints. For example, a top view and a third-person camera that follows a character. The position and movement of the camera is up to you. You can also create a cutscene to add new viewpoints.
- [x] (Vincent) Advanced shading: include more complex material models, such as PBR shaders.
- [x] (Vincent) Material textures: e.g., kd, ks, shininess, roughness.
- [x] (Qinxin & Vincent) Normal mapping.
- [x] (Qinxin & Vincent) Environment mapping.
- [x] (Hazel) Smooth paths: at least one feature of your demo should make use of smooth paths using Bézier curves. It can be the movement of a model, a light source, or a camera. Use a path composed of at least three Cubic Bézier curves.
- [x] (Hazel) Hierarchical transformations: include one object with several animated components. This should be done in code, not imported from Blender. Practical examples are robot arm, snake, solar system, vehicle composed of multiple parts

## Extra effects:

For example: (but we can come up with any others)
- [x] (Hazel) Move at constant speed along Bézier curve (Cam move, predefined bezier, 3 curves)
- [x] (Hazel) Particle effects (explosions, magic spells, fire) (Simple flame)
- [x] (Hazel & Vincent) Point-lights like torches or light bulbs (color and pos changeable)
- [ ] The illusion of an infinite terrain by adding new tiles on the fly
- [ ] (Hazel) Post-processing effects
- [ ] Procedurally generated terrains
- [ ] Water surfaces that are procedurally generated (produce a vertex grid and move the vertices according to a combination of several sinus functions that you can evaluate in the vertex shader).
- [x] (Qinxin) Animated textures (by switching textures frame to frame)
- [x] (Qinxin) Zoom effects / perspective changes
- [x] (Hazel & Vincent) (Really simple, will improve) Changing lighting conditions (like a day-night system) (Simple day/night change, affecting global illum, light bulb stay unchanged)
- [x] (Vincent) A minimap (by rendering from a camera high up in the sky, and displaying it on a quad)
- [ ] (Qinxin) Collision between the player and objects
- [ ] Generate a maze or dungeon for the player to move through
- [ ] Generate environment props like procedural buildings
- [x] (Hazel) Inverse kinematics

## Grading

The final project counts for 60% of the final grade. The grade of the final project is divided as follows.

The final project grade is capped to a maximum of 10 points for each student.

- Part 1: max 2 points
- Part 2 minimum requirements: max 4 points
- Part 3 extra features: unlimited

You can also enrich your demo by using 3D models and textures from external sources. Make sure you have the correct rights to use the models and that you reference any third-party assets in your report.

You are judged on the graphics techniques you used and how well they were applied. In short, the artistic aspects of your demo are nice to have but should not be necessary to achieve a high grade. Still, try to be creative and make a demo that you are proud of!

## Submission

- [ ] the source code (do not include build files!)
- [ ] a short report (max 500 words) of the implemented techniques each accompanied by at least two screen shots (more are welcome) to well illustrate each method. Do not forget to include references!
- [ ] a list of the work done by each individual group member with a percentage indication (first page of report, not included in the word count).
- [ ] a video for your tech-demo. No slides are necessary, your demo should showcase all your features. Max 4 minutes! Since there is no live presentation your video will be posted in Brightspace so other groups can also appreciate your work.
