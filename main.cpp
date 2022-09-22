#include<Windows.h>
#include <shellapi.h>
#include <shellscalingapi.h>
#include <strsafe.h>
#include<CommCtrl.h>
#include <windowsx.h> 
#include <cstdarg> 
#include"resource.h"

#pragma comment(lib, "Shcore.lib")
#pragma comment(lib,"Winmm.lib")
/*MACRO START*/

#define PRESETFILE L".preset"
#define LASTTIMEFILE L".time"

#define DEFAULT_ERROR_TITLE L"Error Happened"

#define mainWindowWidth 400
#define mainWindowHeight 300
#define CBXTIMERLISTWIDTH 150 
#define CBXTIMERLISTHEIGHT 100 
#define BUTTONWIDTH 120
#define BUTTONHEIGHT 30
#define EDITWIDTH 60
#define EDITHEIGHT 30
#define COLONWIDTH 10
#define COLONHEIGHT 30

#define menuWindowWidth 90
#define menuWindowHeight 30
#define tipWindowWidth 300
#define tipWindowHeight 150

#define RTN1 return -1
#define RT0 return 0
#define RT1 return 1
#define RT2 return 2
#define RT3 return 3

#define CBXID 0xCB1
#define EIDH 0xE01
#define EIDM 0xE02
#define EIDS 0xE03
#define BIDPRESET 0xB01
#define BIDSTART 0xB02
#define BIDRESET 0xB03

#define HK_ALT_S 0xE1
#define HK_ALT_R 0xE2
#define HK_ALT_C 0xE13
#define VK_S 0x53
#define VK_R 0x52
#define VK_C 0x56

#define COLOR_BLUE RGB(162,229,247)
#define COLOR_ORANGE RGB(0,0,0)

#define TEXTLIMIT 2

// Error tips message box with parent Window, without Title
#define ErrorTipNT(parentHwnd,Text) \
  MessageBoxW(parentHwnd, Text,DEFAULT_ERROR_TITLE, MB_OK)
// Error tips message box without neither parent window and title 
#define ErrorTipNTNP(Text) MessageBoxW(NULL, Text,DEFAULT_ERROR_TITLE, MB_OK) 
// Error tips message box with parent window and title
#define ErrorTip(parentHwnd,Text,Title) \
  MessageBoxW(parentHwnd, Text, Title, MB_OK)
// Error tips message box with  title, without parent window 
#define ErrorTipNP(Text,Title) MessageBoxW(NULL, Text,Title, MB_OK)

#define MSGSTR_TASKBAR_RESATRT L"TaskbarCreated"
#define MSG_SYSTRAYICON (WM_USER+0xA01)

/*MACRO END*/

/*STRUCTURE DEFINITION START*/

typedef struct _TimeOfTimer
{
	CHAR status;
	CHAR iHour;
	CHAR iMinute;
	CHAR iSecond;
}TimeOfTimer,* pTimeOfTimer;

/*STRUCTURE DEFINITION END*/

/*GENERAL FUNC START*/

ATOM WindowClassRegister ( HINSTANCE hIns,WNDPROC wndProc,LPCWSTR className,
						   HBRUSH hBackground,LPCWSTR menuName,HICON hIcon,
						   HCURSOR hCursor,UINT style );

VOID AddNotificationIcon ( HWND hWnd );
VOID DeleteNotificationIcon ();

int GetCenterX ( int pw,int cw );
int GetCenterY ( int ph,int ch );


int getstartx ( int pw,int space,int rn,int num,... );
int getstarty ( int ph,int ch,int starty );

VOID DrawTextTip ( HWND hWnd,int fontSize,COLORREF fontColor,LPWSTR text );


/*GENERAL FUNC END*/


/*SPECIAL FUNC START*/

int GetXMainWin ();
int GetYMainWin ();

int GetYTipWin ();
int GetXTipWin ();

LRESULT CALLBACK mainWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam );
LRESULT CALLBACK menuWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam );
LRESULT CALLBACK tipWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam );

DWORD WINAPI TipThread ( LPVOID lParameter );
BOOL tipThreadFlag = TRUE;
BOOL tipShow = FALSE;

DWORD WINAPI TimerThread ( LPVOID lp );

void AddControls ( HWND hParent );

BOOL LoadPreset ();
BOOL StorePreset ();
BOOL LoadLast ();
BOOL StoreLast ();
/*SPECIAL FUNC END*/

/*GLOBAL VARIABLE START*/

const WCHAR * clsnameMainWindow = L"TimerMainWindow";
const WCHAR * clsnameMenuWindow = L"TimerMenuWindow";
const WCHAR * clsnameTipWindow = L"TimerTipWindow";
const WCHAR * captionMainWindow = L"Timer";
const WCHAR  tipNotificationIcon[ 128 ] = L"[DoubleClick/ALT+V]\n show/hide Window\n[RightClick]\n show menu\n[Alt+S]\n start/pause/continue timer\n[Alt+R]\n reset timer";

