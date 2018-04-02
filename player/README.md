# LHE Pi Player
 This is the player implementation for the LHE Pi codec hosted in the same repository. This player is targeted to windows platform. :blush:

## Dependencies
SDL 2 library is neccesary for the execution. [Get it here](https://www.libsdl.org/download-2.0.php)

## Compiling
1. Dump the files into a Visual Studio Solution. 
2. Configure the project to link against SDL. [Dummy tutorial including SLD](https://www.wikihow.com/Set-Up-SDL-with-Visual-Studio-2017)
3. Compile it.

## Usage
 The following options can or must be supplied to the program.

--width or -w. In order to indicate the width of the incoming stream. Must be supplied.

--height or -h. To comunicate the height of the incoming stream. Must be supplied.

--filename or -f. The path to the filename to decodificate. Either --filename or --stdin must be supplied.

--stdin or -s. Sets the decoder to read from the stdin the binary stream. Either --filename or --stdin must be supplied.

```
lhe_pi_player.exe -h 240 -w 320 --stdin
lhe_pi_player.exe --height 360 -width 640 --filename "C:/Users/quinta/Desktop/binary-file.bin"
```
## FAQ




