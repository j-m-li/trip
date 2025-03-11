
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "folder.h"

var quit(void);
var term_deinit();

var print10(var n)
{
	printf("%ld", n);
	return 0;

}

var flush()
{
	fflush(stdout);
	return 0;
}

var print(var txt)
{
	if (!txt) {
		printf("(nullptr)");
		return -1;
	}
	printf("%s", (char*)txt);
	return 0;
}

var printb(var buf, var len)
{
#ifdef _WIN32
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), 
			(char*)buf, len, NULL, NULL);
#else
	fwrite((void*)buf, len, 1, stdout);
#endif
	return 0;
}


var file_delete(var path)
{
#ifdef _WIN32
	return DeleteFileA((char*)path) == 0;
#else
	return unlink((char*)path);
#endif
}

var file_rename(var src, var dest)
{
#ifdef _WIN32
	return MoveFileA((char*)src, (char*)dest) == 0;
#else
	return rename((char*)src, (char*)dest);
#endif
}


var file_size(var path)
{
	FILE *fp;
	var si;
	fp = fopen((char*)path, "rb");
    
	if (!fp) {
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	si = ftell(fp);
	fclose(fp);
	return si;
}

var file_load(var path, var offset, var size)
{
	char *buf;
	FILE *fp;
	var ret;
	fp = fopen((char*)path, "rb");
	if (!fp) {
		return 0;
	}
	buf = malloc(size+1);
	if (!buf) {
		return 0;
	}
	fseek(fp, offset, SEEK_SET);
	ret = fread(buf, 1, size, fp);
	if (ret != size) {
		free(buf);
		buf = 0;
	}
	buf[size] = '\0';
	fclose(fp);
	return (var)buf;
}

var file_save(var path, var offset, var buf, var size)
{
	FILE *fp;
	var ret;
	char *mode1 = "rb+";
	char *mode2 = "wb+";
	if (offset < 0) {
		mode1 = "wb";	
	}
	fp = fopen((char*)path, mode1);
	if (!fp) {
		fp = fopen((char*)path, mode2);
		if (!fp) {
			return -1;
		}
	}
	if (offset > 0) {
		if (fseek(fp, offset, SEEK_SET)) {
			fseek(fp, 0, SEEK_SET);
		}
	}
	ret = fwrite((void*)buf, 1, size, fp);
	fclose(fp);
	return ret;
}

var folder_create(var path)
{
	return mkfldr((char*)path);
}

var folder_delete(var path)
{
	return rmfldr((char*)path);
}

var buffer__append(var b, var data, var len) 
{
	var *buf = (var*)b;
	var end;
	if (buf[0] == 0) {
		buf[0] = (var)malloc(4096);
		buf[1] = 0;
		buf[2] = 4096;
	}
	end = buf[1] + len;
	if ((end+2) >= buf[2]) {
		buf[2] = end + 4096;
		buf[0] = (var)realloc((void*)buf[0], buf[2]);
	}
	memcpy((char*)(buf[0] + buf[1]), (char*)data, len);
	buf[1] = end;
	return 0;
}

var folder_list(var path)
{
	FOLDER *f;
	char *entry;
	var buf[3];
	buf[0] = 0;
	f = openfldr((char*)path);
	if (!f) {
		return 0;
	}
	entry = readfldr(f);
	if (entry) {
		buffer__append((var)buf, (var)entry, strlen(entry));
	}
	while (entry) {
		entry = readfldr(f);
		if (!entry) {
			break;
		}
		buffer__append((var)buf, (var)"\n", 1);
		buffer__append((var)buf, (var)entry, strlen(entry));
	}
	closefldr(f);
	if (buf[0]) {
		((char*)buf[0])[buf[1]] = '\0';
	} else {
		return (var)strdup("");
	}
	return buf[0];
}



var str_cmp(var a, var b)
{
	return strcmp((void*)a, (void*)b);
}

var str_dup(var a)
{
	return (var)strdup((void*)a);
}

var term_size(var a)
{
	var *size = (void*)a;
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	size[1] = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	size[2] = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
#else 
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	size[1] = w.ws_col;
	size[2] = w.ws_row;
#endif
	return 0;
}


#ifdef _WIN32
HANDLE hStdin;
HANDLE hStdout;
DWORD fdwSaveOldMode;
DWORD fdwSaveOldModeOut;

VOID ErrorExit(LPSTR lpszMessage);
VOID KeyEventProc(KEY_EVENT_RECORD, char *buf, DWORD max, DWORD *ret);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);