LPCWSTR presetFirst = L"07:00:00";
LPCWSTR presetSecond = L"03:00:00";
LPCWSTR presetThird = L"02:00:00";
LPCWSTR presetFourth = L"01:00:00";
LPCWSTR presetFifth = L"00:30:00";

WCHAR tipTimerStart[] = L"Timer Start";
WCHAR tipTimerPause[] = L"Timer Pause";
WCHAR tipTimerContinue[] = L"Timer Continue";
WCHAR tipTimerReset[] = L"Timer Reset";
WCHAR tipTimerFinish[] = L"Timer Finish";

HINSTANCE hInstance;
HWND hMainWindow;
HWND hcbxList;
HWND hbtnAddPreset;
HWND hudButtonHour;
HWND hudButtonMinute;
HWND hudButtonSecond;
HWND hssColon1;
HWND hssColon2;
HWND hbtnStart;
HWND hbtnReset;

HWND hMenuWindow;
HWND hTipWindow;
HICON hProgramIcon;
HBRUSH hBackgroundBrush;

HANDLE semaphoreForTipThread;
HANDLE semaphoreForTimerThread;

HANDLE hfPreset = INVALID_HANDLE_VALUE;
HANDLE hfLastTime = INVALID_HANDLE_VALUE;

//HANDLE hwave = INVALID_HANDLE_VALUE;
//HANDLE hrs = INVALID_HANDLE_VALUE;

const int tipFontSize = 18;
BOOL ishover;
TimeOfTimer lefttime {};
BOOL isAltS = FALSE;
BOOL isAltR = FALSE;
BOOL isFinish = FALSE;

BOOL isUsing = TRUE;
BOOL isButtonReset = FALSE;

WCHAR ehour[ 3 ] {};
WCHAR eminute[ 3 ] {};
WCHAR esecond[ 3 ] {};

WCHAR bufPreset[ 1024 ] {};
/*GLOBAL VARIABLE END*/

//Entry Point
INT WINAPI wWinMain ( _In_ HINSTANCE hIns,_In_opt_ HINSTANCE hPrevIns,
					  _In_ LPWSTR wstrCmdLine,_In_ int showCmd )
{
	hInstance = hIns;
	hProgramIcon = (HICON) LoadImageW ( hIns,MAKEINTRESOURCEW ( IDI_TIMER ),IMAGE_ICON,0,0,LR_DEFAULTSIZE );
	hBackgroundBrush = (HBRUSH) GetStockObject ( WHITE_BRUSH );
	HBRUSH tipBKBrush = CreateSolidBrush ( COLOR_BLUE );
	/*hwave = FindResourceW ( hIns,MAKEINTRESOURCE ( IDR_WAVE1 ),L"WAVE" );
	if( hwave==NULL ) {
		RTN1;
	}
	hrs = LoadResource ( hIns,(HRSRC) hwave );
	if( hrs==NULL ) {
		RTN1;
	}*/
	if( NULL==hProgramIcon||NULL==hBackgroundBrush ) {
		ErrorTipNTNP ( L"Initial Icon or Background Brush Error!" );
		//FreeResource ( hrs );
		RTN1;
	}

	if( !WindowClassRegister ( hIns,mainWindowProcedure,clsnameMainWindow,hBackgroundBrush,NULL,hProgramIcon,NULL,NULL )
		||!WindowClassRegister ( hIns,menuWindowProcedure,clsnameMenuWindow,hBackgroundBrush,NULL,NULL,NULL,NULL )
		||!WindowClassRegister ( hIns,tipWindowProcedure,clsnameTipWindow,hBackgroundBrush,NULL,NULL,NULL,NULL ) ) {
		ErrorTipNTNP ( L"Register Window Class Failed" );
	//	FreeResource ( hrs );
		RTN1;
	}



	hMainWindow = CreateWindowExW ( NULL,clsnameMainWindow,captionMainWindow,WS_OVERLAPPED|WS_SYSMENU,
									GetXMainWin (),GetYMainWin (),mainWindowWidth,mainWindowHeight,NULL,NULL,hIns,NULL );
	//addcontrol



	hMenuWindow = CreateWindowExW ( WS_EX_TOPMOST|WS_EX_NOACTIVATE,clsnameMenuWindow,NULL,WS_POPUP|WS_BORDER,0,0,
									menuWindowWidth,menuWindowHeight,NULL,NULL,hIns,NULL );

	hTipWindow = CreateWindowExW ( WS_EX_TOPMOST|WS_EX_LAYERED|WS_EX_NOACTIVATE,clsnameTipWindow,NULL,WS_POPUP|WS_BORDER,
								   GetXTipWin (),GetYTipWin (),tipWindowWidth,tipWindowHeight,NULL,NULL,hIns,NULL );
	SetLayeredWindowAttributes ( hTipWindow,RGB ( 192,135,242 ),160,LWA_COLORKEY|LWA_ALPHA );

	//Get Create Window Result
	if( NULL==hMainWindow||NULL==hMenuWindow||NULL==hTipWindow ) {
		ErrorTipNTNP ( L"Create Window failed." );
	//	FreeResource ( hrs );
		RTN1;
	}
	semaphoreForTipThread = CreateSemaphoreW ( NULL,0,1,NULL );
	semaphoreForTimerThread = CreateSemaphoreW ( NULL,0,1,NULL );
	HANDLE hTipThread = CreateThread ( NULL,0,TipThread,NULL,NULL,NULL );
	HANDLE hTimerThread = CreateThread ( NULL,0,TimerThread,NULL,NULL,NULL );
	if( hTipThread==INVALID_HANDLE_VALUE||semaphoreForTipThread==INVALID_HANDLE_VALUE||semaphoreForTimerThread==INVALID_HANDLE_VALUE ) {
		DeleteObject ( tipBKBrush );
		ErrorTipNTNP ( L"Create Thread failed." );
		//FreeResource ( hrs );
		RTN1;
	}

	RegisterHotKey ( hTipWindow,HK_ALT_S,MOD_ALT,VK_S );
	RegisterHotKey ( hTipWindow,HK_ALT_R,MOD_ALT,VK_R );
	RegisterHotKey ( hTipWindow,HK_ALT_C,MOD_ALT,VK_C );
	ShowWindow ( hMainWindow,showCmd );
	MSG msg {};
	while( GetMessageW ( &msg,0,0,0 ) ) {
		TranslateMessage ( &msg );
		DispatchMessageW ( &msg );
	}
	UnregisterHotKey ( hTipWindow,HK_ALT_S );
	UnregisterHotKey ( hTipWindow,HK_ALT_R );
	UnregisterHotKey ( hTipWindow,HK_ALT_C );
	DeleteObject ( tipBKBrush );

	//clsoe Threads
	tipThreadFlag = FALSE;
	isUsing = FALSE;
	ReleaseSemaphore ( semaphoreForTipThread,1,NULL );
	ReleaseSemaphore ( semaphoreForTimerThread,1,NULL );
	WaitForSingleObject ( hTipThread,-1 );
	WaitForSingleObject ( hTimerThread,-1 );
	CloseHandle ( semaphoreForTipThread );
	CloseHandle ( semaphoreForTimerThread );
	StorePreset ();
	CloseHandle ( hfLastTime );
	//FreeResource ( hrs );
	RT0;
}

