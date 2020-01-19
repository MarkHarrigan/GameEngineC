// MAIN_ENGINE_0_1.CPP basic full-screen pixel plotting DirectDraw demo
// Use OutputDebugString(""); to write messages to the output window

// INCLUDES ///////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN  // just say no to MFC

#define INITGUID // make sure directX guids are included.  Include this before headers

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




// DEFINES ////////////////////////////////////////////////

#ifdef UNICODE
#undef UNICODE
#endif

// defines for windows 
#define WINDOW_CLASS_NAME "WINCLASS1"

// default screen size
#define SCREEN_WIDTH    	1440  // size of screen
#define SCREEN_HEIGHT   	900
#define SCREEN_BPP      	8    // bits per pixel
#define MAX_COLORS      	256  // maximum colors
#define MAX_COLORS_PALETTE  256
#define BITMAP_ID			0x4D42		// Universal bitmap ID

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

// GLOBALS ////////////////////////////////////////////////
HWND      main_window_handle	 = NULL; // globally track main window
HINSTANCE hinstance_app     	 = NULL; // globally track hinstance

// directdraw stuff

LPDIRECTDRAW7        				lpdd    		 = NULL;   // dd object
LPDIRECTDRAWSURFACE7  				lpddsprimary 	 = NULL;   // dd primary surface
LPDIRECTDRAWSURFACE7  				lpddsback    	 = NULL;   // dd back surface
LPDIRECTDRAWPALETTE   				lpddpal      	 = NULL;   // a pointer to the created dd palette
LPDIRECTDRAWCLIPPER   				lpddclipper 	 = NULL;   // dd clipper
PALETTEENTRY          				palette[256];			   // color palette
PALETTEENTRY          				save_palette[256];		   // used to save palettes
DDSURFACEDESC2        				ddsd;                      // a direct draw surface description struct
DDBLTFX               				ddbltfx;                   // used to fill
DDSCAPS2              				ddscaps;                   // a direct draw surface capabilities struct
HRESULT               				ddrval;                    // result back from dd calls
DWORD                 				start_clock_count = 0;     // used for timing
HBITMAP								testbitmap;
#define 							filepath "H:\\Programming\\_WorkingGameEngine_0_1\\test.bmp";

// Version 0.2 - used for line drawing
int gx0 = 5, 
	gy0 = 5, 
	gx1 = 5, 
	gy1 = 5;

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

// FUNCTIONS //////////////////////////////////////////////
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



///////////////////////////////////////////////////////////

int Game_Main(void *parms = NULL, int num_parms = 0)
{
// this is the main loop of the game, do all your processing
// here

// for now test if user is hitting ESC and send WM_CLOSE
if (KEYDOWN(VK_ESCAPE))
   SendMessage(main_window_handle,WM_CLOSE,0,0);


// plot 1000 random pixels to the primary surface and return
// clear ddsd and set size, never assume it's clean
memset(&ddsd,0,sizeof(ddsd)); 
ddsd.dwSize = sizeof(ddsd);

if (FAILED(lpddsprimary->Lock(NULL, &ddsd,
                   DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
                   NULL)))
   {
   // error
   return(0);
   } // end if

// now ddsd.lPitch is valid and so is ddsd.lpSurface

// make a couple aliases to make code cleaner, so we don't
// have to cast
int mempitch        = (int)ddsd.lPitch;
UCHAR *video_buffer = (UCHAR *)ddsd.lpSurface;
UCHAR color = 255; // version 0.2

int return_val = 0; // version 0.2



for (int position=0; position<300; position++)
{
gx0 = gx0++;
gy0 = gy0++;
gy1 = gy1 + 2;
gx1 = gx1 + 2;

return_val = Draw_Line(gx0,gy0,gx1,gy1,color,video_buffer,mempitch); // version 0.2
Sleep(1);
//return_val = Draw_Line(50,500,500,50,color,video_buffer,mempitch); // version 0.2
//return_val = Draw_Line(5,5,5,500,color,video_buffer,mempitch); // version 0.2
}

gx0 = 5;
gy0 = gx0;
gx1 = 5;
gy1 = gx1;

// plot 1000 random pixels with random colors on the
// primary surface, they will be instantly visible
//for (int index=0; index < 1000; index++)
//    {
//    // select random position and color for the screen (width and height)
//    UCHAR color = rand()%256;
//    int x = rand()%SCREEN_WIDTH;
//    int y = rand()%SCREEN_HEIGHT;
//
//    // plot the pixel
//    video_buffer[x+y*mempitch] = color;        
//
//    } // end for index

// now unlock the primary surface
if (FAILED(lpddsprimary->Unlock(NULL)))
   return(0);

// sleep a bit
Sleep(30);

// return success or failure or your own return code here
return(1);

} // end Game_Main

////////////////////////////////////////////////////////////

int Game_Init(void *parms = NULL, int num_parms = 0)
{
// this is called once after the initial window is created and
// before the main event loop is entered, do all your initialization
// here

BITMAP_FILE bmpf;

//if(!(Load_Bitmap_File(&bmpf,"H:\\Programming\\_WorkingGameEngine_0_1\\test.bmp")))
//   return(0);
//
//if(!(Unload_Bitmap_File(&bmpf)))
//   return(0);

// create IDirectDraw interface 7.0 object and test for error
if (FAILED(DirectDrawCreateEx(NULL, (void **)&lpdd, IID_IDirectDraw7, NULL)))
   return(0);

// set cooperation to full screen
if (FAILED(lpdd->SetCooperativeLevel(main_window_handle, 
                                      DDSCL_FULLSCREEN | DDSCL_ALLOWMODEX | 
                                      DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT)))
   {
   // error
   return(0);
   } // end if

// set display mode
if (FAILED(lpdd->SetDisplayMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,0,0)))
   {
   // error
   return(0);
   } // end if


// clear ddsd and set size
memset(&ddsd,0,sizeof(ddsd)); 
ddsd.dwSize = sizeof(ddsd);

// enable valid fields
ddsd.dwFlags = DDSD_CAPS;

// request primary surface
ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

// create the primary surface
if (FAILED(lpdd->CreateSurface(&ddsd, &lpddsprimary, NULL)))
   {
   // error
   return(0);
   } // end if

// build up the palette data array
for (int color=1; color < 255; color++)
    {
    // fill with random RGB values
    palette[color].peRed   = rand()%256;
    palette[color].peGreen = rand()%256;
    palette[color].peBlue  = rand()%256;

    // set flags field to PC_NOCOLLAPSE
    palette[color].peFlags = PC_NOCOLLAPSE;
    } // end for color

// now fill in entry 0 and 255 with black and white
palette[0].peRed   = 0;
palette[0].peGreen = 0;
palette[0].peBlue  = 0;
palette[0].peFlags = PC_NOCOLLAPSE;

palette[255].peRed   = 255;
palette[255].peGreen = 255;
palette[255].peBlue  = 255;
palette[255].peFlags = PC_NOCOLLAPSE;

// create the palette object
if (FAILED(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256 | 
                                DDPCAPS_INITIALIZE, 
                                palette,&lpddpal, NULL)))
{
// error
return(0);
} // end if

// finally attach the palette to the primary surface
if (FAILED(lpddsprimary->SetPalette(lpddpal)))
    {
    // error
    return(0);
    } // end if

// Version 0.2 - create and attach clipper
if (FAILED(lpdd->CreateClipper(0,&lpddclipper,NULL)))
return(0);

// Version 0.2 - attach the clipper to the primary surface.
// Note that the clipper only works when using the blitter.
// You can still directly draw out of the bounds of the primary surface
if (FAILED(lpddclipper->SetHWnd(0, main_window_handle)))
return(0);

// return success or failure or your own return code here
return(1);

} // end Game_Init

/////////////////////////////////////////////////////////////

int Game_Shutdown(void *parms = NULL, int num_parms = 0)
{
// this is called after the game is exited and the main event
// loop while is exited, do all you cleanup and shutdown here

// first the palette
if (lpddpal)
   {
   lpddpal->Release();
   lpddpal = NULL;
   } // end if

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

// return success or failure or your own return code here
return(1);

} // end Game_Shutdown

// WINMAIN ////////////////////////////////////////////////

int WINAPI WinMain(	HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{

WNDCLASSEX winclass; // this will hold the class we create
HWND	   hwnd;	 // generic window handle
MSG		   msg;		 // generic message
HDC        hdc;      // graphics device context

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
						    SCREEN_WIDTH,SCREEN_HEIGHT,  // initial width, height
						    NULL,	  // handle to parent 
						    NULL,	  // handle to menu
						    hinstance,// instance of this application
						    NULL)))	// extra creation parms
return(0);

// save main window handle
main_window_handle = hwnd;

// initialize game here
Game_Init();

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

// closedown game here
Game_Shutdown();

// return to Windows like this
return(msg.wParam);

} // end WinMain


int Load_Bitmap_File(BITMAP_FILE_PTR bitmap, char *filename)
{
	int file_handle, index;

	UCHAR *temp_buffer = NULL;
	OFSTRUCT file_data;

	if ((file_handle = OpenFile(filename, &file_data, OF_READ)) == -1)
		return(0);

	_lread(file_handle, &bitmap->bitmapfileheader,sizeof(BITMAPFILEHEADER));

	if(bitmap->bitmapfileheader.bfType!=BITMAP_ID)
	{
		_lclose(file_handle);
		return(0);
	}

	_lread(file_handle,&bitmap->bitmapinfoheader, sizeof(BITMAPINFOHEADER));

	if (bitmap->bitmapinfoheader.biBitCount == 8)
	{
		_lread(file_handle, &bitmap->palette, MAX_COLORS_PALETTE*sizeof(PALETTEENTRY));

		for(index=0; index < MAX_COLORS_PALETTE; index++)
		{
			int temp_color					= bitmap->palette[index].peRed;
			bitmap->palette[index].peRed = bitmap->palette[index].peBlue;
			bitmap->palette[index].peBlue = temp_color;

			bitmap->palette[index].peFlags = PC_NOCOLLAPSE;

		} // end for index

	} // end if

// load the image data
	// THIS LINE CAUSES A DEBUG ASSERTION ERROR
//	_lseek(file_handle, (-(int)(bitmap->bitmapinfoheader.biSizeImage)),SEEK_END);

	// now read in the image
	 if (bitmap->bitmapinfoheader.biBitCount==8 ||
		 bitmap->bitmapinfoheader.biBitCount==16 ||
		 bitmap->bitmapinfoheader.biBitCount==24)
		{
		 if(bitmap->buffer)
			 free(bitmap->buffer);

		 //allocate memory for the image
		 if(!(bitmap->buffer = (UCHAR *)malloc(bitmap->bitmapinfoheader.biSizeImage)))
		 {
			 _lclose(file_handle);
			 return(0);
		 } // end if

		 // now read it in
		 _lread(file_handle,bitmap->buffer,bitmap->bitmapinfoheader.biSizeImage);

		} // end if
		else
		{
			 return(0);
		} // end else
			 
		 // close the file
		 _lclose(file_handle);
}

int Unload_Bitmap_File(BITMAP_FILE_PTR bitmap)
{
	if(bitmap->buffer)
	{
		free(bitmap->buffer);

		bitmap->buffer = NULL;
	} // end if

	return(1);

} // end Unload_Bitmap_File

int Draw_Line(int x0, int y0,
			  int x1, int y1,
			  UCHAR color,
			  UCHAR *vb_start,
			  int lpitch)

{
	int dx,		// difference in x's
		dy,		// difference in y's
		dx2,	// dx, dy * 2
		dy2,	
		x_inc,	// amount in pixel space to move during drawing
		y_inc,  // amount in pixel space to move during drawing
		error,  // the discriminant i.e. error i.e. decision variable
		index;  // used for looping

	// precompute first pixel address is video buffer
	vb_start = vb_start + x0 + y0 + y0*lpitch;

	// compute horizontal and vertical deltas
	dx = x1 - x0;
	dy = y1 - y0;

	// test which direction the line is going
	if (dx>=0)
	{
		x_inc = 1;
	}
	else
	{
		x_inc = -1;
		dx = -dx;
	}

	// test y component of slope
	if (dy>=0)
	{
		y_inc = lpitch;
	}
	else
	{
		y_inc = -lpitch;
		dy = -dy;
	}

	// compute (dx,dy) * 2
	dx2 = dx << 1;
	dy2 = dy << 1;

	// now based on which delta is greater we can draw the line
	if (dx > dy)
	{
		// initialise the error term
		error = dy2 - dx;

		// draw the line
		for (index=0; index <= dx; index++)
		{
			//set the pixel
			*vb_start = color;

			// test if error has overflowed
			if (error >= 0)
			{
				error-=dx2;

				//move to the next line
				vb_start+=y_inc;
			}// end if error overflowed
			
			// adjust the error term
			error+=dy2;

			// move to the next pixel
			vb_start+=x_inc;

		} // end for
	} // end if slope

	else
	{
		// initialise the error term
		error = dx2 - dy;

		// draw the line
		for (index=0; index <=dy; index++)
		{
			// set the pixel
			*vb_start = color;

			// test if error overflowed
			if (error >= 0)
			{
				error-=dy2;

				// move to the next line
				vb_start+=x_inc;
			} // end if error overflowed

			// adjust the error term
			error+=dx2;

			// move to the next pixel
			vb_start+=y_inc;

		} // end for

	} // end else slope

return(1);

} // end draw line

///////////////////////////////////////////////////////////

