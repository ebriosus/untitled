# untitled

Enviroment:<br />
Windows10 x64<br />
What you need:<br />
1.openframeworks 0.98ver<br />
   1.1 ofxGui<br />
   1.2 ofxOculusRiftCV1 https://github.com/secondstory/ofxOculusRiftCV1<br />
   1.3 ofxOvrvision     https://github.com/kuanju/ofxOvrvision<br />
   1.4 ofxRealsense     https://github.com/tyhenry/ofxRealSense<br />
   1.5 ofxYolo2         https://github.com/lab101/ofxYolo2<br />
2.OvrvisionPro SDK<br />
3.Oculus Runtime for Windows<br />
4.Yolo v2 for Windows https://github.com/AlexeyAB/darknet (if you want to modify yolo you should download from here)<br />
4.Realsense SDK R2 for windows    <br />
How to build(Release x64):<br />
   4.1 Generate openframeworks project with addons metioned above<br />
   4.2 What you need to attach in the include option:<br />
         4.2.1 "include"(which stores your yolo_cpp_dll.hpp file)<br />
         4.2.2 the include path of "ovrvision_pro.h" (which should be in your ovrvision sdk)<br />
         4.2.3 the include path of realsense (for me, its like C:\Intel\RSSDK\include\)<br />
   4.3 What you need to attach in the Linker <br />
        For Additional Lib Dir: <br />
        4.3.1 "dll"  (which stores yolo's dll and lib)<br />
        4.3.2  "..\Intel\RSSDK\lib\$(Platform)" //please adjust to your own path<br />
        4.3.3  "..\OvrvisionPro\bin\x64"   //please adjust to your own path<br />
        4.3.4  $(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVR\libs\Windows\x64\Release\VS2015<br />
        4.3.5  $(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVRKernel\libs\Windows\x64\Release\VS2015<br />
        For Input : <br />
        4.3.6  yolo_cpp_dll.lib;LibOVR.lib;libOVRKernel.lib;ovrvision64.lib<br />
   4.4 Remove "Tracing" "Util" folders at ..\addons\ofxOculusRiftCV1\libs\LibOVRKernel\src  from your project<br />
   4.5 Modify in addon code:<br />
       you should replace some addon files with files in the Modified floder (to get some modified function work)<br />
   Now the build should pass<br />
   <br />
To run : <br />
  Put ovrvison.dll OpenAl32.dll yolo_cpp_dll.dll under bin folder <br />
  Put yolo weights file (coco version) under bin/data <br />
 
        
