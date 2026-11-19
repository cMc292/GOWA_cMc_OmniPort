# The keyboard beside a controller, and a controller read that stalls (2026-11-15)

Issue #16 (0.1.0-alpha.10, no saves yet): at the brightness screen neither the controller nor the keyboard did anything,
and the launcher had found the controller.

## What the report showed

- Two sessions: Device on Controller only with the Controller API on SDL3, then both on Auto. Both found one
  controller, which SDL3 calls "Xbox 360 Controller", on XInput slot 0.
- The pad recordings (`session-*.pad`) hold the first report, nothing pressed, and no change after it in either session,
  while the title read port 0 all along and reached the brightness screen at about 25 s. In our own sessions a press is
  in the recording at once.
- The keyboard was not read in either session: Controller only shuts it out, and Auto read it only while port 0 had no
  controller.
- Keyboard only read the keyboard once and never again, since the Device setting came (2026-10-26): in that mode the
  controllers are not polled, the keyboard's read marked port 0 connected, and the keyboard was read only while port 0
  was not connected.
- What the report cannot tell: whether no press came from the controller, or the port stopped reading it after its first
  look. A driver call that does not come back holds the pad's lock; every poll after it is skipped and the game gets
  nothing new. In the first session (dead zone 0.10) the first report had the left stick a little off centre (126/129),
  so the controller sent a real position once; nothing changed after it.

## What changed

- `cellPad.c`: Auto reads the keyboard beside the controllers. It goes into port 0 by itself when no controller is
  there, else with the controller's own presses: the buttons of both, a stick the keyboard pushes where the keyboard has
  it, the larger trigger. Keyboard only reads it alone, every poll. The launcher's Device choice reads "Controller and
  keyboard", and its help and `recomplauncher.json`'s say both play.
- `cellPad.c` logs, for the next report of this kind:
  - `[cellPad] port N: the controller's first press (XInput|SDL3, buttons XXXX)`, once per port, and
    `[cellPad] the keyboard's first press (buttons XXXX)`;
  - `[cellPad] the controllers have not been read for N s: the read is still in <call>`, from a reader that finds the
    pad's lock held 2 s or more, while it lasts;
  - `[cellPad] the controllers took N s, M s of it in <call>`, when a hold of 250 ms or more ends (the first 8).

  The holder names each call it makes (`pad_step`): XInputGetState, SDL_InitSubSystem, SDL_UpdateGamepads,
  SDL_GetGamepads, SDL_OpenGamepad, SDL_RumbleGamepad, GetAsyncKeyState and the like.

## How it was checked

A temporary probe (`PAD_PROBE`, removed) faked the window's focus, Space (Cross in the reporter's key map) at 35, 45 and
55 s after cellPadInit, the controller's L3 at 40 s, and a 3 s stall inside the poll at 25 s. The runs used a fresh test
hdd0 (no saves, so the game stops at the brightness screen), the reporter's settings, and this PC's XInput controller.

- Auto, the controller plugged in: `XInput controller connected on port 0`; the recording has Cross at 34.7, 44.7 and
  54.7 s and L3 at 39.7 s; the first Cross took the game past the brightness screen (the screen position page at 42 s,
  the profile's save thread at 45 s, the intro at 60 s); both stall lines came, naming the probe's stall.
- Keyboard only: no controller read; three Cross presses at 35.0, 45.0 and 55.0 s, each let go after about 0.3 s, so the
  keyboard is read on every poll; the game went past the brightness screen as above.
- Without the probe, Auto with the controller: it was found, the title read the pad, and no stall line came.

## What a next report of this kind shows

- The controller's first press in the log while the recording has no change: the press came and was lost in the port.
- No first press and no stall line: no press came from the controller.
- A stall line: the call that did not come back, and for how long.
