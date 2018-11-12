/* main code */

#include <Pilot.h>
// #include "callback.h"
// #include "dbprintf.h"
#include "res.h"

#define INTERVAL_TIMER		(30)
#define CREATER_ID			((DWord)'hexC')
#define CREATER_VER			0x0041

typedef struct {
	DWord		dwMem1;
	DWord		dwMem2;
	DWord		dwMem3;
	Word		wMath;
	short		nBits;
	short		nBase;
	Boolean		fEqual;
	Boolean		fSecond;
	Boolean		fSigned;
} t_Prefs;

/* Global variables. */
t_Prefs tPrefs;

/*
*/
static VoidPtr GetObjPtr( Int objectID )
{
	FormPtr frm;
	frm = FrmGetActiveForm();
	return( FrmGetObjectPtr( frm, FrmGetObjectIndex(frm, objectID)) );
}

/*
*/
static void DrawBitmap( short id, short x, short y )
{
	VoidHand h;
	VoidPtr p;

	h = DmGet1Resource( 'Tbmp', id );
	if( h != NULL ){
		p = MemHandleLock( h );
		WinDrawBitmap( (BitmapPtr)p, x, y );
		MemHandleUnlock( h );
		DmReleaseResource( h );
	}
}

/*
*/
static void DrawNumber( DWord dwNum, short x, short y )
{
	short i, j;
	short nMask;
	DWord dwBuf1;
	DWord dwBuf2;
	Boolean fDraw;
	Boolean fAbs;
	RectangleType rect;
	static DWord dwMaskTable[8] = {
		0x0000000F, 0x000000FF, 0x00000FFF, 0x0000FFFF,
		0x000FFFFF, 0x00FFFFFF, 0x0FFFFFFF, 0xFFFFFFFF
	};
	static DWord dwSignTable[8] = {
		0x00000008, 0x00000080, 0x00000800, 0x00008000,
		0x00080000, 0x00800000, 0x08000000, 0x80000000
	};

	nMask = tPrefs.nBits / 4;
	dwNum &= dwMaskTable[nMask-1];

	switch( tPrefs.nBase ){
	case 2:
		rect.topLeft.x = x;
		rect.topLeft.y = y;
		rect.extent.x  = 0;
		rect.extent.y  = 12;
		j = nMask * 2;
		rect.extent.x += ( 4 * (( 8 - nMask ) * 4 ));
		WinEraseRectangle( &rect, 0 );
		x += ( 8 * 15 );
		for( i = 0; i < j; i++ ){
			dwBuf1 = dwNum & 0x00000003;
			DrawBitmap( BITMAP_BIN00 + dwBuf1, x, y );
			dwNum >>= 2;
			x -= 8;
		}
		break;
	case 16:
		rect.topLeft.x = x;
		rect.topLeft.y = y;
		rect.extent.x  = 8 * 8;
		rect.extent.y  = 12;
		j = nMask;
		rect.extent.x += ( 8 * ( 8 - nMask ));
		WinEraseRectangle( &rect, 0 );
		x += ( 8 * 15 );
		fDraw = true;
		for( i = 0; i < j; i++ ){
			dwBuf1 = dwNum & 0x0F;
			DrawBitmap( BITMAP_NUM00 + dwBuf1, x, y );
			dwNum >>= 4;
			x -= 8;
		}
		break;
	case 10:
	default:
		fAbs = false;
		if( tPrefs.fSigned == true ){
			if( ( dwNum & dwSignTable[nMask-1] ) != 0 ){
				dwNum *= (DWord)-1;
				dwNum &= dwMaskTable[nMask-1];
				fAbs = true;
			}
		}
		rect.topLeft.x = x;
		rect.topLeft.y = y;
		rect.extent.x  = 8 * 6;
		rect.extent.y  = 12;
		WinEraseRectangle( &rect, 0 );
		x += ( 8 * 6 );
		fDraw = false;
		dwBuf1 = 1000000000;
		for( i = 0; i < 9; i++ ){
			if( dwNum < dwBuf1 && fDraw == false ){
				rect.topLeft.x = x;
				rect.topLeft.y = y;
				rect.extent.x  = 8;
				rect.extent.y  = 12;
				WinEraseRectangle( &rect, 0 );
			} else {
				dwBuf2 = dwNum / dwBuf1;
				DrawBitmap( BITMAP_NUM00 + dwBuf2, x, y );
				dwNum -= ( dwBuf2 * dwBuf1 );
				fDraw = true;
				if( fAbs == true ){
					DrawBitmap( BITMAP_MINUS, x - 8, y );
					fAbs = false;
				}
			}
			dwBuf1 /= 10;
			x += 8;
		}
		if( dwNum >= dwBuf1 ){
			dwBuf2 = dwNum / dwBuf1;
		} else {
			dwBuf2 = 0;
		}
		DrawBitmap( BITMAP_NUM00 + dwBuf2, x, y );
		if( fAbs == true ){
			DrawBitmap( BITMAP_MINUS, x - 8, y );
			fAbs = false;
		}
		break;
	}
}

