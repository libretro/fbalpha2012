// Direct3D9 video output
// rewritten by regret  (Motion Blur source from VBA-M)
#include "burner.h"
#include "vid_directx_support.h"
#include "vid_filter.h"
#include <xgraphics.h>

#ifdef _MSC_VER
#pragma comment(lib, "d3d9")
#endif

#include <d3d9.h>
#include <d3dx9effect.h>

const float PI = 3.14159265358979323846f;

extern int ArcadeJoystick;
int dx9RenderStretch();
void dx9SetTargets();

typedef struct _VERTEX {
		float x, y, z;
		float rhw;
		float tx, ty;
		_VERTEX() {}
		_VERTEX(float x,float y,float z,float rhw,float tx,float ty) {
			this->x=x;this->y=y;this->z=z;this->rhw=rhw;this->tx=tx;this->ty=ty;
		}
} VERTEX; //our custom vertex with a constuctor for easier assignment

VERTEX triangleStripVertices[4];

struct d3dvertex {
	float x, y, z, rhw; //screen coords
	float u, v;         //texture coords
};

struct transp_vertex {
	float x, y, z, rhw;
	D3DCOLOR color;
	float u, v;
};

D3DVERTEXELEMENT9 decl[] = 
{
{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
D3DDECL_END()
};

//--------------------------------------------------------------------------------------
// Vertex and pixel shaders for gradient background rendering
//--------------------------------------------------------------------------------------
static const CHAR* g_strGradientShader =
    "struct VS_IN                                              \n"
    "{                                                         \n"
    "   float4   Position     : POSITION;                      \n"
    "   float2   UV        : TEXCOORD0;                        \n"
    "};                                                        \n"
    "                                                          \n"
    "struct VS_OUT                                             \n"
    "{                                                         \n"
    "   float4 Position       : POSITION;                      \n"
    "   float2 UV        : TEXCOORD0;                        \n"
    "};                                                        \n"
    "                                                          \n"
    "VS_OUT GradientVertexShader( VS_IN In )                   \n"
    "{                                                         \n"
    "   VS_OUT Out;                                            \n"
    "   Out.Position = In.Position;                            \n"
    "   Out.UV  = In.UV;                               \n"
    "   return Out;                                            \n"
    "}                                                         \n";
   
//-------------------------------------------------------------------------------------
// Pixel shader
//-------------------------------------------------------------------------------------
const char*                                 g_strPixelShaderProgram =
    " struct PS_IN                                 "
    " {                                            "
    "     float2 TexCoord : TEXCOORD;              "
    " };                                           "  // the vertex shader
    "                                              "
    " sampler detail;                              "
    "                                              "
    " float4 main( PS_IN In ) : COLOR              "
    " {                                            "
    "     return tex2D( detail, In.TexCoord );     "  // Output color
    " }     ";   

LPDIRECT3DVERTEXBUFFER9 vertexBuffer = NULL;	 
D3DVertexShader* g_pGradientVertexShader = NULL;
D3DVertexDeclaration* g_pGradientVertexDecl = NULL;
IDirect3DPixelShader9* g_pPixelShader = NULL;
LPD3DXBUFFER ppShader = NULL;

#define D3DFVF_LVERTEX2 (D3DFVF_XYZRHW | D3DFVF_TEX1)

extern IDirect3DDevice9 *pDevice;
extern IDirect3D9 *pD3D;
static IDirect3D9* pD3D = NULL;				// Direct3D interface
static D3DPRESENT_PARAMETERS d3dpp;
static IDirect3DDevice9* pD3DDevice = NULL;

static unsigned char mbCurrentTexture = 0;	// current texture for motion blur
static bool mbTextureEmpty = true;
static d3dvertex vertex[4];
static transp_vertex transpVertex[4];

static IDirect3DTexture9* pTexture = NULL;
static int nTextureWidth = 0;
static int nTextureHeight = 0;
D3DFORMAT textureFormat;
 
static D3DCOLOR osdColor = D3DCOLOR_ARGB(0xFF, 0xFF, 0xFF, 0xFF);

static int nGameImageWidth, nGameImageHeight;
static int nImageWidth, nImageHeight, nImageZoom;


extern int nXOffset;
extern int nYOffset;
extern int nXScale;
extern int nYScale;

extern int bAdjustScreen;

#if 0
struct {
	unsigned int t_usage, v_usage;
	unsigned int t_pool,  v_pool;
	unsigned int lock;
	unsigned int filter;
} flags;

struct {
	bool dynamic;     //device supports dynamic textures
	bool stretchrect; //device supports StretchRect
} caps;
#endif

static RECT Dest;
VOID* pPhysicalAddress = NULL;
// ----------------------------------------------------------------------------

static inline TCHAR* TextureFormatString(D3DFORMAT nFormat)
{
	switch (nFormat) {
		case D3DFMT_X1R5G5B5:
			return _T("16-bit xRGB 1555");
		case D3DFMT_R5G6B5:
			return _T("16-bit RGB 565");
		case D3DFMT_X8R8G8B8:
			return _T("32-bit xRGB 8888");
		case D3DFMT_A8R8G8B8:
			return _T("32-bit ARGB 8888");
		case D3DFMT_A16B16G16R16F:
			return _T("64-bit ARGB 16161616fp");
		case D3DFMT_A32B32G32R32F:
			return _T("128-bit ARGB 32323232fp");
	}

	return _T("unknown format");
}

// Select optimal full-screen resolution
static inline int dx9SelectFullscreenMode(VidSDisplayScoreInfo* pScoreInfo)
{
	pScoreInfo->nBestWidth = nVidWidth;
	pScoreInfo->nBestHeight = nVidHeight; 

	if (!bDrvOkay && (pScoreInfo->nBestWidth < 640 || pScoreInfo->nBestHeight < 480)) {
		return 1;
	}

	return 0;
}

// ----------------------------------------------------------------------------

static inline void dx9ReleaseTexture()
{
 
	pD3DDevice->SetTexture( 0, NULL);
	RELEASE(pTexture);

}

static inline int dx9Exit()
{
	VidFilterExit();

	dx9ReleaseTexture();

	VidSFreeVidImage();
 	

	nRotateGame = 0;
 
	if (g_pPixelShader)
	{
		pD3DDevice->SetPixelShader( NULL );		 
		delete [] g_pPixelShader;
		g_pPixelShader = NULL;
	}

	if (pPhysicalAddress)
	{
		XPhysicalFree(pPhysicalAddress);
		pPhysicalAddress = NULL;
	}


	return 0;
}

static inline int dx9Resize(int width, int height)
{	
 
#if 0
	if (!pTexture)
	{
		pTexture = new IDirect3DTexture9;

		DWORD dwTextureSize = XGSetTextureHeader( width,
											  height,
											  1, 
											  0,
											  D3DFMT_LIN_R5G6B5,
											  0,
											  0,
											  XGHEADER_CONTIGUOUS_MIP_OFFSET,
											  0,
											  pTexture,
											  NULL,
											  NULL ); 
		pBuffer = XPhysicalAlloc( dwTextureSize, MAXULONG_PTR, 0, PAGE_READWRITE | PAGE_WRITECOMBINE );
		XGOffsetResourceAddress( pTexture, pBuffer ); 
	}
#endif
     
        if (FAILED(pD3DDevice->CreateTexture(width, height, 1, D3DUSAGE_CPU_CACHED_MEMORY, D3DFMT_LIN_R5G6B5, 0, &pTexture, NULL))) {
#ifdef PRINT_DEBUG_INFO
                dprintf(_T("  * Error: Couldn't create texture.\n"));
#endif
                return 1;
        }
#ifdef PRINT_DEBUG_INFO
        dprintf(_T("  * Allocated a %i x %i (%s) image texture.\n"), width, height, TextureFormatString(textureFormat));
#endif

	return 0;
}

void doStretch(void)
{	
	XINPUT_STATE state;

	dx9SetTargets();

	while ( 1 )
	{

		XInputGetState( 0, &state );

		dx9RenderStretch();
	 		 	
		if (ArcadeJoystick != 1)
		{
			if (state.Gamepad.sThumbLX <= -32000)
			{
				nXOffset -= 1;
			}
			else if (state.Gamepad.sThumbLX >= 32000)
			{
				nXOffset += 1;
			}

			if (state.Gamepad.sThumbLY <= -32000)
			{
				nYOffset += 1;
			}	
			else if (state.Gamepad.sThumbLY >= 32000)
			{
				nYOffset -= 1;
			}

			if (state.Gamepad.sThumbRX <= -32000 )
			{
				nXScale -= 1;
			}
			else if (state.Gamepad.sThumbRX >= 32000)
			{
				nXScale += 1;
			}

			if (state.Gamepad.sThumbRY <= -32000 )
			{
				nYScale += 1;
			}	
			else if (state.Gamepad.sThumbRY >= 32000)
			{
				nYScale -= 1;
			}
		}
		else
		{

			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT && state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				nXScale -= 1;
			}
			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT && state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				nXScale += 1;
			}

			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN && state.Gamepad.wButtons & XINPUT_GAMEPAD_B )
			{
				nYScale += 1;
			}	
			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP && state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{
				nYScale -= 1;
			}
			
			if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
			{
				nXOffset -= 1;
			}
			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
			{
				nXOffset += 1;
			}

			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
			{
				nYOffset += 1;
			}	
			else if (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
			{
				nYOffset -= 1;
			}

			

		}

		if(state.Gamepad.wButtons & XINPUT_GAMEPAD_Y)
		{	
				nYOffset = 0;
				nXOffset = 0;
				nYScale = 0;
				nXScale = 0;
		}

		if (ArcadeJoystick != 1)
		{
			if(state.Gamepad.wButtons & XINPUT_GAMEPAD_B)
			{	
				pDevice->SetTexture( 0, NULL); 
				pDevice->SetStreamSource(0, NULL, 0, 0);
				bAdjustScreen = 0;		
				break;
			}
		}
		else
		{
			if(state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK)
			{	
				pDevice->SetTexture( 0, NULL); 
				pDevice->SetStreamSource(0, NULL, 0, 0);
				bAdjustScreen = 0;		
				break;
			}




		}
		
	}
	 
}



