#include <stdint.h>
#include <cell/codec.h>
#include <sys/sys_time.h>

#include "burner.h"
#include "vid_support-ps3.h"
#include "vid_psgl.h"

#define MAX_SHADERS 3
#define SHADER_ONE 0
#define SHADER_TWO 1
#define MENU_SHADER 2

extern std::vector<std::string> m_ListShaderData;
extern std::vector<std::string> m_ListShader2Data;

static PSGLdevice* psgl_device = NULL;
static PSGLcontext* psgl_context = NULL;

uint8_t *decode_buffer;

GLuint gl_width = 0;
GLuint gl_height = 0;
int nImageWidth;
int nImageHeight;
static int nGameWidth = 0;
static int nGameHeight = 0;
static int nRotateGame = 0;

// textures
static GLuint tex = 0;
static GLuint tex_menu = 0;
static GLuint tex_backdrop = 0;

static CGcontext CgContext = NULL;
static CGprogram VertexProgram[MAX_SHADERS];
static CGprogram FragmentProgram[MAX_SHADERS];
static CGparameter ModelViewProj_cgParam[MAX_SHADERS];
static CGparameter cg_video_size[MAX_SHADERS];
static CGparameter cg_texture_size[MAX_SHADERS];
static CGparameter cg_output_size[MAX_SHADERS];
static CGparameter cg_v_video_size[MAX_SHADERS];
static CGparameter cg_v_texture_size[MAX_SHADERS];
static CGparameter cg_v_output_size[MAX_SHADERS];
static CGparameter cgp_timer[MAX_SHADERS];
static CGparameter cgp_vertex_timer[MAX_SHADERS];
static GLuint cg_viewport_width;
static GLuint cg_viewport_height;
static GLuint vbo[2];
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
unsigned int __cdecl HighCol15(int r, int g, int b, int);
unsigned int __cdecl HighCol16(int r, int g, int b, int);
//unsigned int __cdecl HighCol24(int r, int g, int b, int);

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

static void get_cg_params(unsigned index)
{
      /* fragment params */
      cg_video_size[index] = cgGetNamedParameter(FragmentProgram[index], "IN.video_size");
      cg_texture_size[index] = cgGetNamedParameter(FragmentProgram[index], "IN.texture_size");
      cg_output_size[index] = cgGetNamedParameter(FragmentProgram[index], "IN.output_size");
      cgp_timer[index] = cgGetNamedParameter(FragmentProgram[index], "IN.frame_count");

      /* vertex params */
      cg_v_video_size[index] = cgGetNamedParameter(VertexProgram[index], "IN.video_size");
      cg_v_texture_size[index] = cgGetNamedParameter(VertexProgram[index], "IN.texture_size");
      cg_v_output_size[index] = cgGetNamedParameter(VertexProgram[index], "IN.output_size");
      cgp_vertex_timer[index] = cgGetNamedParameter(VertexProgram[index], "IN.frame_count");
      ModelViewProj_cgParam[index] = cgGetNamedParameter(VertexProgram[index], "modelViewProj");

      cgGLSetStateMatrixParameter(ModelViewProj_cgParam[index], CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
}

static CGprogram _psglLoadShaderFromSource(CGprofile target, const char* filename, const char *entry)
{
	const char* args[] = { "-fastmath", "-unroll=all", "-ifcvt=all", 0};
	CGprogram id = cgCreateProgramFromFile(CgContext, CG_SOURCE, filename, target, entry, args);

	return id;
}

static void psglDeinitShader(unsigned index)
{
	if (VertexProgram[index])
	{
		cgGLUnbindProgram(CG_PROFILE_SCE_VP_RSX);
		cgDestroyProgram(VertexProgram[index]);
		VertexProgram[index] = NULL;
	}

	if (FragmentProgram[index])
	{
		cgGLUnbindProgram(CG_PROFILE_SCE_FP_RSX);
		cgDestroyProgram(FragmentProgram[index]);
		FragmentProgram[index] = NULL;
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
	psglInitShader(shaderFile, SHADER_ONE);
	//psglInitShader(DEFAULT_MENU_SHADER_FILE, MENU_SHADER);
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
	//free(decode_buffer);

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
	glDeleteBuffers(2, vbo); 
	pVidImage = NULL;
	nRotateGame = 0;
	return 0;
}

void setlinear(unsigned int smooth)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, smooth ? GL_LINEAR : GL_NEAREST);
}

