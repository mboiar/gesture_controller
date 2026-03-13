#pragma once

/**
 * List of command names for device control.
 */
enum Command {
  Idle,
  Up,    // dislike
  Down,  // fist
  Left,  // four
  Right, // like
  Forth, // ok
  Back,  // mute
  Stop,  // palm
  NoGesture = 18
};