static inline int dx9TextureInit()
{
	if (nRotateGame & 1) {
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	} else {
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	nGameImageWidth = nVidImageWidth;
	nGameImageHeight = nVidImageHeight;

	nVidImageDepth = nVidScrnDepth;

	// Determine if we should use a texture format different from the screen format
	if ((bDrvOkay && VidFilterCheckDepth(nPreScaleEffect, 32) != 32)
		|| (bDrvOkay && bVidForce16bit)) {
		nVidImageDepth = 16;
	}

	switch (nVidImageDepth) {
		case 32:
			textureFormat = D3DFMT_X8R8G8B8;
			break;
		case 16:
			textureFormat = D3DFMT_R5G6B5;
			break;
		case 15:
			textureFormat = D3DFMT_X1R5G5B5;
			break;
	}

	nVidImageBPP = (nVidImageDepth + 7) >> 3;
	nBurnBpp = nVidImageBPP;					// Set Burn library Bytes per pixel

	// Use our callback to get colors:
	SetBurnHighCol(nVidImageDepth);

	// Make the normal memory buffer
	if (VidSAllocVidImage()) {
		dx9Exit();
		return 1;
	}

	nTextureWidth = VidGetTextureSize(nGameImageWidth * nPreScaleZoom);
	nTextureHeight = VidGetTextureSize(nGameImageHeight * nPreScaleZoom);

	if (dx9Resize(nTextureWidth, nTextureHeight)) {
		return 1;
	}

	return 0;
}

//  Vertex format:
  //
  //  0----------1
  //  |         /|
  //  |       /  |
  //  |     /    |
  //  |   /      |
  //  | /        |
  //  2----------3
  //
  //  (x,y) screen coords, in pixels
  //  (u,v) texture coords, betweeen 0.0 (top, left) to 1.0 (bottom, right)
static inline int dx9SetVertex(
	unsigned int px, unsigned int py, unsigned int pw, unsigned int ph,
    unsigned int tw, unsigned int th,
    unsigned int x, unsigned int y, unsigned int w, unsigned int h
    )
{
	// configure triangles
	// -0.5f is necessary in order to match texture alignment to display pixels
	float diff = -0.5f;

	void *pLockedVertexBuffer;

#if 1
	if (nRotateGame & 1) {
		if (nRotateGame & 2) {
			vertex[2].x = vertex[3].x = (double)(y    ) + diff + nXOffset + nXScale;
			vertex[0].x = vertex[1].x = (double)(y + h) + diff + nXOffset;
			vertex[1].y = vertex[3].y = (double)(x + w) + diff + nYOffset + nYScale;
			vertex[0].y = vertex[2].y = (double)(x    ) + diff + nYOffset;
		} else {
			vertex[0].x = vertex[1].x = (double)(y    ) + diff + nXOffset + nXScale;
			vertex[2].x = vertex[3].x = (double)(y + h) + diff + nXOffset;
			vertex[1].y = vertex[3].y = (double)(x    ) + diff + nYOffset + nYScale;
			vertex[0].y = vertex[2].y = (double)(x + w) + diff + nYOffset;
		}
	} else {
		if (nRotateGame & 2) {
			vertex[1].x = vertex[3].x = (double)(y    ) + diff + nXOffset + nXScale;
			vertex[0].x = vertex[2].x = (double)(y + h) + diff + nXOffset;
			vertex[2].y = vertex[3].y = (double)(x    ) + diff + nYOffset + nYScale;
			vertex[0].y = vertex[1].y = (double)(x + w) + diff + nYOffset;
		} else {
			vertex[0].x = vertex[2].x = (double)(x    ) + diff + nXOffset + nXScale;
			vertex[1].x = vertex[3].x = (double)(x + w) + diff + nXOffset;
			vertex[0].y = vertex[1].y = (double)(y    ) + diff + nYOffset + nYScale;
			vertex[2].y = vertex[3].y = (double)(y + h) + diff + nYOffset;
		}
	}

	double rw = (double)w / (double)pw * (double)tw;
	double rh = (double)h / (double)ph * (double)th;

	vertex[0].u = vertex[2].u = (double)(px    ) / rw;
	vertex[1].u = vertex[3].u = (double)(px + w) / rw;
	vertex[0].v = vertex[1].v = (double)(py    ) / rh;
	vertex[2].v = vertex[3].v = (double)(py + h) / rh;
#else
	if (nRotateGame & 1) {
		if (nRotateGame & 2) {
			vertex[1].x = vertex[3].x = (float)(y + h) + diff;
			vertex[0].x = vertex[2].x = (float)(y    ) + diff;
			vertex[2].y = vertex[3].y = (float)(x + w) + diff;
			vertex[0].y = vertex[1].y = (float)(x    ) + diff;
		} else {
			vertex[0].x = vertex[2].x = (float)(y + h) + diff;
			vertex[1].x = vertex[3].x = (float)(y    ) + diff;
			vertex[2].y = vertex[3].y = (float)(x    ) + diff;
			vertex[0].y = vertex[1].y = (float)(x + w) + diff;
		}
	} else {
		if (nRotateGame & 2) {
			vertex[2].x = vertex[3].x = (float)(x    ) + diff;
			vertex[0].x = vertex[1].x = (float)(y + h) + diff;
			vertex[0].y = vertex[2].y = (float)(y    ) + diff;
			vertex[1].y = vertex[3].y = (float)(x + w) + diff;
		} else {
			vertex[0].x = vertex[1].x = (float)(x    ) + diff;
			vertex[2].x = vertex[3].x = (float)(x + w) + diff;
			vertex[0].y = vertex[2].y = (float)(y + h) + diff;
			vertex[1].y = vertex[3].y = (float)(y    ) + diff;
		}
	}

	vertex[0].u = vertex[1].u = 0.0f;
	vertex[2].u = vertex[3].u = (float)pw / (float)tw;
	vertex[0].v = vertex[2].v = (float)ph / (float)th;
	vertex[1].v = vertex[3].v = 0.0f;
#endif

	// Z-buffer and RHW are unused for 2D blit, set to normal values
	vertex[0].z = vertex[1].z = vertex[2].z = vertex[3].z = 0.0f;
	vertex[0].rhw = vertex[1].rhw = vertex[2].rhw = vertex[3].rhw = 1.0f;

	// configure semi-transparent triangles
	if (vidMotionBlur) {
		D3DCOLOR semiTrans = D3DCOLOR_ARGB(0x7F, 0xFF, 0xFF, 0xFF);
		transpVertex[0].x = vertex[0].x;
		transpVertex[0].y = vertex[0].y;
		transpVertex[0].z = vertex[0].z;
		transpVertex[0].rhw = vertex[0].rhw;
		transpVertex[0].color = semiTrans;
		transpVertex[0].u = vertex[0].u;
		transpVertex[0].v = vertex[0].v;
		transpVertex[1].x = vertex[1].x;
		transpVertex[1].y = vertex[1].y;
		transpVertex[1].z = vertex[1].z;
		transpVertex[1].rhw = vertex[1].rhw;
		transpVertex[1].color = semiTrans;
		transpVertex[1].u = vertex[1].u;
		transpVertex[1].v = vertex[1].v;
		transpVertex[2].x = vertex[2].x;
		transpVertex[2].y = vertex[2].y;
		transpVertex[2].z = vertex[2].z;
		transpVertex[2].rhw = vertex[2].rhw;
		transpVertex[2].color = semiTrans;
		transpVertex[2].u = vertex[2].u;
		transpVertex[2].v = vertex[2].v;
		transpVertex[3].x = vertex[3].x;
		transpVertex[3].y = vertex[3].y;
		transpVertex[3].z = vertex[3].z;
		transpVertex[3].rhw = vertex[3].rhw;
		transpVertex[3].color = semiTrans;
		transpVertex[3].u = vertex[3].u;
		transpVertex[3].v = vertex[3].v;
	}

	pD3DDevice->SetStreamSource(0,NULL,0,NULL);	
	HRESULT hr = vertexBuffer->Lock(0,0,&pLockedVertexBuffer,NULL);
	memcpy(pLockedVertexBuffer,vertex,sizeof(vertex));
	vertexBuffer->Unlock();
 
	return 0;
}

// ==> osd for dx9 video output (ugly), added by regret
static inline int dx9CreateFont()
{
	return 0;
}

static inline void dx9DrawText()
{
 
}
// <== osd for dx9 video output (ugly)

static inline int dx9Init()
{

#ifdef ENABLE_PROFILING
	ProfileInit();
#endif

#ifdef PRINT_DEBUG_INFO
	dprintf(_T("*** Initialising Direct3D 9 blitter.\n"));
#endif

	XVIDEO_MODE VideoMode; 
	XMemSet( &VideoMode, 0, sizeof(XVIDEO_MODE) ); 
	XGetVideoMode( &VideoMode );

	hVidWnd = (HWND)1;

	// check selected atapter
	if (nVidAdapter >= pD3D->GetAdapterCount()) {
		nVidAdapter = D3DADAPTER_DEFAULT;
	}

	nVidAdapter = D3DADAPTER_DEFAULT;
	nVidFullscreen = 1;
	memset(&d3dpp, 0, sizeof(d3dpp));
	
	VidSDisplayScoreInfo ScoreInfo;
	memset(&ScoreInfo, 0, sizeof(VidSDisplayScoreInfo));

	if (dx9SelectFullscreenMode(&ScoreInfo)) {
		dx9Exit();
#ifdef PRINT_DEBUG_INFO
		dprintf(_T("  * Error: Couldn't determine display mode.\n"));
#endif
		return 1;
	}

	if (!VideoMode.fIsWideScreen)
	{
		d3dpp.Flags |=  D3DPRESENTFLAG_NO_LETTERBOX;
	}

	// Determine high-definition back buffer values.
	d3dpp.BackBufferWidth = 1280;
	d3dpp.BackBufferHeight = 720;

	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;		 
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;		 
	

	pD3DDevice = pDevice;

	//if (FAILED(pD3DDevice->Reset(&d3dpp))) {
	//	return 1;
	//}	

	nVidScrnWidth = d3dpp.BackBufferWidth; nVidScrnHeight = d3dpp.BackBufferHeight;
	nVidScrnDepth = 16;
 
	VidInitInfo();
	
	mbCurrentTexture = 0;
	pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	// apply vertex alpha values to texture

	// for filter
	VidFilterParamInit();

	// Initialize the buffer surfaces
	if (dx9TextureInit()) {
		dx9Exit();
		return 1;
	}

	if (nPreScaleEffect) {
		if (VidFilterInit(nPreScaleEffect, 0)) {
#ifdef PRINT_DEBUG_INFO
			dprintf(_T("  * Error: Couldn't initialise pixel filter.\n"));
#endif
			dx9Exit();
			return 1;
		}
	}

	pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, vidFilterLinear ? D3DTEXF_LINEAR : D3DTEXF_POINT);
	pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, vidFilterLinear ? D3DTEXF_LINEAR : D3DTEXF_POINT);

	nImageWidth = 0; nImageHeight = 0;

	// Create osd font
	dx9CreateFont();

	    // Compile pixel shader.
    ID3DXBuffer* pPixelShaderCode;
    ID3DXBuffer*  pPixelErrorMsg;
    HRESULT hr = D3DXCompileShader( g_strPixelShaderProgram,
                            ( UINT )strlen( g_strPixelShaderProgram ),
                            NULL,
                            NULL,
                            "main",
                            "ps_2_0",
                            0,
                            &pPixelShaderCode,
                            &pPixelErrorMsg,
                            NULL );
    if( FAILED( hr ) )
    {
        if( pPixelErrorMsg )
            OutputDebugString( ( char* )pPixelErrorMsg->GetBufferPointer() );
        return E_FAIL;
    }

    // Create pixel shader.
    //pD3DDevice->CreatePixelShader( ( DWORD* )pPixelShaderCode->GetBufferPointer(),
    //                                 &g_pPixelShader );


	if (pPhysicalAddress == NULL)
	{
		XGMICROCODESHADERPARTS parts;
		XGGetMicrocodeShaderParts( ( DWORD* )pPixelShaderCode->GetBufferPointer(), &parts );

		g_pPixelShader = (IDirect3DPixelShader9*) new char[parts.cbCachedPartSize];
		XGSetPixelShaderHeader( g_pPixelShader, parts.cbCachedPartSize, &parts );

		pPhysicalAddress = XPhysicalAlloc( parts.cbPhysicalPartSize, 
											 MAXULONG_PTR,
											 D3DSHADER_ALIGNMENT,
											 PAGE_READWRITE | PAGE_WRITECOMBINE );
		memcpy( pPhysicalAddress, parts.pPhysicalPart, parts.cbPhysicalPartSize );
		XGRegisterPixelShader( g_pPixelShader, pPhysicalAddress );

		pD3DDevice->SetPixelShader( g_pPixelShader );
	}

	// Create vertex declaration
    if( NULL == g_pGradientVertexDecl )
    {
        static const D3DVERTEXELEMENT9 decl[] =
        {
			{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
			{ 0, 16, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
            D3DDECL_END()
        };

        if( FAILED( pD3DDevice->CreateVertexDeclaration( decl, &g_pGradientVertexDecl ) ) )
            return E_FAIL;
    }
 
	    // Create vertex shader
    if( NULL == g_pGradientVertexShader )
    {
        ID3DXBuffer* pShaderCode;
        if( FAILED( D3DXCompileShader( g_strGradientShader, strlen( g_strGradientShader ),
                                       NULL, NULL, "GradientVertexShader", "vs_2_0", 0,
                                       &pShaderCode, NULL, NULL ) ) )
            return false;

        if( FAILED( pD3DDevice->CreateVertexShader( ( DWORD* )pShaderCode->GetBufferPointer(),
                                                      &g_pGradientVertexShader ) ) )
            return false;

		if (pShaderCode)
		{
			pShaderCode->Release();
			pShaderCode = NULL;
		}
    }
 

	if (NULL == vertexBuffer)
	{
		hr = pD3DDevice->CreateVertexBuffer(sizeof(triangleStripVertices),D3DUSAGE_CPU_CACHED_MEMORY,0,0,&vertexBuffer,NULL);
		if(FAILED(hr)) {
			 
			return false;
		}
	}
 

	pD3DDevice->SetVertexShader( g_pGradientVertexShader );			
    pD3DDevice->SetVertexDeclaration( g_pGradientVertexDecl );
	pD3DDevice->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );


	return 0;
}



