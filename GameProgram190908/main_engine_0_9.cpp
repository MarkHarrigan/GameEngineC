// MAIN_ENGINE_0_1.CPP basic full-screen pixel plotting DirectDraw demo
// Use OutputDebugString(""); to write messages to the output window

// Version 0.5 - Includes a blit of a rectangle onto the primary surface
// Version 0.6 - Changes made to blit to the back buffer and flip the surfaces.  Also a call to DDLoadBitmap has been inserted
//				 into Game_Init
// Version 0.7 - New function added to create a clipper, set the clip list and attach the clipper to any surface.
//				 'Draw_Line' function removed as not needed. Load_Bitmap and Unload_Bitmap functions removed.
// Version 0.8 - Can't remember
// Version 0.9 - Version that draws a yellow circle to the screen and moves it along the x-axis.  Basic x-oriente edge collision detection

// DEFINES ////////////////////////////////////////////////

#ifdef UNICODE
#undef UNICODE
#endif

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    	1440  // size of screen
#define SCREEN_HEIGHT   	900
#define SCREEN_BPP      	32    // bits per pixel
#define MAX_COLORS      	256  // maximum colors
#define MAX_COLORS_PALETTE  256
#define BITMAP_ID			0x4D42		// Universal bitmap ID

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define INITGUID // make sure directX guids are included.  Include this before headers
#define 							filepath "H:\\Programming\\_WorkingGameEngine_0_1\\test.bmp";

// INCLUDES ///////////////////////////////////////////////
#include <Windows.h>   // include important windows stuff
#include <windowsx.h> 
#include <mmsystem.h>
#include <iostream> // include important C/C++ stuff
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> 
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <ddraw.h> // include directdraw
#include <dos.h> // required for the sleep() command
#include "ddutil.h"

// TYPES //////////////////////////////////////////////////////

// basic unsigned types
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;

// typedef for bitmap files
typedef struct BITMAP_FILE_TAG
{
	BITMAPFILEHEADER bitmapfileheader;
	BITMAPINFOHEADER bitmapinfoheader;
	PALETTEENTRY palette[256];
	UCHAR *buffer;
} BITMAP_FILE, *BITMAP_FILE_PTR;

// MACROS /////////////////////////////////////////////////

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

// initializes a direct draw struct
#define DD_INIT_STRUCT(ddstruct) { memset(&ddstruct,0,sizeof(ddstruct)); ddstruct.dwSize=sizeof(ddstruct); }

// PROTOTYPES
int Load_Bitmap_File(BITMAP_FILE_PTR , char *);
int Unload_Bitmap_File(BITMAP_FILE_PTR);
int Draw_Line(int, int, int, int,UCHAR ,UCHAR *,int);
int screen_x, screen_y, screen_bits;
int clear_screen();
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7, int, LPRECT);
void DisplayCaps();



// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle	 = NULL; // globally track main window
HINSTANCE hinstance_app     	 = NULL; // globally track hinstance
HDC		  hdc;
char outputtitle[] = "Working Game Engine";
char outputvsn[] = "Version 0.6\n";

// directdraw stuff
LPDIRECTDRAW7        				lpdd    		 = NULL;   // dd object
LPDIRECTDRAWSURFACE7  				lpddsprimary 	 = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  				lpddsback    	 = NULL;   // dd back surface
LPDIRECTDRAWSURFACE7  				lpddsbmp    	 = NULL;   // dd bitmap surface
LPDIRECTDRAWSURFACE7  				lpddsbackground	 = NULL;   // dd bitmap surface
LPDIRECTDRAWPALETTE   				lpddpal      	 = NULL;   // a pointer to the created dd palette
LPDIRECTDRAWCLIPPER   				lpddclipper 	 = NULL;   // dd clipper
DDSURFACEDESC2        				ddsd;                      // a direct draw surface description struct
DDBLTFX               				ddbltfx;                   // used to fill
DDSCAPS2              				ddscaps;                   // a direct draw surface capabilities struct
HRESULT               				ddrval;                    // result back from dd calls

// general globals
DWORD                 				start_clock_count = 0;     // used for timing
HBITMAP								testbitmap;
RECT								dest_rect, source_rect;	// Used for surface blitting
int									x_vel = 1;				// Used for the x velocity component

// Version 0.2 - used for line drawing
int gx0 = 5, 
	gy0 = 5, 
	gx1 = 5, 
	gy1 = 5;

