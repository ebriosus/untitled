#pragma once

#include "ofMain.h"
#include "ofxYolo2.h"
#include "ofxRealSense.h"
#include "ofxOculusRiftCV1.h"
#include "ofxGui.h"
#include "OvrPro.h"

//to restore and position and size data of those objects which will show at OculusCv1
struct posAndSize_
{
	float xpos;
	float ypos;
	float zpos;
	float sizeWidth;
	float sizeHeight;
	int number;
};

class ofApp : public ofBaseApp {


	ofTexture lefteye;
	ofTexture righteye;
	ofTexture realsensergb;

	int whicheye = 0;

	//ofTrueTypeFont verdana;	
	ofxYolo2 yolo;
	
	ofPixels colorFromRealsense;
	ofRectangle bounds;
	ofVec3f position;
	ofQuaternion orientation;
	
	struct axis_
	{
		ofVec3f x_axis;
		ofVec3f y_axis;
		ofVec3f z_axis;
    };

	axis_ axis;
    
	std::vector<bbox_t> lastResults;
    std::vector<posAndSize_> positionAndSizeOfObj_;

	float probability;

	ofParameter<float> x_parameter;
	ofParameter<float> y_parameter;
	ofParameter<float> z_parameter;
	ofParameter<float> zd_parameter;
	ofParameter<int>  size_parameter;
	ofParameter<float> xd_parameter;
	ofParameter<float> yd_parameter;
	ofParameter<float> ztexture;
	ofParameter<float> gap;
	ofxPanel gui;
	
	ofImage obj[80];

public:
	void setup();
	void update();
	void draw();
	void exit();

	void drawSceneLeft();
	void drawSceneRight();


	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	std::vector<posAndSize_> getPositionOfObj(const std::vector<bbox_t>& lastResults, float prob);
	void drawNameAndBoxFromYolo(const std::vector<bbox_t>& lastResults, float prob);     //draw box on screen, the range of prob is from 0 to 1;
	void billboarding(ofVec3f & position, axis_ & axis);
	void drawplane(const std::vector<posAndSize_>& positionAndSizeOfObj_, const axis_ & axis);
	
	void drawovrvisionsceneleft();
	void drawovrvisionsceneright();
	
	
	void drawrealsensescene(const axis_  axis);
	
	ofxRealSense realsense;
	ofTexture colorInDepth, depthInColor;
	ofxOculusRiftCV1 cv1;
	
	OvrPro ovrPro;

	ofPoint tlx;
	ofPoint tly;
	ofPoint brx;
	ofPoint bry;
	ofPoint tlx1;
	ofPoint tly1;
	ofPoint brx1;
	ofPoint bry1;

};