//WindowClassRegister,fail return zero,success return an unique id
ATOM WindowClassRegister ( HINSTANCE hIns,WNDPROC wndProc,LPCWSTR className,
						   HBRUSH hBackground,LPCWSTR menuName,HICON hIcon,
						   HCURSOR hCursor,UINT style )
{
	WNDCLASSEXW wc {};
	wc.cbSize = sizeof ( WNDCLASSEXW );
	wc.hInstance = hIns;
	wc.lpfnWndProc = wndProc;
	wc.lpszClassName = className;
	wc.hbrBackground = hBackground;
	wc.lpszMenuName = menuName;
	if( style==NULL )
		wc.style = CS_HREDRAW|CS_VREDRAW;
	else
		wc.style = style;
	wc.hIcon = hIcon;
	wc.hCursor = hCursor;
	return RegisterClassExW ( &wc );
}

LRESULT CALLBACK mainWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam )
{
	static UINT msgIDTaskbarRestart;
	switch( uMsg ) {
		case WM_CREATE:
		{
			msgIDTaskbarRestart = RegisterWindowMessageW ( MSGSTR_TASKBAR_RESATRT );
			AddNotificationIcon ( hWnd );
			AddControls ( hWnd );
			break;
		}
		case WM_CLOSE:
		{
			ShowWindow ( hWnd,SW_HIDE );
			RT0;
		}
		case WM_DESTROY:
		{
			DeleteNotificationIcon ();
			PostQuitMessage ( 0 );
			break;
		}
		case WM_COMMAND:
		{
			WORD cid = LOWORD ( wParam );
			WORD nid = HIWORD ( wParam );
			HWND hwnd = (HWND) lParam;
			if( hwnd==hudButtonHour||hwnd==hudButtonMinute||hwnd==hudButtonSecond ) {
				switch( nid ) {
					case EN_CHANGE:
					{
						WCHAR str[ 3 ] {};
						Edit_GetText ( hwnd,str,TEXTLIMIT+1 );
						int number = wcstol ( str,NULL,10 );
						if( !(number>=0&&number<60) ) {
							Edit_SetText ( hwnd,L"0" );
							RT0;
						}
						break;
					}
				}
			} else if( hwnd==hcbxList ) {
				switch( nid ) {
					case CBN_SELCHANGE:
					{
						WCHAR timestr[ 100 ] {};
						GetWindowTextW ( hwnd,timestr,100 );
						WCHAR * str = NULL;
						WCHAR * ptr = NULL;
						str = wcstok_s ( timestr,L":\r\n",&ptr );
						int i = 0;
						while( str!=NULL ) {
							if( i==0 )
								Edit_SetText ( hudButtonHour,str );
							else if( i==1 )
								Edit_SetText ( hudButtonMinute,str );
							else
								Edit_SetText ( hudButtonSecond,str );
							str = wcstok_s ( NULL,L":\r\n",&ptr );
							i++;
						}
						break;
					}
				}
			} else if( hwnd==hbtnStart ) {
				if( nid==BN_CLICKED ) {
					switch( lefttime.status ) {
						case 0:
						{
							WCHAR numstr[ TEXTLIMIT+1 ] {};
							Edit_GetText ( hudButtonHour,numstr,TEXTLIMIT+1 );
							lefttime.iHour = wcstol ( numstr,NULL,10 );
							Edit_GetText ( hudButtonMinute,numstr,TEXTLIMIT+1 );
							lefttime.iMinute = wcstol ( numstr,NULL,10 );
							Edit_GetText ( hudButtonSecond,numstr,TEXTLIMIT+1 );
							lefttime.iSecond = wcstol ( numstr,NULL,10 );

							lefttime.status = 1;
							ReleaseSemaphore ( semaphoreForTimerThread,1,NULL );
							SetWindowTextW ( hwnd,L"Pause" );
							break;
						}
						case 1:
						{
							lefttime.status = 2;
							SetWindowTextW ( hwnd,L"Continue" );
							break;
						}
						case 2:
						{

							lefttime.status = 1;
							ReleaseSemaphore ( semaphoreForTimerThread,1,NULL );
							SetWindowTextW ( hwnd,L"Pause" );
							break;
						}
						default:
							break;
					}
				}
			} else if( hwnd==hbtnReset ) {
				if( nid==BN_CLICKED ) {
					isButtonReset = TRUE;
					lefttime.status = 0;
					lefttime.iHour = 0;
					lefttime.iMinute = 0;
					lefttime.iSecond = 0;
					Edit_SetText ( hudButtonHour,L"00" );
					Edit_SetText ( hudButtonMinute,L"00" );
					Edit_SetText ( hudButtonSecond,L"00" );

					SetWindowTextW ( hbtnStart,L"Start" );
				}
			} else if( hbtnAddPreset==hwnd ) {
				//status==0, get edit content ,combine as xx:xx:xx,compare with combobox list and 00:00:00,if no same item exists,add it to list.
				WCHAR hstr[ TEXTLIMIT+1 ] {};
				WCHAR mstr[ TEXTLIMIT+1 ] {};
				WCHAR sstr[ TEXTLIMIT+1 ] {};
				WCHAR tstr[ TEXTLIMIT+1 ] {};
				WCHAR combine[ TEXTLIMIT*3+3 ] {};
				int l = 0;
				Edit_GetText ( hudButtonHour,tstr,TEXTLIMIT+1 );
				l = wcslen ( tstr );
				if( l<1 ) {
					StringCbPrintfW ( hstr,sizeof ( hstr ),L"00" );
				} else if( l<2 ) {
					StringCbPrintfW ( hstr,sizeof ( hstr ),L"0%s",tstr );
				}

				Edit_GetText ( hudButtonMinute,tstr,TEXTLIMIT+1 );
				l = wcslen ( tstr );
				if( l<1 ) {
					StringCbPrintfW ( mstr,sizeof ( mstr ),L"00" );
				} else if( l<2 ) {
					StringCbPrintfW ( mstr,sizeof ( mstr ),L"0%s",tstr );
				}

				Edit_GetText ( hudButtonSecond,tstr,TEXTLIMIT+1 );
				l = wcslen ( tstr );
				if( l<1 )
					StringCbPrintfW ( sstr,sizeof ( sstr ),L"00" );
				else if( l<2 ) {
					StringCbPrintfW ( sstr,sizeof ( sstr ),L"0%s",tstr );
				}

				StringCbPrintfW ( combine,sizeof ( combine ),L"%s:%s:%s",hstr,mstr,sstr );
				if( CompareStringOrdinal ( L"00:00:00",-1,combine,-1,TRUE )==CSTR_EQUAL )
					break;
				int count = ComboBox_GetCount ( hcbxList );
				if( ComboBox_FindStringExact ( hcbxList,-1,combine )==CB_ERR ) {
					ComboBox_AddString ( hcbxList,combine );
				}
			}


			break;
		}
		case MSG_SYSTRAYICON:
		{
			WORD msgID = LOWORD ( lParam );
			WORD iconID = HIWORD ( lParam );
			int x = GET_X_LPARAM ( wParam );
			int y = GET_Y_LPARAM ( wParam );
			if( iconID==IDI_TIMER )
				switch( msgID ) {
					case WM_RBUTTONDOWN:
					{
						SetWindowPos ( hMenuWindow,NULL,x,y-menuWindowHeight*1.5,0,0,SWP_NOSIZE|SWP_NOZORDER );
						ShowWindow ( hMenuWindow,SW_NORMAL );
						SetForegroundWindow ( hMenuWindow );
						break;
					}
					case WM_LBUTTONDBLCLK:
					{
						if( IsWindowVisible ( hMainWindow ) )
							ShowWindow ( hMainWindow,SW_HIDE );
						else
							ShowWindow ( hMainWindow,SW_NORMAL );
						break;
					}
				}
			break;
		}
		default:
			if( uMsg==msgIDTaskbarRestart ) AddNotificationIcon ( hWnd );
			break;
	}
	return DefWindowProcW ( hWnd,uMsg,wParam,lParam );
}