/*
*/
static void DrawNumbers( void )
{
	short x, y;
	char szBuf[16];
	RectangleType rect;

	FntSetFont( stdFont /* boldFont */ );

	x = 6;
	y = 20;

	DrawNumber( tPrefs.dwMem1, x, y );
	DrawNumber( tPrefs.dwMem2, x, y + 12 );
	DrawNumber( tPrefs.dwMem3, x, y + 24 );

	rect.topLeft.x = 140;
	rect.topLeft.y = y;
	rect.extent.x  = 16;
	rect.extent.y  = 36;
	WinEraseRectangle( &rect, 0 );

	switch( tPrefs.wMath ){
	case ButtonAdd:
		StrCopy( szBuf, "+" );
		break;
	case ButtonSub:
		StrCopy( szBuf, "-" );
		break;
	case ButtonMul:
		StrCopy( szBuf, "*" );
		break;
	case ButtonDiv:
		StrCopy( szBuf, "/" );
		break;
	case ButtonAnd:
		StrCopy( szBuf, "and" );
		break;
	case ButtonOr:
		StrCopy( szBuf, "or" );
		break;
	case ButtonXor:
		StrCopy( szBuf, "xor" );
		break;
	default:
		StrCopy( szBuf, "" );
		break;
	}
	WinDrawChars( szBuf, StrLen(szBuf), rect.topLeft.x, y + 12 );

	StrCopy( szBuf, "=" );
	if( tPrefs.fEqual == true ){
		WinDrawChars( szBuf, StrLen(szBuf), rect.topLeft.x, y + 24 );
	}
}

/*
*/
static void DoButtonEqual( Boolean fDraw )
{
	if( tPrefs.wMath == 0 ){
		return;
	}

	switch( tPrefs.wMath ){
	case ButtonAdd:
		tPrefs.dwMem3 = tPrefs.dwMem1 + tPrefs.dwMem2;
	//	DebugPrintF( "%ld + %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonSub:
		tPrefs.dwMem3 = tPrefs.dwMem1 - tPrefs.dwMem2;
	//	DebugPrintF( "%ld - %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonMul:
		tPrefs.dwMem3 = tPrefs.dwMem1 * tPrefs.dwMem2;
	//	DebugPrintF( "%ld * %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonDiv:
		if( tPrefs.dwMem2 > 0 ){
			tPrefs.dwMem3 = tPrefs.dwMem1 / tPrefs.dwMem2;
		} else {
			tPrefs.dwMem3 = 0;
		}
	//	DebugPrintF( "%ld / %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonAnd:
		tPrefs.dwMem3 = tPrefs.dwMem1 & tPrefs.dwMem2;
	//	DebugPrintF( "%ld and %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonOr:
		tPrefs.dwMem3 = tPrefs.dwMem1 | tPrefs.dwMem2;
	//	DebugPrintF( "%ld or %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	case ButtonXor:
		tPrefs.dwMem3 = tPrefs.dwMem1 ^ tPrefs.dwMem2;
	//	DebugPrintF( "%ld xor %ld = %ld \n", tPrefs.dwMem1, tPrefs.dwMem2, tPrefs.dwMem3 );
		break;
	}

	tPrefs.fEqual	= true;
	tPrefs.fSecond	= false;

	if( fDraw == true ){
		DrawNumbers();
	}
}

