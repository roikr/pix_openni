////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////


#include "pix_openni.h"
#include "Gem/State.h"
#include "Gem/Exception.h"

using namespace openni;

#ifdef __APPLE__
	static int index_offset=1;
#else
	static int index_offset=0;
#endif


CPPEXTERN_NEW_WITH_GIMME(pix_openni);
//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------

#define DISPLAY_MODE_OVERLAY	1
#define DISPLAY_MODE_DEPTH		2
#define DISPLAY_MODE_IMAGE		3
#define DEFAULT_DISPLAY_MODE	DISPLAY_MODE_DEPTH

#define MAX_DEPTH 10000



#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) < (b)) ? (b) : (a))


/////////////////////////////////////////////////////////
//
// pix_openni
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

pix_openni :: pix_openni(int argc, t_atom *argv)
{
	// second inlet/outlet for depthmap
	m_depthoutlet = outlet_new(this->x_obj, 0);
	m_depthinlet  = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("depth_state"));

	m_dataout = outlet_new(this->x_obj, 0);
	
	post("pix_openni 0.20 - 2014 by Roee Kremer");

	// init status variables
	
    
	depth_wanted = false;
	depth_started= false; 
	rgb_wanted = false;
	rgb_started= false;
	rgb_index = 0;
    depth_index = 0;

	m_registration=false;
	m_registration_wanted=false;
	
	openni_ready = false;
		
	depth_output = 0;
	req_depth_output = 0;
	
	min_thresh = 0;
	max_thresh = 1000;

    device_id = 1;

    
	// CHECK FOR ARGS AND ACTIVATE STREAMS
	if (argc >= 1)
	{
		post("chosen device Nr: %d", atom_getint(&argv[0]));
        device_id = atom_getint(&argv[0]);
	}
	if (argc >= 2)
	{
		if (atom_getint(&argv[1]) != 0)
		{
			rgb_wanted = true;
		}
	}
	if (argc >= 3)
	{
		rgb_index = atom_getint(&argv[2]);
		
	}
	if (argc >= 4)
	{
		if (atom_getint(&argv[3]) != 0)
		{
			depth_wanted = true;
		}
	}
	if (argc >= 5)
	{
		depth_index = atom_getint(&argv[4]);
	}
	if (argc >= 6)
	{
		if (atom_getint(&argv[5]) != 0)
		{
			m_registration_wanted = true;
		}
	}
	
    
    Init(); // Init OpenNI
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_openni :: ~pix_openni()
{ 
	
	post("destructor");
	
	if (depth_started)
	{
		g_depth.destroy();
	}
		
	if (rgb_started)
	{
		g_image.destroy();
	}
	
	
	/*
    g_Device.Release();
    */
}

/////////////////////////////////////////////////////////
// Init OpenNI
//
/////////////////////////////////////////////////////////

