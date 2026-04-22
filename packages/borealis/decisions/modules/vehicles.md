# Borealis Module Decision - Vehicles

- Status: proposed
- Date: 2026-04-22
- Type: module / gameplay API
- Scope: `borealis.game.vehicles`

## Summary

Vehicles group specialized controllers for car-like and future vehicle types.

## Implementation

- keep vehicle behavior separate from generic controllers;
- allow future expansion to bikes, boats, planes, and other vehicles;
- expose stateful controllers with explicit update methods.

## Proposed API

- `car_create(config) -> Controller`: creates a car controller.
- `car_update(controller, input, dt)`: updates car movement each frame.
- `car_steer(controller, direction)`: changes steering direction.
- `car_accelerate(controller, amount)`: applies throttle.
- `car_brake(controller, amount)`: applies braking.
- `car_handbrake(controller, bool)`: enables or disables handbrake.
- `car_boost(controller, amount, duration)`: applies temporary boost.
- `car_set_traction(controller, value)`: adjusts traction.
- `car_set_grip(controller, value)`: adjusts grip in turns.

## Notes

- `vehicles` should host all future vehicle families.
- vehicle control should not live in generic movement.
