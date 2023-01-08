---
title: router

description: Route messages

categories:
 - object

pdcategory: Message management

arguments:
- type: float
  description: number of outlets (2 to 512)
  default: 2
- type: float
  description: sets initially open outlet
  default: 0

inlets:
  1st:
  - type: anything
    description: message to send through a specified outlet
  2nd:
  - type: float
    description: - sets outlet number (0 is none)

outlets:
 nth:
  - type: anything
    description:  outlets for routing any received message

draft: false
---

[router] routes a message from the left inlet to an outlet number specified by the float into the right inlet (if the number is out of range, the message is not output).