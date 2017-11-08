#include "ofxRealSense.h"

//--------------------------------------------------------------
bool ofxRealSense::setup(bool grabColor, bool useTextures, bool mirror, bool trackFaces, bool doScan)
{
	if (bIsOpen || mSenseMgr != NULL)
	{
		ofLogError("ofxRealSense") << "can't setup Real Sense, already open!";
		return false;
	}

	// set configuration here - actual SDK init will happen in open()
	bColor = grabColor;
	bUseTex = useTextures;
	bMirror = mirror;
	bFaces = trackFaces;
	bScan = doScan;

	bBGR = isLittleEndian(); // Real Sense using BGR or RGB channel order?

	return true;
}

//--------------------------------------------------------------
bool ofxRealSense::open()
{

	if (bIsOpen || mSenseMgr != NULL)
	{
		ofLogError("ofxRealSense") << "can't open Real Sense, already open!";
		return false;
	}

	// instantiate RS SDK interface manager
	mSenseMgr = PXCSenseManager::CreateInstance();
	if (mSenseMgr == NULL) 
	{ 
		ofLogError("ofxRealSense") << "couldn't create Real Sense manager";
		return false; 
	}

	pxcStatus status;

	// init depth stream
	status = mSenseMgr->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, 640, 480, 30.f);
	if (status < PXC_STATUS_NO_ERROR)
	{
		ofLogError("ofxRealSense") << "couldn't enable depth stream, error status: " << status;
		close();
		return false;
	}

	status = mSenseMgr->EnableHand();
	if (status < PXC_STATUS_NO_ERROR) {
		throw std::runtime_error("手の検出の有効化に失敗しました");
	}

	// init RGB stream, if desired
	if (bColor)
	{
		status = mSenseMgr->EnableStream(PXCCapture::STREAM_TYPE_COLOR, 640, 480, 30.f);
		if (status < PXC_STATUS_NO_ERROR)
		{
			ofLogError("ofxRealSense") << "couldn't enable color stream, error status: " << status;
			close();
			return false;
		}
	}

	// enable face tracking module
	if (bFaces)
	{
		if (mFaceTracker.enable(mSenseMgr) == false)
		{
			close();
			return false;
		}
	}

	// enable 3D scanning module, if desired
	if (bScan)
	{ 
		if (mScanner.enable(mSenseMgr) == false) // try
		{ 
			close(); // close if failed
			return false; 
		}
	}

    // init streams - this must be done before 3D scan config
	status = mSenseMgr->Init();

	if (status < PXC_STATUS_NO_ERROR)
	{
		ofLogError("ofxRealSense") << "couldn't initialize Real Sense capture stream, error status: " << status;
		close();
		return false;
	}

	//initializeHandTracking();

	// create coordinate mapper
	mCoordinateMapper = mSenseMgr->QueryCaptureManager()->QueryDevice()->CreateProjection();

	// set mirror mode
	PXCCapture::Device::MirrorMode mirrorMode = bMirror ? PXCCapture::Device::MirrorMode::MIRROR_MODE_HORIZONTAL : PXCCapture::Device::MirrorMode::MIRROR_MODE_DISABLED;
	status = mSenseMgr->QueryCaptureManager()->QueryDevice()->SetMirrorMode(mirrorMode);

	if (status < PXC_STATUS_NO_ERROR)
	{
		string mode = bMirror ? "enabled" : "disabled";
		ofLogError("ofxRealSense") << "issue setting mirror mode to " << mode << ", error status: " << status;
	}

	// config 3D scanning
	if (bScan) { bScan = mScanner.configure(); }

	//1-7
	//status = mSenseMgr->QueryCaptureManager()->QueryDevice()->SetDepthConfidenceThreshold(1);
	//status = mSenseMgr->QueryCaptureManager()->QueryDevice()->SetIVCAMFilterOption(6);


	/* to do: face tracker */

	bIsOpen = true;
	return true;
}

//--------------------------------------------------------------
bool ofxRealSense::update()
{
	// make sure we have a valid RSSDK sense manager
	if (!bIsOpen || mSenseMgr == NULL)
	{
		ofLogError("ofxRealSense") << "unable to update, not yet opened!";
		return false;
	}

	pxcStatus status;

	// get new frame
	/* TO DO: threading a la ofxKinect */
	status = mSenseMgr->AcquireFrame(true , 30); // this is blocking for 20ms, returns error if not all frames ready (e.g. depth + color + scan)
	if (status < PXC_STATUS_NO_ERROR)
	{
		ofLogWarning("ofxRealSense") << "unable to acquire new frame, error status: " << status;
		return false;
	}

	//updateHandFrame();           

	/* process images & algorithm modules data */

	// get image sample for depth and color processing
	PXCCapture::Sample *sample = mSenseMgr->QuerySample();

	updateDepth(sample); // update depth

	if (bColor) { updateColor(sample); } // update color
	
	if (bColor && bUVMap && mCoordinateMapper && sample->color && sample->depth) // map color <--> depth if desired
	{
		mapDepthAndColor(sample);
	}

	if (bFaces) { mFaceTracker.update(); }

	// update face scanning preview image
	if (bScan) { mScanner.updatePreview(bUseTex); }

	// release frame to continue
	mSenseMgr->ReleaseFrame();

	return true;
}