void dx9ClearTargets()
{
	pD3DDevice->SetVertexShader(NULL);
	pD3DDevice->SetPixelShader(NULL);
	pD3DDevice->SetStreamSource(0,NULL,0,NULL);		
	pD3DDevice->SetTexture( 0, NULL); 	 
}
 
static inline int dx9Reset()
{
#ifdef PRINT_DEBUG_INFO
	dprintf(_T("*** Resestting Direct3D device.\n"));
#endif

	dx9ReleaseTexture();

	if (FAILED(pD3DDevice->Reset(&d3dpp))) {
		return 1;
	}

	dx9TextureInit();

	nImageWidth = 0; nImageHeight = 0;

	return 0;
}

void doResetDX9()
{
	pD3DDevice->Reset(&d3dpp);
}

void dx9SetTargets()
{
    pD3DDevice->SetVertexShader( NULL );
    pD3DDevice->SetVertexDeclaration( NULL );
    pD3DDevice->SetPixelShader( NULL );
	pD3DDevice->SetTexture(0, NULL );

	pD3DDevice->SetVertexShader( g_pGradientVertexShader );		
	pD3DDevice->SetPixelShader( g_pPixelShader );
    pD3DDevice->SetVertexDeclaration( g_pGradientVertexDecl );
	pD3DDevice->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );
}

