#include "precomp.hpp"

#include <pylon/PylonIncludes.h>

using namespace Pylon;

using namespace GenApi;

#define CV_CAP_PYLON 1600
#define IMAGES_TO_GRAB 5

class PylonInterface  {
	public:
		static PylonInterface * Instance() {
			if (_instance == 0) {
				_instance = new PylonInterface;
			}
			return _instance;
		};
		static int deviceCount() { return numDevices;};
		Pylon::IPylonDevice * getControl( int index) {
			return ( index >= numDevices ? NULL : CTlFactory::GetInstance().CreateDevice( list[index] ));
		}
	protected:
		PylonInterface() {
            CTlFactory& tlFactory = CTlFactory::GetInstance();

			numDevices = tlFactory.EnumerateDevices( list, true);
		}
	private:
		DeviceInfoList_t list;
        Pylon::PylonAutoInitTerm autoInitTerm;
		static PylonInterface * _instance;
		static int numDevices;
};
/**********************************************************************************/

class CvCaptureCAM_Pylon : public CvCapture
{
public:
    CvCaptureCAM_Pylon() { init(); }
    virtual ~CvCaptureCAM_Pylon() { close(); }

    virtual bool open( int index );
    virtual void close();
    virtual double getProperty(int);
    virtual bool setProperty(int, double);
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);
    virtual int getCaptureDomain() { return CV_CAP_PYLON; } // Return the type of the capture object: CV_CAP_VFW, etc...

private:
    void init();
    void errMsg(const char* msg, int errNum);
    void resetCvImage();
    IplImage* frame;
    PylonInterface * pylon;
    Pylon::CInstantCamera camera;
    CGrabResultPtr ptrGrabResult;
    CPylonImage image;
    unsigned int width;
    unsigned int height;
    bool mono;
    int       timeout;
};

/**********************************************************************************/

CvCapture* cvCreateCameraCapture_Pylon( int index )
{
    CvCaptureCAM_Pylon* capture = new CvCaptureCAM_Pylon;

    if( capture->open( index ))
        return capture;

    delete capture;
    return 0;
}

/**********************************************************************************/
// Enumerate connected devices
void CvCaptureCAM_Pylon::init()
{
	pylon = PylonInterface::Instance();
    frame = NULL;
    timeout = 0;
    mono = false;
}


/**********************************************************************************/
// Initialize camera input
bool CvCaptureCAM_Pylon::open( int index )
{

    if(pylon->deviceCount() == 0)
        return false;

    camera.Attach(pylon->getControl(index));
    INodeMap& nodemap = camera.GetNodeMap();
    CIntegerPtr p = nodemap.GetNode( "Width");
    long int lwidth = p->GetValue();
    p = nodemap.GetNode( "Height");
    long int lheight = p->GetValue();
    height = lheight;
    width = lwidth;
    mono = Pylon::IsMono(image.GetPixelType());
    camera.MaxNumBuffer = 5;
    frame = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, mono ? 1: 3);
    camera.StartGrabbing(  1, GrabStrategy_OneByOne);

    return camera.IsOpen();
}

/**********************************************************************************/

void CvCaptureCAM_Pylon::close()
{
    if(frame)
        cvReleaseImage(&frame);

    if(camera.IsGrabbing())
        camera.StopGrabbing();
    if(camera.IsOpen())
        camera.Close();
    if(camera.IsPylonDeviceAttached())
        camera.DetachDevice();

}

/**********************************************************************************/

bool CvCaptureCAM_Pylon::grabFrame()
{
        return false;
}

/**********************************************************************************/

IplImage* CvCaptureCAM_Pylon::retrieveFrame(int)
{
    // update cvImage after format has changed
    resetCvImage();

    return frame;
}

/**********************************************************************************/

void CvCaptureCAM_Pylon::resetCvImage()
{

    if( image.GetWidth() != width || image.GetHeight() != height )
    {
        width = image.GetWidth();
        height = image.GetHeight();
        if(frame) cvReleaseImage(&frame);
        frame = NULL;

        frame = cvCreateImage(cvSize( width, height), IPL_DEPTH_8U, 3);
    }
    cvZero(frame);
}
/**********************************************************************************/

