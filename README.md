## DX11ComputeShaderFilter V 1.0


9 Jan 2017 


## Info
====

- This project implements an DX11 Effects View which are implemented with hlsl compute shader.
- It serves as an effect test framework.
- It compiled OK on Window 10 with VS2015.

## Demo
======

Wating for update...

## Controls
========

- F1     : Navigate to next effect
- F2     : Navigate to previous effect
- F3     : Loop to next image
- ESC    : Exit the application
- 'd'    : Switch between different display mode
  - only source image
  - only result image
  - both source and result image
  
## Assumption 
========
1. only apply one "pixel effect" to one image at one time.
1. just view "static effects", user is unable to adjust effect parameter.
1. all demoed effects are one-pass effects. 
l. only DX11 is supported. 


## Plan 
========
1. much more flexible design to extend the capability of adding new feature. 
1. try to add support to multipass-effect. 