static inline int dx9Scale(RECT* rect, int width, int height)
{
	if (vidUseFilter && vidForceFilterSize) {
		return VidFilterScale(rect, width, height);
	}
	return VidSScaleImage(rect, width, height);
}

// Copy BlitFXsMem to pddsBlitFX
static inline int dx9Render()
{

	if (pTexture==NULL)
	{
		return 0;
	}


 
	getClientScreenRect(hVidWnd, &Dest);

	dx9Scale(&Dest, nGameWidth, nGameHeight);

	{
		int nNewImageWidth  = nRotateGame ? (Dest.bottom - Dest.top) : (Dest.right - Dest.left);
		int nNewImageHeight = nRotateGame ? (Dest.right - Dest.left) : (Dest.bottom - Dest.top);

		if (nImageWidth != nNewImageWidth || nImageHeight != nNewImageHeight) {
			nImageWidth  = nNewImageWidth;
			nImageHeight = nNewImageHeight;

			int nWidth = nGameImageWidth;
			int nHeight = nGameImageHeight;

			if (nPreScaleEffect) {
				if (nPreScale & 1) {
					nWidth *= nPreScaleZoom;
				}
				if (nPreScale & 2) {
					nHeight *= nPreScaleZoom;
				}
			}

			dx9SetVertex(0, 0, nWidth, nHeight, nTextureWidth, nTextureHeight, nRotateGame ? Dest.top : Dest.left, nRotateGame ? Dest.left : Dest.top, nImageWidth, nImageHeight);

		 
		}
	}

#if 1
	{
		// Copy the game image onto a texture for rendering
		D3DLOCKED_RECT d3dlr;

		pTexture->LockRect(0, &d3dlr, NULL, D3DLOCK_NOOVERWRITE);

		int pitch = d3dlr.Pitch;
		unsigned char* pd = (unsigned char*)d3dlr.pBits;

		if (nPreScaleEffect) {
			VidFilterApplyEffect(pd, pitch);
		} else {
			unsigned char* ps = pVidImage + nVidImageLeft * nVidImageBPP;
			int s = nVidImageWidth * nVidImageBPP;

			switch (nVidImageDepth) {
				case 32:
					VidSCpyImg32(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
				case 16:
					VidSCpyImg16(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
			}
		}

		pTexture->UnlockRect(0);
	}
#endif 
 
	// draw the current frame to the screen
	pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET,  D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	pD3DDevice->SetTexture( 0, pTexture );	 
	pD3DDevice->SetStreamSource(0,vertexBuffer,0,sizeof(VERTEX));			
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2); 
	 

	pD3DDevice->EndScene();
 
	return 0;
}


int dx9RenderStretch()
{

	if (pTexture==NULL)
	{
		return 0;
	}
 
	getClientScreenRect(hVidWnd, &Dest);

	dx9Scale(&Dest, nGameWidth, nGameHeight);

	{
		int nNewImageWidth  = nRotateGame ? (Dest.bottom - Dest.top) : (Dest.right - Dest.left);
		int nNewImageHeight = nRotateGame ? (Dest.right - Dest.left) : (Dest.bottom - Dest.top);

		//if (nImageWidth != nNewImageWidth || nImageHeight != nNewImageHeight) {
			nImageWidth  = nNewImageWidth;
			nImageHeight = nNewImageHeight;

			int nWidth = nGameImageWidth;
			int nHeight = nGameImageHeight;

			if (nPreScaleEffect) {
				if (nPreScale & 1) {
					nWidth *= nPreScaleZoom;
				}
				if (nPreScale & 2) {
					nHeight *= nPreScaleZoom;
				}
			}

			dx9SetVertex(0, 0, nWidth, nHeight, nTextureWidth, nTextureHeight, nRotateGame ? Dest.top : Dest.left, nRotateGame ? Dest.left : Dest.top, nImageWidth, nImageHeight);

		 
		//}
	}

#if 1
	{
		// Copy the game image onto a texture for rendering
		D3DLOCKED_RECT d3dlr;

		pTexture->LockRect(0, &d3dlr, NULL, D3DLOCK_NOOVERWRITE);

		int pitch = d3dlr.Pitch;
		unsigned char* pd = (unsigned char*)d3dlr.pBits;

		if (nPreScaleEffect) {
			VidFilterApplyEffect(pd, pitch);
		} else {
			unsigned char* ps = pVidImage + nVidImageLeft * nVidImageBPP;
			int s = nVidImageWidth * nVidImageBPP;

			switch (nVidImageDepth) {
				case 32:
					VidSCpyImg32(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
				case 16:
					VidSCpyImg16(pd, pitch, ps, s, nVidImageWidth, nVidImageHeight);
					break;
			}
		}

		pTexture->UnlockRect(0);
	}
#endif 
 
	// draw the current frame to the screen
	pD3DDevice->Clear(0, NULL, D3DCLEAR_TARGET,  D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	pD3DDevice->SetTexture( 0, pTexture );	 
	pD3DDevice->SetStreamSource(0,vertexBuffer,0,sizeof(VERTEX));			
	pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2); 
	 

	pD3DDevice->EndScene();
	pD3DDevice->Present(NULL, NULL, NULL, NULL);

	return 0;
}

// Run one frame and render the screen
inline int dx9Frame(bool bRedraw)		// bRedraw = 0
{
	if (pVidImage == NULL) {
		return 1;
	}

	 
#ifdef ENABLE_PROFILING
//	ProfileProfileStart(0);
#endif

	if (bDrvOkay) {
		if (bRedraw) {					// Redraw current frame
			if (BurnDrvRedraw()) {
				BurnDrvFrame();			// No redraw function provided, advance one frame
			}
		} else {
			BurnDrvFrame();				// Run one frame and draw the screen
		}
	}

#ifdef ENABLE_PROFILING
//	ProfileProfileEnd(0);
	ProfileProfileStart(1);
#endif

	dx9Render();

#ifdef ENABLE_PROFILING
	ProfileProfileEnd(1);

	dprintf(_T("    blit %3.2lf (effect p1 %3.2lf - effect p2 %3.2lf)\n"),
		ProfileProfileReadAverage(1), ProfileProfileReadAverage(2), ProfileProfileReadAverage(0));
#endif

	if (bAdjustScreen == 1)
	{
		doStretch();
	}

	return 0;
}

// Paint the BlitFX surface onto the primary surface
static inline int dx9Paint(int bValidate)
{
 
	RECT rect = { 0, 0, 0, 0 };

	if (!nVidFullscreen) {
		getClientScreenRect(hVidWnd, &rect);

		dx9Scale(&rect, nGameWidth, nGameHeight);

		if ((rect.right - rect.left) != (Dest.right - Dest.left)
			|| (rect.bottom - rect.top ) != (Dest.bottom - Dest.top)) {
			bValidate |= 2;
		}
	}

	if (bValidate & 2) {
		dx9Render();
	}

	pD3DDevice->Present(NULL, NULL, NULL, NULL);

	return 0;
}

// ----------------------------------------------------------------------------

static inline int dx9GetSettings(InterfaceInfo* pInfo)
{
	return 0;
}

// The Video Output plugin:
struct VidOut VidOutD3D = { dx9Init, dx9Exit, dx9Frame, dx9Paint, dx9Scale, dx9GetSettings };
