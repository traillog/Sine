//
//  Sine.c
//  Plot the sine function
//

#include <windows.h>
#include <math.h>
#include <stdio.h>

#define     TWOPI           6.283185
#define     DBG_BUF         128
#define     TXT_BUF         128

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void createPtsAr( PPOINT *dirPtsAr, int ptsLength );
void deletePtsAr( PPOINT *dirPtsAr );
void setupPtsAr( PPOINT *dirPtsAr, int ptsLength, int caX, int caY );
double roundDbl( double n, int ini, int end );

int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PWSTR szCmdLine, int nCmdShow )
{
    PTCHAR szAppName = TEXT( "Sine Demo" );

    HWND hwnd;
    MSG msg;
    BOOL bRet;
    WNDCLASS wc;

    // ------------------------------------------------------------------------
    //  Step 1: Registering the window class
    // ------------------------------------------------------------------------
    memset( &wc, 0, sizeof( WNDCLASS ) );
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor       = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = ( HBRUSH )GetStockObject( WHITE_BRUSH );
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = szAppName;

    if ( !RegisterClass( &wc ) )
    {
        MessageBox( NULL,
            TEXT( "Window Registration Failed!" ),
            szAppName,
            MB_ICONERROR | MB_OK );

        return 0;
    }

    // ------------------------------------------------------------------------
    //  Step 2: Creating the window
    // ------------------------------------------------------------------------
    hwnd = CreateWindow(
        szAppName,                          // Window class name
        TEXT( "Sine Demo" ),                // Window title
        WS_OVERLAPPEDWINDOW,                // Window style
        CW_USEDEFAULT,                      // Ini H pos
        CW_USEDEFAULT,                      // Ini V pos
        616,                                // Window width
        438,                                // Window height
        NULL,                               // Parent Window
        NULL,                               // Menu
        hInstance,                          // Application instance handle
        NULL );                             // Additional creation data.

    if ( hwnd == NULL )
    {
        MessageBox( NULL,
            TEXT( "Window Creation Failed!" ),
            szAppName,
            MB_ICONERROR | MB_OK );

        return 0;
    }

    ShowWindow( hwnd, nCmdShow );       // Display the main window
    UpdateWindow( hwnd );               // Send first WM_PAINT message

    // ------------------------------------------------------------------------
    //  Step 3: The Message Loop
    // ------------------------------------------------------------------------

    while( ( bRet = GetMessage( &msg, NULL, 0, 0 ) ) != 0 )
    { 
        if ( bRet == -1 )
        {
            // GetMessage error
            MessageBox( NULL,
                TEXT( "GetMessage error!" ),
                szAppName,
                MB_ICONERROR | MB_OK );

            return 0;
        }
        else
        {
            TranslateMessage( &msg ); 
            DispatchMessage( &msg ); 
        }
    }

    return msg.wParam;
}