int mouse_x = 0;
int mouse_y = 0;

int velocity_x = 5, velocity_y = 5;

// these defined the general clipping rectangle
int 	min_clip_x = 0,                          // clipping rectangle 
    	max_clip_x = SCREEN_WIDTH-1,
    	min_clip_y = 0,
    	max_clip_y = SCREEN_HEIGHT-1;

// these are overwritten globally by DD_Init()
int 	screen_width 	= SCREEN_WIDTH,            // width of screen
    	screen_height 	= SCREEN_HEIGHT,           // height of screen
    	screen_bpp    	= SCREEN_BPP;              // bits per pixel


char 	buffer[80];                     // general printing buffer

//====================================================================================================================================
//
// WINDOWPROC
//
//====================================================================================================================================

LRESULT CALLBACK WindowProc(HWND hwnd, 
							UINT msg, 
         					WPARAM wparam, 
                            LPARAM lparam)
{
// this is the main message handler of the system
PAINTSTRUCT		ps;		// used in WM_PAINT
HDC				hdc;	// handle to a device context
//char 			buffer[80];        // used to print strings

// what is the message 
switch(msg)
	{	
	case WM_CREATE: 
        {
		// do initialization stuff here
        // return success
		return(0);
		} break;

	case WM_MOUSEMOVE:
		{
			mouse_x = (int)LOWORD(lparam);
			mouse_y = (int)HIWORD(lparam);
		} break;
   
	case WM_PAINT: 
		{
		// simply validate the window 
   	    hdc = BeginPaint(hwnd,&ps);	 
        
        // end painting
        EndPaint(hwnd,&ps);

        // return success
		return(0);
   		} break;

	case WM_DESTROY: 
		{

		// kill the application, this sends a WM_QUIT message 
		PostQuitMessage(0);

        // return success
		return(0);
		} break;

	default:break;

    } // end switch

// process any messages that we didn't take care of 
return (DefWindowProc(hwnd, msg, wparam, lparam));

} // end WinProc

//====================================================================================================================================
//
// GAME_MAIN
//
//====================================================================================================================================

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

// make a couple aliases to make code cleaner, so we don't
// have to cast
int mempitch        = (int)ddsd.lPitch;
// Pointer to back buffer surface
UCHAR *back_buffer = (UCHAR *)ddsd.lpSurface;
UCHAR color = 255; // version 0.2
int return_val = 0; // version 0.2
//char output;
char xPosStr[25] = "";
char yPosStr[25] = "";


// clear ddsd and set size, never assume it's clean
DD_INIT_STRUCT(ddsd);

// now ddsd.lPitch is valid and so is ddsd.lpSurface
DD_INIT_STRUCT(ddbltfx);
ddbltfx.dwFillColor = 0,0,0;

if(lpddsback)
{

	// Clear the screen to ensure that the previously blitted bitmap image does not remain on the surface
	clear_screen(); 

	// Blit the bitmap from its surface onto the back buffer
	ddrval = lpddsback->Blt(NULL, // pointer to dest RECT
							lpddsbackground, // pointer to source surface
							NULL, // pointer to source RECT
							DDBLT_WAIT | DDBLT_KEYDEST, // do a colour fill and wait if you have to
							NULL);
	


	// Blit the bitmap from its surface onto the back buffer
	ddrval = lpddsback->Blt(&dest_rect, // pointer to dest RECT
							lpddsbmp, // pointer to source surface
							&source_rect, // pointer to source RECT
							DDBLT_WAIT | DDBLT_KEYDEST, // do a colour fill and wait if you have to
							NULL);

// Check the return code from the blit operation
if (ddrval != DD_OK)
 {
	switch(ddrval)
	{
		case DDERR_GENERIC:
			 OutputDebugString("DDERR_GENERIC\n");
	         break;

		case DDERR_INVALIDCLIPLIST:
			 OutputDebugString("DDERR_INVALIDCLIPLIST\n");
			 break;

		case DDERR_INVALIDOBJECT:
			 OutputDebugString("DDERR_INVALIDOBJECT\n");
			 break;

		case DDERR_INVALIDPARAMS:
			 OutputDebugString("DDERR_INVALIDPARAMS\n");
			 break;
 
		case DDERR_INVALIDRECT:
			 OutputDebugString("DDERR_INVALIDRECT\n");
			 break;
 
		case DDERR_NOALPHAHW:
			 OutputDebugString("DDERR_NOALPHAHW\n");
			 break;
 
		case DDERR_NOBLTHW:
			 OutputDebugString("DDERR_NOBLTHW\n");
			 break;	
 
		case DDERR_NOCLIPLIST:
			 OutputDebugString("DDERR_NOCLIPLIST\n");
			 break;
 
		case DDERR_NORASTEROPHW:
			 OutputDebugString("DDERR_NORASTEROPHW\n");
			 break;
 
		case DDERR_NOSTRETCHHW:
			 OutputDebugString("DDERR_NOSTRETCHHW\n");
			 break;
 
		case DDERR_SURFACEBUSY:
			 OutputDebugString("DDERR_SURFACEBUSY\n");
			 break;
 
		case DDERR_SURFACELOST:
			 OutputDebugString("DDERR_SURFACELOST\n");
			 break;
 
		case DDERR_UNSUPPORTED:
			 OutputDebugString("DDERR_UNSUPPORTED\n");
			 break;
 
		case DDERR_WASSTILLDRAWING:
			 OutputDebugString("DDERR_WASSTILLDRAWING\n");
			 break;

	} // End Switch

 } //end if

} // end check on lpddsback

if(lpddsback)
{

	// Clear the screen to ensure that the previously blitted bitmap image does not remain on the surface

	// Blit the bitmap from its surface onto the back buffer
	ddrval = lpddsback->Blt(&dest_rect, // pointer to dest RECT
							lpddsbmp, // pointer to source surface
							&source_rect, // pointer to source RECT
							DDBLT_WAIT | DDBLT_KEYDEST, // do a colour fill and wait if you have to
							NULL);

// Check the return code from the blit operation
if (ddrval != DD_OK)
 {
	switch(ddrval)
	{
		case DDERR_GENERIC:
			 OutputDebugString("DDERR_GENERIC\n");
	         break;

		case DDERR_INVALIDCLIPLIST:
			 OutputDebugString("DDERR_INVALIDCLIPLIST\n");
			 break;

		case DDERR_INVALIDOBJECT:
			 OutputDebugString("DDERR_INVALIDOBJECT\n");
			 break;

		case DDERR_INVALIDPARAMS:
			 OutputDebugString("DDERR_INVALIDPARAMS\n");
			 break;
 
		case DDERR_INVALIDRECT:
			 OutputDebugString("DDERR_INVALIDRECT\n");
			 break;
 
		case DDERR_NOALPHAHW:
			 OutputDebugString("DDERR_NOALPHAHW\n");
			 break;
 
		case DDERR_NOBLTHW:
			 OutputDebugString("DDERR_NOBLTHW\n");
			 break;	
 
		case DDERR_NOCLIPLIST:
			 OutputDebugString("DDERR_NOCLIPLIST\n");
			 break;
 
		case DDERR_NORASTEROPHW:
			 OutputDebugString("DDERR_NORASTEROPHW\n");
			 break;
 
		case DDERR_NOSTRETCHHW:
			 OutputDebugString("DDERR_NOSTRETCHHW\n");
			 break;
 
		case DDERR_SURFACEBUSY:
			 OutputDebugString("DDERR_SURFACEBUSY\n");
			 break;
 
		case DDERR_SURFACELOST:
			 OutputDebugString("DDERR_SURFACELOST\n");
			 break;
 
		case DDERR_UNSUPPORTED:
			 OutputDebugString("DDERR_UNSUPPORTED\n");
			 break;
 
		case DDERR_WASSTILLDRAWING:
			 OutputDebugString("DDERR_WASSTILLDRAWING\n");
			 break;

	} // End Switch

 } //end if

} // end check on lpddsback

// Now flip the Primary Surface.  The Primary now becomes the back buffer and vice-versa
while (FAILED(lpddsprimary->Flip(NULL,DDFLIP_WAIT)));

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   SendMessage(main_window_handle,WM_CLOSE,0,0);

// First time through the left value is 0. 
// This will increment the value to 1
	dest_rect.left += x_vel;
	dest_rect.right += x_vel;

// Check for collision with the sides of the screen
if(dest_rect.left == 0 || dest_rect.right > SCREEN_WIDTH -1)
  x_vel *= -1;	
	
// return success or failure or your own return code here
return(1);

} // end Game_Main

//====================================================================================================================================
//
// GAME_INIT
//
//====================================================================================================================================

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

//BITMAP_FILE bmpf;
DDCOLORKEY colour_key;

colour_key.dwColorSpaceHighValue = 0;
colour_key.dwColorSpaceLowValue = 0;

screen_x = GetSystemMetrics(SM_CXSCREEN);
screen_y = GetSystemMetrics(SM_CYSCREEN);

// Now write the version number to the debug console
OutputDebugString(outputvsn);

OutputDebugString("Creating DirectDraw interface.\n");

		// create IDirectDraw interface 7.0 object and test for error
if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
{
	OutputDebugString("Failed to create DirectDraw interface.\n");
   return(0);
}

OutputDebugString("...DirectDraw interface created succesfully.\n");

OutputDebugString("Setting Co-operative level.\n");

// set co-operation to full screen
if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
                                      DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | 
                                      DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)))
   {
   // error
   OutputDebugString("Failed to set Co-operative level.\n");
   return(0);
   } // end if

OutputDebugString("...co-operative level set.\n");

OutputDebugString("Setting display mode.\n");

// set display mode
if (FAILED(lpdd->SetDisplayMode(screen_x, screen_y, SCREEN_BPP,0,0)))
   {
   // error
   OutputDebugString("Failed to set display mode.\n");
   return(0);
   } // end if

OutputDebugString("...display mode set.\n");

// clear ddsd and set size
DD_INIT_STRUCT(ddsd);

// enable valid fields
ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;

ddsd.dwBackBufferCount = 1;

// request primary surface
ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_COMPLEX | DDSCAPS_FLIP;

ddsd.ddckCKSrcBlt.dwColorSpaceHighValue = 0;
ddsd.ddckCKSrcBlt.dwColorSpaceLowValue = 0;

OutputDebugString("Creating surface.\n");

// create the primary surface
if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
   {
   // error
   OutputDebugString("Failed to create primary surface.\n");
   return(0);
   } // end if

OutputDebugString("...primary surface created.\n");

ddsd.ddsCaps.dwCaps = DDSCAPS_BACKBUFFER;

OutputDebugString("Getting attached surface.\n");

// Get the attached back buffer
if (FAILED(lpddsprimary->GetAttachedSurface(&ddsd.ddsCaps,&lpddsback )))
{
	OutputDebugString("Cant get attached surface.\n");
}

OutputDebugString("...attached surface retrieved.\n");

// Set up a RECT structure for the clipping region of the back surface
RECT rect_list[1] = {0,0,screen_x-1, screen_y-1};

// Create the clipper and attach it to the back surface.  Use the RECT as defined above.
lpddclipper = DDraw_Attach_Clipper(lpddsback,1,rect_list);

hdc	= GetDC(main_window_handle);
SetTextColor(hdc,RGB(255,255,255));
SetBkColor(hdc,RGB(0,0,0));
SetBkMode(hdc,TRANSPARENT);

OutputDebugString("Attempting to load bitmap.\n");

// Load the bitmap image
if(!(lpddsbmp = DDLoadBitmap(lpdd, "test.bmp", 0, 0)))
{
	OutputDebugString("Cant load the bitmap.\n");
	return(0);
}

OutputDebugString("...bitmap loaded successfully.\n");

OutputDebugString("Attempting to load background.\n");

// Load the bitmap image
if(!(lpddsbackground = DDLoadBitmap(lpdd, "background.bmp", 0, 0)))
{
	OutputDebugString("Cant load the background.\n");
	return(0);
}

OutputDebugString("...background loaded successfully.\n");

OutputDebugString("Attempting to set the colour key on the back surface.\n");

if(FAILED(lpddsback->SetColorKey(DDCKEY_DESTBLT, &colour_key)))
{
	OutputDebugString("Cant set colour key for back surface.\n");
	return(0);

}

OutputDebugString("...colour key set successfully.\n");

OutputDebugString("Initialising destination rectangle.\n");

dest_rect.left = 0;
dest_rect.top = 0;
dest_rect.right = 254;
dest_rect.bottom = 254;

source_rect.left = 0;
source_rect.top = 0;
source_rect.right = 254;
source_rect.bottom = 254;

OutputDebugString("...destination rectangle initialised successfully.\n");

// return success or failure or your own return code here
return(1);

} // end Game_Init

