#include <windows.h>
#define internal static 
#define local_persist static 
#define global_variable static 
//Todo this is a global temporarily
global_variable bool Running;

global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void ResizeDIBSection(int Width, int Height)
{
	//Todo: Maybe don't free first, free after, then free first if that fails
	if(BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}
	if(!BitmapDeviceContext)
	{
		//Todo: Should we recreate these under certain special circumstances
		BitmapDeviceContext = CreateCompatibleDC(0);
	}
	BITMAPINFO BitmapInfo;
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	BitmapHandle = CreateDIBSection(
						  BitmapDeviceContext, &BitmapInfo,
						  DIB_RGB_COLORS,
						  &BitmapMemory,
						  0,0);
	
}
internal void WinUpdateWindow(HDC DeviceContext,int X,int Y,int Width,int Height)
{
	
	StretchDIBits(DeviceContext,
			  X ,Y , Width, Height,
			  X ,Y , Width, Height,
			  BitmapMemory,
			  &BitmapInfo,
			  DIB_RGB_COLORS, SRCCOPY);
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
			RECT ClientRect;
			GetClientRect( Window,&ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;
			ResizeDIBSection(Width,Height);
			OutputDebugStringA("WM_SIZE\n");
			break;	
		}
	case WM_DESTROY:
		{
			//Todo: Handle this as an error - recreate window?
			Running=false;
			break;	
		}
	case WM_CLOSE:
		{
			//Todo Handle this with a message to the user?
			Running=false;
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
			BeginPaint(Window,&Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
			WinUpdateWindow(DeviceContext,X,Y,Width,Height);
			
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
					   WS_OVERLAPPEDWINDOW|WS_VISIBLE,
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
			Running = true;
			while(Running)
			{
				MSG Message;
				
				BOOL MessageResult = GetMessage(&Message,0,0,0);
				if(MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				else
				{
					break;
				}
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
