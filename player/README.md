# LHE Pi Player
 This is the player implementation for the LHE Pi codec hosted in the same repository. This player is targeted to Ubuntu platform. :blush:

## Dependencies
SDL 2 library
Openmp

## Compiling Ubuntu
1. Install  libraries. In Ubuntu `sudo apt-get install libsdl2-dev libgomp-dev`
2. Go to the player directory and run `make`
3. The binary is located in the same folder. The name is lhe_rpi_player

## Usage
 The following options can or must be supplied to the program.

--fullscreen or -f. Optional flag in order to turn on the fullscreen mode. Default is non fullscreen mode.

--input or -i. The path to the filename to decodificate. Either --filename or --stdin must be supplied.

--stdin or -s. Sets the decoder to read from the stdin the binary stream. Default if -s or -i are not present.

--output or -o. Decodes each frame into a file. The string supplied must end in .bmp as this is the format used. Can contain a wildcard in the form "%d" in order to store secuantially all the frames.

```
lhe_pi_player  (defaults -s)

lhe_pi_player --stdin --output "test-%d05.bmp"
```

## FAQ


## Known Bugs :disappointed_relieved:
