# LHE_Pi
implementation of codec LHE for Raspberry Pi

## Compiling
1. Go to the player directory and run `make`
2. The binary is located in the same folder. The name is lhepi.

## Usage
 The following options can or must be supplied to the program.

-rtp. The coder sent the H.264 initiationsequence. Defaults to false

-pppx. Tells the coder how many pppx must be set. Defaults to two.

-pppy. Tells the coder how many pppy must be set. Defaults to two.

--input or -i. The path to the filename to decode.If sequence option is set it will substitute "%d" for each iteration. If not present rapi camera is used.

--sequence or -s. Sets decoder in sequence mode. Use to code sequence of images. A touple must be supplied in the form "init:lenght" to tell where to start and how many picture to code.

```
lhepi -rtp 
lhepi --input "lena.bmp"
lhepi -i "lena-%07d" -s 1:300
lhepi -rtp -pppx 1
```