LRESULT CALLBACK menuWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam )
{
	static TRACKMOUSEEVENT tme { sizeof ( TRACKMOUSEEVENT ),TME_LEAVE|TME_HOVER ,hWnd ,HOVER_DEFAULT };
	UINT interval = 100;
	SystemParametersInfo ( SPI_SETMOUSEHOVERTIME,interval,NULL,0 );
	switch( uMsg ) {
		case WM_PAINT:
		{
			HFONT font = CreateFontW ( 0,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,GB2312_CHARSET,CLIP_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,NULL );
			HBRUSH brush;
			if( ishover )
				brush = CreateSolidBrush ( RGB ( 220,220,220 ) );
			else
				brush = (HBRUSH) GetStockObject ( WHITE_BRUSH );
			PAINTSTRUCT ps;
			RECT rect {};
			rect.left = 0;
			rect.right = menuWindowWidth;
			rect.top = 0;
			rect.bottom = menuWindowHeight;
			HDC hdc = BeginPaint ( hWnd,&ps );
			FillRect ( hdc,&rect,brush );
			SetBkMode ( hdc,TRANSPARENT );
			SelectObject ( hdc,font );
			WCHAR text[] = L"Exit";
			DrawTextExW ( hdc,text,-1,&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE,NULL );
			EndPaint ( hWnd,&ps );
			DeleteObject ( font );
			if( ishover )
				DeleteObject ( brush );
			break;
			// RT0;
		}
		case WM_KILLFOCUS:
		{
			ShowWindow ( hMenuWindow,SW_HIDE );
			break;
		}
		case WM_LBUTTONDOWN:
		{
			isUsing = FALSE;
			lefttime.status = 4;
			if( lefttime.status!=1&&lefttime.status!=3 )
				ReleaseSemaphore ( semaphoreForTimerThread,1,NULL );


			break;
		}
		case WM_MOUSEMOVE:
		{
			TrackMouseEvent ( &tme );
			break;
		}
		case WM_MOUSEHOVER:
		{
			ishover = TRUE;
			InvalidateRect ( hWnd,NULL,TRUE );
			break;
		}
		case WM_MOUSELEAVE:
		{
			ishover = FALSE;
			InvalidateRect ( hWnd,NULL,TRUE );
			break;
		}
	}
	return DefWindowProcW ( hWnd,uMsg,wParam,lParam );
}

