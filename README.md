I wasn't satisfied with the Logitech Saitek Throttle quadrant and wanted something that more closely simulated the experience of flying a Cessna 150/152. So, taking inspiration from other projects (including this one) I set about designing and building my own from scratch. It uses an Arduino Leonardo board and works flawlessly in X-Plane as I'm sure it would in MSFS.  Things You'll Need (other than a good load of PLA filament and a bit of patience):

	•	Arduino Leonardo board (Amazon)
	•	10k Sliding Potentiometers (AliExpress)
	•	Rotary Encoder (Amazon)
	•	5mm Stainless Steel Rod (AliExpress)
	•	3x PG7 Stainless Steel Cable Glands (AliExpress)
	•	Self Returning - Toggle Switch (Amazon)
	•	M3 - 6mm Nuts (Amazon)
	•	M3 Heat Set Inserts (AliExpress)
	•	M4 Heat Set Inserts (from same bundle as M3 ones above)
	•	Dupont cables (AliExpress)
	•	JST Connectors (if you want to use them instead / alongside the dupont cables) (Amazon)

Assembly Instructions

	1	With your parts all printed begin by setting 4x M4 heat set inserts in the underside of the base - this is where you will use the existing screws from the Logitech Saitek mount if you're using this.
	2	Set 4 M3 heat set inserts on the inside per controller - this is where you'll mount each of the potentiometers. Note the mount holes are set out specifically for the potentiometers listed above that come soldered on a board which saves a bunch of time.
	3	Set 6 M3 heat set inserts on the left hand side of the main case - this is where you'll mount the rotary encoder (left) and the arduino board (right).
	4	Set 16 M3 heat set inserts around the perimeter of the case - this is where the lid will mount to the main case. Note, I went overboard on the screws here as it was the aesthetic I was going for, feel free to use less.
	5	I recommend you test everything before final assembly as the wiring can be a bit fiddly
	5.1	Join the VCC pin of each of the pots to the + pin on the rotary encoder and connect all of that to 5v on the arduino board
	5.2	Join the GND pin of each of the pots to the centre pin on the toggle switch and to the GND pin on the rotary encoder then connect that up to GND on the arduino board
	5.3	Connect DTB of what will be the left pot to A0 on the arduino board
	5.4	Connect DTB of what will be the central pot to A1 on the arduino board
	5.5	Connect DTB of what will be the left pot to A2 on the arduino board
	5.6	Connect CLK on the rotary encoder to D2 on the arduino board
	5.7	Connect DT on the rotary encoder to D3 on the arduino board
	5.8	Connect the up contact of the toggle switch to D6 on the arduino board
	5.9	Connect the down contact of the toggle switch to D7 on the arduino board
	6	With everything connected - flash the debug file to the arduino board and whilst connected via USB, test all the controls using the serial output monitor, you should be able to see values change for each of the controls.
	7	Disconnect your arduino board and mount this to the main case along with the rotary encoder (with the pins of the rotary encoder facing downwards) and the toggle switch in the furthest right hole.
	8	Mount the 3 stainless steel cable glands to the 3 remaining holes.
	9	Wire everything as you like for the final assembly, you might want to use JST connectors etc. for the various connections, you may also find that standard lengthy dupont cables aren't long enough so ceating your own wire lengths might be preferable. However you do it, ensure there is enough space for the trim wheel to fix to the rotary encoder. I had to bend the wires flat to the headers on the board. Don't forget to connect the USB cable via the pass-through hole in the case before mounting it as this will be easier.
	10	Attach the slider brackets to the pots with the hole facing where the rods will come in via the cable glands.
	11	Cut your stainless steel rods to your preference, mine ended up being around 150mm in length but varied based on where I wanted each control to be.
	12	The knobs are a snug fit but feel free to superglue them to the rod for extra security.
	13	Use the usual clamping nuts that came with the cable glands for the left and right control but use the 3D printed ‘friction nut’ for the middle one if you'd like to simulate the friction changing capabilities of the real thing (this is my favourite touch!)
	14	Pass the rods through the cable glands and connect up to the slider brackets - these are better off super glued as the frequent pushing and pulling may lead to them coming loose without being glued.
	15	The flaps switch is a bit tricky as the switch itself is tapered and the print just has a straight cavity to connect it. What I had to do was print tiny wedges that I could super glue to the top and bottom face of the metal switch which I then used to apply a bit glue to before sliding the printed switch over and wedging it a bit. If you add this part just take care to not put the wedges or the switch part to far down the metal shaft as it will block the switch movement.
	16	The last thing (although it doesn't necessarily need to be done in this order) is to attach the trim wheel to the rotary encoder. I did this by passing it through the slot in the case, it took a bit of forcing but it went in the end.
	17	Flash the final firmware code to the arduino board and finally, attach the lid to the case with the (many) hex screws.

Unfortunately, I didn't document it very well as I was making it and don't feel like deconstructing it so don't have that many photos to help but have uploaded what I have - it's a fairly straightforward build though so shouldn't be too taxing.

If you're still with me by this point… Nice.

Feel free to tweak the code to your preference, specifically, you can adjust the sensitivity of the trim wheel, making it send more or less turn commands per turn by changing the values on the following lines:

 // ---------------- User options ---------------- 
 const uint16_t TRIM_PULSE_MS = 85; // length of each trim button pulse 
 const int8_t ENC_FULL_STEP = 2; // fragments per detent (try 2 or 4 as needed)

If there's enough interest I'll be happy to provide some instructions on setting up a .joy profile in X-Plane to get a nice interface for mapping the controls inside X-Plane.
