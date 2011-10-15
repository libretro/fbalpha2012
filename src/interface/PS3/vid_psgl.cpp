#include <sys/sys_time.h>

#include "burner.h"
#include "vid_support-ps3.h"
#include "vid_psgl.h"

extern std::vector<std::string> m_ListShaderData;
extern std::vector<std::string> m_ListShader2Data;

static PSGLdevice* psgl_device = NULL;
static PSGLcontext* psgl_context = NULL;

GLuint gl_width = 0;
GLuint gl_height = 0;
static int nImageWidth;
static int nImageHeight;
static int nGameWidth = 0;
static int nGameHeight = 0;
static int nRotateGame = 0;

static CGcontext CgContext = NULL;
static CGprogram VertexProgram = NULL;
static CGprogram FragmentProgram = NULL;
static CGparameter ModelViewProj_cgParam;
static CGparameter cg_video_size;
static CGparameter cg_texture_size;
static CGparameter cg_output_size;
static CGparameter cg_v_video_size;
static CGparameter cg_v_texture_size;
static CGparameter cg_v_output_size;
static CGparameter cgp_timer;
static CGparameter cgp_vertex_timer;
static GLuint cg_viewport_width;
static GLuint cg_viewport_height;
static GLuint bufferID = 0;
static uint32_t frame_count;
extern GLfloat m_left, m_right, m_bottom, m_top, m_zNear, m_zFar;
uint32_t m_overscan;
float m_overscan_amount;

typedef struct dstResType
{
	uint32_t w;
	uint32_t h;
	uint32_t resId;
}; 

struct dstResType availableResolutions[8]; 

static const dstResType allResolutions[] = 
{
	{720,480, CELL_VIDEO_OUT_RESOLUTION_480},
	{720,576, CELL_VIDEO_OUT_RESOLUTION_576}, 
	{1280,720, CELL_VIDEO_OUT_RESOLUTION_720},
	{960,1080,CELL_VIDEO_OUT_RESOLUTION_960x1080},
	{1280,1080,CELL_VIDEO_OUT_RESOLUTION_1280x1080},
	{1440,1080,CELL_VIDEO_OUT_RESOLUTION_1440x1080},
	{1600,1080,CELL_VIDEO_OUT_RESOLUTION_1600x1080},
	{1920,1080, CELL_VIDEO_OUT_RESOLUTION_1080}
};

static int availableResolutionsCount = 0; 
uint32_t currentAvailableResolutionNo;
uint32_t currentAvailableResolutionId;

// forward declarations
unsigned int __cdecl HighCol16(int r, int g, int b, int);
unsigned int __cdecl HighCol24(int r, int g, int b, int);

// normal vertex
static const GLfloat verts[] = {
	0, 0,
	1, 0,
	1, 1,
	0, 1,
};

static const GLfloat   tverts[] = {
      0, 1,						
      1, 1, 
      1, 0, 
      0, 0
};

static const GLfloat   tvertsFlippedRotated[] = {	
      1.0f, 1.0f,						
      1.0f, 0.0f, 
      0.0f, 0.0f, 
      0.0f, 1.0f
};

static const GLfloat   tvertsFlipped[] = {	
      1.0f, 0.0f,						
      0.0f, 0.0f, 
      0.0f, 1.0f, 
      1.0f, 1.0f
};

static const GLfloat   tvertsVertical[] ={
	0.0f, 0.0f,						
	0.0f, 1.0f, 
	1.0f, 1.0f, 
	1.0f, 0.0f
};

static void VidSFreeVidImage()
{
	if (pVidImage)
	{
		free(pVidImage);
		pVidImage = NULL;
	}
}