LRESULT CALLBACK tipWindowProcedure ( HWND hWnd,UINT uMsg,WPARAM wParam,LPARAM lParam )
{
	switch( uMsg ) {
		case WM_CREATE:
		{
			break;
		}
		case WM_CLOSE:
		{
			DestroyWindow ( hWnd );
			break;
		}
		case WM_DESTROY:
		{
			PostQuitMessage ( 0 );
			break;
		}
		case  WM_PAINT:
		{
			if( isAltS ) {
				if( 0==lefttime.status ) {
					DrawTextTip ( hWnd,tipFontSize,COLOR_ORANGE,tipTimerReset );

				} else if( 1==lefttime.status ) {
					DrawTextTip ( hWnd,tipFontSize,COLOR_ORANGE,tipTimerStart );
				} else if( 2==lefttime.status ) {
					DrawTextTip ( hWnd,tipFontSize,COLOR_ORANGE,tipTimerPause );
				}
			} else if( isAltR ) {
				DrawTextTip ( hWnd,tipFontSize,COLOR_ORANGE,tipTimerReset );
			} else if( isFinish ) {
				DrawTextTip ( hWnd,tipFontSize,COLOR_ORANGE,tipTimerFinish );
			}
			isAltR = FALSE;
			isFinish = FALSE;
			isAltS = FALSE;
			break;
		}
		case WM_HOTKEY:
		{
			int id = (INT) wParam;
			UINT modifierKey = LOWORD ( lParam );
			UINT virtualKey = HIWORD ( lParam );
			if( modifierKey==MOD_ALT )
				if( VK_S==virtualKey ) {
					if( IsWindowVisible ( hWnd ) )
						ShowWindow ( hWnd,SW_HIDE );
					isAltS = TRUE;
					SendMessageW ( hMainWindow,WM_COMMAND,MAKEWPARAM ( BIDSTART,BN_CLICKED ),(LPARAM) hbtnStart );
					ShowWindow ( hWnd,SW_NORMAL );
				} else if( VK_R==virtualKey ) {
					if( IsWindowVisible ( hWnd ) )
						ShowWindow ( hWnd,SW_HIDE );
					isAltR = TRUE;
					SendMessageW ( hMainWindow,WM_COMMAND,MAKEWPARAM ( BIDRESET,BN_CLICKED ),(LPARAM) hbtnReset );
					ShowWindow ( hWnd,SW_NORMAL );
				} else if( VK_C==virtualKey ) {
					if( IsWindowVisible ( hMainWindow ) )
						ShowWindow ( hMainWindow,SW_HIDE );
					else {
						ShowWindow ( hMainWindow,SW_NORMAL );
						SetForegroundWindow ( hMainWindow );
					}
				} else
					break;

				//InvalidateRect ( hWnd,NULL,TRUE );
				break;
		}
		case WM_SHOWWINDOW:
		{
			if( wParam&&lParam==0&&tipShow==FALSE ) {
				tipShow = TRUE;
				ReleaseSemaphore ( semaphoreForTipThread,1,NULL );
			}
			break;
		}
	}
	return DefWindowProcW ( hWnd,uMsg,wParam,lParam );
}

