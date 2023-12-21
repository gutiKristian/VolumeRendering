# DCM lib
For some reason when using kPixelData tag and Get[DataType]Array function (e.g. GetUint16Array)
to read data it doesn't update the tag and length is set to zero druing the read operation.
Workaround is to inside data_element.h on line 224 make VR mutable and then call vr_.set_code(vr.code()); 
inside bool GetNumberArray(VR vr, std::vector<T>* values) const function (line 195 cca).

# About DICOM

Sourced from ChatGPT

## Shortcuts in tags, defs.h file (DCM)

- OB, binary data, sequence of bytes without any specific structure or interpretation (e.g. pixel data tag)
- AE, text strings, specifically application entity titles, such as devices, systems, or applications
- IS, integer,  (for DCM use GetInt)
- CS, code string (GetString)
- US, unsigned short
- LT, text strings of potentially unlimited length

## Bits Stored and Bits Allocated
### Bits Allocated
- DICOM attribute that specifies the number of bits used to represent each pixel in the image
### Bits Stored
- DICOM attribute related to the pixel data, it indicates the number of bits that are actually used to represent the pixel values

## Photometric interpration

In DICOM (Digital Imaging and Communications in Medicine), "Photometric Interpretation" (tag 0028,0004) is an attribute that describes how 
the pixel data is intended to be interpreted and displayed in medical images. It provides information about the color representation and 
the relationship between pixel values and image colors. The photometric interpretation is especially relevant for images that contain color information, 
such as multi-frame images or images from modalities like ultrasound, endoscopy, or microscopy.

The "Photometric Interpretation" tag typically contains one of the following values:

### MONOCHROME1:
This value indicates that the image is monochromatic (grayscale), and higher pixel values correspond to darker shades, with 0 typically representing white or background.

### MONOCHROME2: 
Like "MONOCHROME1," this value represents a monochromatic image, but higher pixel values correspond to brighter shades, 
with 0 typically representing black or background. "MONOCHROME2" is the more common representation for grayscale images.

### PALETTE COLOR:
This value indicates that the image uses a color palette. In this case, the image is typically represented in a pseudocolor 
format where pixel values correspond to indices in a predefined color lookup table (palette).

### RGB:
This value indicates that the image is represented in full color, with each pixel having separate Red, Green, and Blue color components. 
In this mode, each pixel typically has multiple values to represent the intensity of the three color channels.

### YBR_FULL:
This value represents a full color image using a YCbCr color space, which is a color encoding system where "Y" represents luma (luminance) and 
"Cb" and "Cr" represent chroma (color difference) information.

## Pixel spacing
Information about the physical size of the individual pixels in a medical image.
It specifies the spacing between adjacent pixel centers in the horizontal (x) and vertical (y) dimensions of the image.
The "Pixel Spacing" attribute consists of two numerical values separated by a backslash ("/),
like "0.2\0.2" (in which "0.2" could represent, for example, 0.2 millimeters).
For example, if the Pixel Spacing is "0.2\0.2," and the image has 512 rows and 512 columns,
you can calculate that the image covers a physical area of 102.4 mm x 102.4 mm (0.2 mm x 512 rows in the vertical direction and 0.2 mm x 512 columns in the horizontal direction).