//--------------------------------------------------------------
bool ofxRealSense::updateDepth(PXCCapture::Sample *sample)
{
	if (!sample->depth)
	{
		ofLogVerbose("ofxRealSense") << "can't update depth, invalid sample";
		return false;
	}
	
	// load raw depth data into mDepthRawPix (raw depth is measured in mm, as 16bit single channel ofShortPixels)
	bool hasDepthRaw = pxcImageToOfPixels(sample->depth, PXCImage::PIXEL_FORMAT_DEPTH, &mDepthRawPix);

	// load 8-bit grayscale depth image data into mDepthPix (for easy drawing)
	bool hasDepth8 = pxcImageToOfPixels(sample->depth, PXCImage::PIXEL_FORMAT_RGB32, &mDepthPix);

	// load 8-bit depth to texture (if needed + available)
	if (bUseTex && hasDepth8) { mDepthTex.loadData(mDepthPix); }

	return (hasDepthRaw && hasDepth8);
}

//--------------------------------------------------------------
bool ofxRealSense::updateColor(PXCCapture::Sample *sample)
{
	if (!sample->color)
	{
		ofLogVerbose("ofxRealSense") << "can't update color, invalid sample";
		return false;
	}

	// copy color image data to ofPixels
	bool hasColor = pxcImageToOfPixels(sample->color, PXCImage::PIXEL_FORMAT_RGB24, &mColorPix);

	// load pixels to texture (if needed + available)
	if (bUseTex & hasColor) { mColorTex.loadData(mColorPix); }

	return hasColor;
}

//--------------------------------------------------------------
bool ofxRealSense::mapDepthAndColor(PXCCapture::Sample* sample)
{
	if (!sample->depth || !sample->color)
	{
		string which = (!sample->depth) ? "depth" : "color";
		ofLogError("ofxRealSense") << "can't UV map depth and color images, no " << which << " image available";
		return false;
	}

	bool success = true;

	// map color img to depth frame, and copy to ofPixels
	PXCImage* colorInDepth = mCoordinateMapper->CreateColorImageMappedToDepth(sample->depth, sample->color); // map
	bool hasColorInDepth = pxcImageToOfPixels(colorInDepth, PXCImage::PIXEL_FORMAT_RGB32, &mColorInDepthFrame); // copy
	colorInDepth->Release();

	// same for depth and raw depth
	PXCImage* depthInColor = mCoordinateMapper->CreateDepthImageMappedToColor(sample->depth, sample->color);
	bool hasDepthRawInColor = pxcImageToOfPixels(depthInColor, PXCImage::PIXEL_FORMAT_DEPTH, &mDepthRawInColorFrame); // 16 bit gray (raw)
	bool hasDepthInColor = pxcImageToOfPixels(depthInColor, PXCImage::PIXEL_FORMAT_RGB32, &mDepthInColorFrame); // 8 bit rgb

	depthInColor->Release();
	
	return (hasColorInDepth && hasDepthInColor && hasDepthRawInColor);
}

bool ofxRealSense::uvMapDepthToColor(PXCImage* depth)
{
	/*
	if (!depth) return false;
	
	PXCImage::ImageInfo info = depth->QueryInfo();
	size_t nDepthPts = info.width * info.height;

	// Calculate the UV map if needed
	//PXCPointF32 *uvmap = new PXCPointF32[nDepthPts];
	pxcStatus status = PXC_STATUS_NO_ERROR;
	if (mUVDepthToColor.size() != nDepthPts) {
		mUVDepthToColor.resize(nDepthPts);
		status = mCoordinateMapper->QueryUVMap(depth, mUVDepthToColor.data());
	}
	if (status < PXC_STATUS_NO_ERROR)
	{
		ofLogError("ofxRealSense") << "couldn't acquire UV map, error status: " << status;
		return false;
	}

	ofPixels d2c = ofPixels(mColorPix);
	if (!d2c.isAllocated()) return false;

	auto& uv = mUVDepthToColor;
	size_t w = d2c.getWidth(); size_t h = d2c.getHeight();

	// Translate depth points uv[] to color ij[]
	for (int i = 0; i<nDepthPts; i++) {
		d2c.setColor(uv[i].x*w, uv[i].y*h, )
		ij[i].x = ;
		ij[i].y = uv[i].y*d2c.getHeight();
	}
	*/
	
	return true;
}

