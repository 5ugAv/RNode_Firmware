# Display simulator

Runs the firmware's **actual** drawing code on a host and renders what the
panel would show. No board, no flash, no emulator.

`draw_stat_area()` and `draw_disp_area()` are pure software: they paint into two
64x64 `GFXcanvas1` buffers and are only afterwards blitted to the panel. So they
compile and run anywhere, given a handful of stubs.

```
make            # build
make states     # render every state to *.pgm
./sim linked out.pgm
```

States: `linked` `nophone` `pairing` `noradio` `corrupt` `busy` `lowbatt`.

## Why

Every display defect in this branch escaped code review and was found by
someone holding the hardware at 5am — a waterfall scrolling on every draw, a
status page rotating on a timer, a node ID drawn after the refresh. All of them
are visible here in one frame, before anything is written to a board.

It also guards against re-introducing them: any element that changes without a
state change shows up as two differing frames for the same state.

## Trust, and its limits

Validated against photographs of the real panel: the identity, bitrate,
airtime and channel-load screens, the pairing screen with its PIN, and the
`FIRMWARE CORRUPT` plate all render identically to the hardware.

It simulates **drawing only**. The panel stubs are no-ops, so it says nothing
about refresh timing, ghosting, waveforms, or anything electrical. It cannot
tell you the screen is being repainted too often — only what a repaint contains.
