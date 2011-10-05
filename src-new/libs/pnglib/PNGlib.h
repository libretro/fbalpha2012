/******************************************************************************
	PNGlib version 1.0
	C/C++ header for Microsoft Visual C++
	(C) 2002 by Thomas Bleeker
	http://www.MadWizard.org
 ******************************************************************************/
#pragma pack(1)

typedef struct
{
	DWORD	iWidth;				// Width of image
	DWORD	iHeight;			// Height of image
	DWORD	dwLastError;		// Last error code (PNGLIB_ERR_*)
	DWORD	nColors;			// Number of palette entries, if present
	BYTE	PNGColorType;		// Original PNG color type (PNG_CT_*)
	BYTE	PNGBitDepth;		// Original PNG bit depth (bits/sample)
	BYTE	PNGInterlaced;		// Interlace method (PNG_IM_*)
	// The following structure members are used internally
	// by PNGlib. You should not rely in any way on these values,
	// nor assume they will be present in later versions.
	// They are included *only* to reserve space for them.
	LPBYTE	lpPNGData;
	DWORD	lnPNGData;
	LPBYTE	lpCurrent;
	LPBYTE	lpOutput;
	DWORD	lnOutput;
	LPBYTE  pPNGPalette;
	DWORD	dwLoadType;
	LPBYTE	pHeader;
	LPBYTE  pIDAT;
	DWORD	curState;
} PNGINFO;

typedef PNGINFO *LPPNGINFO;

/******************************************************************************
	Function declararations
 ******************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
	bool __stdcall PNG_Init(LPPNGINFO pPNGInfo);
	bool __stdcall PNG_Cleanup(LPPNGINFO pPNGInfo);
	bool __stdcall PNG_LoadFile(LPPNGINFO pPNGInfo, LPSTR lpFileName);
	bool __stdcall PNG_LoadResource(LPPNGINFO pPNGInfo, HMODULE hInstance, LPSTR lpResName);
	bool __stdcall PNG_LoadCustom(LPPNGINFO pPNGInfo, LPBYTE lpData, DWORD lnData);
	bool __stdcall PNG_Decode(LPPNGINFO pPNGInfo);
	bool __stdcall PNG_GetPalette(LPPNGINFO pPNGInfo, LPVOID lpOutput, DWORD dwFormat);
	bool __stdcall PNG_OutputRaw(LPPNGINFO pPNGInfo, LPBYTE lpDest, DWORD dwFormat);
	HBITMAP __stdcall PNG_CreateBitmap(LPPNGINFO pPNGInfo, HWND hWndCompatible, DWORD dwFormat, BOOL bDDB);
#ifdef __cplusplus
}
#endif

/******************************************************************************
	Copyproc types, used to build the PNG_OUTF_* values
 ******************************************************************************/

#define	PNGCP_X_X					0
#define	PNGCP_16_16					1
#define	PNGCP_16_16_BGR				2
#define	PNGCP_16_16_BGR_SA			3
#define	PNGCP_16_16_BGRA			4
#define	PNGCP_16_16_SA				5
#define	PNGCP_16_8					6
#define	PNGCP_16_8_BGR				7
#define	PNGCP_16_8_BGR_SA			8
#define	PNGCP_16_8_BGRA				9
#define	PNGCP_16_8_SA				10
#define	PNGCP_2_4					11
#define	PNGCP_8_8_BGR				12
#define	PNGCP_8_8_BGR_SA			13
#define	PNGCP_8_8_BGRA				14
#define PNGCP_8_8_SA 				PNGCP_16_8

/******************************************************************************
	Output formats
 ******************************************************************************/

#define PNG_OUTF_G1_G1				   0x00010100+PNGCP_X_X			//DIB default
#define PNG_OUTF_G2_G2                 0x00020000+PNGCP_X_X
#define PNG_OUTF_G2_G4                 0x00020100+PNGCP_2_4			//DIB default
#define PNG_OUTF_G4_G4                 0x00040100+PNGCP_X_X			//DIB default
#define PNG_OUTF_G8_G8                 0x00080100+PNGCP_X_X			//DIB default
#define PNG_OUTF_G16_G16               0x00100000+PNGCP_16_16
#define PNG_OUTF_G16_G8                0x00100100+PNGCP_16_8		//DIB default