int _psglInit(void)
{
	decode_buffer = (uint8_t*)memalign(128, 2048 * 2048 * 4);
	memset(decode_buffer, 0, (2048 * 2048 * 4));

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

	glGenBuffers(2, vbo);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glBindBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, vbo[0]);

	_psglInitCG();

	VidInitInfo();

	// Initialize the buffer surfaces
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

	VidHighCol = HighCol15;
	BurnHighCol = VidHighCol;

	//End of callback

	// Make the normal memory buffer
	if (VidSAllocVidImage())
	{
		psglExit();
		return 1;
	}

	glBufferData(GL_TEXTURE_REFERENCE_BUFFER_SCE, (nVidImageWidth * nVidImageHeight) << SCREEN_RENDER_TEXTURE_BPP_SHIFT, NULL, GL_SYSTEM_DRAW_SCE);

	apply_rotation_settings();

	setlinear(vidFilterLinear);

	psglSetVSync(bVidVSync);

	// VBO #2
	// Use some initial values for the screen quad.
	GLfloat vertexes[] = {
		0, 0, 0,
		0, 1, 0,
		1, 1, 0,
		1, 0, 0,
		0, 1,
		0, 0,
		1, 0,
		1, 1
	};

	GLfloat vertex_buf[128];
	memcpy(vertex_buf, vertexes, 12 * sizeof(GLfloat));
	memcpy(vertex_buf + 32, vertexes + 12, 8 * sizeof(GLfloat));
	memcpy(vertex_buf + 32 * 3, vertexes + 12, 8 * sizeof(GLfloat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 512, vertex_buf, GL_STREAM_DRAW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)128);

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

#define set_cg_params(index) \
cgGLSetStateMatrixParameter(ModelViewProj_cgParam[index], CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY); \
cgGLSetParameter2f(cg_video_size[index], nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_texture_size[index], nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_output_size[index], cg_viewport_width, cg_viewport_height); \
cgGLSetParameter2f(cg_v_video_size[index], nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_texture_size[index], nVidImageWidth, nVidImageHeight); \
cgGLSetParameter2f(cg_v_output_size[index], cg_viewport_width, cg_viewport_height); \
cgGLSetParameter1f(cgp_timer[index], frame_count); \
cgGLSetParameter1f(cgp_vertex_timer[index], frame_count);

#define texture_backdrop(number) \
	if (tex_backdrop) \
	{ \
		/* Set up texture coord array (TEXCOORD1). */ \
		glClientActiveTexture(GL_TEXTURE1); \
		glEnableClientState(GL_TEXTURE_COORD_ARRAY); \
		glTexCoordPointer(2, GL_FLOAT, 0, (void*)(128 * 3)); \
		glClientActiveTexture(GL_TEXTURE0); \
		\
		CGparameter param = cgGetNamedParameter(FragmentProgram[number], "bg"); \
		cgGLSetTextureParameter(param, tex_backdrop); \
		cgGLEnableTextureParameter(param); \
	}

void psglRender(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, tex);

	uint8_t * texture = (uint8_t*)glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_WRITE_ONLY);
	VidSCopyImage(texture);
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);

	frame_count += 1;
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, nVidImageWidth, nVidImageHeight, 0, SCREEN_RENDER_TEXTURE_PIXEL_FORMAT, nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT, 0);
	set_cg_params(SHADER_ONE);

	texture_backdrop(0);

	glDrawArrays(GL_QUADS, 0, 4);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (bShowFPS)
	{
		ShowFPS();
	}
}

#define ALPHA 0xA0

void psglRenderAlpha(void)
{
	frame_count++;

	glBindTexture(GL_TEXTURE_2D, tex);
	uint16_t* texture = (uint16_t*)glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_READ_WRITE);
	for(int i = 0; i != nVidImageHeight; i++)
	{
		for(int j = 0; j != nVidImageWidth; j++)
		{
			unsigned char r = (texture[(i) * nVidImageWidth + (j)] >> 10) & 0x1f;
			unsigned char g = (texture[(i) * nVidImageWidth + (j)] >> 5) & 0x1f;
			unsigned char b = (texture[(i) * nVidImageWidth + (j)]) & 0x1f;
			r/=2;
			g/=2;
			b/=2;
			uint16_t pix = (r << 10) | (g << 5 ) | b | 0x8000;
			texture[i * nVidImageWidth + j] = pix;
		}
	}
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, nVidImageWidth, nVidImageHeight, 1, SCREEN_RENDER_TEXTURE_PIXEL_FORMAT, nVidImageWidth << SCREEN_RENDER_TEXTURE_BPP_SHIFT, 0);
	set_cg_params(SHADER_ONE);
	glDrawArrays(GL_QUADS, 0, 4);
	glBindTexture(GL_TEXTURE_2D, tex);
}

