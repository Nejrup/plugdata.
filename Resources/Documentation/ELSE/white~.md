---
title: white~

description: White noise generator

categories:
 - object
 
pdcategory: Audio Oscillators And Tables

arguments: (none)
  
inlets:
  - type: float
    description: sets seed (no float — unique internal)

outlets:
  1st:
  - type: signal
    description: white noise

flags:
    - name: -seed <float>
      description: sets seed 
      default: unique internal
    - name: -clip
      description: sets to clip mode
      default:
      
methods:
    - type: seed <float>
        description: a float sets seed, no float sets a unique internal
    - type: clip
        description: sets to clip mode

draft: false
---

[white~] is a white noise generator from a pseudo random number generator algorithm.