
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

var print10(var n)
{
	printf("%ld", n);
	return 0;

}
var print(var txt)
{
	printf("%s", (char*)txt);
	return 0;
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

var file_save(var path, var offset, var size, var buf)
{
	FILE *fp;
	var ret;
	fp = fopen((char*)path, "rb+");
	if (!fp) {
		fp = fopen((char*)path, "wb+");
		if (!fp) {
			return -1;
		}
	}
	fseek(fp, offset, SEEK_SET);
	ret = fwrite((void*)buf, 1, size, fp);
	fclose(fp);
	return ret;
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
DWORD fdwSaveOldMode;

VOID ErrorExit(LPCSTR);
VOID KeyEventProc(KEY_EVENT_RECORD);
VOID MouseEventProc(MOUSE_EVENT_RECORD);
VOID ResizeEventProc(WINDOW_BUFFER_SIZE_RECORD);

int main(VOID)
{
    DWORD cNumRead, fdwMode, i;
    INPUT_RECORD irInBuf[128];
    int counter=0;

    // Get the standard input handle.

    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdin == INVALID_HANDLE_VALUE)
        ErrorExit("GetStdHandle");

    // Save the current input mode, to be restored on exit.

    if (! GetConsoleMode(hStdin, &fdwSaveOldMode) )
        ErrorExit("GetConsoleMode");

    // Enable the window and mouse input events.

    fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT;
    if (! SetConsoleMode(hStdin, fdwMode) )
        ErrorExit("SetConsoleMode");

    // Loop to read and handle the next 100 input events.

    while (counter++ <= 100)
    {
        // Wait for the events.

        if (! ReadConsoleInput(
                hStdin,      // input buffer handle
                irInBuf,     // buffer to read into
                128,         // size of read buffer
                &cNumRead) ) // number of records read
            ErrorExit("ReadConsoleInput");

        // Dispatch the events to the appropriate handler.

        for (i = 0; i < cNumRead; i++)
        {
            switch(irInBuf[i].EventType)
            {
                case KEY_EVENT: // keyboard input
                    KeyEventProc(irInBuf[i].Event.KeyEvent);
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
    }

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    return 0;
}

VOID ErrorExit (LPSTR lpszMessage)
{
    fprintf(stderr, "%s\n", lpszMessage);

    // Restore input mode on exit.

    SetConsoleMode(hStdin, fdwSaveOldMode);

    ExitProcess(0);
}

VOID KeyEventProc(KEY_EVENT_RECORD ker)
{
    printf("Key event: ");

    if(ker.bKeyDown)
        printf("key pressed\n");
    else printf("key released\n");
}

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
                printf("left button press \n");
            }
            else if(mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
            {
                printf("right button press \n");
            }
            else
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
            printf("mouse moved\n");
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

#else /*_WIN32 */
struct termios orig_termios;
struct sigaction old_action;

var term_deinit()
{
	printf("\x1B[?1000l\x1B[?1003l\x1B[?1015l\x1B[?1006l"); 
	fflush(stdout);
	tcsetattr(0, TCSANOW, &orig_termios);
	/*system("tset");*/
	printf("\n");
	return 0;
}

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

#endif /* _WIN32 */

var run(var a)
{
	return system((char*)a);
}


var quit()
{
	term_deinit();
	exit(0);
	return -1;
}