// ------------------------------------------------------------------------
//  Step 4: the Window Procedure
// ------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL axesON = TRUE;
    static BOOL marksON = TRUE;
    static int cxClient, cyClient;
    static int i;
    HDC hdc;
    PAINTSTRUCT ps;
    static PPOINT ptsAr = NULL;
    static int ptsSize = 21;            // One cycle points + 1
    static BOOL changePtsSize = FALSE;
    static TCHAR txtMsg[ TXT_BUF ] = { 0 };
    static int orgTxtX, hightTxtX;

    switch( message )
    {
    case WM_CREATE :
        // Create points array
        createPtsAr( &ptsAr, ptsSize );

        // Fall through

    case WM_SIZE :
        // Get client area size
        cxClient = LOWORD( lParam );
        cyClient = HIWORD( lParam );

        // Set up points value
        setupPtsAr( &ptsAr, ptsSize, cxClient, cyClient );
        return 0;

    case WM_KEYDOWN :
        switch ( wParam )
        {
        case 0x41 :           // Virtual key code for 'A'
            // Toggle axes
            if ( axesON == TRUE )
                axesON = FALSE;
            else
                axesON = TRUE;
            break;

        case 0x4D :           // Virtual key code for 'D'
            // Toggle point marks
            if ( marksON == TRUE )
                marksON = FALSE;
            else
                marksON = TRUE;
            break;

        case VK_UP :        // Virtual key code for 'Arrow Up'
            // Increment points amount
            ptsSize++;
            changePtsSize = TRUE;
            break;

        case VK_DOWN :      // Virtual key code for 'Arrow Down'
            // Decrement points amount
            ptsSize--;
            changePtsSize = TRUE;
            break;

        case VK_ESCAPE :    // Virtual key code for 'Esc'
            // Close the application
            DestroyWindow( hwnd );
            return 0;

        default :
            return 0;
        }

        if ( changePtsSize == TRUE )
        {
            // Destroy points array
            deletePtsAr( &ptsAr );

            // Create points array
            ptsSize = max( 2, ptsSize );
            createPtsAr( &ptsAr, ptsSize );

            // Set up points value
            setupPtsAr( &ptsAr, ptsSize, cxClient, cyClient );

            // Reset change pts array size flag
            changePtsSize = FALSE;
        }
        
        // Force a repaint
        InvalidateRect( hwnd, NULL, TRUE );
        return 0;

    case WM_PAINT :
        hdc = BeginPaint( hwnd, &ps );

        //============================================
        // Draw axes;
        //============================================
        if ( axesON == TRUE )
        {
            MoveToEx( hdc,
                0, (int)roundDbl( ( double )( cyClient ) / 2, 2, 0 ),
                NULL );
            LineTo( hdc,
                cxClient, (int)roundDbl( ( double )( cyClient ) / 2, 2, 0 ) );
        }

        //============================================
        // Draw sine wave
        //============================================
        Polyline( hdc, ptsAr, ptsSize );

        //============================================
        // Mark each point
        //============================================
        if ( marksON == TRUE )
        {
            SelectObject( hdc, GetStockObject( NULL_BRUSH ) );
            SelectObject( hdc, CreatePen( PS_SOLID, 1, RGB( 255, 0, 0 ) ) );

            for ( i = 0; i < ptsSize; i++ )
            {
                Ellipse( hdc,
                    ptsAr[ i ].x - 10, ptsAr[ i ].y - 10,
                    ptsAr[ i ].x + 10, ptsAr[ i ].y + 10 );
            }

            DeleteObject( SelectObject( hdc, GetStockObject( BLACK_PEN ) ) );
        }

        //============================================
        // Draw text
        //============================================
        SelectObject( hdc, GetStockObject( SYSTEM_FIXED_FONT ) );
        SetBkMode( hdc, TRANSPARENT );
        
        hightTxtX = 15;
        orgTxtX   = cxClient - 190;

        TextOut( hdc,
            orgTxtX, 1 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "caX            : %4d" ),
                cxClient ) );

        TextOut( hdc,
            orgTxtX, 2 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "caY            : %4d" ),
                cyClient ) );

        TextOut( hdc,
            orgTxtX, 3 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "Axes       (A) : %4s" ),
                axesON == TRUE ? TEXT( "ON" ) : TEXT( "OFF" ) ) );

        TextOut( hdc,
            orgTxtX, 4 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "Marks      (M) : %4s" ),
                marksON == TRUE ? TEXT( "ON" ) : TEXT( "OFF" ) ) );

        TextOut( hdc,
            orgTxtX, 5 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "# Pts (UP/DWN) : %4d" ),
                ptsSize ) );

        TextOut( hdc,
            orgTxtX, 6 * hightTxtX, txtMsg,
            swprintf( txtMsg, TXT_BUF,
                TEXT( "Exit     (ESC)" ) ) );

        EndPaint( hwnd, &ps );
        return 0 ;

    case WM_DESTROY :
        deletePtsAr( &ptsAr );
        PostQuitMessage( 0 );
        return 0;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}

void createPtsAr( PPOINT *dirPtsAr, int ptsLength )
{
    if ( *dirPtsAr )
        deletePtsAr( &*dirPtsAr );

    *dirPtsAr = ( PPOINT )calloc( ptsLength, sizeof( POINT ) );
}

void deletePtsAr( PPOINT *dirPtsAr )
{
    if ( *dirPtsAr )
    {
        free( *dirPtsAr );
        *dirPtsAr = NULL;
    }
}

void setupPtsAr( PPOINT *dirPtsAr, int ptsLength, int caX, int caY )
{
    int i;
    double xIncPx;
    double xIncRad;
    double amp;
    double ptsOneCycle;

#ifdef _DEBUG
    static TCHAR dbgMsg[ DBG_BUF ] = { 0 };
#endif

    // Set up amount pts one cycle
    ptsOneCycle = ptsLength - 1;

    // Set up horizontal increments
    xIncPx = ( double )( caX ) / ( ptsOneCycle );
    xIncRad = TWOPI / ( ptsOneCycle );

    // Set up amplitude
    amp = ( double )( caY - 1 ) / 2;

    // Set up ( x, y ) points
    for ( i = 0; i < ptsLength; i++ )
    {
        ( *dirPtsAr )[ i ].x =
            ( int )roundDbl( xIncPx * i, 8, 0 );

        ( *dirPtsAr )[ i ].y =
            ( int )roundDbl( amp * ( 1 - sin( xIncRad * i ) ), 8, 0 );
    }

//=================================================
//  _DEBUG is defined per Default by Visual Studio
//  as a preprocessor project property
//  for the 'Debug' Configuration.
//  It is undefined for the 'Release' Configuration
//=================================================
#ifdef _DEBUG
        OutputDebugString( TEXT( "dbg ------------------" ) );
        swprintf( dbgMsg, DBG_BUF,
            TEXT( "dbg - cxClient    : %6.2f, cyClient    : %6.2f" ),
            (double)caX, (double)caY );
        OutputDebugString( dbgMsg );

        swprintf( dbgMsg, DBG_BUF,
            TEXT( "dbg - cxClient / 2: %6.2f, cyClient / 2: %6.2f" ),
            (double)caX / 2, (double)caX / 2 );
        OutputDebugString( dbgMsg );

        swprintf( dbgMsg, DBG_BUF,
            TEXT( "dbg - pts[  0 ].x : %6.2f, pts[  0 ].y : %6.2f" ),
            (double)( *dirPtsAr )[ 0 ].x, (double)( *dirPtsAr )[ 0 ].y );
        OutputDebugString( dbgMsg );

        swprintf( dbgMsg, DBG_BUF,
            TEXT( "dbg - pts[ 99 ].x : %6.2f, pts[ 99 ].y : %6.2f" ),
            (double)( *dirPtsAr )[ 99 ].x, (double)( *dirPtsAr )[ 99 ].y );
        OutputDebugString( dbgMsg );

        swprintf( dbgMsg, DBG_BUF,
            TEXT( "dbg - cpts[ 99 ].x: %.5f, cpts[ 99 ].y: %.5f" ),
            xIncPx * 99, amp * ( 1 - sin( xIncRad * 99 ) ) );
        OutputDebugString( dbgMsg );
#endif

}

//======================================================
// roundDbl( n, ini, end )
// 
// round n starting at the 'ini' decimal place
//     and ending   at the 'end' decimal place
//
// Examples:
//
// Round n considering only 2 decimal places
// down to 1 decimal place:
// roundDbl( 3.1234567, 2, 1 ) --> 3.1230000 to 3.1
//
// Round n to the 2nd decimal place:
// roundDbl( 3.1234567, 2, 2 ) --> 3.1230000 to 3.12
//
// The bigger the 'ini' position,
// the better the aproximation.
//======================================================
double roundDbl( double n, int ini, int end )
{
    double res = n;
    int iter = 0;

    if ( ( ini < 0 ) || ( end < 0 ) )
        return res;

    for ( iter = ini; iter >= end; iter-- )
        if ( n >= 0 )
            // Round down
            res = floor( res * pow( 10.0, iter ) + 0.5 ) / pow( 10.0, iter );
        else
            // Round up
            res = ceil( res * pow( 10.0, iter ) - 0.5 ) / pow( 10.0, iter );

    return res;
}