#include <windows.h>
#include <stdint.h>
#define internal static 
#define local_persist static 
#define global_variable static 

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;


struct win_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int BytesPerPixel;
	int Pitch;
};
struct win_window_dimension
{
	int Width;
	int Height;
};
global_variable bool GlobalRunning;
//Todo this is a global temporarily
global_variable win_offscreen_buffer GlobalBackBuffer;

internal win_window_dimension GetDimension (HWND Window)
{
	win_window_dimension Result;
	RECT ClientRect;
	GetClientRect( Window,&ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	return Result;
}

internal void RenderWeirdGradient(win_offscreen_buffer *Buffer,int XOffset, int YOffset)
{
	u8 *Row= (u8 *)Buffer->Memory;
	
	for(int Y=0; Y<Buffer->Height; Y++)
	{
		u32 *Pixel = (u32 *)Row;
		
		for(int X=0; X<Buffer->Width; X++)
		{
			u8 Blue = (X+XOffset);
			u8 Green = (Y+YOffset);
			*Pixel = ((Green<<8) | Blue);
			Pixel++;
		}
		Row += Buffer->Pitch;
	}
}

internal void ResizeDIBSection(win_offscreen_buffer *Buffer,int Width, int Height)
{

	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory,0,MEM_RELEASE);
	}
	Buffer->Width = Width;
	Buffer->Height = Height;

	Buffer->BytesPerPixel =4;
	Buffer->Pitch = Width * Buffer->BytesPerPixel;
	

	//Todo: Maybe don't free first, free after, then free first if that fails

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;
	Buffer->Info.bmiHeader.biSizeImage = 0;
	Buffer->Info.bmiHeader.biXPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biYPelsPerMeter = 0;
	Buffer->Info.bmiHeader.biClrImportant = 0;
	
	Buffer->BytesPerPixel = 4;
	
	int BitmapMemorySize = (Buffer->Width*Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0,BitmapMemorySize,MEM_COMMIT,PAGE_READWRITE);
	
	RenderWeirdGradient(&GlobalBackBuffer,128,0);

}
internal void DisplayBufferToWindow(HDC DeviceContext,int WindowWidth,int WindowHeight,win_offscreen_buffer *Buffer,
					int X,int Y,int Width,int Height)
{
	StretchDIBits(DeviceContext,
			  /*X ,Y , Width, Height,
			    X ,Y , Width, Height,*/
			  0,0, WindowWidth, WindowHeight,
			  0,0, Buffer->Width, Buffer->Height,
			  Buffer->Memory,
			  &Buffer->Info,
			  DIB_RGB_COLORS,
			  SRCCOPY);
}

LRESULT CALLBACK MainWindowCallback(HWND Window,
						UINT Message,
						WPARAM WParam,
						LPARAM LParam)
{
	LRESULT Result = 0;
	switch(Message)
	{
	case WM_SIZE:
		{
			break;	
		}
	case WM_DESTROY:
		{
			//Todo: Handle this as an error - recreate window?
			GlobalRunning=false;
			break;	
		}
	case WM_CLOSE:
		{
			//Todo Handle this with a message to the user?
			GlobalRunning=false;
			break;	
		}
	case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			break;	
		}
	case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			
			win_window_dimension Dimension =  GetDimension(Window);
			ResizeDIBSection(&GlobalBackBuffer,Dimension.Width,Dimension.Height);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;

			LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
			
			
			DisplayBufferToWindow(DeviceContext,Dimension.Width,Dimension.Height,
					    &GlobalBackBuffer,X,Y,Width,Height);
			
			EndPaint(Window, &Paint);

		}
	default:
		{
			//OutputDebugStringA("default\n");
			Result = DefWindowProc(Window,Message,WParam, LParam);
			break;
		}
	}
	return (Result);
}
	
int CALLBACK WinMain(
			   HINSTANCE Instance,
			   HINSTANCE PrevInstance,
			   LPSTR CommandLine,
			   int ShowCode )
{
	WNDCLASS WindowClass = {};

	ResizeDIBSection(&GlobalBackBuffer,1280,720);
	
	//TODO: Check if HREDRAW/VREDRAW/OWNDC still matter
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW; 
	WindowClass.lpfnWndProc = MainWindowCallback; 
	// int cbClsExtra; 
	// int cbWndExtra; 
	WindowClass.hInstance = Instance; 
	//HICON hIcon; 
	//LPCTSTR lpszMenuName; 
	WindowClass.lpszClassName = "CppEngineWindowClass";

	if(RegisterClass(&WindowClass))
	{
		HWND WindowHandle =
			CreateWindowEx(
					   0,
					   WindowClass.lpszClassName,
					   "CppEngine",
					   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					   CW_USEDEFAULT,
					   CW_USEDEFAULT,
					   CW_USEDEFAULT,
					   CW_USEDEFAULT,
					   0,
					   0,
					   Instance,
					   0);
		if(WindowHandle != NULL)
		{
			GlobalRunning = true;
			MSG Message;
			int XOffset = 0;
			int YOffset = 0;
			while(GlobalRunning)
			{
				BOOL MessageResult = PeekMessage(&Message,0,0,0,PM_REMOVE);
				if(MessageResult)
				{
					if(Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				RenderWeirdGradient(&GlobalBackBuffer,XOffset,YOffset);
				{
					win_window_dimension Dimension = GetDimension(WindowHandle);
					HDC DeviceContext = GetDC(WindowHandle);

					DisplayBufferToWindow(DeviceContext,Dimension.Width,Dimension.Height,
							    &GlobalBackBuffer,
							    0,0, Dimension.Width,Dimension.Height);
					ReleaseDC(WindowHandle,DeviceContext);
					
				}
				XOffset++;
				YOffset +=2;

			}

		}
		else
		{
			//Logging
		}
	}
	else
	{
		//Logging
	}
	return (0);					
}
