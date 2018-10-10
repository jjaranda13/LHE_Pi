# LHE Pi Player
 This is the player implementation for the LHE Pi codec hosted in the same repository. This player is targeted to Ubuntu platform. :blush:

## Dependencies
SDL 2 library is neccesary for the execution. [Get it here](https://www.libsdl.org/download-2.0.php) or download it from Ubuntu repositories.

## Compiling Ubuntu
1. Install SDL libraries. In Ubuntu `sudo apt-get install libsdl2-dev`
2. Go to the player directory and run `make`
3. The binary is located in the same folder. The name is lhe_rpi_player

## Usage
 The following options can or must be supplied to the program.

--width or -w. In order to indicate the width of the incoming stream. Must be supplied.

--height or -h. To comunicate the height of the incoming stream. Must be supplied.

--fullscreen or -f. Optional flag in order to turn on the fullscreen mode.

--input or -i. The path to the filename to decodificate. Either --filename or --stdin must be supplied.

--stdin or -s. Sets the decoder to read from the stdin the binary stream. Either --filename or --stdin must be supplied.

```
lhe_pi_player.exe -h 240 -w 320 -f --stdin
lhe_pi_player.exe --height 360 -width 640 --input "C:/Users/quinta/Desktop/binary-file.bin"
```
## FAQ

## Known Bugs :disappointed_relieved:
