#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	realsense.setup(true, true); // setup realsense camera
	realsense.open();            //open the camera
	
	yolo.setup();                //set up yolo
	probability = 0.0;           //when the result's prob larger than this ,the box shows;  
	
	ovrPro.init();
    //set up and open OculusRiftCv1
    bool bOK = cv1.init(); 
    if (!bOK) {
	   ofLogNotice() << "OculusRiftCV1 failed to initialize";}
	else{
		ofLogNotice() << "Initialized OculusRiftCV1";
		bounds = cv1.getHMDSize();
	}
	
	x_parameter.set("x_parameter", 1, 0, 5.0);
	xd_parameter.set("xd_parameter", -0.02, -1.0, 1.0);
	y_parameter.set("y_parameter", 0.425, 0, 5.0);
	yd_parameter.set("yd_parameter", 0.12, -1.0, 1.0);
	z_parameter.set("z_parameter", 770, 500, 2000);
	zd_parameter.set("zd_parameter", 0.14, 0, 2.0);
	size_parameter.set("sizeparameter", 100000, 100000, 1000000);
    ztexture.set("ztexture", 0.86, 0, 4);
	gap.set("gap", 0.064, -0.2, 0.2);
	gapy.set("gapy", 0, -0.2, 0.2);

	gui.setup();
	gui.add(x_parameter);
	gui.add(xd_parameter);
	gui.add(y_parameter);
	gui.add(yd_parameter);
	gui.add(z_parameter);
	gui.add(zd_parameter);
	gui.add(size_parameter);
	gui.add(ztexture);
	gui.add(gap);
	gui.add(gapy);

	
	for (int i = 0; i < 80; i++) {
		obj[i].loadImage("1 - コピー (" + ofToString(i) + ").png");
	}

	colorFromRealsense.allocate(640, 480, OF_PIXELS_RGB);
	lefteye.allocate(ovrPro.ovr_camWidth, ovrPro.ovr_camHeight, GL_RGBA);
	righteye.allocate(ovrPro.ovr_camWidth, ovrPro.ovr_camHeight, GL_RGBA);
	
	//depthInColor.allocate(640,480,GL_LUMINANCE);
	//setup for font 
	// ofTrueTypeFont::setGlobalDpi(72);
	// verdana.load("verdana.ttf", 18); // フォントのデータを指定する
	// verdana.setLineHeight(24);       // 行間を指定する
	// verdana.setLetterSpacing(1.0);
}

//--------------------------------------------------------------
void ofApp::update(){
	    realsense.update(); //update for realsense camera	
        
		colorFromRealsense = realsense.getColorPixelsRef();    

	    //depthInColor.loadData(realsense.getDepthPixelsInColorFrame());
		//realsensergb.loadData(colorFromRealsense);            
		
		lastResults = yolo.detect(colorFromRealsense);          //recive detection results from yolo
		//lastResults = yolo.tracking(lastResults);                   //add tracking option(optional)
		
		positionAndSizeOfObj_ = getPositionOfObj(probability);//caculate the depth for objects
		
		cv1.update();      //update OculusCv1 for posistion and orientation?
		
		ovrPro.update();
		
		lefteye = ovrPro.getTextureLeft();
		righteye = ovrPro.getTextureRight();
		
		billboarding(position, axis);        //billboarding

        tlx = (axis.x_axis * -.960 + axis.y_axis * 0.95);
		tly = (axis.x_axis * .960 + axis.y_axis * 0.95);
		brx = (axis.x_axis * .960 + axis.y_axis * -0.95);
		bry = (axis.x_axis * -.960 + axis.y_axis * -0.95);
		tlx1 = (axis.x_axis * (-.960 + gap) + axis.y_axis * (0.95 + gapy));
		tly1 = (axis.x_axis * (.960 + gap) + axis.y_axis * (0.95 + gapy));
		brx1 = (axis.x_axis * (.960 + gap) + axis.y_axis * (-0.95 + gapy));
		bry1 = (axis.x_axis * (-.960 + gap) + axis.y_axis * (-0.95 + gapy));
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofBackground(200);
    
	realsense.drawColor(0, 0); //draw color image on top left
	//draw calabrated depth image on top right
	//if (depthInColor.isAllocated()) {
	//	depthInColor.draw(640, 0);
	//}
	drawNameAndBoxFromYolo(probability);       //draw names and boxs of obj on screen depending on the probability;
    // draw left eye first
	whicheye = 0;
    cv1.begin(ovrEye_Left);
	ofClear(0,255,255);
	drawSceneLeft();
	cv1.end(ovrEye_Left); 
	// then right eye
	// fyi--the order is critical!
	whicheye = 1;
	cv1.begin(ovrEye_Right);
	ofClear(0,255,255);
	drawSceneRight();
	cv1.end(ovrEye_Right);
	//display the stereo view in the OF window (optional)
    cv1.draw(0, 480, bounds.width,bounds.height);  
	
	gui.setPosition(bounds.width ,480);
	gui.draw();
    }

void ofApp::drawSceneLeft() {
    ofPushMatrix;
	ofTranslate(position); 	
	drawovrvisionsceneleft();
	ofFill();
	drawplane(); 
	//drawhands();
    ofPopMatrix;
}

void ofApp::drawSceneRight() {
	ofPushMatrix;
	ofTranslate(position);
	drawovrvisionsceneright();
	ofFill();
	drawplane();
	//drawhands();
	ofPopMatrix;
}

