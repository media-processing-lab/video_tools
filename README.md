# Video tools for Video Encoding Research

## difftool

### Requirements

* Only support Little Endian ISA (e.g. amd64)

### Build

```
make
```

### Usage

```
Usages
Generate difference image     : ./diff.out -g -a INPUT_A -b INPUT_B -o OUTPUT_DIFFERENCE_IMAGE [-w IMAGE_WIDTH] [-h IMAGE_WIDTH] [-f FRAME_COUNT]
Recover from difference image : ./diff.out -r -b INPUT_B -d INPUT_DIFFERENCE_IMAGE -o OUTPUT_RECOVERED_IMAGE [-w IMAGE_WIDTH] [-h IMAGE_WIDTH] [-f FRAME_COUNT]

Pixel value calculation
  Generate : d = (a - b + offset) & mask
  Recover  : a = (b + d - offset) & mask

Limitations
  * This program supports only yuv420p10le pixel format.

Options

Mode selection
  -d : Specify generating mode.
  -r : Specify recovering mode.

Input and Output
  -a : Specify input A RAW yuv image filename.
  -b : Specify input B RAW yuv image filename.
  -d : Specify input difference yuv image filename.
  -o : Specify output image filename.
  -w : Specify image width. (default : 3840)
  -h : Specify image height. (default : 2160)
  -f : Specify number of frames in image. (default : 1)
```

## generate_histogram

### Requirements

* Python > 3.9
* Matplotlib

### Usage

```
generate_histogram.py [-h] --input INPUT --output OUTPUT
```
