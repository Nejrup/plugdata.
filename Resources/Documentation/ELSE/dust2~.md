---
title: dust2~

description: Random impulses

categories:
 - object

pdcategory: General

arguments:
- type: float
  description: density
  default: 0

inlets:
  1st:
  - type: float/signal
    description: density (rate) of random impulses

outlets:
  1st:
  - type: signal
    description: random impulses

flags:
  - name: -seed <float>
    description: seed value (default: unique internal)

methods:
  - type: seed <float>
    description: a float sets seed, no float sets a unique internal

---

[dust2~] is based on SuperCollider's "Dust2" UGEN and outputs random impulse values (from -1 to 1) at random times according to a density parameter. The difference to SuperCollider's is that it only produces actual impulses (one non zero value in between 0 valued samples).