void ofApp::exit() {
	colorFromRealsense.clear();
	lefteye.clear();
	righteye.clear();
	realsense.close();

}

void ofApp::drawNameAndBoxFromYolo(float prob) {
	
	for (auto& r : lastResults)
	{
		if (r.prob >= prob) //draw box based on prob;
		{
			ofNoFill();
			ofDrawBitmapString(yolo.getName(r.obj_id) + " - " + ofToString(r.prob), r.x, r.y);
			ofDrawRectangle(r.x, r.y, r.w, r.h);
		}
	}
}


//still have a lot of to do with this :(     
std::vector<posAndSize_> ofApp::getPositionOfObj(float prob){
	
	std::vector<posAndSize_> positionAndSizeOfObj;
	posAndSize_ position_and_size;
	
	
	for (auto& r : lastResults)
	{  
		if (r.prob >= prob)
		{   
			float xcenter = r.x + r.w / 2; // get center position of the box;
			float ycenter = r.y + r.h / 2;

			std::vector<float> everydis;
			for (int x_ = r.x + r.w/4; x_ < r.x+3*r.w/4; x_++)        //So the method is detect depth for every pixels
			{                                                         //that in the center(1/4size of box)of box, and
				for (int y_ = r.y+r.h/4 ; y_ < r.y + 3*r.h/4; y_++)   //choose the maxmaximum date for the distance 
				{             
                                        //of box. Must have better way to do this;
					float dis_ = realsense.getDistanceAt(x_, y_);
					if(0 < dis_ && dis_ < 5000 )
					{
						float dis = realsense.getDistanceAt(x_, y_);
						everydis.push_back(dis);
				    }
				}
			}
			    float sumdis = 0;
				for(auto & r : everydis)
				{   
						sumdis += r;
                }
				float avragedis = sumdis / everydis.size();
				position_and_size.xpos = (xcenter - 320) / 320 * x_parameter + xd_parameter;              //trying to calabrite with the real word but so far really cranky :(
				position_and_size.ypos = (240 - ycenter) / 240 * y_parameter + yd_parameter;
				position_and_size.zpos = avragedis / z_parameter + zd_parameter;
				position_and_size.sizeHeight = r.h;
				position_and_size.sizeWidth = r.w;
				position_and_size.number = r.obj_id;

				positionAndSizeOfObj.push_back(position_and_size);
				
				ofLog(OF_LOG_NOTICE, yolo.getName(r.obj_id) + ":" + "x:" + ofToString(xcenter) + "y:"
				+ ofToString(ycenter) + "distance:" + ofToString(avragedis));     //show the obj's position on screen
		}
	}
    return positionAndSizeOfObj;
}

void ofApp::billboarding(ofVec3f & position, axis_ & axis) {	
	cv1.getHMDTrackingState(position, orientation);	
	Node.setGlobalPosition(position);
	Node.setGlobalOrientation(orientation);       //omg ofnode is amazing!;	
	axis.x_axis = Node.getSideDir();                   //set xyz axis
	axis.y_axis = Node.getUpDir();
	axis.z_axis = Node.getLookAtDir();	
}

void ofApp::drawplane() {
	for (auto& i : positionAndSizeOfObj_) {
		float size = sqrt(i.sizeWidth * i.sizeHeight);
		ofPlanePrimitive plane;
		ofVec3f objpos = axis.x_axis * i.xpos + axis.y_axis * i.ypos + axis.z_axis * i.zpos;
		plane.setPosition(objpos.x, objpos.y, objpos.z);
		plane.setResolution(2, 2);
		//plane.lookAt(-axis.z_axis, axis.y_axis);
		ofVec3f up(0.f, 1.f, 0.f);
		plane.lookAt(-axis.z_axis, -up);
		obj[i.number].getTextureReference().bind();
		plane.mapTexCoordsFromTexture(obj[i.number].getTexture());
		plane.resizeToTexture(obj[i.number].getTexture(), size / size_parameter);
		plane.draw();
		obj[i.number].getTextureReference().unbind();
	}
}

void ofApp::drawovrvisionsceneleft() {
	ofTranslate(axis.z_axis * ztexture);
	lefteye.draw(tlx, tly, brx, bry);
    ofTranslate(axis.z_axis * -ztexture);
}

void ofApp::drawovrvisionsceneright() {
	ofTranslate(axis.z_axis * ztexture);
    righteye.draw(tlx1, tly1, brx1, bry1);
	ofTranslate(axis.z_axis * -ztexture);
}

void ofApp::drawrealsensescene() {
    ofTranslate(axis.z_axis * ztexture);
	realsensergb.draw(tlx, tly, brx, bry);
	ofTranslate(axis.z_axis * -ztexture);
}

void ofApp::drawhands() {
	for (int i = 0; i < realsense.handsnumber; i++) {
		//ofLog(OF_LOG_NOTICE, ofToString(realsense.centers[i].x) + "and" + ofToString(realsense.centers[i].y));
		float handis = realsense.getDistanceAt(realsense.centers[i].x, realsense.centers[i].y);
		ofVec3f handPoint = (
			((realsense.centers[i].x - 320) / 320 * x_parameter + xd_parameter) *axis.x_axis +
		    ((240 - realsense.centers[i].y) / 240 * y_parameter + yd_parameter) * axis.y_axis +
			((handis / z_parameter + zd_parameter)* axis.z_axis)
				);

		//sphereの描画
		 ofDrawSphere(handPoint.x, handPoint.y, handPoint.z, 0.05);
	}
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