/*
*/
static void DoButtonNumber( Word wID )
{
	DWord dwBuf;
	Boolean fAbs;

	switch( wID ){
	case ButtonZero:
	case ButtonOne:
		dwBuf = wID - ButtonZero;
		if( tPrefs.fEqual == true ){
			tPrefs.dwMem1	= 0;
			tPrefs.dwMem2	= 0;
			tPrefs.dwMem3	= 0;
			tPrefs.wMath	= 0;
			tPrefs.fEqual	= false;
			tPrefs.fSecond	= false;
		}
		if( tPrefs.wMath == 0 ){
			tPrefs.dwMem1 *= tPrefs.nBase;
			tPrefs.dwMem1 += dwBuf;
		} else {
			tPrefs.dwMem2 *= tPrefs.nBase;
			tPrefs.dwMem2 += dwBuf;
			tPrefs.fSecond = true;
		}
		DrawNumbers();
		break;
	case ButtonTwo:
	case ButtonThree:
	case ButtonFour:
	case ButtonFive:
	case ButtonSix:
	case ButtonSeven:
	case ButtonEight:
	case ButtonNine:
		if( tPrefs.nBase == 10 || tPrefs.nBase == 16 ){
			dwBuf = wID - ButtonZero;
			if( tPrefs.fEqual == true ){
				tPrefs.dwMem1	= 0;
				tPrefs.dwMem2	= 0;
				tPrefs.dwMem3	= 0;
				tPrefs.wMath	= 0;
				tPrefs.fEqual	= false;
				tPrefs.fSecond	= false;
			}
			if( tPrefs.wMath == 0 ){
				fAbs = false;
				if( tPrefs.nBase == 10 && tPrefs.fSigned == true ){
					if( (Long)tPrefs.dwMem1 < 0 ){
						tPrefs.dwMem1 *= (DWord)-1;
						fAbs = true;
					}
				}
				tPrefs.dwMem1 *= tPrefs.nBase;
				tPrefs.dwMem1 += dwBuf;
				if( fAbs == true ){
					tPrefs.dwMem1 *= (DWord)-1;
				}
			} else {
				fAbs = false;
				if( tPrefs.nBase == 10 && tPrefs.fSigned == true ){
					if( (Long)tPrefs.dwMem2 < 0 ){
						tPrefs.dwMem2 *= (DWord)-1;
						fAbs = true;
					}
				}
				tPrefs.dwMem2 *= tPrefs.nBase;
				tPrefs.dwMem2 += dwBuf;
				tPrefs.fSecond = true;
				if( fAbs == true ){
					tPrefs.dwMem2 *= (DWord)-1;
				}
			}
			DrawNumbers();
		}
		break;
	case ButtonTen:
	case ButtonEleven:
	case ButtonTwelve:
	case ButtonThirteen:
	case ButtonFourteen:
	case ButtonFifteen:
		if( tPrefs.nBase == 16 ){
			dwBuf = wID - ButtonZero;
			if( tPrefs.fEqual == true ){
				tPrefs.dwMem1	= 0;
				tPrefs.dwMem2	= 0;
				tPrefs.dwMem3	= 0;
				tPrefs.wMath	= 0;
				tPrefs.fEqual	= false;
				tPrefs.fSecond	= false;
			}
			if( tPrefs.wMath == 0 ){
				tPrefs.dwMem1 *= tPrefs.nBase;
				tPrefs.dwMem1 += dwBuf;
			} else {
				tPrefs.dwMem2 *= tPrefs.nBase;
				tPrefs.dwMem2 += dwBuf;
				tPrefs.fSecond = true;
			}
			DrawNumbers();
		}
		break;
	}
}

/*
*/
static void DoButtonMath( Word wID )
{
	if( tPrefs.fSecond == true && tPrefs.wMath != 0 ){
		DoButtonEqual( false );
	}

	if( tPrefs.fEqual == true ){
		tPrefs.dwMem1	= tPrefs.dwMem3;
		tPrefs.dwMem2	= 0;
		tPrefs.dwMem3	= 0;
		tPrefs.fEqual	= false;
		tPrefs.fSecond	= false;
	}

	tPrefs.wMath = wID;
	DrawNumbers();
}

/*
*/
static void DoButtonClear( void )
{
	tPrefs.dwMem1	= 0;
	tPrefs.dwMem2	= 0;
	tPrefs.dwMem3	= 0;
	tPrefs.wMath	= 0;
	tPrefs.fEqual	= false;
	tPrefs.fSecond	= false;
	DrawNumbers();
}

