# Volume rendering using WebGPU
The program uses Google's Dawn implementation of the WebGPU. Compilation for the web using Emscripten **has not been tested**, there's still work to be done in the underlying framework.
The primary focus is on DICOM imaging datasets, but the project also supports RTStruct datasets containing contours.
These contours can be converted into 3D masks, which can then be visualized alongside the imaging data.

## Setup
Prerequisites
Boost (for DCM lib)

1. Clone non-recursively
2. Run Init.py
3. DCM library requires boost filesystem and system. If cmake cannot find your boost installation set BOOST_ROOT inside vendor/dcm/CMakeLists.txt and App/CMakeLists.txt

