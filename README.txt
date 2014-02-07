Bong is a FreeWare game for BeOS,  by Gertjan van Ratingen

This is version 1.4 for R4


Description

Most of you will probably recognize it, as it's based on the old
arcade classic PONG.

This program was originally intended to be a joystick sample app.
but somewhere on the way it has morphed itself into a pong clone.

Bong can use two joysticks in both BeBox ports, or a splitter
with two joysticks in either port.
(I've made my own controls using slider-pots, see Remarks below)

The program reads the values of the joysticks, and stores the minimum
and maximum values to determine the active range of the stick.

For the joystickless BeOSers, there's keyboard support.



Usage

Bong can be started like any other application by double-clicking
it's icon.

The main menu can be reached by clicking your mouse inside the court.
This menu lets you change Bong's settings. Just play around with it,
and find out what each item does :)

If you play using the keyboard, here's the controls:
Left player: left Shift-key and Ctrl-key
Right player: right Shift-key and Option-key (a.k.a. right Ctrl-key)

Select "New Game" from the menu to start a new game.
Play ends if one of the players has scored 15 points.



Remarks

- Topspin anyone? Bong looks at the movement of the bat, and adjusts
  the angle of the ball if needed.

- If you quit Bong, the current settings are saved to a file
  (in /boot/home/config/settings/Bong_prefs) for the next start of Bong.
  Todo: use PrefServer or libprefs.

- Bong tries to read the audio-samples (intro.raw, lbat1raw...lbat8.raw,
  rbat1.raw...rbat8.raw, wall1.raw...wall8.raw, over.raw and out.raw)
  from the directory 'bongdata' where the executable is started.
  If a soundfile cannot be read, Bong will use a beep instead.
  Todo: add the soundeffects as resources to the executable.

- Would you like to make your own sounds? The sampleformat is
  8 bit signed, mono, 11025 Hz, raw (no header).

- If you want a nice controller for this game, and you're
  handy with a soldering iron, just get two slider-pots of 100 k,
  4 resistors or 33 k, one or two 15-p male sub-d connectors
  and some wire.

  For a "split joystick in port 1":

	               1
	                  9     For a description of the ports, see
	               2        the Be User's Guide for DR8, page 194
	                 10
	               3           33 k
	   33 k          11      +-/\/\-+
	 +-/\/\-+      4     GND |      |
	 |      | GND    12 -----+     +-+
	+-+     +----- 5               | | 100 k
	| |100k          13 ---------->| |
	| |<---------- 6     Y1        | |
	| |        Y0    14            +-+
	+-+            7                |
	 |               15 -----/\/\/--+
	 +--/\/\------ 8    +5v   33 k
	    33 k   +5v

For "Joystick in ports 1 and 2" only connect the left pot
(pins 5, 6 and 8) and make another cable the same way.

Note: in this drawing, moving the slider "down" sends a higher
      voltage to pin 6, and makes the bat move "up".
      Remember that when using this setup ;-)


You're welcome to send any comments to me by e-mail:

		gertjan@iae.nl  (home)
	or
		gj@codim.nl  (work)



Disclaimer

Officially:
	Use it at your own risk. I know I do!

Practically:
	Enjoy the game, it won't ruin your BeBox.
	It only takes a small bit of your precious time ;-)

