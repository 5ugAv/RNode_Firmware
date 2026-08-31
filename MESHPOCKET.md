# Heltec MeshPocket → RNode

RNode firmware for the **Heltec MeshPocket** (`HT-298B` / board-ID `HT-n5262`),
so it can join a [Reticulum](https://reticulum.network) network instead of
running Meshtastic.

Converted and exercised on air on 2026-09-01. What was measured, on one board:
four announces transmitted (`txb` 0 → 768, channel load 0 → 20%), 832 bytes
received, a second radio on a separate Reticulum instance gained a routing entry
for its destination over LoRa, and a path request answered from a node two hops
away that was not in the table beforehand. One board, one bench — not a range
test, and not a claim about yours.

No build **from the published source** has been reported working before this;
the port's own binaries were flashed successfully by others (see below).

---

## Whose work this is

**This branch is [@TheBeadster](https://github.com/TheBeadster)'s port**, from
[PR #87](https://github.com/liberatedsystems/RNode_Firmware_CE/pull/87), with
sixteen code commits on top. The board definition, the pin map, the e-ink plumbing,
the BLE passkey rewrite and the working binaries are all theirs. Two typos stopped it
building, and the fixes below are mostly in code it inherited rather than code
it wrote.

The commit history is the credit, and it is intact:

| commits | who | what |
|---:|---|---|
| 34 | [@jacobeva](https://github.com/jacobeva) | RNode_Firmware_CE, and the review of PR #87 |
| 29 | [Mark Qvist](https://github.com/markqvist) | RNode firmware and Reticulum itself |
| 16 (+2 docs) | this branch | the fixes below |
| 14 | [@TheBeadster](https://github.com/TheBeadster) | **the MeshPocket port** |
| 6 | Kevin Brosius, [@tomuk5](https://github.com/tomuk5), Owen, 0x62 | upstream RNode and CE work |

Based on **RNode_Firmware_CE v1.75** (`2a4d6c7`), itself a fork of Mark Qvist's
RNode_Firmware. That base matters for reading what follows: some of the
defects below are in the port, and some are in CE and reach every board it
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

## What those commits change

**Two typos that stopped it building.** A missing comma after `12 // pin_reset`
let C++ fold `12 -1` into `11`, so the array initialised with nine values
instead of ten. `pin_reset` became 11, so `sx126x::reset()` toggled P0.11
instead of P0.12 and the radio could not be reset. (`pin_tcxo_enable` defaulted
to 0, which this variant maps to `0xff` — "no pin" — so that half was inert.)
It accounts for the split in the thread, where the author's published binaries
worked and the one person who built from source got silence. A stray `Search` token in the *T114* block
(absent from upstream CE) broke T114 builds from this branch.

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
was real. `retrieve_application_size()` read a 4-byte value from `0xFF008`, in
the bootloader's settings page, and used it to bound the hashed region. That
value is not dependable across flash routes — exactly how it goes wrong depends
on how the board was last written, and I could not settle the mechanism from
sources I trust, so I am not going to state one. Rather than disable the check, the image size
now comes from the linker (`__etext + sizeof(.data) − APPLICATION_START`), which
is correct whatever the settings page holds and however the board was
flashed. On this board, across
successive flashes, the device-computed hash matched `sha256sum` of the flashed
`.bin` exactly each time — two of those are recorded in the commit history.

**Two SX1262 errata, missing from CE.** 15.4 (IQ polarity, register `0x0736`)
must be re-applied after every `SetPacketParams`; upstream's comment warns that
without it *"LoRa RX demodulation fails silently while TX continues to work"*,
though this firmware only ever uses standard IQ and no such failure was observed
here. 15.1 (`0x0889`, `REG_TX_MODULATION`) is
*Modulation Quality with 500 kHz LoRa Bandwidth* — a transmit fix, applied at
every bandwidth except 500 kHz; CE had it as an empty stub. Both ported from
upstream RNode_Firmware, for parity. Upstream's comment additionally claims RX
benefits; that is theirs, and unmeasured here.

**TCXO was enabled after calibration**, so the radio was trimmed against the
internal RC oscillator. Affects every TCXO board in CE.

**P0.21 was driven HIGH at boot.** Sources disagree on what that pin is: one
reading has it unconnected, and Meshtastic's variant marks it `SX1262_DIO3`,
*"connected internally to power the tcxo, do not drive from the main CPU"*.
Driving it is wrong under either reading, so it no longer is.

**BLE — four defects in CE's shared nRF52 code, not in the port.** These are
present in RNode_Firmware_CE v1.75 and affect every nRF52 board it supports. The
disconnect handler compares an HCI disconnect reason against a GAP *security*
status, so a disconnect reported as 0 would leave the firmware believing the
phone is still there (HCI does not define 0 as a disconnect reason, so this is
a wrong-namespace comparison fixed on inspection, not an observed failure). `bt_stop()` sets a flag without stopping anything.
`bt_debond_all()` is an empty `{}` on nRF52, making unpair a silent no-op.
`bt_start()` is not idempotent — advertising records append, so a third call
appended a truncated name and the board advertised as `RNode`, which would fail the
`"RNode "` prefix filter the Reticulum and Columba clients match on.

The port's own BLE work went the other way. It added a real passkey callback
that puts the same six digits on the screen and on the wire to `rnodeconf` — and
it fixed a stack overflow in CE:

```c
- char pin_char[6];  sprintf(pin_char, "%lu", pin);   // six digits + NUL into six bytes
+ char pin_char[7];  snprintf(pin_char, sizeof(pin_char), "%06lu", (unsigned long)pin);
```

Pairing on this board works because of that commit.

The BLE fix that mattered most in this branch was to a bug this branch
introduced. Stopping Bluetooth cleanly meant clearing
`restartOnDisconnect`, which is permanent for the rest of the boot — so
toggling Bluetooth off and on and then walking out of range left the board
advertising nothing, invisible to its paired phone until rebooted, while the
display still said Bluetooth was on. `bt_start()` now re-arms it, and only
reports `ON` if advertising actually started.

**The e-ink repainted every two seconds regardless of content** — 43,200
refreshes a day. The port was already managing the partial/full balance against
a real constraint (this panel must finish before the next update), but the gate
was time rather than content. The frame is now fingerprinted and repaints only
when the picture actually differs, with a deliberate floor: a full anti-ghost
refresh every 5 minutes regardless, so roughly 288 a day on a static screen. Three things defeated that fix in turn
and each had to be found by looking at the panel rather than the code: the
scrolling waterfall (appends a sample every draw), the status page rotation
(flips every 4 s), and — as a principle rather than a
sighting, since this layout has no such element — any unquantised value printed
beside a quantised bar. The real full-refresh call had been
commented out in favour of a controller re-init; it is restored, though the
re-init did also produce a full refresh indirectly, and ghosting has not been
observed either way on a settled panel.

**`pin_disp_miso` was `-1`.** `SPIClass`'s constructor takes a `uint8_t`, so
that became index 255 into a 48-entry pin map — an out-of-bounds read whose low
byte was then configured as a real GPIO. It landed on an unused pin by luck,
and it *moved* when unrelated strings changed. Now 0, which the variant maps to
"no pin". Also: `HAS_NP true` on a board with no NeoPixel, and a leftover debug
block that printed the BLE passkey into the main display area every frame.

**The button legend did not match the firmware.** It promised
"Bluetooth on/off" at 1 s (that sleeps the board) and "Sleep" above 8 s (that
does nothing). The serial log announced "Bluetooth pairing" while calling
`sleep_now()`. And the node ID on the sleep screen — so you can identify a
sleeping node without waking it, the best idea in the port — was drawn *after*
the refresh and had never once reached the panel.

---

## Flashing it

**Read the hardware notes below first.** Some of them will cost you the board.

**Steps 1–2 confirm the MODULE, not the board.** `HT-n5262` and PID `0x0071`
are shared by the T114, Mesh Node T1, Mesh Solar and MeshPocket, so a T114
passes that gate unchanged and would then be flashed with MeshPocket firmware.
Resolve the target by its own USB serial first and use that path throughout:

```
ls -l /dev/serial/by-id/ | grep <the board's iSerial>
PORT=$(readlink -f /dev/serial/by-id/*<iSerial>*-if00)
```

Everything below uses `$PORT`. Never a bare `/dev/ttyACM0` — on a bench with any
other board attached, that is whichever enumerated first.

The procedure that worked, in full:

0. **Resolve `$PORT` by iSerial**, as above, and re-resolve it after every step
   that resets the board — the port number moves.
1. **Enter DFU.** Double-tap `RST`, or send a 1200-baud touch over the serial
   port (open at 1200 with DTR low, then close). Confirm `239a:0071`.
2. **Read `INFO_UF2.TXT`** from the mounted drive. **Abort** unless it says both
   `Board-ID: HT-n5262` and `SoftDevice: S140 6.1.1`.
3. **Do not run a factory-erase UF2.** `rnodeconf --eeprom-wipe` formats the same
   LittleFS region in software, over serial, and self-recovers. The erase UF2 is
   the only drag-and-drop step in the procedure and the only one that cannot
   recover itself — see the torn-UF2 note below.
4. **Build and flash by serial DFU:**
   ```
   arduino-cli compile -e --fqbn Heltec_nRF52:Heltec_nRF52:HT-n5262 \
       --build-property "compiler.cpp.extra_flags=\"-DBOARD_MODEL=0x46\""
   adafruit-nrfutil dfu serial \
       -pkg build/Heltec_nRF52.Heltec_nRF52.HT-n5262/RNode_Firmware_CE.ino.zip \
       -p "$PORT" -b 115200 --singlebank
   ```
   **`adafruit-nrfutil` exits 0 even when the flash fails.** Do not trust the
   exit code; grep the output for `Device programmed.` The board re-enumerates
   afterwards, so **re-resolve `$PORT` before step 6** — on this bench it moved
   from `ttyACM1` to `ttyACM2` mid-run.
5. **Patch `rnodeconf`** — upstream does not know product `0xD2` or model
   `0xCE`. `-r` works unpatched via a generic hex fallback, but `-i` and `-T`
   both raise `KeyError` on an unknown model.
6. **Provision:**
   ```
   rnodeconf "$PORT" --eeprom-wipe
   rnodeconf "$PORT" -r --platform NRF52 --product d2 --model ce --hwrev 1
   rnodeconf "$PORT" -T --freq 915125000 --bw 125000 --sf 9 --cr 5 --txp 17
   ```
   `--eeprom-wipe` also clears the Bluetooth-enable byte and the display
   config, so the board returns on defaults. It goes quiet for ~13 s and the
   port disappearing at the end is success, not failure.
7. **Write the firmware hash**, or the board sits showing `FIRMWARE CORRUPT`:
   ```
   rnodeconf "$PORT" -L                     # prints the device's own hash
   rnodeconf "$PORT" --firmware-hash <hex>
   ```
   `-i` reporting "Device signature validated" refers to the EEPROM signature,
   which is a different thing.

`rnodeconf -i` should end with
`Product : Heltec MeshPocket 863 - 928 MHz (d2:ce:46)`. The `46` is the board
byte **reported by the firmware itself** — that is your proof the MeshPocket
build is running and not the T114 build that shares its FQBN.

---

## Hardware notes that will bite you

**The USB-C port is charge-only.** Heltec's own wiki says a standard USB cable
"will not allow the system to recognize the serial port interface", and the
published node schematic carries no USB-C connector — the MCU's USB pins go only
to the pogo header. Flashing works only through the bundled magnetic pogo cable,
and **no replacement source was found** — not on Heltec's store, not a
reseller, not a user offering one. Losing that cable strands the board.

**It cannot be power-cycled.** Read from Heltec's published schematic: the 3V3
regulator's enable is pulled to `BAT+` with no control line, and the cells are
soldered. Heltec's own datasheet confirms the power button leaves "power still
supplied to the wireless communication section"; the deepest documented state is
a software sleep. Running the battery flat is the
only true cold cycle short of opening a glued case.

**`HT-n5262` does not identify this board.** It is Heltec's *module* name,
shared by at least the Mesh Node T114, Mesh Node T1, Mesh Solar and the
MeshPocket. Same product string, same DFU PID `0x0071`, same FQBN. Only the
per-unit USB serial separates them. Two MeshPockets are reported on Heltec's forum as boot-looping
after the wrong board's firmware, unrecovered. **Never let a flasher choose
firmware by board-ID alone.**

**Heltec publishes no bootloader recovery image for this board** — only two
stock Meshtastic UF2s. The sibling T114 directory ships both a bootloader hex
and an erase image. So recovery is reliable only while the bootloader survives.
Never apply a bootloader-update UF2.

**A torn drag-and-drop UF2 may not fall back to DFU.** The bootloader decides an
application is valid partly from a CRC that a previous successful UF2 write can
leave disabled, and partly from whether the first words of the app region are
still erased. On a board previously written by UF2, and with blocks arriving
lowest-address-first, an interrupted copy can therefore leave an image the
bootloader considers valid and jumps into. Recoverable by double-tapping RST, but not
automatic. **Serial DFU erases first and does fail safe.** This is the reason to
prefer it.

**While BLE is connected, USB serial is ignored** — one host at a time, and
Bluetooth wins. `rnodeconf` then reports *"Serial port opened, but RNode did not
respond. Is a valid firmware installed?"*, which right after a flash reads
exactly like a failed flash. Disconnecting in the app was **not** enough — the link
stayed up until the phone's Bluetooth was switched off entirely, which suggests
the OS holds it for a bonded device. Tapping `USR` does not help either: that
toggle is gated on not-being-connected, so while the firmware believes a phone
is attached the button is inert. Turn the phone's Bluetooth off, or
press `RST`. One-step diagnostic: scan for the board — a peripheral stops
advertising while connected, so *not advertising and not answering USB* means
something still holds the link, not a bad flash.

**Pairing** is `USR` held for **more than 5 seconds** — longer than feels
natural. A 6-digit passkey appears on the screen; it is regenerated every boot,
which is cosmetic and does not invalidate an existing bond.

**Qi charging is reported to degrade LoRa reception.** Not measured here, but
worth knowing before diagnosing poor range on a device designed to sit on the
back of a phone while charging it.

---

## For other boards

Several of these fixes are not MeshPocket-specific and are broken for **every**
board on RNode_Firmware_CE. The two SX1262 errata and the TCXO ordering are in
the shared `sx126x` class, so they reach **every SX1262 board including the
ESP32 ones**. The application-size calculation and the four BLE defects are
nRF52-wide. Those are worth taking upstream on their own.

---

## Status

Working and in daily use as a phone-paired RNode. Not merged anywhere; this
branch exists so the work is not lost and so the people above are credited.

Untested: anything other than 915 MHz, and the 5000 mAh variant.
