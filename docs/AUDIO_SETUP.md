# Audio Setup Guide - ESP32-4848S040C_I

## Current Implementation

The current code uses **PWM-based tone generation** for a passive buzzer, NOT the onboard speaker.

## Two Audio Options

### Option 1: PWM Buzzer (Current Implementation)

**Hardware Required:**
- Passive buzzer (piezo buzzer)
- Connect to GPIO2 (or GPIO1/GPIO40)

**Configuration:**
- Already implemented in code
- Enable by uncommenting `#define ENABLE_SOUND` in `src/main.cpp` line 13
- Uses ESP32-S3 LEDC PWM peripheral

**Limitations:**
- Simple tones only (beeps)
- External buzzer required
- Will NOT work with onboard speaker port

### Option 2: I2S Speaker (Onboard NS4168 Amplifier)

**Hardware on Board:**
- NS4168 Class D Audio Amplifier (mono, 2.5W)
- Physical speaker connector (external port)
- I2S interface

**CRITICAL: Hardware Modification Required**

To enable the onboard speaker, you MUST physically move three 0-ohm resistors on the PCB:

| Move From | Move To | Purpose |
|-----------|---------|---------|
| R25 | R21 | GPIO1 → I2S SDATA |
| R26 | R22 | GPIO2 → I2S BCLK |
| R27 | R23 | GPIO40 → I2S LRCLK |

**⚠️ IMPORTANT TRADE-OFF:**
- These GPIOs are shared with the 3 onboard relays
- You can use either the relays OR I2S audio, but NOT both
- Moving the resistors disables relay functionality

**I2S Pin Configuration:**
- **SDATA (Serial Data):** GPIO1
- **BCLK (Bit Clock):** GPIO2
- **LRCLK (Word Select/Left-Right Clock):** GPIO40

## Why Your Speaker Doesn't Work

**Problem:**
You connected a speaker to the external port, but the current code uses PWM on GPIO2.

**Root Cause:**
1. The external speaker port expects **I2S audio** from the NS4168 amplifier
2. The NS4168 requires I2S data on GPIO1, GPIO2, GPIO40
3. By default, these pins are routed to relays (not I2S) via resistors R25, R26, R27
4. The current code generates PWM tones, not I2S audio
5. PWM buzzer code is designed for a separate passive buzzer, not the onboard amplifier

## Solution Options

### A. Use PWM with External Buzzer (Simplest)

1. Keep current code as-is
2. Remove speaker from external port
3. Connect passive buzzer directly to GPIO2 (or GPIO1/GPIO40)
4. Uncomment `#define ENABLE_SOUND` in src/main.cpp line 13
5. Rebuild and upload

**Pros:** No hardware modification, simple implementation
**Cons:** Need external buzzer, simple tones only

### B. Enable I2S for Onboard Speaker (Requires Soldering)

1. Open the device (remove back panel)
2. Locate resistors R25, R26, R27 on PCB
3. Use soldering iron to move:
   - R25 → R21
   - R26 → R22
   - R27 → R23
4. Modify code to use I2S instead of PWM
5. Rebuild and upload

**Pros:** Uses onboard speaker, better sound quality, supports complex audio
**Cons:** Requires soldering skills, disables relay functionality, more complex code

### C. No Audio (Visual Feedback Only)

Keep sound disabled - the visual feedback (white/red flashes) provides excellent user experience without audio.

## Recommendation

Since you have a speaker connected to the external port, you have two choices:

1. **If you haven't moved the resistors yet:**
   - Don't move them (avoids hardware modification)
   - Get a passive buzzer and connect to GPIO2
   - Enable PWM sound in code

2. **If you want to use the onboard speaker:**
   - Physically move resistors R25→R21, R26→R22, R27→R23
   - I can implement I2S audio code for you
   - Note: This disables the 3 onboard relays

## References

- [ESP32-4848S040C_I Audio Details](https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/)
- [NS4168 Datasheet](https://michiel.vanderwulp.be/domotica/Modules/SmartDisplay-ESP32-S3-4.0inch/SCT-NS4168.pdf)
- [GitHub Discussion: Relay GPIO Pins](https://github.com/HASwitchPlate/openHASP/discussions/681)