int32_t psglInitShader(const char* filename, unsigned index)
{
	psglDeinitShader(index);

	CGprogram id = _psglLoadShaderFromSource(CG_PROFILE_SCE_FP_RSX, filename, "main_fragment");
	CGprogram v_id = _psglLoadShaderFromSource(CG_PROFILE_SCE_VP_RSX, filename, "main_vertex");

	if (id)
	{
		FragmentProgram[index] = id;
		VertexProgram[index] = v_id;
		// bind and enable the vertex and fragment programs
		cgGLEnableProfile(CG_PROFILE_SCE_VP_RSX);
		cgGLEnableProfile(CG_PROFILE_SCE_FP_RSX);
		cgGLBindProgram(VertexProgram[index]);
		cgGLBindProgram(FragmentProgram[index]);

		get_cg_params(index);

		if(index == 1 && VertexProgram[0] && FragmentProgram[0])
		{
			cgGLBindProgram(VertexProgram[0]);
			cgGLBindProgram(FragmentProgram[0]);
		}

		return CELL_OK;
	}
	return !CELL_OK;
}

/******************************************************************************* 
	Image decompression
********************************************************************************/

/******************************************************************************* 
	Image decompression - structs
********************************************************************************/

typedef struct CtrlMallocArg
{
	uint32_t mallocCallCounts;
} CtrlMallocArg;

typedef struct CtrlFreeArg
{
	uint32_t freeCallCounts;
} CtrlFreeArg;

void *img_malloc(uint32_t size, void * a)
{
	CtrlMallocArg *arg;
	arg = (CtrlMallocArg *) a;
	arg->mallocCallCounts++;
	return malloc(size);
}

static int img_free(void *ptr, void * a)
{
	CtrlFreeArg *arg;
	arg = (CtrlFreeArg *) a;
	arg->freeCallCounts++;
	free(ptr);
	return 0;
}

/******************************************************************************* 
	Image decompression - libJPEG
********************************************************************************/

bool load_jpeg(const char * path, unsigned &width, unsigned &height, uint8_t *data)
{
	// More Holy shit
	CtrlMallocArg              MallocArg;
	CtrlFreeArg                FreeArg;
	CellJpgDecMainHandle       mHandle = NULL;
	CellJpgDecSubHandle        sHandle = NULL;
	CellJpgDecThreadInParam    InParam;
	CellJpgDecThreadOutParam   OutParam;
	CellJpgDecSrc              src;
	CellJpgDecOpnInfo          opnInfo;
	CellJpgDecInfo             info;
	CellJpgDecInParam          inParam;
	CellJpgDecOutParam         outParam;
	CellJpgDecDataOutInfo      dOutInfo;
	CellJpgDecDataCtrlParam    dCtrlParam;

	MallocArg.mallocCallCounts = 0;
	FreeArg.freeCallCounts = 0;
	InParam.spuThreadEnable = CELL_JPGDEC_SPU_THREAD_ENABLE;
	InParam.ppuThreadPriority = 1001;
	InParam.spuThreadPriority = 250;
	InParam.cbCtrlMallocFunc = img_malloc;
	InParam.cbCtrlMallocArg = &MallocArg;
	InParam.cbCtrlFreeFunc = img_free;
	InParam.cbCtrlFreeArg = &FreeArg;

	int ret_jpeg, ret = -1;
	ret_jpeg = cellJpgDecCreate(&mHandle, &InParam, &OutParam);

	if (ret_jpeg != CELL_OK)
	{
		goto error;
	}

	memset(&src, 0, sizeof(CellJpgDecSrc));
	src.srcSelect        = CELL_JPGDEC_FILE;
	src.fileName         = path;
	src.fileOffset       = 0;
	src.fileSize         = 0;
	src.streamPtr        = NULL;
	src.streamSize       = 0;

	src.spuThreadEnable  = CELL_JPGDEC_SPU_THREAD_ENABLE;

	ret = cellJpgDecOpen(mHandle, &sHandle, &src, &opnInfo);

	if (ret != CELL_OK)
	{
		goto error;
	}

	ret = cellJpgDecReadHeader(mHandle, sHandle, &info);

	if (ret != CELL_OK)
	{
		goto error;
	}

	inParam.commandPtr         = NULL;
	inParam.method             = CELL_JPGDEC_FAST;
	inParam.outputMode         = CELL_JPGDEC_TOP_TO_BOTTOM;
	inParam.outputColorSpace   = CELL_JPG_ARGB;
	inParam.downScale          = 1;
	inParam.outputColorAlpha = 0xfe;
	ret = cellJpgDecSetParameter(mHandle, sHandle, &inParam, &outParam);

	if (ret != CELL_OK)
	{
		sys_process_exit(0);
		goto error;
	}

	dCtrlParam.outputBytesPerLine = outParam.outputWidth * 4;
	ret = cellJpgDecDecodeData(mHandle, sHandle, data, &dCtrlParam, &dOutInfo);

	if (ret != CELL_OK || dOutInfo.status != CELL_JPGDEC_DEC_STATUS_FINISH)
	{
		sys_process_exit(0);
		goto error;
	}

	width = outParam.outputWidth;
	height = outParam.outputHeight;

	cellJpgDecClose(mHandle, sHandle);
	cellJpgDecDestroy(mHandle);

	return true;

error:
	if (mHandle && sHandle)
		cellJpgDecClose(mHandle, sHandle);
	if (mHandle)
		cellJpgDecDestroy(mHandle);
	return false;
}