/*
*/
static void DoButtonBack( void )
{
	if( tPrefs.fEqual == false ){
		if( tPrefs.wMath == 0 ){
			tPrefs.dwMem1 /= tPrefs.nBase;
		} else {
			tPrefs.dwMem2 /= tPrefs.nBase;
		}
		DrawNumbers();
	}
}

/*
*/
static void DoButtonFlag( void )
{
	if( tPrefs.fEqual == false ){
		if( tPrefs.wMath == 0 ){
			tPrefs.dwMem1 *= -1;
		} else {
			tPrefs.dwMem2 *= -1;
		}
		DrawNumbers();
	}
}

/*
*/
static void DoInitApp( FormPtr frm )
{
	Word wBuf;
	CharPtr pszBuf;

	// list bits table
	wBuf = ( tPrefs.nBits / 4 ) - 1;
	LstSetSelection( GetObjPtr( ListBitsTbl ), wBuf );
	pszBuf = LstGetSelectionText( GetObjPtr( ListBitsTbl ), wBuf );
	CtlSetLabel( GetObjPtr( ListBitsTrg ), pszBuf );

	// list singned/unsigned
	//	wBuf = ( tPrefs.nBits / 4 ) - 1;
	//	FrmSetControlGroupSelection( frm, GroupBits, ButtonBit04 + wBuf );

	switch( tPrefs.nBase ){
	case 2:
		wBuf = ButtonBin;
		break;
	case 16:
		wBuf = ButtonHex;
		break;
	case 10:
	default:
		wBuf = ButtonDec;
		break;
	}
	FrmSetControlGroupSelection( frm, GroupBase, wBuf );

	// button signed/unsigned
	if( tPrefs.fSigned == true ){
		FrmSetControlGroupSelection( frm, GroupSign, ButtonSigned );
	} else {
		FrmSetControlGroupSelection( frm, GroupSign, ButtonUnsigned );
	}

	// list signed/unsigned
	//	wBuf = ( tPrefs.fSigned == true ? 0 : 1 );
	//	LstSetSelection( GetObjPtr( ListSignedTbl ), wBuf );
	//	pszBuf = LstGetSelectionText( GetObjPtr( ListSignedTbl ), wBuf );
	//	CtlSetLabel( GetObjPtr( ListSignedTrg ), pszBuf );

	DrawNumbers();
}