VOID AddNotificationIcon ( HWND hWnd )
{
	NOTIFYICONDATAW nid {};
	nid.cbSize = sizeof ( nid );
	nid.hWnd = hWnd;
	nid.uID = (UINT) IDI_TIMER;
	nid.uFlags = NIF_MESSAGE|NIF_ICON|NIF_TIP;
	nid.uCallbackMessage = MSG_SYSTRAYICON;
	nid.hIcon = hProgramIcon;
	HRESULT hr = StringCbCopyW ( nid.szTip,sizeof ( nid.szTip ),tipNotificationIcon );
	Shell_NotifyIconW ( NIM_ADD,&nid );
	nid.uVersion = NOTIFYICON_VERSION_4;
	Shell_NotifyIconW ( NIM_SETVERSION,&nid );
}

VOID DeleteNotificationIcon ()
{
	NOTIFYICONDATAW nid {};
	nid.cbSize = sizeof ( nid );
	nid.hWnd = hMainWindow;
	nid.uID = IDI_TIMER;
	Shell_NotifyIconW ( NIM_DELETE,&nid );
}

int GetCenterX ( int pw,int cw )
{
	return pw/2-cw/2;
}

int GetCenterY ( int ph,int ch )
{
	return ph/2-ch/2;
}

int GetXMainWin ()
{
	return GetCenterX ( GetSystemMetrics ( SM_CXSCREEN ),mainWindowWidth );
}

int GetYMainWin ()
{
	return GetCenterY ( GetSystemMetrics ( SM_CYSCREEN ),mainWindowHeight );
}


int GetXTipWin ()
{
	return GetCenterX ( GetSystemMetrics ( SM_CXSCREEN ),tipWindowWidth );
}

int GetYTipWin ()
{
	return GetCenterY ( GetSystemMetrics ( SM_CYSCREEN ),tipWindowHeight );
}

VOID DrawTextTip ( HWND hWnd,int fontSize,COLORREF fontColor,LPWSTR text )
{
	HFONT font = CreateFontW ( fontSize+10,fontSize,0,0,FW_NORMAL,FALSE,FALSE,FALSE,GB2312_CHARSET,CLIP_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH|FF_DONTCARE,NULL );

	PAINTSTRUCT ps;
	RECT rect {};
	rect.left = 0;
	rect.right = tipWindowWidth;
	rect.top = 0;
	rect.bottom = tipWindowHeight;
	HDC hdc = BeginPaint ( hWnd,&ps );
	SetBkMode ( hdc,TRANSPARENT );
	SetBkColor ( hdc,fontColor );
	SetTextColor ( hdc,fontColor );
	SelectObject ( hdc,font );
	DrawTextExW ( hdc,text,-1,&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE,NULL );
	EndPaint ( hWnd,&ps );
	DeleteObject ( font );
}

DWORD WINAPI TipThread ( LPVOID lParameter )
{
	while( tipThreadFlag ) {
		WaitForSingleObject ( semaphoreForTipThread,-1 );
		if( tipShow==TRUE ) {
			Sleep ( 1000 );
			ShowWindow ( hTipWindow,SW_HIDE );
			tipShow = FALSE;
		}
	}
	RT0;
}

int getstartx ( int pw,int space,int rn,int num,... )
{
	va_list vl;
	va_start ( vl,num );
	int tw = 0;
	int nspace = (rn-1)*space;
	int swn = 0;
	int tmp = 0;
	while( num>0 ) {
		tmp = va_arg ( vl,int );
		tw += tmp;
		if( rn>1 )
			swn += tmp;
		num--;
		rn--;
		if( num>0 )
			tw += space;
	}
	va_end ( vl );
	tmp = pw/2-tw/2+swn+nspace;
	return tmp;
}

int getstarty ( int ph,int ch,int starty )
{
	return starty+ph/2-ch/2;

}