#define PNG_OUTF_T8_T8_BGR             0x02080100+PNGCP_8_8_BGR		//DIB default
#define PNG_OUTF_T16_T16_BGR           0x02100000+PNGCP_16_16_BGR
#define PNG_OUTF_T16_T8_BGR            0x02100100+PNGCP_16_8_BGR	//DIB default

#define PNG_OUTF_P1_P1                 0x03010100+PNGCP_X_X			//DIB default
#define PNG_OUTF_P2_P2                 0x03020000+PNGCP_X_X
#define PNG_OUTF_P2_P4                 0x03020100+PNGCP_2_4			//DIB default
#define PNG_OUTF_P4_P4                 0x03040100+PNGCP_X_X			//DIB default
#define PNG_OUTF_P8_P8				   0x03080100+PNGCP_X_X			//DIB default

#define PNG_OUTF_GA8_GA8               0x04080000+PNGCP_X_X
#define PNG_OUTF_GA8_G8                0x04080100+PNGCP_8_8_SA		//DIB default
#define PNG_OUTF_GA16_GA16             0x04100000+PNGCP_16_16
#define PNG_OUTF_GA16_G16              0x04100000+PNGCP_16_16_SA
#define PNG_OUTF_GA16_GA8              0x04100000+PNGCP_16_8
#define PNG_OUTF_GA16_G8               0x04100100+PNGCP_16_8_SA		//DIB default

#define PNG_OUTF_TA8_TA8_BGRA          0x06080100+PNGCP_8_8_BGRA	//DIB default
#define PNG_OUTF_TA8_T8_BGR            0x06080100+PNGCP_8_8_BGR_SA	//DIB optional
#define PNG_OUTF_TA16_TA16_BGRA        0x06100000+PNGCP_16_16_BGRA
#define PNG_OUTF_TA16_T16_BGR          0x06100000+PNGCP_16_16_BGR_SA
#define PNG_OUTF_TA16_TA8_BGRA         0x06100100+PNGCP_16_8_BGRA	//DIB default
#define PNG_OUTF_TA16_T8_BGR           0x06100100+PNGCP_16_8_BGR_SA	//DIB optional

#define	PNG_OUTF_AUTO					-1


/******************************************************************************
	Error codes
 ******************************************************************************/
#define PNGLIB_ERR_OPENFILE					0x1000		//Couldn't open file
#define PNGLIB_ERR_MEMALLOC					0x1001		//Couldn't allocate memory
#define PNGLIB_ERR_ALREADY_LOADED			0x1002		//Other data has already been loaded

#define PNGLIB_ERR_INVALIDPNG				0x2000		//Corrupt or invalid PNG
#define PNGLIB_ERR_UNSUPPORTED				0x2001		//PNG format not supported
#define PNGLIB_ERR_INVALIDFORMAT			0x2002		//User error (!): Invalid format specified
#define PNGLIB_ERR_WRONGSTATE				0x2003		//User error (!): Function call at the wrong time
														//(e.g. decoding before loading data)
#define PNGLIB_ERR_NOPALETTE				0x2004		//Palette requested for a non-paletted image
#define PNGLIB_ERR_INVALIDPARAMETER 		0x2005		//Invalid parameter used for function
#define PNGLIB_ERR_CREATEBITMAP				0x2006		//Couldn't create bitmap

#define PNGLIB_ERR_FINDRESOURCE				0x3001		//Couldn't find resource
#define PNGLIB_ERR_LOADRESOURCE				0x3002		//Couldn't load resource


#define PNG_PF_RGB			0	//output: red byte, green byte, blue byte
#define PNG_PF_BGRX			1	//output: blue byte, green byte, red byte, zero byte

#pragma pack()