static int VidSAllocVidImage()
{
	nVidImagePitch = nVidImageWidth * SCREEN_RENDER_TEXTURE_BPP;
	unsigned int nMemLen = (nVidImageHeight + 4) * nVidImagePitch;

	VidSFreeVidImage();

	pVidImage = (uint8_t*)realloc(pVidImage, nMemLen);

	if (pVidImage)
	{
		memset(pVidImage, 0, nMemLen);
		pVidImage += nVidImagePitch;
		return 0;
	}
	else
	{
		pVidImage = NULL;
		return 1;
	}
}

// init game info, added by regret
void VidInitInfo()
{
	nGameWidth = nVidImageWidth;
	nGameHeight = nVidImageHeight;
	nRotateGame = 0;

	if (bDrvOkay)
	{
		// Get the game screen size
		BurnDrvGetVisibleSize(&nGameWidth, &nGameHeight);

		if (BurnDrvGetFlags() & BDF_ORIENTATION_VERTICAL)
		{
			if (nVidRotationAdjust & 1)
			{
				int n = nGameWidth;
				nGameWidth = nGameHeight;
				nGameHeight = n;
				nRotateGame |= (nVidRotationAdjust & 2);
			}
			else
				nRotateGame |= 1;
		}

		if (BurnDrvGetFlags() & BDF_ORIENTATION_FLIPPED)
			nRotateGame ^= 2;
	}
}

