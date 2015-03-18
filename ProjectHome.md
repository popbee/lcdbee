"Arduino/ChipKit" Graphic library for LCD.

Built for a 320x240 ILI9325 controller bundled in a ITDB02 module from ITead Studio. (in 16 bit mode)

Nothing is committed yet. Check this space for updates or send me a message if there is interest for a specific LCD controller/Arduino board.

This library has unique features and the speediest implementation I have seen so far for both Arduino and Chipkit.

  * Fonts: Compact font format, anti-aliased option, partial overlap of glyphs, generated from any True-Type font, standard UTF-8 encoding (support for Latin-1 unicode from 0x0000 to 0x00FF a.k.a. ISO/IEC 8859-1), cherry pick exactly which letter to include for maximum compactness.
  * Lines: anti-aliased lines, start and stop coordinates not centered to pixel (good for complex & detailed drawing).
  * Alpha Blending: Different alpha-blending algorithms for different composition use cases.
  * Attempt to follow the style of the Processing graphics API (when possible).
  * Rectangle clipping region