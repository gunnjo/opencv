
#include "precomp.hpp"

#define LOG4CPP_FIX_ERROR_COLLISION 1 

#include <pylon/PylonIncludes.h>

using namespace std;

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
            return ( index >= numDevices ? NULL : Pylon::CTlFactory::GetInstance().CreateDevice( list[index] ));
        }

    protected:
        PylonInterface() {
            Pylon::CTlFactory& tlFactory = Pylon::CTlFactory::GetInstance();

            numDevices = tlFactory.EnumerateDevices( list, true);
    fprintf( stderr, "Pylon found %d devices\n", numDevices);
        }
    private:
        Pylon::DeviceInfoList_t list;
        Pylon::PylonAutoInitTerm autoInitTerm;
        static PylonInterface * _instance;
        static int numDevices;
};
PylonInterface * PylonInterface::_instance = 0;
int PylonInterface::numDevices = 0;

/**********************************************************************************/

class CvCaptureCAM_PYLON : public CvCapture
{
public:
    CvCaptureCAM_PYLON(): rawImage(0){ init(); }
    virtual ~CvCaptureCAM_PYLON() { close(); }

    virtual bool open( int index );
    virtual void close();
    virtual double getProperty(int);
    virtual bool setProperty(int, double);
    virtual bool grabFrame();
    virtual IplImage* retrieveFrame(int);
    virtual int getCaptureDomain() { return CV_CAP_XIAPI; } // Return the type of the capture object: CV_CAP_VFW, etc...

private:
    void init();
    void errMsg(const char* msg, int errNum);
    void resetCvImage();
    int  ocvParamtoPylonParam(int value);
    PylonInterface * pylon;
    IplImage* rawImage;
    Pylon::CInstantCamera camera;
    Pylon::CGrabResultPtr ptrGrabResult;
};

/**********************************************************************************/

CvCapture* cvCreateCameraCapture_PYLON( int index )
{
    CvCaptureCAM_PYLON* capture = new CvCaptureCAM_PYLON;
    if( capture->open( index ))
        return capture;

    delete capture;
    return 0;
}

/**********************************************************************************/
// Enumerate connected devices
void CvCaptureCAM_PYLON::init()
{
    pylon = PylonInterface::Instance();
}


/**********************************************************************************/
// Initialize camera input
bool CvCaptureCAM_PYLON::open( int index )
{
    Pylon::IPylonDevice *d = pylon->getControl(index);
    if(d == 0)
        return false;
    camera.Attach(d);
    fprintf( stderr, "PYLON:: attached to a Pylon device\n");
    camera.Open();
    camera.StartGrabbing();
    return true;

}

/**********************************************************************************/

void CvCaptureCAM_PYLON::close()
{
    if ( camera.IsOpen()) camera.Close();
    camera.DetachDevice();
}

/**********************************************************************************/

bool CvCaptureCAM_PYLON::grabFrame()
{

    bool r = camera.RetrieveResult( 5000, ptrGrabResult, Pylon::TimeoutHandling_ThrowException);
    return r;
}

/**********************************************************************************/

IplImage* CvCaptureCAM_PYLON::retrieveFrame(int)
{
    if (rawImage) {
        if (((int)ptrGrabResult->GetWidth() != rawImage->width) || ((int)ptrGrabResult->GetHeight() != rawImage->height) ){
            cvReleaseImageHeader(&rawImage);
            rawImage = 0;            
        }

    }
    if( ptrGrabResult->GrabSucceeded()){
        int depth = ptrGrabResult->GetPixelType()== Pylon::PixelType_BayerBG8 ? 1:1;
// TODO Pylon::PixelType_Mono8
        if (!rawImage) {
            rawImage = cvCreateImageHeader (cvSize(ptrGrabResult->GetWidth(), ptrGrabResult->GetHeight()),IPL_DEPTH_8U,1);
        }
        rawImage->origin = IPL_ORIGIN_TL;
        rawImage->dataOrder =  IPL_DATA_ORDER_PIXEL;
        rawImage->widthStep = ptrGrabResult->GetWidth();
        rawImage->imageData = (char*)(ptrGrabResult->GetBuffer());
    } else {
        fprintf( stderr, "Pylon: grab failed %x\n", ptrGrabResult->GetErrorCode());
        return 0;
    }

    return rawImage;
}

/**********************************************************************************/

void CvCaptureCAM_PYLON::resetCvImage()
{
}

/**********************************************************************************/

int CvCaptureCAM_PYLON::ocvParamtoPylonParam(int property_id)
{
    return 0;
}

/**********************************************************************************/

bool CvCaptureCAM_PYLON::setProperty( int property_id, double value )
{
    return false;
}

/**********************************************************************************/

double CvCaptureCAM_PYLON::getProperty( int property_id )
{
        return 0;
}

/**********************************************************************************/

void CvCaptureCAM_PYLON::errMsg(const char* msg, int errNum)
{
}

/**********************************************************************************/