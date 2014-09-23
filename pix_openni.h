/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Adaptive threshold object

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2002 IOhannes m zmoelnig. forum::für::umläute. IEM. zmoelnig@iem.kug.ac.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef INCLUDE_pix_openni_H_
#define INCLUDE_pix_openni_H_



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <assert.h>
//#include <iostream>
//#include <cmath>
#include <vector>
#include <stdint.h>

#include "OpenNI.h"


#include "Base/GemBase.h"
#include "Gem/Properties.h"
#include "Gem/Image.h"
#include "Base/GemPixObj.h"


using namespace openni;
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_openni
    

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/
#ifdef _WIN32
class GEM_EXPORT pix_openni : public GemBase
#else
class GEM_EXTERN pix_openni : public GemBase
#endif
{
    CPPEXTERN_HEADER(pix_openni, GemBase);

    public:

	    //////////
	    // Constructor
    	pix_openni(int argc, t_atom *argv);
    
        bool Init();
    
    	t_outlet 	*m_dataout;
			
			
    Device g_Device;
    VideoStream g_depth;
    VideoStream g_image;
    VideoFrameRef g_depthFrame;
    VideoFrameRef g_imageFrame;
    
   
    int device_id;
    
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_openni();

			virtual void	startRendering();
    	//////////
    	// Rendering 	
			virtual void 	render(GemState *state);
			
			virtual void 	postrender(GemState *state);
		
		  // Stop Transfer
			virtual void	stopRendering();
    	
	//////////
    	// Settings/Info
    	void	    	     bangMess();
    	
		void				renderDepth(int argc, t_atom*argv);
			
			
  // Settings
  		

		int openni_ready;
			
    
      bool rgb_started;
      bool depth_started;
      bool audio_started;
	
      
      bool rgb_wanted;
      int rgb_index;
      bool depth_wanted;
      int depth_index;
      
      float min_thresh;
      float max_thresh;
      
			
	bool m_registration_wanted;
	bool m_registration;
	
	int	depth_output;
	int	req_depth_output;	
	
    	//////////
    	// The pixBlock with the current image
    	pixBlock    	m_image;
    	pixBlock    	m_depth;
    	
    	GemState					*depth_state;
    	
						
    private:
    	//////////
    	// Static member functions
    	static void    	bangMessCallback(void *data);
		static void    	floatRgbMessCallback(void *data, float rgb);
    	static void    	floatDepthMessCallback(void *data, float depth);
    	static void    	floatDepthOutputMessCallback(void *data, float depth_output);
    	static void     floatMinMessCallback(void *data,float min_thresh);
        static void     floatMaxMessCallback(void *data,float max_thresh);
        static void    	renderDepthCallback(void *data, t_symbol*s, int argc, t_atom*argv);

			t_outlet        *m_depthoutlet; 
			t_inlet         *m_depthinlet;

};

#endif	// for header file
