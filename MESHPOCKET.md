# Heltec MeshPocket → RNode

RNode firmware for the **Heltec MeshPocket** (`HT-298B` / board-ID `HT-n5262`),
so it can join a [Reticulum](https://reticulum.network) network instead of
running Meshtastic.

Converted and **proven on air** on 2026-09-01: it transmits, it receives, a
second radio heard it over LoRa, and it reached a node two hops away. As far as
we could establish, no one had run RNode firmware on this board before — there
is no forum post, no Reddit comment, and no GitHub issue describing it working.

---

## Whose work this is

**This branch is [@TheBeadster](https://github.com/TheBeadster)'s port**, from
[PR #87](https://github.com/liberatedsystems/RNode_Firmware_CE/pull/87), with
sixteen commits on top. The board definition, the pin map, the e-ink plumbing,
the BLE passkey rewrite and the working binaries are all theirs. Two typos
stopped it building; everything underneath was sound.

The commit history is the credit, and it is intact:

| commits | who | what |
|---:|---|---|
| 34 | [@jacobeva](https://github.com/jacobeva) | RNode_Firmware_CE, and the review of PR #87 |
| 29 | [Mark Qvist](https://github.com/markqvist) | RNode firmware and Reticulum itself |
| 16 | this branch | the fixes below |
| 14 | [@TheBeadster](https://github.com/TheBeadster) | **the MeshPocket port** |
| 6 | Kevin Brosius, [@tomuk5](https://github.com/tomuk5), Owen, 0x62 | upstream RNode and CE work |

Based on **RNode_Firmware_CE v1.75** (`2a4d6c7`), itself a fork of Mark Qvist's
RNode_Firmware. That base matters for reading what follows: some of the defects
below are in the port, and some are in CE and affect every nRF52 board it
supports. Each is labelled.

Also owed:

* **[@clarkee1066](https://github.com/clarkee1066)** spotted the missing comma
  in review. The fix in this branch is their catch.
* **[@AlexSkorni](https://github.com/AlexSkorni)** wrote the v2 installation
  guide and reported the failures that root-caused the erase requirement.
* **[@shortwavesurfer2009](https://github.com/shortwavesurfer2009)** got it
  flashing first and documented the route.
* **[Meshtastic](https://github.com/meshtastic/firmware)** — their
  `heltec_mesh_pocket` variant is the pin ground truth that settled two
  questions the port itself was unsure about.

GPL-3.0, like everything it descends from.

---

## What the sixteen commits change

**Two typos that stopped it building.** A missing comma after `12 // pin_reset`
let C++ fold `12 -1` into `11`, so the array initialised with nine values
instead of ten: `pin_reset` became 11 and `pin_tcxo_enable` became 0 — the
32 kHz crystal pin. That is a dead radio — and it accounts for the split in the
thread, where the author's published binaries worked and the one person who
built from source got silence. A stray
`Search` token in the *T114* block was a hard compile error for that board.

**The radio could never be reset.** This matters more than it sounds. The
MeshPocket cannot be power-cycled — the 3V3 rail's enable is tied to the battery
with no control line, and the cells are soldered. The SX1262's only reset is the
MCU driving P0.12. With `pin_reset` at 11, `sx126x::reset()` toggled the wrong
pin, so a wedged modem had no recovery at all short of running the battery flat.

**The radio and the display shared one SPI peripheral.** Both were on `NRF_SPIM1`
with different pins, worked around by disabling SPIM1 and hand-writing its
`PSEL` registers, with the re-enable left commented out. The display now has
SPIM3 and the radio keeps SPIM1.

**Firmware validation was disabled for every board.** On a hash *mis*match the
port set `fw_signature_validated = true`, not scoped to this board. The cause
was real — `IMG_SIZE_START` (`bank_0_size` in the bootloader settings page) is
written by serial DFU but never by UF2 drag-and-drop, so a UF2-flashed board
hashes the wrong region forever. Rather than disable the check, the image size
now comes from the linker (`__etext + sizeof(.data) − APPLICATION_START`), which
is correct however the board was flashed. **Verified on hardware four times: the
device-computed hash matched `sha256sum` of the flashed `.bin` exactly.**

**Two SX1262 errata, missing from CE.** 15.4 (IQ polarity, register `0x0736`)
must be re-applied after every `SetPacketParams` — without it *"LoRa RX
demodulation fails silently while TX continues to work"*. 15.1 (register
`0x0889`) improves sensitivity at every bandwidth except 500 kHz, and was an
empty stub. Both ported from upstream RNode_Firmware.

**TCXO was enabled after calibration**, so the radio was trimmed against the
internal RC oscillator. Affects every TCXO board in CE.

**P0.21 was driven HIGH at boot.** Meshtastic's variant for this board marks it
`SX1262_DIO3`, *"connected internally to power the tcxo, do not drive from the
main CPU"*. It is no longer driven.

**BLE — four defects in CE's shared nRF52 code, not in the port.** These are
present in RNode_Firmware_CE v1.75 and affect every nRF52 board it supports. The
disconnect handler compares an HCI disconnect reason against a GAP *security*
status, so a disconnect reported as 0 leaves the firmware believing the phone is
still there. `bt_stop()` sets a flag without stopping anything.
`bt_debond_all()` is an empty `{}` on nRF52, making unpair a silent no-op.
`bt_start()` is not idempotent — advertising records append, so a third call
appended a truncated name and the board advertised as `RNode`, failing the
`"RNode "` prefix filter every client uses.

The port's own BLE work went the other way. It added a real passkey callback
that puts the same six digits on the screen and on the wire to `rnodeconf` — and
it fixed a stack overflow in CE:

```c
- char pin_char[6];  sprintf(pin_char, "%lu", pin);   // six digits + NUL into six bytes
+ char pin_char[7];  snprintf(pin_char, sizeof(pin_char), "%06lu", (unsigned long)pin);
```

Pairing on this board works because of that commit.

**The e-ink repainted every two seconds regardless of content** — 43,200
refreshes a day. The port was already managing the partial/full balance against
a real constraint (this panel must finish before the next update), but the gate
was time rather than content. The frame is now fingerprinted and repaints
only when the picture actually differs. Three things defeated that fix in turn
and each had to be found by looking at the panel rather than the code: the
scrolling waterfall (appends a sample every draw), the status page rotation
(flips every 4 s), and unquantised values printed beside quantised bars. The
real full-refresh waveform — the only thing that clears ghosting — had been
commented out and replaced with a controller re-init.

**The button legend did not match the firmware.** It promised
"Bluetooth on/off" at 1 s (that sleeps the board) and "Sleep" above 8 s (that
does nothing). The serial log announced "Bluetooth pairing" while calling
`sleep_now()`. And the node ID on the sleep screen — so you can identify a
sleeping node without waking it, the best idea in the port — was drawn *after*
the refresh and had never once reached the panel.

---

## Flashing it

**Read the hardware notes below first.** Some of them will cost you the board.

The procedure that worked, in full:

1. **Enter DFU.** Double-tap `RST`, or send a 1200-baud touch over the serial
   port (open at 1200 with DTR low, then close). Confirm `239a:0071`.
2. **Read `INFO_UF2.TXT`** from the mounted drive. **Abort** unless it says both
   `Board-ID: HT-n5262` and `SoftDevice: S140 6.1.1`.
3. **Do not run a factory-erase UF2.** `rnodeconf --eeprom-wipe` formats the same
   LittleFS region in software, over serial, and self-recovers. The erase UF2 is
   the only drag-and-drop step in the procedure and the only one that cannot
   recover itself — see "a torn UF2 does not fall back" below.
4. **Build and flash by serial DFU:**
   ```
   arduino-cli compile -e --fqbn Heltec_nRF52:Heltec_nRF52:HT-n5262 \
       --build-property "compiler.cpp.extra_flags=\"-DBOARD_MODEL=0x46\""
   adafruit-nrfutil dfu serial -pkg <build>/RNode_Firmware_CE.ino.zip \
       -p <port> -b 115200 --singlebank
   ```
   **`adafruit-nrfutil` exits 0 even when the flash fails.** Do not trust the
   exit code; grep the output for `Device programmed.`
5. **Patch `rnodeconf`** — upstream does not know product `0xD2` or model
   `0xCE`. `-r` works unpatched via a generic hex fallback, but `-i` and `-T`
   both raise `KeyError` on an unknown model.
6. **Provision:**
   ```
   rnodeconf <port> --eeprom-wipe
   rnodeconf <port> -r --platform NRF52 --product d2 --model ce --hwrev 1
   rnodeconf <port> -T --freq 915125000 --bw 125000 --sf 9 --cr 5 --txp 17
   ```
7. **Write the firmware hash**, or the board sits showing `FIRMWARE CORRUPT`:
   ```
   rnodeconf <port> -L                      # prints the device's own hash
   rnodeconf <port> --firmware-hash <hex>
   ```
   `-i` reporting "Device signature validated" refers to the EEPROM signature,
   which is a different thing.

`rnodeconf -i` should end with
`Product : Heltec MeshPocket 863 - 928 MHz (d2:ce:46)`. The `46` is the board
byte **reported by the firmware itself** — that is your proof the MeshPocket
build is running and not the T114 build that shares its FQBN.

---

## Hardware notes that will bite you

**The USB-C port is charge-only.** It does not appear on Heltec's published node
schematic at all. Flashing works only through the bundled magnetic pogo cable,
and **there is no replacement source** — not from Heltec, not a reseller, not a
user. Losing that cable strands the board.

**It cannot be power-cycled.** The 3V3 regulator's enable is tied to `BAT+`
through a 100 kΩ pull-up with no control line, the cells are soldered, and the
deepest documented state is a software sleep. Running the battery flat is the
only true cold cycle short of opening a glued case.

**`HT-n5262` does not identify this board.** It is Heltec's *module* name,
shared by at least the Mesh Node T114, Mesh Node T1, Mesh Solar and the
MeshPocket. Same product string, same DFU PID `0x0071`, same FQBN. Only the
per-unit USB serial separates them. Two MeshPockets are reported boot-looping
after the wrong board's firmware, unrecovered. **Never let a flasher choose
firmware by board-ID alone.**

**Heltec publishes no bootloader recovery image for this board** — only two
stock Meshtastic UF2s. The sibling T114 directory ships both a bootloader hex
and an erase image. So recovery is reliable only while the bootloader survives.
Never apply a bootloader-update UF2.

**A torn drag-and-drop UF2 does not fall back to DFU.** Every successful UF2
write stores `bank_0_crc = 0`, which permanently disables CRC validation, and
blocks are written lowest-address-first — so after an interrupted copy the app's
first two words are not `0xFFFFFFFF`, the bootloader considers it valid, and
jumps into a half-written image. Recoverable by double-tapping RST, but not
automatic. **Serial DFU erases first and does fail safe.** This is the reason to
prefer it.

**While BLE is connected, USB serial is ignored** — one host at a time, and
Bluetooth wins. `rnodeconf` then reports *"Serial port opened, but RNode did not
respond. Is a valid firmware installed?"*, which right after a flash reads
exactly like a failed flash. Disconnecting in the app is **not** enough (Android
keeps the GATT link for bonded devices) and tapping `USR` does not help either
(the toggle is gated on not-being-connected). Turn the phone's Bluetooth off, or
press `RST`. One-step diagnostic: scan for the board — a peripheral stops
advertising while connected, so *not advertising and not answering USB* means
something still holds the link, not a bad flash.

**Pairing** is `USR` held for **more than 5 seconds** — longer than feels
natural. A 6-digit passkey appears on the screen; it is regenerated every boot,
which is cosmetic and does not invalidate an existing bond.

**Qi charging degrades LoRa reception.** Relevant, since the device is designed
to sit on the back of a phone.

---

## For other boards

Several of these fixes are not MeshPocket-specific and are broken for **every**
nRF52 board on RNode_Firmware_CE: the two SX1262 errata, the TCXO ordering, the
application-size calculation, and all four BLE defects above. Those are worth taking upstream on their own.

---

## Status

Working and in daily use as a phone-paired RNode. Not merged anywhere; this
branch exists so the work is not lost and so the people above are credited.

Untested: anything other than 915 MHz, and the 5000 mAh variant.