#ifdef _MSC_VER
void __builtin_trap()
{
    __debugbreak();
}
#endif



var term_init(var a)
{
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];
    int counter=0;
    var *term = (void*)a;

    // Get the standard input handle.

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdout == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");
    // Save the current input mode, to be restored on exit.

    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");
    if (! GetConsoleMode(hStdout, &fdwSaveOldModeOut) )
        ErrorExit("GetConsoleMode");
    // https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences
    //
    // Enable the window and mouse input events.

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS /*ENABLE_PROCESSED_INPUT*/ /*| ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_VIRTUAL_TERMINAL_INPUT*/;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");
        /*
    fdwMode = ENABLE_PROCESSED_OUTPUT  | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (! SetConsoleMode(hStdout, fdwMode) )
        ErrorExit("SetConsoleMode");
*/
    atexit((void(*)(void))(*term_deinit));

    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
            ErrorExit("SetConsoleCtrlHandler");
    }
     /* UTF-8 */
     SetConsoleOutputCP(65001); //chcp 65001
     SetConsoleCP(65001);

    term_size(a);
     return 0;
}

var term_wait(var term_, var timeout)
 {
    DWORD rd;
    DWORD cNumRead,i;
    INPUT_RECORD irInBuf[128];
    static char buffer[4096];
    var *term = (void*)term_;
    term[3] = 0;
    rd = 0;
    cNumRead = 0;
    while (timeout >= 0) {
        rd = 0;
        if (WaitForSingleObject(hStdin, 1)) {
            timeout--;
            continue;
        }
        if (! ReadConsoleInputA(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");
        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                    KeyEventProc(irInBuf[i].Event.KeyEvent, buffer, sizeof(buffer), &rd);
                    break;

                case MOUSE_EVENT: // mouse input
                    MouseEventProc(irInBuf[i].Event.MouseEvent);
                    break;

                case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing
                    ResizeEventProc( irInBuf[i].Event.WindowBufferSizeEvent );
                    break;

                case FOCUS_EVENT:  // disregard focus events

                case MENU_EVENT:   // disregard menu events
                    break;

                default:
                    ErrorExit("Unknown event type");
                    break;
            } 
        }
        if (rd > 0) {
            break;
        }
        timeout -= 1;
    }
    
    if (rd > 0) {
        buffer[rd] = 0;
        term[3] = 1; /* keyboard event type */
	    term[4] = rd; /* event data length */
        term[5] = (var)buffer; /* event data */
    }
    
    return 0;
}