/******************************************************************************* 
	Image decompression - libPNG
********************************************************************************/

bool load_png(const char * path, unsigned &width, unsigned &height, uint8_t *data)
{
	// Holy shit, Sony!
	CtrlMallocArg              MallocArg;
	CtrlFreeArg                FreeArg;
	CellPngDecMainHandle       mHandle = NULL;
	CellPngDecSubHandle        sHandle = NULL;
	CellPngDecThreadInParam    InParam;
	CellPngDecThreadOutParam   OutParam;
	CellPngDecSrc              src;
	CellPngDecOpnInfo          opnInfo;
	CellPngDecInfo             info;
	CellPngDecInParam          inParam;
	CellPngDecOutParam         outParam;
	CellPngDecDataOutInfo      dOutInfo;
	CellPngDecDataCtrlParam    dCtrlParam;

	MallocArg.mallocCallCounts = 0;
	FreeArg.freeCallCounts = 0;
	InParam.spuThreadEnable = CELL_PNGDEC_SPU_THREAD_ENABLE;
	InParam.ppuThreadPriority = 512;
	InParam.spuThreadPriority = 200;
	InParam.cbCtrlMallocFunc = img_malloc;
	InParam.cbCtrlMallocArg = &MallocArg;
	InParam.cbCtrlFreeFunc = img_free;
	InParam.cbCtrlFreeArg = &FreeArg;

	int ret_png, ret = -1;
	ret_png = cellPngDecCreate(&mHandle, &InParam, &OutParam);

	if (ret_png != CELL_OK)
		goto error;

	memset(&src, 0, sizeof(CellPngDecSrc));
	src.srcSelect        = CELL_PNGDEC_FILE;
	src.fileName         = path;
	src.fileOffset       = 0;
	src.fileSize         = 0;
	src.streamPtr        = 0;
	src.streamSize       = 0;

	src.spuThreadEnable  = CELL_PNGDEC_SPU_THREAD_ENABLE;

	ret = cellPngDecOpen(mHandle, &sHandle, &src, &opnInfo);

	if (ret != CELL_OK)
		goto error;

	ret = cellPngDecReadHeader(mHandle, sHandle, &info);

	if (ret != CELL_OK)
		goto error;

	inParam.commandPtr         = NULL;
	inParam.outputMode         = CELL_PNGDEC_TOP_TO_BOTTOM;
	inParam.outputColorSpace   = CELL_PNGDEC_ARGB;
	inParam.outputBitDepth     = 8;
	inParam.outputPackFlag     = CELL_PNGDEC_1BYTE_PER_1PIXEL;
	inParam.outputAlphaSelect  = CELL_PNGDEC_STREAM_ALPHA;
	ret = cellPngDecSetParameter(mHandle, sHandle, &inParam, &outParam);

	if (ret != CELL_OK)
		goto error;

	dCtrlParam.outputBytesPerLine = outParam.outputWidth * 4;
	ret = cellPngDecDecodeData(mHandle, sHandle, data, &dCtrlParam, &dOutInfo);

	if (ret != CELL_OK || dOutInfo.status != CELL_PNGDEC_DEC_STATUS_FINISH)
		goto error;

	width = outParam.outputWidth;
	height = outParam.outputHeight;

	cellPngDecClose(mHandle, sHandle);
	cellPngDecDestroy(mHandle);

	return true;

error:
	if (mHandle && sHandle)
		cellPngDecClose(mHandle, sHandle);
	if (mHandle)
		cellPngDecDestroy(mHandle);
	return false;
}

