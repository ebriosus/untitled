# untitled

## Environment

- Windows10 x64
- Visual Studio Community 2015

### devices

- Oculus Rift CV1
- Ovrvision Pro
- Intel Realsense SR300 (new version)

## Setup

### oF and addons

- openFrameworks v0.9.8
- ofxGui

download and put them into <$OFROOT>

- [ofxOculusRiftCV1](https://github.com/secondstory/ofxOculusRiftCV1)
- [ofxOvrvision](https://github.com/kuanju/ofxOvrvision)
- [ofxRealsense](https://github.com/tyhenry/ofxRealSense)
- [ofxYolo2](https://github.com/lab101/ofxYolo2)

### Other Libraries

#### Requirement

- Download and unzip [OvrvisionPro SDK for Windows](http://ovrvision.com/setup/) anywhere you want (for example `C:\libraries\ovrvisionprosdk_windows\`)
- Install [Oculus Runtime for Windows](https://www.oculus.com/setup/)
- Install [Intel® RealSense™ Camera SR300 Firmware](https://software.intel.com/en-us/realsense-sdk-windows-eol)
- Install [Realsense SDK 2016 R2 for windows 2016 R2](https://software.intel.com/en-us/realsense-sdk-windows-eol)

#### if you need

- Download and build [Yolo v2 for Windows](https://github.com/AlexeyAB/darknet) (if you want to modify yolo)

## How To Build (Release x64)

### Generate oF project

- Generate openFrameworks project with addons mentioned above

### Add additional include paths

`Project Property > Configuration Properties > C/C++ > General > Additional Include Directories`

- The pass to `yolo_cpp_dll.hpp` (I guess it is `yolo_v2_class.hpp` ?)
- The pass to `ovrvision_pro.h` (which should be in your ovrvision sdk)
- the include path of realsense (for me, its like `C:\Intel\RSSDK\include\`)

### Add Additional Library Directories

`Project Property > Configuration Properties > Linker > General > Additional Library Directories`

- `.\dll` (which stores yolo's dll and lib)
- Realsense lib directory (like `C:\Program Files (x86)\Intel\RSSDK\lib\x64` //please adjust to your own path)
- Ovrvision Pro SDK bin/x64 directory (for example `C:\libraries\ovrvisionprosdk_windows\bin\x64`)
- ofxOculusRiftCV1 libraries
  - `$(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVR\libs\Windows\x64\Release\VS2015`
  - `$(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVRKernel\libs\Windows\x64\Release\VS2015`

### Add .lib file

`Project Property > Configuration Properties > Linker > Input > Additional Dependencies`

- `yolo_cpp_dll.lib`
- `LibOVR.lib`
- `libOVRKernel.lib`
- `ovrvision64.lib`

### Remove obstacle files

- `$(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVRKernel\src\Tracing`
- `$(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVRKernel\src\Util`

### Replace edited files in ofxAddons/libraries.

you should replace some addon files with files in the `Modified` floder (to get some modified function work)

### Build

`Build > Build Solution (Ctrl+Shift+B)`

## How To Run (x64)

### Put DLLs in bin folder

- `ovrvison.dll`
- `OpenAl32.dll`
- `yolo_cpp_dll.dll`

### Put YOLO needed files under `bin/data`

- under construction...
