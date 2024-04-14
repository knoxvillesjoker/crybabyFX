# crybabyFX
"CryBaby" Prop FX  - a compact, customizeable Arduino-based soundboard and effects driver.

Dependencies: Requires Adafruit's Soundboard, LED Backpack, and GFX libraries present in your Arduino IDE in order to compile.

Old Version: 1.0
- Stable and usable, with muzzle strobe, LED alphanumeric display, and serial audio control all functional

Current Version: 2.0
- Added second display, grenade firing, counter, fire select modes, removed unused code
- Reprogramed for Adrduino pro Micro/Leonardo

PLANNED CHANGES:

For next build (1.1):
- Add music support.  There are some currently unused switch positions on the front control grip that I would like to use for audio playback.  Since the audio board is running in UART Serial mode for faster response, this will need to be done via software logic.
- Remove all uses of delay() function.  For smoother operation, I want to replace all delay() calls with millis() timer loops.  Need to verify that Pro Trinket is capable of this amount of "multitasking", it might not be.
- Re-tweak audio. Reminder: Firing "Tail" sound is currently disabled.  This will be re-enabled as part of the audio loop cleanup.
- v2.0 removed firing tail, 

For 1.2: 
- Create a Pulse Rifle branch of the code
- This has been addressed in version 2.0

Future:
- Create a "Barebones" branch supporting MOSFET LED control, LED alphanumeric display, serial audio control, and non-blocking timer loops, intended for customization by the end user to meet the needs of a specific prop.
- Most of these have been addressed in version 2.0 code