static void get_cg_params(void)
{
      cgGLBindProgram(FragmentProgram);
      cgGLBindProgram(VertexProgram);

      /* fragment params */
      cg_video_size = cgGetNamedParameter(FragmentProgram, "IN.video_size");
      cg_texture_size = cgGetNamedParameter(FragmentProgram, "IN.texture_size");
      cg_output_size = cgGetNamedParameter(FragmentProgram, "IN.output_size");
      cgp_timer = cgGetNamedParameter(FragmentProgram, "IN.frame_count");

      /* vertex params */
      cg_v_video_size = cgGetNamedParameter(VertexProgram, "IN.video_size");
      cg_v_texture_size = cgGetNamedParameter(VertexProgram, "IN.texture_size");
      cg_v_output_size = cgGetNamedParameter(VertexProgram, "IN.output_size");
      cgp_vertex_timer = cgGetNamedParameter(VertexProgram, "IN.frame_count");
      ModelViewProj_cgParam = cgGetNamedParameter(VertexProgram, "modelViewProj");

      cgGLSetStateMatrixParameter(ModelViewProj_cgParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
}

static CGprogram _psglLoadShaderFromSource(CGprofile target, const char* filename, const char *entry)
{
	const char* args[] = { "-fastmath", "-unroll=all", "-ifcvt=all", 0};
	CGprogram id = cgCreateProgramFromFile(CgContext, CG_SOURCE, filename, target, entry, args);

	return id;
}

static void psglDeinitShader()
{
	if (VertexProgram)
	{
		cgGLUnbindProgram(CG_PROFILE_SCE_VP_RSX);
		cgDestroyProgram(VertexProgram);
		VertexProgram = NULL;
	}

	if (FragmentProgram)
	{
		cgGLUnbindProgram(CG_PROFILE_SCE_FP_RSX);
		cgDestroyProgram(FragmentProgram);
		FragmentProgram = NULL;
	}


}

void _psglInitCG()
{
	char shaderFile[255];

	CgContext = cgCreateContext();
	cgRTCgcInit();

	strcpy(shaderFile, SHADER_DIRECTORY);
	strcat(shaderFile, m_ListShaderData[shaderindex].c_str());

	const char *shader = shaderFile;
	VertexProgram = _psglLoadShaderFromSource(CG_PROFILE_SCE_VP_RSX, shader, "main_vertex");
	FragmentProgram = _psglLoadShaderFromSource(CG_PROFILE_SCE_FP_RSX, shader, "main_fragment");

	cgGLEnableProfile(CG_PROFILE_SCE_VP_RSX);
	cgGLEnableProfile(CG_PROFILE_SCE_FP_RSX);

	get_cg_params();
}

void reset_frame_counter()
{
	frame_count = 0;
}

void apply_rotation_settings(void)
{
	if (nRotateGame & 1)		// do not rotate the graphics for vertical games
	{
		glVertexPointer(2, GL_FLOAT, 0, verts);
		if (nRotateGame & 2)	// reverse flipping for vertical games
		{
			glTexCoordPointer(2, GL_FLOAT, 0, tvertsFlippedRotated);
		}
		else
		{
			glTexCoordPointer(2, GL_FLOAT, 0, tvertsVertical);
		}
	}
	else				// rotate the graphics for vertical games
	{
		glVertexPointer(2, GL_FLOAT, 0, verts);
		if (nRotateGame & 2)	// reverse flipping for vertical games
		{
			glTexCoordPointer(2, GL_FLOAT, 0, tvertsFlipped);
		}
		else
		{
			glTexCoordPointer(2, GL_FLOAT, 0, tverts);
		}
	}
}

void psglInitGL(void)
{
	
	PSGLdeviceParameters params;
	params.enable = PSGL_DEVICE_PARAMETERS_COLOR_FORMAT | PSGL_DEVICE_PARAMETERS_DEPTH_FORMAT | PSGL_DEVICE_PARAMETERS_MULTISAMPLING_MODE;
	params.colorFormat = GL_ARGB_SCE;
	params.depthFormat = GL_NONE;
	params.multisamplingMode = GL_MULTISAMPLING_NONE_SCE;

	if (bVidTripleBuffer)
	{
		params.enable |= PSGL_DEVICE_PARAMETERS_BUFFERING_MODE;
		params.bufferingMode = PSGL_BUFFERING_MODE_TRIPLE;
	}

	PSGLinitOptions initOpts; 
	initOpts.enable = PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS;
#if(CELL_SDK_VERSION > 0x192001)
	initOpts.enable |= PSGL_INIT_TRANSIENT_MEMORY_SIZE;
#else
	initOpts.enable |= PSGL_INIT_HOST_MEMORY_SIZE;
#endif
	initOpts.maxSPUs = 1;
	initOpts.initializeSPUs = GL_FALSE;
	initOpts.persistentMemorySize = 0;
	initOpts.transientMemorySize = 0;
	initOpts.errorConsole = 0;
	initOpts.fifoSize = 0;
	initOpts.hostMemorySize = 0;

	psglInit(&initOpts);
	
	availableResolutionsCount = 0;
	for(int i = 0; i < 8; i++)
	{ 
		if (cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, allResolutions[i].resId,CELL_VIDEO_OUT_ASPECT_AUTO,0)) 
		{
			//set availableResolutions same as allResolutions entry since resolution //is available

			availableResolutions[availableResolutionsCount].w = allResolutions[i].w; 	
			availableResolutions[availableResolutionsCount].h = allResolutions[i].h; 	
			availableResolutions[availableResolutionsCount].resId = allResolutions[i].resId;

			if(bBurnFirstStartup)							// on initial boot, set the highest res possible
				currentAvailableResolutionNo = availableResolutionsCount; 	//set the current resolution to this one
			else									// on subsequent boots, obtain the resolution from the config file
			{
				if(allResolutions[i].resId == currentAvailableResolutionId)	// if we've found the resolution, set the index number for it
					currentAvailableResolutionNo = availableResolutionsCount;
			}

			//increment resolution count by one
			availableResolutionsCount += 1;

		}
	}

	currentAvailableResolutionId = availableResolutions[currentAvailableResolutionNo].resId;

	if(bBurnFirstStartup)
		bBurnFirstStartup = 0;

	if(availableResolutions[currentAvailableResolutionNo].resId == CELL_VIDEO_OUT_RESOLUTION_576)
	{
                params.enable |= PSGL_DEVICE_PARAMETERS_RESC_PAL_TEMPORAL_MODE;
                params.rescPalTemporalMode = RESC_PAL_TEMPORAL_MODE_60_INTERPOLATE;
                params.enable |= PSGL_DEVICE_PARAMETERS_RESC_RATIO_MODE;
                params.rescRatioMode = RESC_RATIO_MODE_FULLSCREEN;
	}
	else
	{
		params.width = availableResolutions[currentAvailableResolutionNo].w; 
		params.height = availableResolutions[currentAvailableResolutionNo].h; 
		params.enable |= PSGL_DEVICE_PARAMETERS_WIDTH_HEIGHT; 
	}
	
	psgl_device = psglCreateDeviceExtended(&params); 
	psgl_context = psglCreateContext();

	psglGetDeviceDimensions(psgl_device, &gl_width, &gl_height); 

	if(m_viewport_width == 0)
		m_viewport_width = gl_width;
	if(m_viewport_height == 0)
		m_viewport_height = gl_height;

	psglMakeCurrent(psgl_context, psgl_device);

	psglResetCurrentContext();

	psglGetRenderBufferDimensions(psgl_device, &gl_width, &gl_height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vidFilterLinear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vidFilterLinear ? GL_LINEAR : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
 
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	 
}

uint32_t psglGetCurrentResolutionId()
{
	return availableResolutions[currentAvailableResolutionNo].resId;
}

void psglResolutionPrevious()
{
	if(currentAvailableResolutionNo > 0)
		currentAvailableResolutionNo -= 1;	
}

void psglResolutionNext()
{
	if(currentAvailableResolutionNo < availableResolutionsCount-1)
		currentAvailableResolutionNo += 1;	
}

void dbgFontInit(void)
{
	// initialize debug font library 
	CellDbgFontConfig cfg;
	cfg.bufSize      = 512;
	cfg.screenWidth  = gl_width;
	cfg.screenHeight = gl_height;
	cellDbgFontInit(&cfg);
}

void psglResolutionSwitch(void)
{
	cellDbgFontExit();
	if (psgl_context)
	{
		psglDestroyContext(psgl_context);
		psgl_context = NULL;
	}

	if (psgl_device)
	{
		psglDestroyDevice(psgl_device);
		psgl_device = NULL;
	}

	currentAvailableResolutionId = availableResolutions[currentAvailableResolutionNo].resId;
	psglInitGL();
	dbgFontInit();
}

void psglExitGL(void)
{
	cellDbgFontExit();

	if (CgContext)
	{
		cgDestroyContext(CgContext);
		CgContext = NULL;
	}

	if (psgl_context)
	{
		psglDestroyContext(psgl_context);
		psgl_context = NULL;
	}

	if (psgl_device)
	{
		psglDestroyDevice(psgl_device);
		psgl_device = NULL;
	}

}

static void dbgFontPrintf(float x,float y, float scale,char* fmt,...)
{
	//build the output string
	char tempstr[512];

	va_list arglist;
	va_start(arglist, fmt);
	vsprintf(tempstr, fmt, arglist);
	va_end(arglist);

	cellDbgFontPuts((x/gl_width),(y/gl_height),scale, 0xffffffff, tempstr);
}

#define ShowFPS() \
   static uint64_t last_time = 0; \
   static uint64_t last_fps = 0; \
   static uint64_t frames = 0; \
   static float fps = 0.0; \
   frames++; \
   if (frames == 100) \
   { \
      uint64_t new_time = sys_time_get_system_time(); \
      uint64_t delta = new_time - last_time; \
      last_time = new_time; \
      frames = 0; \
      fps = 100000000.0 / delta; \
   } \
	dbgFontPrintf(40,40,0.75f,"%s %.5f FPS", "", fps );  \
   cellDbgFontDraw();

void psglSetVSync(uint32_t enable)
{
	if(enable)
		glEnable(GL_VSYNC_SCE);
	else
		glDisable(GL_VSYNC_SCE);
}

int _psglExit(void)
{
	glDeleteBuffers(1, &bufferID); 
	pVidImage = NULL;
	nRotateGame = 0;
	return 0;
}

static int _psglTextureInit()
{
	if (nRotateGame & 1)
	{
		nVidImageWidth = nGameHeight;
		nVidImageHeight = nGameWidth;
	}
	else
	{
		nVidImageWidth = nGameWidth;
		nVidImageHeight = nGameHeight;
	}

	nBurnBpp = SCREEN_RENDER_TEXTURE_BPP;		// Set Burn library Bytes per pixel

	// Use our callback to get colors:
	VidRecalcPal();

	switch(nBurnBpp)
	{
		case BPP_16_SCREEN_RENDER_TEXTURE_BPP:
			VidHighCol = HighCol16;
			break;
		case BPP_32_SCREEN_RENDER_TEXTURE_BPP:
			VidHighCol = HighCol24;
			break;
	}

	if (bDrvOkay && !(BurnDrvGetFlags() & BDF_16BIT_ONLY))
		BurnHighCol = VidHighCol;

	//End of callback

	// Make the normal memory buffer
	if (VidSAllocVidImage())
	{
		psglExit();
		return 1;
	}

	glBufferData(GL_TEXTURE_REFERENCE_BUFFER_SCE, (nVidImageWidth * nVidImageHeight) << SCREEN_RENDER_TEXTURE_BPP_SHIFT, NULL, GL_SYSTEM_DRAW_SCE);

	return 0;
}

void setlinear(unsigned int smooth)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}