bool pix_openni :: Init()
{
    Status rc;  // ERROR STATUS
	
    //// INIT IN CODE::
    
    //Context context;
    rc = OpenNI::initialize(); // key difference: Init() not InitFromXml()
    if (rc != STATUS_OK)
    {
        post("OPEN NI init() failed.");
    } else {
        post("OPEN NI initialised successfully.");
        openni_ready = true;
    }

    if (openni_ready)
    {
    	rc = g_Device.open(openni::ANY_DEVICE);

    	if (rc != STATUS_OK)
	    {
	        post("open device failed.");
	        return false;
	    } else {
	        post("device opend successfully.");
	        return true;
	    }
    }
	
	/*
    if (openni_ready)
    {
        EnumerationErrors errors;
        XnStatus nRetVal;  // ERROR STATUS
        // find devices
        NodeInfoList list;
        nRetVal = g_context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, list, &errors);
        //XN_IS_STATUS_OK(nRetVal);
        
        post("The following devices were found:");
        int i = 1;
        for (NodeInfoList::Iterator it = list.Begin(); it != list.End(); ++it, ++i)
        {
            NodeInfo deviceNodeInfo = *it;
            
            Device deviceNode;
            deviceNodeInfo.GetInstance(deviceNode);
            XnBool bExists = deviceNode.IsValid();
            if (!bExists)
            {
                g_context.CreateProductionTree(deviceNodeInfo, deviceNode);
                // this might fail.
            }
            
            if (deviceNode.IsValid() && deviceNode.IsCapabilitySupported(XN_CAPABILITY_DEVICE_IDENTIFICATION))
            {
                const XnUInt32 nStringBufferSize = 200;
                XnChar strDeviceName[nStringBufferSize];
                XnChar strSerialNumber[nStringBufferSize];
                
                XnUInt32 nLength = nStringBufferSize;
                deviceNode.GetIdentificationCap().GetDeviceName(strDeviceName, nLength);
                nLength = nStringBufferSize;
                deviceNode.GetIdentificationCap().GetSerialNumber(strSerialNumber, nLength);
                post("[%d] %s (%s)", i, strDeviceName, strSerialNumber);
            }
            else
            {
                post("[%d] %s", i, deviceNodeInfo.GetCreationInfo());
            }
            
            // release the device if we created it
            if (!bExists && deviceNode.IsValid())
            {
                deviceNode.Release();
            }
        }
        
        // select device
        
        NodeInfoList::Iterator it = list.Begin();
        
        if (it == list.End()) {
            post("ERROR: No Device found!");
            openni_ready = false;
            //throw(GemException("Init() failed\n"));
            return false;
        }
            for (i = 1; i < device_id; ++i)
            {
                it++;
                if (it == list.End()) {
                    post("ERROR: Kinect Device ID not valid: %i", device_id);
                    openni_ready = false;
                    return false;
                }
            }
            
            NodeInfo deviceNode = *it;
            rc = g_context.CreateProductionTree(deviceNode, g_Device);
            return true;
    }
    */
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////

void pix_openni :: startRendering(){
	
  post("startRendering");
  
  //return true;
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////

void pix_openni :: render(GemState *state)
{
	if (openni_ready)
		{
			/*
			// UPDATE ALL  --> BETTER IN THREAD?!
			Status rc =STATUS_OK;
			rc = g_context.WaitNoneUpdateAll();
			if (rc != XN_STATUS_OK)
			{
				post("Read failed: %s\n", xnGetStatusString(rc));
				return;
			} else {
				//post("Read: %s\n", xnGetStatusString(rc));
			}
			*/
		
		if (rgb_wanted && !rgb_started)
		{
			post("trying to start rgb stream");
			Status rc;

			rc = g_image.create(g_Device, SENSOR_COLOR);
		    if (rc != STATUS_OK) {
		        post("Couldn't find color stream: ");
		        rgb_wanted=false;
		    } else {


    			const Array<VideoMode> &colorArray = g_image.getSensorInfo().getSupportedVideoModes();
    			g_image.setVideoMode(colorArray[rgb_index]); 
    
				rc = g_image.start();
				if (rc != STATUS_OK)
				{
					post("Couldn't start color stream! %d", rc);
					g_image.destroy();
					rgb_wanted=false;
				} else {
					rgb_started = true;

					VideoMode mode = VideoMode(g_image.getVideoMode());
    				post("RGB Video Mode %i : %ix%i @ %d Hz, pixelFormat: %d", rgb_index, mode.getResolutionX(), mode.getResolutionY(), mode.getFps(),mode.getPixelFormat());
					m_image.image.xsize = mode.getResolutionX();
					m_image.image.ysize = mode.getResolutionY();
					m_image.image.setCsizeByFormat(GL_RGBA);
					//m_image.image.csize=3;
			  		m_image.image.reallocate();
			  		post("Image stream created!");
					
				}
				
			}
		}
		
		if (!rgb_wanted && rgb_started)
		{
			g_image.destroy();
			rgb_started = false;
		}
		
		// OUTPUT RGB IMAGE
		if (rgb_wanted && rgb_started)
		{
			g_image.readFrame(&g_imageFrame);

			if (g_imageFrame.isValid()) // new data??
			{
				
				if (((int)g_imageFrame.getWidth() != (int)m_image.image.xsize) || ((int)g_imageFrame.getHeight() != (int)m_image.image.ysize))
				{
					m_image.image.xsize = g_imageFrame.getWidth();
					m_image.image.ysize = g_imageFrame.getHeight();
					m_image.image.reallocate();
				}

				// m_image.image.fromRGB(g_imageMD.Data()); // use gem internal method to convert colorspace

				//const XnUInt8* 
				const unsigned char *pImage = (const unsigned char *)g_imageFrame.getData();
				int size = m_image.image.xsize * m_image.image.ysize;
				unsigned char *pixels=m_image.image.data;
				while (size--) {
					*pixels=*pImage;
					pixels[chRed]=pImage[0];
					pixels[chGreen]=pImage[1];
					pixels[chBlue]=pImage[2];
					pixels[chAlpha]=255;
					pImage+=3;
					pixels+=4;
				}

				m_image.newimage = 1;
				m_image.image.notowned = true;
				m_image.image.upsidedown=true;
				state->set(GemState::_PIX, &m_image);
				
			} else {
				m_image.newimage = 0;
				state->set(GemState::_PIX, &m_image);				
			}
		}
			
		
	}
}

void pix_openni :: renderDepth(int argc, t_atom*argv)
{
	
	if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER) // is it gem_state?
	{
		depth_state =  (GemState *) (argv+1)->a_w.w_gpointer;
    
		
        if (openni_ready)
        {
            // start depth stream if wanted or needed by other generator
            if (depth_wanted  && !depth_started)
            {
                
                post("trying to start depth stream");
				Status rc;
				rc = g_depth.create(g_Device, SENSOR_DEPTH);
			    if (rc != STATUS_OK) {
			        post("Couldn't find depth stream");//: %s",OpenNI::getExtendedError());
			        depth_wanted=false;
			    } else {

			    	const Array<VideoMode> &depthArray = g_depth.getSensorInfo().getSupportedVideoModes();
    				g_depth.setVideoMode(depthArray[depth_index]);
    
					rc = g_depth.start();
					if (rc != STATUS_OK)
					{
						post("Couldn't start depth stream");//: %s",OpenNI::getExtendedError());
						g_depth.destroy();
						depth_wanted=false;
					} else {
						depth_started = true;
						
						post("Depth stream created!");
	                    VideoMode mode(g_depth.getVideoMode());
	                    post("Depth Video Mode %i : %ix%i @ %d Hz, pixelFormat: %d", depth_index, mode.getResolutionX(), mode.getResolutionY(), mode.getFps(),mode.getPixelFormat());
					
						m_depth.image.xsize = mode.getResolutionX();
						m_depth.image.ysize = mode.getResolutionY();
						m_depth.image.setCsizeByFormat(GL_RGBA);
						m_depth.image.reallocate();
						
					}
				}
            }
            
            if (!depth_wanted && depth_started)
            {
                g_depth.destroy();
                depth_started = false;
            }
            
            if (m_registration_wanted && rgb_started && depth_started && !m_registration) // set registration if wanted
            {
                post("registration wanted");
                if (g_Device.isImageRegistrationModeSupported(IMAGE_REGISTRATION_DEPTH_TO_COLOR)) {
        
        			Status rc = g_Device.setImageRegistrationMode(IMAGE_REGISTRATION_DEPTH_TO_COLOR);
			        if (rc != STATUS_OK) {
			            post("setImageRegistrationMode IMAGE_REGISTRATION_DEPTH_TO_COLOR has been failed");//: " << openni::OpenNI::getExtendedError() << endl;
			            m_registration_wanted = false;
			        } else {
			        	m_registration=true;
			        	post("registration");
			        }
        
    			}
        
            }
            
            if (depth_wanted && depth_started) //DEPTH OUTPUT
            {
                g_depth.readFrame(&g_depthFrame);
                
                if (g_depthFrame.isValid()) // new data??
                {
                	
                    // check if depth output request changed -> reallocate image_struct
                    if (req_depth_output != depth_output)
                    {
                        if (req_depth_output == 0)
                        {
                            m_depth.image.setCsizeByFormat(GL_RGBA);
                        }
                        if (req_depth_output == 1)
                        {
                            m_depth.image.setCsizeByFormat(GL_YCBCR_422_GEM);
                        }
                        m_depth.image.reallocate();
                        depth_output=req_depth_output;
                    }
                    
                    DepthPixel* pDepth = (DepthPixel*)g_depthFrame.getData();
                    
                    if (depth_output == 0) // RAW RGBA -> R 8 MSB, G 8 LSB of 16 bit depth value, B->userid if usergen 1 and usercoloring 1
                    {
                        uint8_t *pixels = m_depth.image.data;
                        
                        //uint16_t *depth_pixel = (uint16_t*)pDepth;
                        
                        float b = -min_thresh/(max_thresh-min_thresh);
                        float a = 1000.0/(MAX_DEPTH*(max_thresh-min_thresh));// UINT16_MAX

                        for(int y = 0; y < g_depthFrame.getWidth()*g_depthFrame.getHeight(); y++) {
                            float val = a*pDepth[y]+b; 
                            if (val<0 || val>1) {
                            	 pixels[chRed] = 0;
                            } else {
                            	pixels[chRed] = (1-val)*255;
                            }
                            
                            //pixels[chRed]=(uint8_t)(pDepth[y] >> 8);
                            //pixels[chGreen]=(uint8_t)(pDepth[y] & 0xff);
                            pixels[chGreen] = 0;
                            pixels[chBlue]=0; // set user id to b channel
                            pixels[chAlpha]=255; // set alpha
                            
                            pixels+=4;
                            
                        }
                        
                    }


                    if (depth_output == 1) // RAW YUV -> 16 bit Depth
                    {
                        
                        m_depth.image.data= (unsigned char*)pDepth;
                    }

                   
                    m_depth.newimage = 1;
                    m_depth.image.notowned = true;
                    m_depth.image.upsidedown=true;
                    
                } else { // no new depthmap from openni
					m_depth.newimage = 0;
                }
                
                depth_state->set(GemState::_PIX, &m_depth);
                
                t_atom ap[2];
                ap->a_type=A_POINTER;
                ap->a_w.w_gpointer=(t_gpointer *)m_cache;  // the cache ?
                (ap+1)->a_type=A_POINTER;
                (ap+1)->a_w.w_gpointer=(t_gpointer *)depth_state;
                outlet_anything(m_depthoutlet, gensym("gem_state"), 2, ap);
                
            }
        }
        
    } else {        
		outlet_anything(m_depthoutlet, gensym("gem_state"), argc, argv); // if we don't get a gemstate pointer, we forward all to other objects...
	}
}

///////////////////////////////////////
// POSTRENDERING -> Clear
///////////////////////////////////////

void pix_openni :: postrender(GemState *state)
{

}

///////////////////////////////////////
// STOPRENDERING -> Stop Transfer
///////////////////////////////////////

void pix_openni :: stopRendering(){
	post("stopRendering");
	
	//depth_wanted = false;
}




//////////////////////////////////////////
// Messages - Settings
//////////////////////////////////////////



void pix_openni :: bangMess ()
{
	/*
	// OUTPUT OPENNI DEVICES OPENED
    
    EnumerationErrors errors;
    XnStatus nRetVal;  // ERROR STATUS
    // find devices
    NodeInfoList list;
    nRetVal = g_context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, list, &errors);
    //XN_IS_STATUS_OK(nRetVal);
    
    post("The following devices were found:");
    int i = 1;
    for (NodeInfoList::Iterator it = list.Begin(); it != list.End(); ++it, ++i)
    {
        NodeInfo deviceNodeInfo = *it;
        
        Device deviceNode;
        deviceNodeInfo.GetInstance(deviceNode);
        XnBool bExists = deviceNode.IsValid();
        if (!bExists)
        {
            g_context.CreateProductionTree(deviceNodeInfo, deviceNode);
            // this might fail.
        }
        
        if (deviceNode.IsValid() && deviceNode.IsCapabilitySupported(XN_CAPABILITY_DEVICE_IDENTIFICATION))
        {
            const XnUInt32 nStringBufferSize = 200;
            XnChar strDeviceName[nStringBufferSize];
            XnChar strSerialNumber[nStringBufferSize];
            
            XnUInt32 nLength = nStringBufferSize;
            deviceNode.GetIdentificationCap().GetDeviceName(strDeviceName, nLength);
            nLength = nStringBufferSize;
            deviceNode.GetIdentificationCap().GetSerialNumber(strSerialNumber, nLength);
            post("[%d] %s (%s)", i, strDeviceName, strSerialNumber);
        }
        else
        {
            post("[%d] %s", i, deviceNodeInfo.GetCreationInfo());
        }
        
        // release the device if we created it
        if (!bExists && deviceNode.IsValid())
        {
            deviceNode.Release();
        }
    }
    
	NodeInfoList devicesList;
	nRetVal = g_context.EnumerateExistingNodes(devicesList, XN_NODE_TYPE_DEVICE);
	if (nRetVal != XN_STATUS_OK)
	{
		post("Failed to enumerate device nodes: %s\n", xnGetStatusString(nRetVal));
		return;
	}
	if (devicesList.IsEmpty())
	{
		post("OpenNI:: no device available!");
	}
	for (NodeInfoList::Iterator it = devicesList.Begin(); it != devicesList.End(); ++it, ++i)
	{
				// Create the device node
		NodeInfo deviceInfo = *it;
        const XnUInt32 nStringBufferSize = 200;
        XnChar strDeviceName[nStringBufferSize];
        
        g_Device.GetIdentificationCap().GetDeviceName(strDeviceName,nStringBufferSize);
        
		post ("Opened Device: [%i] %s", device_id, strDeviceName);

	}
	*/
	// OUTPUT AVAILABLE MODES
	if (depth_started)
	{
		//post("\nOpenNI:: Current Depth Output Mode: %ix%i @ %d Hz", g_depthMD.XRes(), g_depthMD.YRes(), g_depthMD.FPS());
		
		post("OpenNI:: Supported depth modes:");

		const Array<VideoMode> &depthArray = g_depth.getSensorInfo().getSupportedVideoModes();
	    
	    for (int i=0;i<depthArray.getSize();i++) {
	        const VideoMode &mode(depthArray[i]);
	        
	        post("Mode %i : %ix%i @ %d Hz, pixelFormat: %d", i, mode.getResolutionX(), mode.getResolutionY(), mode.getFps(),mode.getPixelFormat());
	    }
	    
	} else {
		post("OpenNI:: depth stream not started");
	}


	if (rgb_started)
	{
		// post("OpenNI:: Current Image (rgb) Output Mode: %ix%i @ %d Hz", g_imageMD.XRes(), g_imageMD.YRes(), g_imageMD.FPS());

		
		post("OpenNI:: Supported image (rgb) modes:");

		const Array<VideoMode> &colorArray = g_image.getSensorInfo().getSupportedVideoModes();
    
    	for (int i=0;i<colorArray.getSize();i++) {
        	const VideoMode &mode(colorArray[i]);

        	post("Mode %i : %ix%i @ %d Hz, pixelFormat: %d", i, mode.getResolutionX(), mode.getResolutionY(), mode.getFps(),mode.getPixelFormat());
        }
        
	} else {
		post("OpenNI:: image stream not started");
	}
	
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_openni :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, (t_method)&pix_openni::bangMessCallback,
  		  gensym("bang"), A_NULL);
  class_addmethod(classPtr, (t_method)&pix_openni::floatRgbMessCallback,
  		  gensym("rgb"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, (t_method)&pix_openni::floatDepthMessCallback,
  		  gensym("depth"), A_FLOAT, A_NULL);
	class_addmethod(classPtr, (t_method)&pix_openni::floatDepthOutputMessCallback,
  		  gensym("depth_output"), A_FLOAT, A_NULL);
	class_addmethod(classPtr, (t_method)&pix_openni::floatMinMessCallback,
  		  gensym("min"), A_FLOAT, A_NULL);
	class_addmethod(classPtr, (t_method)&pix_openni::floatMaxMessCallback,
  		  gensym("max"), A_FLOAT, A_NULL);
	class_addmethod(classPtr, (t_method)(&pix_openni::renderDepthCallback),
        gensym("depth_state"), A_GIMME, A_NULL);
}

void pix_openni :: bangMessCallback(void *data)
{
  GetMyClass(data)->bangMess();
}

void pix_openni :: floatDepthOutputMessCallback(void *data, t_floatarg depth_output)
{
  pix_openni *me = (pix_openni*)GetMyClass(data);
  if ((depth_output >= 0) && (depth_output) <= 1)
		me->req_depth_output=(int)depth_output;
}

void pix_openni :: floatRgbMessCallback(void *data, t_floatarg rgb)
{
  pix_openni *me = (pix_openni*)GetMyClass(data);
  if ((int)rgb == 0)
		me->rgb_wanted=false;
  if ((int)rgb == 1)
		me->rgb_wanted=true;
}

void pix_openni :: floatDepthMessCallback(void *data, t_floatarg depth)
{
  pix_openni *me = (pix_openni*)GetMyClass(data);
  if ((int)depth == 0)
		me->depth_wanted=false;
  if ((int)depth == 1)
		me->depth_wanted=true;
}

void pix_openni ::floatMinMessCallback(void *data,t_floatarg min_thresh) {
	pix_openni *me = (pix_openni*)GetMyClass(data);
	me->min_thresh = (float)min_thresh;
	//me->post("min: %f",me->min_thresh);
}
        
 void pix_openni ::floatMaxMessCallback(void *data,t_floatarg max_thresh) {
 	pix_openni *me = (pix_openni*)GetMyClass(data);
 	me->max_thresh = (float)max_thresh;
 	//me->post("max: %f",me->max_thresh);
 }

void pix_openni :: renderDepthCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
	GetMyClass(data)->renderDepth(argc, argv);
}