VOID ErrorExit (LPSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

// https://learn.microsoft.com/fr-fr/windows/win32/inputdev/virtual-key-codes

VOID KeyEventProc(KEY_EVENT_RECORD ker, char *buf, DWORD max,  DWORD *ret)
{
    DWORD i;

    if (ker.dwControlKeyState & CAPSLOCK_ON )
    {

    }
    if (ker.dwControlKeyState & ENHANCED_KEY )
    {
        
    }
    if (ker.dwControlKeyState & LEFT_ALT_PRESSED )
    {
        
    }
    if (ker.dwControlKeyState & LEFT_CTRL_PRESSED )
    {
        if (ker.wVirtualKeyCode == 'C') {
            CtrlHandler(CTRL_C_EVENT);
        }   
    }
    if (ker.dwControlKeyState & NUMLOCK_ON )
    {
        
    }
    if (ker.dwControlKeyState & RIGHT_ALT_PRESSED )
    {
        
    }
    if (ker.dwControlKeyState & RIGHT_CTRL_PRESSED )
    {
        
    }
    if (ker.dwControlKeyState & SCROLLLOCK_ON )
    {
        
    }
    if (ker.dwControlKeyState & SHIFT_PRESSED )
    {
        
    }
    if(ker.bKeyDown) {
        for (i = 0; i < ker.wRepeatCount && (*ret) < (max-1); i++) {
            buf[*ret] = ker.uChar.AsciiChar;
            *ret = *ret + 1;
        }
        switch (ker.wVirtualKeyCode) {
        case VK_BACK:
        case VK_TAB:
        // TODO
            break;
        }
    }
    else  {
        //printf("key released\n");
    }
}

// https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str

VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
    printf("Mouse event: ");

    switch(mer.dwEventFlags)
    {
        case 0:

            if(mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
            {
                // x an y start from 0
                printf("left button press %d %d\n", mer.dwMousePosition.X, mer.dwMousePosition.Y);
            }
            else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                printf("right button press \n");
            }
            else if (mer.dwButtonState == 0) {
                    printf("button release\n");
            } else
            {
                printf("button press\n");
            }
            break;
        case DOUBLE_CLICK:
            printf("double click\n");
            break;
        case MOUSE_HWHEELED:
            printf("horizontal mouse wheel\n");
            break;
        case MOUSE_MOVED:
            printf("mouse moved %d %d\n", mer.dwMousePosition.X, mer.dwMousePosition.Y);
            break;
        case MOUSE_WHEELED:
            printf("vertical mouse wheel\n");
            break;
        default:
            printf("unknown\n");
            break;
    }
}

VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD wbsr)
{
    printf("Resize event\n");
    printf("Console screen buffer is %d columns by %d rows.\n", wbsr.dwSize.X, wbsr.dwSize.Y);
}


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
        printf("Ctrl-C event\n\n");
        Beep(750, 300);
        quit();
        return TRUE;

        // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
        Beep(600, 200);
        printf("Ctrl-Close event\n\n");
        return TRUE;

        // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
        Beep(900, 200);
        printf("Ctrl-Break event\n\n");
        return FALSE;

    case CTRL_LOGOFF_EVENT:
        Beep(1000, 200);
        printf("Ctrl-Logoff event\n\n");
        return FALSE;

    case CTRL_SHUTDOWN_EVENT:
        Beep(750, 500);
        printf("Ctrl-Shutdown event\n\n");
        return FALSE;

    default:
        return FALSE;
    }
}

var clipboard_set(var txt, var len) 
{ 
    wchar_t  *lptstrCopy; 
    static HGLOBAL hglbCopy = 0; 
    var unilen;
 
    if (!OpenClipboard(0)) 
        return -1; 
    unilen = MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        (void*)txt,
        len,
        (void*)0,
        0 
        );
    if (unilen < 1) { 
        CloseClipboard(); 
        return -2; 
    }  
    EmptyClipboard(); 
    if (hglbCopy) {
        GlobalFree(hglbCopy);
    }
    hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * 4); 
    if (hglbCopy == NULL) { 
        CloseClipboard(); 
        return -2; 
    } 
    lptstrCopy = GlobalLock(hglbCopy); 
    
    if (!MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS,
        (void*)txt,
        len,
        lptstrCopy,
        unilen
        )
        )
    {
        CloseClipboard(); 
        return -1; 
    }
    lptstrCopy[unilen] = 0;
    GlobalUnlock(hglbCopy); 
    SetClipboardData(CF_UNICODETEXT, hglbCopy);   
    CloseClipboard(); 
    return 0; 
}


var clipboard_get()
{
    static char *buffer = NULL;
    wchar_t *unistr;
    HANDLE clip;
    int len;

    
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }

    if (!OpenClipboard(0)) 
        return (var)""; 
        
    clip = GetClipboardData(CF_UNICODETEXT);
    if (!clip) {
         CloseClipboard();
         return (var)""; 
    }
    
    unistr = (wchar_t*)GlobalLock(clip);
    if (!unistr) {
         CloseClipboard();
         return (var)""; 
    }
    
    len = WideCharToMultiByte(CP_UTF8, 0, unistr, -1, NULL,  0, NULL, NULL);
    
    buffer = malloc(len + 2);
    
    if (buffer && (len > 0)) {
        len = WideCharToMultiByte(CP_UTF8, 0, unistr, -1, buffer,  len + 1, NULL, NULL);
        buffer[len] = 0;
    } else if (buffer) {
        buffer[0] = 0;
    }
    GlobalUnlock(clip);
    CloseClipboard();
    return (var)buffer;
}