//====================================================================================================================================
//
// GAME_SHUTDOWN
//
//====================================================================================================================================

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

	OutputDebugString("Now shutting down...\n");

// first the palette
if (lpddpal)
   {
   lpddpal->Release();
   lpddpal = NULL;
   } // end if

// clear the surface used to hold the bitmap
if (lpddsbmp)
{
	lpddsbmp->Release();
	lpddsbmp = NULL;
}

// now the primary surface
if (lpddsprimary)
   {
   lpddsprimary->Release();
   lpddsprimary = NULL;
   } // end if

// now blow away the IDirectDraw4 interface
if (lpdd)
   {
   lpdd->Release();
   lpdd = NULL;
   } // end if

OutputDebugString("...cleared objects.\n");

// return success or failure or your own return code here
return(1);

} // end Game_Shutdown

//====================================================================================================================================
//
// WINMAIN
//
//====================================================================================================================================

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{

WNDCLASSEX winclass; // this will hold the class we create
HWND	   hwnd;	 // generic window handle
MSG		   msg;		 // generic message
//HDC        hdc;      // graphics device context

// first fill in the window class stucture
winclass.cbSize         = sizeof(WNDCLASSEX);
winclass.style			= CS_DBLCLKS | CS_OWNDC | 
                          CS_HREDRAW | CS_VREDRAW;
winclass.lpfnWndProc	= WindowProc;
winclass.cbClsExtra		= 0;
winclass.cbWndExtra		= 0;
winclass.hInstance		= hinstance;
winclass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
winclass.hCursor		= LoadCursor(NULL, IDC_ARROW); 
winclass.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
winclass.lpszMenuName	= NULL;
winclass.lpszClassName	= WINDOW_CLASS_NAME;
winclass.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

// save hinstance in global
hinstance_app = hinstance;

// register the window class
if (!RegisterClassEx(&winclass))
	return(0);

// create the window
if (!(hwnd = CreateWindowEx(NULL,                  // extended style
                            WINDOW_CLASS_NAME,     // class
						    "DirectDraw Full-Screen Demo", // title
						    WS_POPUP | WS_VISIBLE,
					 	    0,0,	  // initial x,y
						    GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN),  // initial width, height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	// extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

// initialize game here
Game_Init();

OutputDebugString("Now calling GameMain loop.\n");

// enter main event loop
while(TRUE)
	{
    // test if there is a message in queue, if so get it
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	   { 
	   // test if this is a quit
       if (msg.message == WM_QUIT)
           break;
	
	   // translate any accelerator keys
	   TranslateMessage(&msg);

	   // send the message to the window proc
	   DispatchMessage(&msg);
	   } // end if
    
       // main game processing goes here
       Game_Main();
       
	} // end while

OutputDebugString("Now calling Game shutdown.\n");

// closedown game here
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain
//====================================================================================================================================
//
// DDRAW_ATTACH_CLIPPER
//
//====================================================================================================================================
LPDIRECTDRAWCLIPPER DDraw_Attach_Clipper(LPDIRECTDRAWSURFACE7 lpdds, int num_rects, LPRECT clip_list)
{
// Create a clipper from the sent clip list and attach it to the sent surface

	int index;						// looping variable
	LPDIRECTDRAWCLIPPER lpddclipper;// pointer to the newly created DD clipper
	LPRGNDATA region_data;			// pointer the the region data that contains the header and clip list

	// Create the clipper
	if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
		return(NULL);

	// Now create the clip list

	// Allocate memory
	region_data = (LPRGNDATA)malloc(sizeof(RGNDATA)+num_rects*sizeof(RECT));

	// copy the rects into the region data
	memcpy(region_data->Buffer,clip_list,sizeof(RECT)*num_rects);

	// set up the header fields
	region_data->rdh.dwSize = sizeof(RGNDATAHEADER);
	region_data->rdh.iType = RDH_RECTANGLES;
	region_data->rdh.nCount = num_rects;
	region_data->rdh.nRgnSize = num_rects*sizeof(RECT);
	region_data->rdh.rcBound.left = 64000;
	region_data->rdh.rcBound.top = 64000;
	region_data->rdh.rcBound.right = -64000;
	region_data->rdh.rcBound.bottom = -64000;

	// find bounds of clipping regions
	for(index=0; index<num_rects;index++)
	{
		if(clip_list[index].left < region_data->rdh.rcBound.left)
			region_data->rdh.rcBound.left = clip_list[index].left;

		if(clip_list[index].right < region_data->rdh.rcBound.right)
			region_data->rdh.rcBound.right = clip_list[index].right;

		if(clip_list[index].top < region_data->rdh.rcBound.top)
			region_data->rdh.rcBound.top = clip_list[index].top;

		if(clip_list[index].bottom < region_data->rdh.rcBound.bottom)
			region_data->rdh.rcBound.bottom = clip_list[index].bottom;

	} // end for index

	// now the bounding rectangle region has been computed, set the clipping list

	OutputDebugString("Attempting to set Clipping List.\n");

	if(FAILED(lpddclipper->SetClipList(region_data,0)))
	{
		free(region_data);
		OutputDebugString("Cant set Clipping List.\n");
		return(NULL);
	} // end if

	OutputDebugString("...clipping List set successfully.\n");

	OutputDebugString("Attempting to set Clipper.\n");

	if(FAILED(lpdds->SetClipper(lpddclipper)))
	{
		free(region_data);
		OutputDebugString("Cant set Clipper.\n");
		return(NULL);
	} // end if
		
	OutputDebugString("...clipper set successfully.\n");

	// if processing has got to this part then all has gone well.
	// release memory and send back the pointer to the new clipper
	free(region_data);

	OutputDebugString("DDRAW_ATTACH_CLIPPER SUCCESSFUL.\n");

	return(lpddclipper);

} // end DDraw_Attach_Clipper

//====================================================================================================================================
//
// DISPLAY_CAPS
//
//====================================================================================================================================

void DisplayCaps()
{
	LPDDCAPS hel_caps, hal_caps;
	char buffer;

	//DD_INIT_STRUCT(hel_caps);
	//DD_INIT_STRUCT(hal_caps);

	if(FAILED(lpdd->GetCaps(hel_caps,hal_caps)))
		OutputDebugString("Cant get CAPS.\n");

	_ltoa_s(hal_caps->dwSVBCaps,&buffer,sizeof(hal_caps->dwSVBCaps),10);

//	OutputDebugString(buffer);

}

//====================================================================================================================================
//
// CLEAR_SCREEN
//
//====================================================================================================================================

int clear_screen()
{
	if(lpddsback)
{
    
		ddrval = lpddsback->Blt(NULL, // pointer to dest RECT
								NULL, // pointer to source surface
								NULL, // pointer to source RECT
								DDBLT_WAIT | DDBLT_COLORFILL , // do a colour fill and wait if you have to
								&ddbltfx);

if (ddrval != DD_OK)
{
	switch(ddrval)
	{
		case DDERR_GENERIC:
			OutputDebugString("DDERR_GENERIC\n");
			break;

		case DDERR_INVALIDCLIPLIST:
			OutputDebugString("DDERR_INVALIDCLIPLIST\n");
			break;

		case DDERR_INVALIDOBJECT:
			OutputDebugString("DDERR_INVALIDOBJECT\n");
			break;

		case DDERR_INVALIDPARAMS:
			OutputDebugString("DDERR_INVALIDPARAMS\n");
			break;
 
		case DDERR_INVALIDRECT:
			OutputDebugString("DDERR_INVALIDRECT\n");
			break;
 
		case DDERR_NOALPHAHW:
			OutputDebugString("DDERR_NOALPHAHW\n");
			break;
 
		case DDERR_NOBLTHW:
			OutputDebugString("DDERR_NOBLTHW\n");
			break;
 
		case DDERR_NOCLIPLIST:
			OutputDebugString("DDERR_NOCLIPLIST\n");
			break;
 
		case DDERR_NORASTEROPHW:
			OutputDebugString("DDERR_NORASTEROPHW\n");
			break;
 
		case DDERR_NOSTRETCHHW:
			OutputDebugString("DDERR_NOSTRETCHHW\n");
			break;
 
		case DDERR_SURFACEBUSY:
			OutputDebugString("DDERR_SURFACEBUSY\n");
			break;
 
		case DDERR_SURFACELOST:
			OutputDebugString("DDERR_SURFACELOST\n");
			break;
 
		case DDERR_UNSUPPORTED:
			OutputDebugString("DDERR_UNSUPPORTED\n");
			break;
 
		case DDERR_WASSTILLDRAWING:
			OutputDebugString("DDERR_WASSTILLDRAWING\n");
			break;

	}
}

}
return 0;
}