double CvCaptureCAM_Pylon::getProperty( int property_id )
{
    INodeMap& nodemap = camera.GetNodeMap();
    CIntegerPtr p;

    switch( property_id )
    {
    // OCV parameters
    case CV_CAP_PROP_POS_FRAMES   : 
        return 0; //TODO (double) image.nframe;
    case CV_CAP_PROP_FRAME_WIDTH  : {
        p = nodemap.GetNode( "Width");
        long int lwidth = p->GetValue();
        return lwidth;
    }
    case CV_CAP_PROP_FRAME_HEIGHT : {
        p = nodemap.GetNode( "Height");
        long int lheight = p->GetValue();
        return lheight;
    }

/*    case CV_CAP_PROP_FPS          : return control.GetNodeMap()->GetNode( XI_PRM_FRAMERATE)
    case CV_CAP_PROP_GAIN         : return control.GetNodeMap()->GetNode( XI_PRM_GAIN,)
    case CV_CAP_PROP_EXPOSURE     : return control.GetNodeMap()->GetNode( XI_PRM_EXPOSURE)
*/
    // GENICAM camera properties
/*    case CV_CAP_PROP_XI_DOWNSAMPLING  : return control.GetNodeMap()->GetNode( XI_PRM_DOWNSAMPLING)
    case CV_CAP_PROP_XI_DATA_FORMAT   : return control.GetNodeMap()->GetNode( XI_PRM_IMAGE_DATA_FORMAT)
*/
    case CV_CAP_PROP_XI_OFFSET_X      : {
        p = nodemap.GetNode( "OffsetX");
        long int offset = p->GetValue();
        return offset;
    }
    case CV_CAP_PROP_XI_OFFSET_Y      : {
        p = nodemap.GetNode( "OffsetY");
        long int offset = p->GetValue();
        return offset;
    }
/*    case CV_CAP_PROP_XI_TRG_SOURCE    : return control.GetNodeMap()->GetNode( XI_PRM_TRG_SOURCE)
    case CV_CAP_PROP_XI_GPI_SELECTOR  : return control.GetNodeMap()->GetNode( XI_PRM_GPI_SELECTOR)
    case CV_CAP_PROP_XI_GPI_MODE      : return control.GetNodeMap()->GetNode( XI_PRM_GPI_MODE)
    case CV_CAP_PROP_XI_GPI_LEVEL     : return control.GetNodeMap()->GetNode( XI_PRM_GPI_LEVEL)
    case CV_CAP_PROP_XI_GPO_SELECTOR  : return control.GetNodeMap()->GetNode( XI_PRM_GPO_SELECTOR)
    case CV_CAP_PROP_XI_GPO_MODE      : return control.GetNodeMap()->GetNode( XI_PRM_GPO_MODE)
    case CV_CAP_PROP_XI_LED_SELECTOR  : return control.GetNodeMap()->GetNode( XI_PRM_LED_SELECTOR)
    case CV_CAP_PROP_XI_LED_MODE      : return control.GetNodeMap()->GetNode( XI_PRM_LED_MODE)
    case CV_CAP_PROP_XI_AUTO_WB       : return control.GetNodeMap()->GetNode( XI_PRM_AUTO_WB)
    case CV_CAP_PROP_XI_AEAG          : return control.GetNodeMap()->GetNode( XI_PRM_AEAG)
    case CV_CAP_PROP_XI_EXP_PRIORITY  : return control.GetNodeMap()->GetNode( XI_PRM_EXP_PRIORITY)
    case CV_CAP_PROP_XI_AE_MAX_LIMIT  : return control.GetNodeMap()->GetNode( XI_PRM_EXP_PRIORITY)
    case CV_CAP_PROP_XI_AG_MAX_LIMIT  : return control.GetNodeMap()->GetNode( XI_PRM_AG_MAX_LIMIT)
    case CV_CAP_PROP_XI_AEAG_LEVEL    : return control.GetNodeMap()->GetNode( XI_PRM_AEAG_LEVEL)
*/
    case CV_CAP_PROP_XI_TIMEOUT       : return timeout;

    }
    return 0;
}

/**********************************************************************************/