/*
	Main Form Handle Event
*/
static Boolean MainFormHandleEvent( EventPtr e )
{
	Boolean handled = false;
	FormPtr frm;
	CharPtr pszBuf;
	Word wBuf;

	switch( e->eType ){
	case frmOpenEvent:
		frm = FrmGetActiveForm();
		FrmDrawForm( frm );
		DoInitApp( frm );
		handled = true;
		break;

	case menuEvent:
		MenuEraseStatus( NULL );
		switch( e->data.menu.itemID ){
		case MENU_OPT_ABOUT:
			FrmHelp( ABOUT_HELP_ID );
			// FrmAlert( ABOUT_ALERT_ID );
			break;
		case MENU_EDIT_CLEAR:
			DoButtonClear();
			break;
		}
		handled = true;
		break;

	case popSelectEvent:
		switch( e->data.popSelect.listID ){

		// list bits table
		case ListBitsTbl:
			tPrefs.nBits = ( e->data.popSelect.selection + 1 ) * 4;
			wBuf = LstGetSelection( GetObjPtr( ListBitsTbl ));
			pszBuf = LstGetSelectionText( GetObjPtr( ListBitsTbl ), wBuf );
			CtlSetLabel( GetObjPtr( ListBitsTrg ), pszBuf );
			DrawNumbers();
			break;

		// list signed/unsigned
		/*
		case ListSignedTbl:
			if( e->data.popSelect.selection == 0 ){
				tPrefs.fSigned = true;
			} else {
				tPrefs.fSigned = false;
			}
			wBuf = LstGetSelection( GetObjPtr( ListSignedTbl ));
			pszBuf = LstGetSelectionText( GetObjPtr( ListSignedTbl ), wBuf );
			CtlSetLabel( GetObjPtr( ListSignedTrg ), pszBuf );
			DrawNumbers();
			break;
		*/

		}
		handled = true;
		break;

	case ctlSelectEvent:
		switch( e->data.ctlSelect.controlID ){
		case ButtonClear:
			DoButtonClear();
			/*
			tPrefs.dwMem1	= 0;
			tPrefs.dwMem2	= 0;
			tPrefs.dwMem3	= 0;
			tPrefs.wMath	= 0;
			tPrefs.fEqual	= false;
			tPrefs.fSecond	= false;
			DrawNumbers();
			*/
			handled = true;
			break;
		case ButtonBack:
			DoButtonBack();
			/*
			if( tPrefs.fEqual == false ){
				if( tPrefs.wMath == 0 ){
					tPrefs.dwMem1 /= tPrefs.nBase;
				} else {
					tPrefs.dwMem2 /= tPrefs.nBase;
				}
				DrawNumbers();
			}
			*/
			handled = true;
			break;

		case ButtonBin:
	/*	case ButtonOct:		*/
		case ButtonDec:
		case ButtonHex:
			switch( e->data.ctlSelect.controlID ){
			case ButtonBin:
				tPrefs.nBase = 2;
				break;
		/*
			case ButtonOct:
				tPrefs.nBase = 8;
				break;
		*/
			case ButtonHex:
				tPrefs.nBase = 16;
				break;
			case ButtonDec:
			default:
				tPrefs.nBase = 10;
				break;
			}
			DrawNumbers();
			handled = true;
			break;

		// button bits table
		/*
		case ButtonBit04:
		case ButtonBit08:
		case ButtonBit12:
		case ButtonBit16:
		case ButtonBit20:
		case ButtonBit24:
		case ButtonBit28:
		case ButtonBit32:
			tPrefs.nBits = ( e->data.ctlSelect.controlID - ButtonBit04 + 1 ) * 4;
			DrawNumbers();
			handled = true;
			break;
		*/

		// button signed/unsigned
		case ButtonSigned:
			tPrefs.fSigned = true;
			DrawNumbers();
			handled = true;
			break;
		case ButtonUnsigned:
			tPrefs.fSigned = false;
			DrawNumbers();
			handled = true;
			break;

		case ButtonFlag:
			DoButtonFlag();
			/*
			if( tPrefs.fEqual == false ){
				if( tPrefs.wMath == 0 ){
					tPrefs.dwMem1 *= -1;
				} else {
					tPrefs.dwMem2 *= -1;
				}
				DrawNumbers();
			}
			*/
			handled = true;
			break;
		case ButtonZero:
		case ButtonOne:
		case ButtonTwo:
		case ButtonThree:
		case ButtonFour:
		case ButtonFive:
		case ButtonSix:
		case ButtonSeven:
		case ButtonEight:
		case ButtonNine:
		case ButtonTen:
		case ButtonEleven:
		case ButtonTwelve:
		case ButtonThirteen:
		case ButtonFourteen:
		case ButtonFifteen:
			DoButtonNumber( e->data.ctlSelect.controlID );
			handled = true;
			break;
		case ButtonAdd:
		case ButtonSub:
		case ButtonMul:
		case ButtonDiv:
		case ButtonAnd:
		case ButtonOr:
		case ButtonXor:
			DoButtonMath( e->data.ctlSelect.controlID );
			/*
			if( tPrefs.fSecond == true && tPrefs.wMath != 0 ){
				DoButtonEqual( false );
			}
			if( tPrefs.fEqual == true ){
				tPrefs.dwMem1	= tPrefs.dwMem3;
				tPrefs.dwMem2	= 0;
				tPrefs.dwMem3	= 0;
				tPrefs.fEqual	= false;
				tPrefs.fSecond	= false;
			}
			tPrefs.wMath = e->data.ctlSelect.controlID;
			DrawNumbers();
			*/
			handled = true;
			break;
		case ButtonEqual:
			DoButtonEqual( true );
			handled = true;
			break;
		}
		break;

	case keyDownEvent:
		switch( e->data.keyDown.chr ){
			case '0':	DoButtonNumber( ButtonZero );		break;
			case '1':	DoButtonNumber( ButtonOne );		break;
			case '2':	DoButtonNumber( ButtonTwo );		break;
			case '3':	DoButtonNumber( ButtonThree );		break;
			case '4':	DoButtonNumber( ButtonFour );		break;
			case '5':	DoButtonNumber( ButtonFive );		break;
			case '6':	DoButtonNumber( ButtonSix );		break;
			case '7':	DoButtonNumber( ButtonSeven );		break;
			case '8':	DoButtonNumber( ButtonEight );		break;
			case '9':	DoButtonNumber( ButtonNine );		break;
			case 'a':	DoButtonNumber( ButtonTen );		break;
			case 'b':	DoButtonNumber( ButtonEleven );		break;
			case 'c':	DoButtonNumber( ButtonTwelve );		break;
			case 'd':	DoButtonNumber( ButtonThirteen );	break;
			case 'e':	DoButtonNumber( ButtonFourteen );	break;
			case 'f':	DoButtonNumber( ButtonFifteen );	break;
			case '=':	DoButtonEqual( true );	break;
			case '?':	DoButtonEqual( true );	break;
			case 0x0A:	DoButtonEqual( true );	break;
			case '+':	DoButtonMath( ButtonAdd );	break;
			case '-':	DoButtonMath( ButtonSub );	break;
			case '*':	DoButtonMath( ButtonMul );	break;
			case '/':	DoButtonMath( ButtonDiv );	break;
			case '&':	DoButtonMath( ButtonAnd );	break;
			case '|':	DoButtonMath( ButtonOr );	break;
			case '^':	DoButtonMath( ButtonXor );	break;
			case 0x08:	DoButtonBack();		break;
			case '~':	DoButtonFlag();		break;
		}
		handled = true;
		break;

    case nilEvent:
		break;

	default:
		break;
	}

	return handled;
}

