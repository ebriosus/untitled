# untitled

Enviroment: 
Windows10  x64
What you need:
1.openframeworks 0.98ver
   1.1 ofxGui
   1.2 ofxOculusRiftCV1 https://github.com/secondstory/ofxOculusRiftCV1
   1.3 ofxOvrvision     https://github.com/kuanju/ofxOvrvision
   1.4 ofxRealsense     https://github.com/tyhenry/ofxRealSense
   1.5 ofxYolo2         https://github.com/lab101/ofxYolo2
2.OvrvisionPro SDK
3.Oculus Runtime for Windows
4.Yolo v2 for Windows https://github.com/AlexeyAB/darknet (if you want to modify yolo you should download from here)
4.Realsense SDK R2 for windows    
How to build(Release x64):
   4.1 Generate openframeworks project with addons metioned above
   4.2 What you need to attach in the include option:
         4.2.1 "include"(which stores your yolo_cpp_dll.hpp file)
         4.2.2 the include path of "ovrvision_pro.h" (which should be in your ovrvision sdk)
         4.2.3 the include path of realsense (for me, its like C:\Intel\RSSDK\include\)
   4.3 What you need to attach in the Linker
        For Additional Lib Dir:
        4.3.1 "dll"  (which stores yolo's dll and lib)
        4.3.2  "..\Intel\RSSDK\lib\$(Platform)" //please adjust to your own path
        4.3.3  "..\OvrvisionPro\bin\x64"   //please adjust to your own path
        4.3.4  $(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVR\libs\Windows\x64\Release\VS2015
        4.3.5  $(OF_ROOT)\addons\ofxOculusRiftCV1\libs\LibOVRKernel\libs\Windows\x64\Release\VS2015
        For Input : 
        4.3.6  yolo_cpp_dll.lib;LibOVR.lib;libOVRKernel.lib;ovrvision64.lib
   4.4 Remove "Tracing" "Util" folders at ..\addons\ofxOculusRiftCV1\libs\LibOVRKernel\src  from your project
   4.5 Modify in addon code:
       you need to inplace some addon files with files in the Modified floder (to get some modified function work)
   Now the build should pass
  
 
        