#else /*_WIN32 */

struct termios orig_termios;
struct sigaction old_action;

void sigint_handler(int sig_no)
{
	term_deinit();
    	sigaction(SIGINT, &old_action, NULL);
    	kill(0, SIGINT);
}

var term_init(var a)
{
	var *term = (void*)a;
  	struct termios new_termios;
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	action.sa_handler = &sigint_handler;
    	sigaction(SIGINT, &action, &old_action);
	tcgetattr(0, &orig_termios);
    	memcpy(&new_termios, &orig_termios, sizeof(new_termios));
	atexit((void(*)())(*term_deinit));
	new_termios.c_lflag &= ~(ICANON|ECHO);
	new_termios.c_cc[VMIN] = 1;
    	tcsetattr(0, TCSANOW, &new_termios);
		
	term[0] = 0; /* private handler */
	term[1] = 0; /* width */
	term[2] = 0; /* height */
	term[3] = 0; /* event type */
	term[4] = 0; /* event data length */
       	term[5] = 0; /* event data */
	printf("\x1B[?1003h\x1B[?1015h\x1B[?1006h"); /* Mouse trap all, urxvt, SGR1006 */	
	fflush(stdout);
	term_size(a);
	return 0;
}

var terminal_input(var a)
{
	ssize_t l;
	var *term = (void*)a;
	static char buffer[4096];
	l = read(0, buffer, sizeof(buffer)-1);
	if (l < 1) {
		return -1;
	}
	buffer[l] = 0;
	term[3] = 1; /* keyboard event type */
	term[4] = l; /* event data length */
       	term[5] = (var)buffer; /* event data */
	return 0;
}

var term_wait(var a, var timeout)
{
	var *term = (void*)a;
	struct timeval tv = { 0L, 0L };
    	fd_set fds;
	term[3] = 0; /* event type */
    	FD_ZERO(&fds);
    	FD_SET(0, &fds);
    	if (select(1, &fds, NULL, NULL, &tv) != 0) {
		return terminal_input(a);
	}
	if (timeout == 0) {
		return -1;
	}
	tv.tv_usec = 1000;
	while (timeout > 0) {
    		if (select(1, &fds, NULL, NULL, &tv) != 0) {
			return terminal_input(a);
		}
		timeout--;
	}
	return terminal_input(a);

}

/* https://www.uninformativ.de/blog/postings/2017-04-02/0/POSTING-en.html
https://blog.desdelinux.net/en/send-data-to-kde-clipboard-from-terminal/
   Linux, what a mess...
 */
var clipboard_set(var txt, var len) 
{
	mkfldr("~/.local/share/os-3o3/");
	file_save((var)"~/.local/share/os-3o3/clipboard.txt", -1, len, txt);
	system("xclip -i ~/.local/share/os-3o3/clipboard.txt");
	return 0;
}

var clipboard_get()
{
	static var buf = 0;
	var f = (var)"~/.local/share/os-3o3/clipboard.txt";
	mkfldr("~/.local/share/os-3o3/");
	system("xclip -o ~/.local/share/os-3o3/clipboard.txt");
	if (buf) {
		free((void*)buf);
	}
	buf = file_load(f, 0, file_size(f));
	if (buf) {
		return buf;
	}
	return (var)"";
}

#endif /* _WIN32 */

var term_deinit()
{
	printf("\x1B[?1000l\x1B[?1003l\x1B[?1015l\x1B[?1006l"); 
	printf("\x1B[?1049l\x1B[r"); 
	fflush(stdout);
#ifdef _WIN32
    SetConsoleMode(hStdin, fdwSaveOldMode);
    SetConsoleMode(hStdout, fdwSaveOldModeOut);
#else
	tcsetattr(0, TCSANOW, &orig_termios);
	/*system("tset");*/
#endif
	printf("\n");
	return 0;
}

var run(var a)
{
	return system((char*)a);
}


var quit(void)
{
	term_deinit();
	exit(0);
	return -1;
}

#include "folder.c"
#include "socket.c"