/*
	Application Handle Event
*/
static Boolean ApplicationHandleEvent( EventPtr e )
{
	FormPtr frm;
	Word    formId;
	Boolean handled = false;

	if( e->eType == frmLoadEvent ){
		formId = e->data.frmLoad.formID;
		frm = FrmInitForm( formId );
		FrmSetActiveForm( frm );
		switch( formId ){
		case ID_MAIN_FORM:
			FrmSetEventHandler( frm, MainFormHandleEvent );
			break;
		}
		handled = true;
	}

	return handled;
}

/*
	Get preferences, open (or create) app database
*/
static Word StartApplication( void )
{
	if( PrefGetAppPreferencesV10( CREATER_ID, CREATER_VER, (VoidPtr)&tPrefs, sizeof(t_Prefs) ) == false ){
		tPrefs.dwMem1	= 1;
		tPrefs.dwMem2	= 2;
		tPrefs.dwMem3	= 3;
		tPrefs.wMath	= ButtonAdd;
		tPrefs.nBits	= 32;
		tPrefs.nBase	= 10;
		tPrefs.fEqual	= true;
		tPrefs.fSecond	= false;
		tPrefs.fSigned	= false;
	}

	FrmGotoForm( ID_MAIN_FORM );

	return 0;
}

/*
	Save preferences, close forms, close app database
*/
static void StopApplication( void )
{
	PrefSetAppPreferencesV10( CREATER_ID, CREATER_VER, (VoidPtr)&tPrefs, sizeof(t_Prefs) );

	FrmSaveAllForms();
	FrmCloseAllForms();
}

/*
	The main event loop
*/
static void EventLoop( void )
{
	Word err;
	EventType e;

	do {
		EvtGetEvent( &e, /* evtWaitForever */ INTERVAL_TIMER );
		if( ! SysHandleEvent( &e ))
			if ( ! MenuHandleEvent( NULL, &e, &err ))
				if( ! ApplicationHandleEvent( &e ))
					FrmDispatchEvent( &e );
	} while( e.eType != appStopEvent );
}

/*
	Main entry point; it is unlikely you will need to change
	this except to handle other launch command codes
*/
DWord PilotMain( Word cmd, Ptr cmdPBP, Word launchFlags )
{
	Word err;

	if( cmd == sysAppLaunchCmdNormalLaunch ){
		err = StartApplication();
		if( err )
			return err;
		EventLoop();
		StopApplication();
	} else {
		return sysErrParamErr;
	}

	return 0;
}

/* end-of-file */