void setup_texture(GLuint tex, unsigned width, unsigned height)
{
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_ARGB_SCE, width, height, 0,
			GL_ARGB_SCE, GL_UNSIGNED_INT_8_8_8_8, decode_buffer);

	// Set up texture coord array (TEXCOORD1).
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)(128 * 3));
	glClientActiveTexture(GL_TEXTURE0);

	// Go back to old stuff.
	glBindTexture(GL_TEXTURE_2D, tex);
}

bool LoadMenuTexture(enum menu_type type, const char * path)
{
	GLuint *texture = NULL;
	switch (type)
	{
		case TEXTURE_BACKDROP:
			texture = &tex_backdrop;
			break;

		case TEXTURE_MENU:
			texture = &tex_menu;
			break;

		default:
			return false;
	}

	unsigned width, height;
	if(strstr(path, ".PNG") != NULL || strstr(path, ".png") != NULL)
	{
		if (!load_png(path, width, height, decode_buffer))
			return false;
	}
	else
	{
		if (!load_jpeg(path, width, height, decode_buffer))
			return false;
	}

	if (*texture == 0)
		glGenTextures(1, texture);

	setup_texture(*texture, width, height);

	return true;
}

void psglRenderMenu(int width, int height)
{
	#if 0
	float device_aspect = psglGetDeviceAspectRatio(psgl_device);
	GLuint temp_width = gl_width;
	GLuint temp_height = gl_height;
	cg_viewport_width = temp_width;
	cg_viewport_height = temp_width;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, temp_width, temp_height);
	glOrthof(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	printf("0\n");

	cgGLBindProgram(VertexProgram[MENU_SHADER]);
	cgGLBindProgram(FragmentProgram[MENU_SHADER]);
	cgGLSetStateMatrixParameter(ModelViewProj_cgParam[MENU_SHADER], CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgGLSetParameter2f(cg_video_size[MENU_SHADER], width, height);
	cgGLSetParameter2f(cg_texture_size[MENU_SHADER], width, height);
	cgGLSetParameter2f(cg_output_size[MENU_SHADER], cg_viewport_width, cg_viewport_height);
	cgGLSetParameter2f(cg_v_video_size[MENU_SHADER], width, height);
	cgGLSetParameter2f(cg_v_texture_size[MENU_SHADER], width, height);
	cgGLSetParameter2f(cg_v_output_size[MENU_SHADER], cg_viewport_width, cg_viewport_height);
	printf("1\n");

	cgp_timer[MENU_SHADER] = cgGetNamedParameter(FragmentProgram[MENU_SHADER], "IN.frame_count");
	cgp_vertex_timer[MENU_SHADER] = cgGetNamedParameter(VertexProgram[MENU_SHADER], "IN.frame_count");
	cgGLSetParameter1f(cgp_timer[MENU_SHADER], frame_count);
	cgGLSetParameter1f(cgp_vertex_timer[MENU_SHADER], frame_count);
	CGparameter param = cgGetNamedParameter(FragmentProgram[MENU_SHADER], "bg");
	cgGLSetTextureParameter(param, tex_menu);
	cgGLEnableTextureParameter(param);
	printf("2\n");

	printf("3\n");
	// Set up texture coord array (TEXCOORD1).
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	printf("3.1\n");
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)(128 * 3));
	glClientActiveTexture(GL_TEXTURE0);
	printf("3.2\n");

	glDrawArrays(GL_QUADS, 0, 4); 
	printf("3.3\n");

	printf("4\n");
	// EnableTextureParameter might overwrite bind in TEXUNIT0.
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	cgGLBindProgram(VertexProgram[0]);
	cgGLBindProgram(FragmentProgram[0]);
	#endif
}