int _psglInit(void)
{
	psglResetCurrentContext();

	psglGetRenderBufferDimensions(psgl_device, &gl_width, &gl_height);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vidFilterLinear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vidFilterLinear ? GL_LINEAR : GL_NEAREST);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	/*enable useful and required features */
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glGenBuffers(1, &bufferID);
	glBindBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, bufferID);

	_psglInitCG();

	VidInitInfo();

	// Initialize the buffer surfaces
	if (_psglTextureInit())
	{
		psglExit();
		return 1;
	}

	apply_rotation_settings();

	setlinear(vidFilterLinear);

	psglSetVSync(bVidVSync);

	nImageWidth = nImageHeight = 0;

	return 0;
}

static void glCalculateViewport(int outwidth, int outheight)
{
	float device_aspect = psglGetDeviceAspectRatio(psgl_device);
	GLuint temp_width = gl_width;
	GLuint temp_height = gl_height;
	float desired_aspect = m_ratio;
	float delta;
	if(custom_aspect_ratio_mode)
	{
		m_viewport_x_temp = m_viewport_x;
		m_viewport_y_temp = m_viewport_y;
		m_viewport_width_temp = m_viewport_width;
		m_viewport_height_temp = m_viewport_height;
	}
	else if ( (int)(device_aspect*1000) > (int)(m_ratio *1000) )
	{
		delta = (m_ratio / device_aspect - 1.0) / 2.0 + 0.5;
		m_viewport_x_temp = temp_width * (0.5 - delta);
		m_viewport_y_temp = 0;
		m_viewport_width_temp = (int)(2.0 * temp_width * delta);;
		m_viewport_height_temp = temp_height;
	}
	else if ( (int)(device_aspect*1000) < (int)(m_ratio * 1000) )
	{
		delta = (device_aspect / m_ratio - 1.0) / 2.0 + 0.5;
		m_viewport_x_temp = 0;
		m_viewport_y_temp = temp_height * (0.5 - delta);
		m_viewport_width_temp = temp_width;
		m_viewport_height_temp = (int)(2.0 * temp_height * delta);
	}
	else
	{
		m_viewport_x_temp = 0;
		m_viewport_y_temp = 0;
		m_viewport_width_temp = temp_width;
		m_viewport_height_temp = temp_height;
	}
	if(m_overscan)
	{
		m_left = -m_overscan_amount/2;
		m_right = 1 + m_overscan_amount/2;
		m_bottom = -m_overscan_amount/2;
		m_top = 1 + m_overscan_amount/2;
		m_zFar = -1;
		m_zNear = 1;
	}
	else
	{
		m_left = 0;
		m_right = 1;
		m_bottom = 0;
		m_top = 1;
		m_zNear = -1;
		m_zFar = 1;
	}
	cg_viewport_width = m_viewport_width_temp;
	cg_viewport_height = m_viewport_height_temp;
}