void AddControls ( HWND hParent )
{

	RECT rect;
	GetClientRect ( hParent,&rect );
	int width = rect.right;
	int height = rect.bottom;
	hcbxList = CreateWindowExW ( NULL,WC_COMBOBOXW,NULL,CBS_DROPDOWNLIST|WS_CHILD|WS_VISIBLE|CBS_HASSTRINGS|WS_BORDER|WS_VSCROLL,
								 getstartx ( width,10,1,2,CBXTIMERLISTWIDTH,BUTTONWIDTH ),
								 getstarty ( height/3,BUTTONHEIGHT,0 ),
								 CBXTIMERLISTWIDTH,CBXTIMERLISTHEIGHT,hParent,(HMENU) CBXID,hInstance,NULL );
	if( !LoadPreset () ) {
		int rt = SendMessageW ( hcbxList,CB_ADDSTRING,NULL,(LPARAM) presetFirst );
		rt = SendMessageW ( hcbxList,CB_ADDSTRING,NULL,(LPARAM) presetSecond );
		rt = SendMessageW ( hcbxList,CB_ADDSTRING,NULL,(LPARAM) presetThird );
		rt = SendMessageW ( hcbxList,CB_ADDSTRING,NULL,(LPARAM) presetFourth );
		rt = SendMessageW ( hcbxList,CB_ADDSTRING,NULL,(LPARAM) presetFifth );
	}

	hbtnAddPreset = CreateWindowW ( WC_BUTTONW,L"CreatePreset",BS_CENTER|WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
									getstartx ( width,10,2,2,CBXTIMERLISTWIDTH,BUTTONWIDTH ),
									getstarty ( height/3,BUTTONHEIGHT,0 ),
									BUTTONWIDTH,BUTTONHEIGHT,hParent,(HMENU) BIDPRESET,hInstance,NULL );

	hudButtonHour = CreateWindowW ( WC_EDITW,NULL,WS_VISIBLE|WS_CHILD|ES_CENTER|WS_BORDER|ES_NUMBER,
									getstartx ( width,0,1,5,EDITWIDTH,COLONWIDTH,EDITWIDTH,COLONWIDTH,EDITWIDTH ),
									getstarty ( height/3,EDITHEIGHT,height/3 ),
									EDITWIDTH,EDITHEIGHT,hParent,(HMENU) EIDH,hInstance,NULL );

	hssColon1 = CreateWindowW ( WC_STATICW,L":",SS_CENTER|WS_VISIBLE|WS_CHILD,
								getstartx ( width,0,2,5,EDITWIDTH,COLONWIDTH,EDITWIDTH,COLONWIDTH,EDITWIDTH ),
								getstarty ( height/3,EDITHEIGHT,height/3 ),
								COLONWIDTH,COLONHEIGHT,hParent,(HMENU) 0xC04,hInstance,NULL );
	//SetClassLongPtrW (hssColon1,GCLP_HBRBACKGROUND,(LONG_PTR)hBackgroundBrush);

	hudButtonMinute = CreateWindowW ( WC_EDITW,NULL,WS_VISIBLE|WS_CHILD|ES_CENTER|WS_BORDER|ES_NUMBER,
									  getstartx ( width,0,3,5,EDITWIDTH,COLONWIDTH,EDITWIDTH,COLONWIDTH,EDITWIDTH )
									  ,getstarty ( height/3,EDITHEIGHT,height/3 ),
									  EDITWIDTH,EDITHEIGHT,hParent,(HMENU) EIDM,hInstance,NULL );

	hssColon2 = CreateWindowW ( WC_STATICW,L":",SS_CENTER|WS_VISIBLE|WS_CHILD,
								getstartx ( width,0,4,5,EDITWIDTH,COLONWIDTH,EDITWIDTH,COLONWIDTH,EDITWIDTH ),
								getstarty ( height/3,EDITHEIGHT,height/3 ),
								COLONWIDTH,COLONHEIGHT,hParent,(HMENU) 0xC06,hInstance,NULL );
	//SetClassLongPtrW ( hssColon2,GCLP_HBRBACKGROUND,(LONG_PTR) hBackgroundBrush );

	hudButtonSecond = CreateWindowW ( WC_EDITW,NULL,WS_VISIBLE|WS_CHILD|ES_CENTER|WS_BORDER|ES_NUMBER,
									  getstartx ( width,0,5,5,EDITWIDTH,COLONWIDTH,EDITWIDTH,COLONWIDTH,EDITWIDTH ),
									  getstarty ( height/3,EDITHEIGHT,height/3 ),
									  EDITWIDTH,EDITHEIGHT,hParent,(HMENU) EIDS,hInstance,NULL );
	Edit_LimitText ( hudButtonHour,TEXTLIMIT );
	Edit_LimitText ( hudButtonMinute,TEXTLIMIT );
	Edit_LimitText ( hudButtonSecond,TEXTLIMIT );



	hbtnStart = CreateWindowW ( WC_BUTTONW,L"Start",BS_CENTER|WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
								getstartx ( width,50,1,2,BUTTONWIDTH,BUTTONWIDTH ),
								getstarty ( height/3,BUTTONHEIGHT,height/3*2 ),
								BUTTONWIDTH,BUTTONHEIGHT,hParent,(HMENU) BIDSTART,hInstance,NULL );

	hbtnReset = CreateWindowW ( WC_BUTTONW,L"Reset",BS_CENTER|WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON,
								getstartx ( width,50,2,2,BUTTONWIDTH,BUTTONWIDTH ),
								getstarty ( height/3,BUTTONHEIGHT,height/3*2 ),
								BUTTONWIDTH,BUTTONHEIGHT,hParent,(HMENU) BIDRESET,hInstance,NULL );
	LoadLast ();

}

