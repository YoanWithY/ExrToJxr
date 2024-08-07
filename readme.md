# Exr to Jxr
This is a little softwaretool for Windows that converts single layer .exr files to a .jxr. The tool automatically figures out the floating point format (16bit or 32bit) and the number of components and outputs the .jxr in the same format. This tool does not do any color transformation, it only copies the data from the one format to the other.

## Usage
The program is a CL tool, so you can do:
```shell
./ExrToJxr.exe "input.exr" "output.jxr"
```

## Format Support
If the .exr file has a format that is not supported by .jxr, the error `Failed: Pixel format not supported.` will be displayed an no output will be generated. This is known to happen for 32bit float with RGB color components.