bool CvCaptureCAM_Pylon::setProperty( int property_id, double value )
{
    INodeMap& nodemap = camera.GetNodeMap();
    CIntegerPtr p;
    long int ival = (int) value + 0.5;

    switch(property_id)
    {
    // OCV parameters
    case CV_CAP_PROP_FRAME_WIDTH  : {
        p = nodemap.GetNode( "Width");
        p->SetValue(ival);
        break;
    }
    case CV_CAP_PROP_FRAME_HEIGHT :  {
        p = nodemap.GetNode( "Height");
        p->SetValue(ival);
        break;
    }
/*    case CV_CAP_PROP_FPS          : nodemap.SetNode( XI_PRM_FRAMERATE, fval); break;
    case CV_CAP_PROP_GAIN         : nodemap.SetNode( XI_PRM_GAIN, fval); break;
    case CV_CAP_PROP_EXPOSURE     : nodemap.SetNode( XI_PRM_EXPOSURE, ival); break;
*/    // GENICAM camera properties
/*    case CV_CAP_PROP_XI_DOWNSAMPLING  : nodemap.SetNode( XI_PRM_DOWNSAMPLING, ival); break;
    case CV_CAP_PROP_XI_DATA_FORMAT   : nodemap.SetNode( XI_PRM_IMAGE_DATA_FORMAT, ival); break;
*/
    case CV_CAP_PROP_XI_OFFSET_X      :  {
        p = nodemap.GetNode( "OffsetX");
        p->SetValue(ival);
        break;
    }
    case CV_CAP_PROP_XI_OFFSET_Y      :  {
        p = nodemap.GetNode( "OffsetY");
        p->SetValue(ival);
        break;
    }
/*    case CV_CAP_PROP_XI_TRG_SOURCE    : nodemap.SetNode( XI_PRM_TRG_SOURCE, ival); break;
    case CV_CAP_PROP_XI_GPI_SELECTOR  : nodemap.SetNode( XI_PRM_GPI_SELECTOR, ival); break;
    case CV_CAP_PROP_XI_TRG_SOFTWARE  : nodemap.SetNode( XI_PRM_TRG_SOURCE, 1); break;
    case CV_CAP_PROP_XI_GPI_MODE      : nodemap.SetNode( XI_PRM_GPI_MODE, ival); break;
    case CV_CAP_PROP_XI_GPI_LEVEL     : nodemap.SetNode( XI_PRM_GPI_LEVEL, ival); break;
    case CV_CAP_PROP_XI_GPO_SELECTOR  : nodemap.SetNode( XI_PRM_GPO_SELECTOR, ival); break;
    case CV_CAP_PROP_XI_GPO_MODE      : nodemap.SetNode( XI_PRM_GPO_MODE, ival); break;
    case CV_CAP_PROP_XI_LED_SELECTOR  : nodemap.SetNode( XI_PRM_LED_SELECTOR, ival); break;
    case CV_CAP_PROP_XI_LED_MODE      : nodemap.SetNode( XI_PRM_LED_MODE, ival); break;
    case CV_CAP_PROP_XI_AUTO_WB       : nodemap.SetNode( XI_PRM_AUTO_WB, ival); break;
    case CV_CAP_PROP_XI_MANUAL_WB     : nodemap.SetNode( XI_PRM_LED_MODE, ival); break;
    case CV_CAP_PROP_XI_AEAG          : nodemap.SetNode( XI_PRM_AEAG, ival); break;
    case CV_CAP_PROP_XI_EXP_PRIORITY  : nodemap.SetNode( XI_PRM_EXP_PRIORITY, fval); break;
    case CV_CAP_PROP_XI_AE_MAX_LIMIT  : nodemap.SetNode( XI_PRM_EXP_PRIORITY, ival); break;
    case CV_CAP_PROP_XI_AG_MAX_LIMIT  : nodemap.SetNode( XI_PRM_AG_MAX_LIMIT, fval); break;
    case CV_CAP_PROP_XI_AEAG_LEVEL    : nodemap.SetNode( XI_PRM_AEAG_LEVEL, ival); break;
*/
    case CV_CAP_PROP_XI_TIMEOUT       : timeout = ival; break;
    }

        return false;
}

/**********************************************************************************/

void CvCaptureCAM_Pylon::errMsg(const char* msg, int errNum)
{
#if defined WIN32 || defined _WIN32
    char buf[512]="";
    sprintf( buf, "%s : %d\n", msg, errNum);
    OutputDebugString(buf);
#else
    fprintf(stderr, "%s : %d\n", msg, errNum);
#endif
}

/**********************************************************************************/