//--------------------------------------------------------------
void ofxRealSense::drawColor(float x, float y, float w, float h)
{
	if (!mColorPix.isAllocated()) 
	{ 
		ofLogError("ofxRealSense") << "can't draw color, no pixels stored";
		return; 
	}

	if (!bUseTex) { mColorTex.loadData(mColorPix); }

	if (w > 0 && h > 0) 
	{ 
		mColorTex.draw(x, y, w, h); 
	}
	else
	{
		mColorTex.draw(x, y);
	}
}

//--------------------------------------------------------------
void ofxRealSense::drawDepth(float x, float y, float w, float h)
{
	if (!mDepthPix.isAllocated()) 
	{ 
		ofLogError("ofxRealSense") << "can't draw depth, no pixels stored";
		return; 
	}

	if (!bUseTex) { mDepthTex.loadData(mDepthPix); }

	if (w > 0 && h > 0)
	{
		mDepthTex.draw(x, y, w, h);
	}
	else
	{
		mDepthTex.draw(x, y);
	}
}

void ofxRealSense::drawScanPreview(float x, float y, float w, float h)
{
	if (!mScanner.previewPix.isAllocated())
	{
		ofLogError("ofxRealSense") << "can't draw scan preview, no pixels stored";
		return;
	}

	if (!bUseTex) { mScanner.previewTex.loadData(mScanner.previewPix); }

	if (w > 0 && h > 0)
	{
		mScanner.previewTex.draw(x, y, w, h);
	}
	else
	{
		mScanner.previewTex.draw(x, y);
	}
}

/*void ofxRealSense::GetCursorPoint(ofVec3f icursor[2]) {
    
	int numOfHands = cursorAnalysis->QueryNumberOfCursors();
    for (int i = 0; i < numOfHands; i++)
	{
		//Get hand by time of appearance
		PXCCursorData::ICursor* cursorData;
		if (cursorAnalysis->QueryCursorData(PXCCursorData::AccessOrderType::ACCESS_ORDER_BY_TIME, i, cursorData) == PXC_STATUS_NO_ERROR)
		{
			// cursor point
			PXCPoint3DF32 worldPoint = cursorData->QueryCursorImagePoint();
			icursor[i].x = worldPoint.x;
			icursor[i].y = worldPoint.y;
			icursor[i].z = worldPoint.z;
		}
	}
}*/


void ofxRealSense::initializeHandTracking()
{
	// 手の検出器を取得する
	handAnalyzer = mSenseMgr->QueryHand();
	if (handAnalyzer == 0) {
		throw std::runtime_error("手の検出器の取得に失敗しました");
	}
	// 手のデータを作成する
	handData = handAnalyzer->CreateOutput();
	if (handData == 0) {
		throw std::runtime_error("手の検出器の作成に失敗しました");
	}
	PXCCapture::Device *device = mSenseMgr->QueryCaptureManager()->QueryDevice();
	PXCCapture::DeviceInfo dinfo;
	device->QueryDeviceInfo(&dinfo);
	if (dinfo.model == PXCCapture::DEVICE_MODEL_IVCAM) {
		device->SetDepthConfidenceThreshold(1);
		//device->SetMirrorMode( PXCCapture::Device::MIRROR_MODE_DISABLED );
		device->SetIVCAMFilterOption(6);
	}
	// 手の検出の設定
	PXCHandConfiguration* config = handAnalyzer->CreateActiveConfiguration();
	config->EnableSegmentationImage(true);
	config->ApplyChanges();
	config->Update();
}

void ofxRealSense::updateHandFrame() {

	handData->Update();
	// 検出した手の数を取得する
	auto numOfHands = handData->QueryNumberOfHands();
	handsnumber = numOfHands;
	for (int i = 0; i < numOfHands; i++) {
		// 手を取得する
		pxcUID handID;
		PXCHandData::IHand* hand;
		auto sts = handData->QueryHandData(
			PXCHandData::AccessOrderType::ACCESS_ORDER_BY_ID, i, hand);
		if (sts < PXC_STATUS_NO_ERROR) {
			continue;
		}

		//auto side = hand->QueryBodySide();
		//auto openness = hand->QueryOpenness();
		auto center = hand->QueryMassCenterImage();
		

		if (i < 2) {
			//sides[i] = side;
			//openesses[i] = openness;
			centers[i].x = center.x;
			centers[i].y = center.y;
		}
	}
}
//--------------------------------------------------------------
bool ofxRealSense::close()
{

	bIsOpen = false;
	mScanner.close();

	if (mSenseMgr != NULL)
	{

		mSenseMgr->Close();
		mSenseMgr->Release(); // ? needed
		mSenseMgr = NULL;
	
		return true;
	}
	return false;
}