void glSetViewports()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(m_viewport_x_temp, m_viewport_y_temp, m_viewport_width_temp, m_viewport_height_temp);
	glOrthof(m_left, m_right, m_bottom, m_top, m_zNear, m_zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CalculateViewports(void)
{
	int32_t nrotategame_mask = ((nRotateGame) | -(nRotateGame)) >> 31;
	int nNewImageWidth  = ((gl_height) & nrotategame_mask) | ((gl_width) & ~nrotategame_mask);
	int nNewImageHeight = ((gl_width) & nrotategame_mask) | ((gl_height) & ~nrotategame_mask);

	if (nImageWidth != nNewImageWidth || nImageHeight != nNewImageHeight)
	{
		nImageWidth  = nNewImageWidth;
		nImageHeight = nNewImageHeight;
	}
	glCalculateViewport(nImageWidth, nImageHeight);
	glSetViewports();
	uint8_t * texture = (uint8_t*)glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_WRITE_ONLY);
	VidSCopyImage(texture);
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);
}

#define set_cg_params() \
cgGLSetStateMatrixParameter(ModelViewProj_cgParam, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY); \
cgGLSetParameter2f(cg_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter2f(cg_v_video_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_texture_size, nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_output_size, cg_viewport_width, cg_viewport_height); \
cgGLSetParameter1f(cgp_timer, frame_count); \
cgGLSetParameter1f(cgp_vertex_timer, frame_count);

void psglRender(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	uint8_t * texture = (uint8_t*)glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_WRITE_ONLY);
	VidSCopyImage(texture);
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);

	frame_count += 1;
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, nVidImageWidth, nVidImageHeight, 0, SCREEN_RENDER_TEXTURE_PIXEL_FORMAT, nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT, 0);
	set_cg_params();

	glDrawArrays(GL_QUADS, 0, 4);

	if (bShowFPS)
	{
		ShowFPS();
	}
}