void ChangeEditUI ()
{
	int tmp = lefttime.iHour;
	int tmp1 = lefttime.iMinute;
	int tmp2 = lefttime.iSecond;
	_itow_s ( tmp,ehour,sizeof ( ehour ),10 );
	_itow_s ( tmp1,eminute,sizeof ( eminute ),10 );
	_itow_s ( tmp2,esecond,sizeof ( esecond ),10 );
	Edit_SetText ( hudButtonHour,ehour );
	Edit_SetText ( hudButtonMinute,eminute );
	Edit_SetText ( hudButtonSecond,esecond );
}

DWORD WINAPI TimerThread ( LPVOID lp )
{
	while( isUsing ) {
		WaitForSingleObject ( semaphoreForTimerThread,-1 );
		while( lefttime.status==1 ) {
			Sleep ( 1000 );
			if( lefttime.iSecond>0 ) {
				lefttime.iSecond--;
			} else {
				if( lefttime.iMinute>0 ) {
					lefttime.iMinute--;
					lefttime.iSecond = 59;
				} else {
					if( lefttime.iHour>0 ) {
						lefttime.iHour--;
						lefttime.iMinute = 59;
						lefttime.iSecond = 59;
					} else {
						lefttime.iHour = 0;
						lefttime.status = 3;
						PlaySoundW ( MAKEINTRESOURCE(IDR_WAVE1),hInstance,SND_ASYNC|SND_NODEFAULT|SND_RESOURCE);
						 
						if( !IsWindowVisible ( hMainWindow ) )
							ShowWindow ( hMainWindow,SW_NORMAL );
						SetForegroundWindow ( hMainWindow );
					}
				}
			}
			ChangeEditUI ();
			StoreLast ();
			FlushFileBuffers ( hfLastTime );
			//修改界面
		}
		if( lefttime.status==0&&isButtonReset==FALSE ) {
			lefttime.iHour = 0;
			lefttime.iMinute = 0;
			lefttime.iSecond = 0;
			Edit_SetText ( hudButtonHour,L"00" );
			Edit_SetText ( hudButtonMinute,L"00" );
			Edit_SetText ( hudButtonSecond,L"00" );
			SetWindowTextW ( hbtnStart,L"Start" );
		} else if( lefttime.status==0&&isButtonReset==TRUE ) {
			isButtonReset==FALSE;
		} else if( lefttime.status==3 ) {
			if( IsWindowVisible ( hTipWindow ) )
				ShowWindow ( hTipWindow,SW_HIDE );
			isFinish = TRUE;
			ShowWindow ( hTipWindow,SW_NORMAL );
			SetWindowTextW ( hbtnStart,L"Start" );
			lefttime.status = 0;
		} else if( lefttime.status==4 ) {
			SendMessage ( hMainWindow,WM_DESTROY,NULL,NULL );
		}
	}

	RT0;
}


BOOL LoadPreset ()
{
	hfPreset = CreateFileW ( PRESETFILE,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	if( hfPreset==INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	DWORD read = 0;
	WCHAR wchar;
	int i = 0;
	BOOL flag = FALSE;
	for( ; ReadFile ( hfPreset,&wchar,sizeof ( wchar ),&read,NULL ); i++ ) {
		if( read!=sizeof ( wchar ) )
			break;
		if( wchar!=L'\r'&&wchar!=L'\n' )
			bufPreset[ i ] = wchar;
		else {
			bufPreset[ i ] = NULL;
			if( wcslen ( bufPreset )>0 ) {
				ComboBox_AddString ( hcbxList,bufPreset );
				flag = TRUE;
			}
			i = -1;
		}
	}

	return flag;
}

BOOL StorePreset ()
{
	int c = ComboBox_GetCount ( hcbxList );
	WCHAR b[ 10 ] {};
	SetFilePointer ( hfPreset,NULL,NULL,FILE_BEGIN );
	for( size_t i = 0; i<c; i++ ) {
		ComboBox_GetLBText ( hcbxList,i,b );
		WriteFile ( hfPreset,b,wcslen ( b )*sizeof ( WCHAR ),NULL,NULL );
		WriteFile ( hfPreset,L"\n",1*sizeof ( WCHAR ),NULL,NULL );
	}
	CloseHandle ( hfPreset );

	return TRUE;
}


BOOL LoadLast ()
{

	hfLastTime = CreateFileW ( LASTTIMEFILE,GENERIC_ALL,FILE_SHARE_READ,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL );
	if( hfLastTime==INVALID_HANDLE_VALUE ) {
		return FALSE;
	}
	TimeOfTimer t {};
	DWORD rlen = 0;
	ReadFile ( hfLastTime,&t,sizeof ( t ),&rlen,NULL );
	if( rlen!=sizeof ( t ) ) {
		return FALSE;
	}
	lefttime.iHour = t.iHour;
	lefttime.iMinute = t.iMinute;
	lefttime.iSecond = t.iSecond;
	ChangeEditUI ();
	return TRUE;
}
BOOL StoreLast ()
{

	SetFilePointer ( hfLastTime,NULL,NULL,FILE_BEGIN );
	WriteFile ( hfLastTime,&lefttime,sizeof ( lefttime ),NULL,NULL );
	return TRUE;
}