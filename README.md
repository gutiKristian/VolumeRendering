# Volume rendering using WebGPU
The application uses Google's Dawn implementation of the WebGPU. Compilation for the web using Emscripten **has not been tested**. There's still work to be done in the underlying framework.
The primary focus is on DICOM imaging datasets, but the project also supports RTStruct datasets containing contours.
These contours can be converted into 3D masks, which can then be visualized alongside the imaging data.

---

<div align="center">
  <img src="https://github.com/gutiKristian/VolumeRendering/assets/45439799/cce91243-5c6d-404b-ae53-c13669c52659" alt="Volume Rendering Example">
</div>
<br>
Visualization of anatomical data (CT scan), 3D mask (generated from segmentation/contour data), and RTDose that is used to highlight the radiation dosage.
To enhance overall visuals, Blinn-Phong illumination can be incorporated (normals approximated using gradients). This image includes Blinn-Phong illumination using gradient-based shading.

---

<div align="center">
  <img src="https://github.com/gutiKristian/VolumeRendering/assets/45439799/936c782a-2db5-4319-b91d-11ff7e41ed60" alt="Volume Rendering Example">
</div>
<br>
The application also supports one-dimensional transfer functions for opacity and color.

---

<div align="center">
  <img src="https://github.com/gutiKristian/VolumeRendering/assets/45439799/f8cc7807-2f33-4d93-af27-569183301a9b" alt="Volume Rendering Example">
</div>
<br>

Comparison of non-illuminated and illuminated visualizations.

---


## Setup
Prerequisites
Boost (for DCM lib)

1. Clone non-recursively
2. Run Init.py
3. DCM library requires boost filesystem and system. If cmake cannot find your boost installation set BOOST_ROOT inside vendor/dcm/CMakeLists.txt and App/CMakeLists.txt