#define ALPHA 0xA0

void psglRenderAlpha(void)
{
	frame_count++;

	uint32_t* texture = (uint32_t*)glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_READ_WRITE);
	for(int i = 0; i != nVidImageHeight; i++)
	{
		for(int j = 0; j != nVidImageWidth; j++)
		{
			unsigned char r = (texture[(i) * nVidImageWidth + (j)] >> 16);
			unsigned char g = (texture[(i) * nVidImageWidth + (j)] >> 8 );
			unsigned char b = (texture[(i) * nVidImageWidth + (j)] & 0xFF);
			r/=2;
			g/=2;
			b/=2;
			uint32_t pix = (r << 16) | (g << 8 )|  b | (ALPHA << 24);
			texture[i * nVidImageWidth + j] = pix;
		}
	}
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, nVidImageWidth, nVidImageHeight, 1, SCREEN_RENDER_TEXTURE_PIXEL_FORMAT, nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT, 0);
	set_cg_params();
	glDrawArrays(GL_QUADS, 0, 4);
}

int32_t psglInitShader(const char* filename)
{
	psglDeinitShader();

	CGprogram id = _psglLoadShaderFromSource(CG_PROFILE_SCE_FP_RSX, filename, "main_fragment");
	CGprogram v_id = _psglLoadShaderFromSource(CG_PROFILE_SCE_VP_RSX, filename, "main_vertex");
	if (id)
	{
		FragmentProgram = id;
		VertexProgram = v_id;

		get_cg_params();

		return CELL_OK;
	}
	return !CELL_OK;
}
