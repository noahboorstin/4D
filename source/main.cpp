/*
I realize the input and collision code is really messy right now. I just needed SOMETHING to work with, and I was more
focused on the rendering. I'm clearly going to improve that eventually
*/
#include "gpu.h"
#include "vshader_shbin.h"
#include "gshader_shbin.h"
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <3ds.h>

#define CONFIG_3D_SLIDERSTATE (*(float*)0x1FF81080)

#define CLEAR_COLOR 0x68B0D8FF

//#define vertex_list_count (sizeof(vertex_list)/sizeof(vertex_list[0]))
int vertex_count;
float *vertex_list;

static DVLB_s *vshader_dvlb, *gshader_dvlb;
static shaderProgram_s program;

bool proj1_changed;
static int uLoc_proj1, uLoc_view1, uLoc_view2, uLoc_proj1e, uLoc_view1e;
static matrix_5x5 proj1, view1;
static matrix_4x4 view2;
static proj_mtx44 proj2;

static void* vbo_data;

static void sceneInit(void)
{
	// Load the shaders and create a shader program
	// The geoshader stride is set to 6 so that it processes a triangle at a time
	vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
	gshader_dvlb = DVLB_ParseFile((u32*)gshader_shbin, gshader_shbin_size);
	shaderProgramInit(&program);
	shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
	shaderProgramSetGsh(&program, &gshader_dvlb->DVLE[0], 9);

	// Get the locations of the uniforms
	uLoc_proj1 = shaderInstanceGetUniformLocation(program.vertexShader, "proj1");
        uLoc_view1 = shaderInstanceGetUniformLocation(program.vertexShader, "view1");
        proj2.uLoc = shaderInstanceGetUniformLocation(program.vertexShader, "proj2");
        uLoc_view2 = shaderInstanceGetUniformLocation(program.vertexShader, "view2");
        uLoc_proj1e = shaderInstanceGetUniformLocation(program.vertexShader, "proj1e");
        uLoc_view1e = shaderInstanceGetUniformLocation(program.vertexShader, "view1e");
        
        m5x5_persp(&proj1, 0.10, 100.0);
        //m5x5_identity(&proj1);
	m4x4_persp_tilt(&proj2.mtx, 0.1f, 100.0f);
        m4x4_translate(&proj2.mtx, 0.0f, 0.0f, -2.0f); //not at all good to have this as part of this matrix, but wahtever
        m4x4_identity(&view2);
        m5x5_identity(&view1);
        //m4x4_ortho_tilt(&proj2.mtx, -1.0, 1.0, -1.0, 1.0, -10.0, 10.0);
	// Create the VBO (vertex buffer object)
	vbo_data = linearAlloc(sizeof(float)*8*vertex_count);
	memcpy(vbo_data, vertex_list, sizeof(float)*8*vertex_count);
}
static void sceneRender(void)
{
	// Bind the shader program
	shaderProgramUse(&program);

	// Configure the first fragment shading substage to just pass through the vertex color
	GPU_SetTexEnv(0,
		GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // RGB channels
		GPU_TEVSOURCES(GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR, GPU_PRIMARY_COLOR), // Alpha
		GPU_TEVOPERANDS(0, 0, 0), // RGB
		GPU_TEVOPERANDS(0, 0, 0), // Alpha
		GPU_REPLACE, GPU_REPLACE, // RGB, Alpha
		0xFFFFFFFF);

	// Configure the "attribute buffers" (that is, the vertex input buffers)
	u32 temp[]={0x0};
        u64 temp2[]={0x10};
        u8 temp3[]={2};
	GPU_SetAttributeBuffers(
		2, // Number of inputs per vertex
		(u32*)osConvertVirtToPhys((u32)vbo_data), // Location of the VBO
		GPU_ATTRIBFMT(0, 4, GPU_FLOAT) |
		GPU_ATTRIBFMT(1, 4, GPU_FLOAT), // Format of the inputs (in this case the only input is a 3-element float vector)
		0xFFC, // Unused attribute mask, in our case bit 0 is cleared since it is used
		0x10, // Attribute permutations (here it is the identity)
		1, // Number of buffers
		temp, // Buffer offsets (placeholders)
		temp2, // Attribute permutations for each buffer (identity again)
		temp3); // Number of attributes for each buffer

	// Upload the projection matrices
        if(true){
            vector_4f tempVec;
            matrix_4x4 tempMat;
            to4x4(&tempMat, &tempVec, &proj1, true);
            GPU_SetFloatUniform(GPU_VERTEX_SHADER, uLoc_proj1e, (u32*)tempVec.c, 1);
            GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_proj1, &tempMat);
            proj1_changed=false;
        }
        GPUCMD_GetBuffer(NULL, NULL, &(proj2.offset));
        if(true){GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, proj2.uLoc, &proj2.mtx);proj2.changed=false;}
        //upload view matrices (and vectors))
        vector_4f tempVec;
        matrix_4x4 tempMat;
        to4x4(&tempMat, &tempVec, &view1, false);
        GPU_SetFloatUniform(GPU_VERTEX_SHADER, uLoc_view1e, (u32*)tempVec.c, 1);
        GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_view1, &tempMat);
        GPU_SetFloatUniformMatrix(GPU_VERTEX_SHADER, uLoc_view2, &view2);
        
	// Draw the VBO - GPU_UNKPRIM allows the geoshader to control primitive emission
	GPU_DrawArray(GPU_UNKPRIM, vertex_count);
}

static void sceneExit(void)
{
	// Free the VBO
	linearFree(vbo_data);

	// Free the shader program
	shaderProgramFree(&program);
	DVLB_Free(vshader_dvlb);
	DVLB_Free(gshader_dvlb);
}
typedef struct {float x, y, z, w, theta, phi, psi;}coord;
typedef struct {float theta=0, phi=0;}coord2;
static coord first;
static coord2 second;

int num_boxes;
float* boxes;
int main()
{
	// Initialize graphics
	gfxInitDefault();
        consoleInit(GFX_BOTTOM, NULL);
        //loading vertex stuff
        FILE *file = fopen("/3ds/collision/stuff/untitled.4d","rb");
        if(file!=NULL) {
            printf("\x1b[20;0Hreading file\n");
            fread(&vertex_count, sizeof(vertex_count), 1, file);
            printf("\x1b[21;0Hcount: %i",vertex_count);
            vertex_list=(float*)malloc(vertex_count*8*sizeof(float));
            if(vertex_list) {
                fread(vertex_list,1,vertex_count*8*sizeof(float),file);
            }
            fread(&num_boxes, sizeof(num_boxes), 1, file);
            boxes=(float*)malloc(num_boxes*8*sizeof(float));
            if(boxes) {
                fread(boxes,1,num_boxes*8*sizeof(float),file);
            }
            printf("\x1b[22;0H%f",boxes[0]);
            printf("\n%f",boxes[1]);
            printf("\n%f",boxes[2]);
            printf("\n%f",boxes[3]);
            fclose(file);
        }
        else
            printf("\x1b[20;0Hno file :(\n");
        
        
	printf("\x1b[0;0Hx:\ny:\nz:\nw:\ntheta:\nphi:\npsi:\ntheta':\nphi':\npsi':");
        printf("\nI know the depth isn't quite right.\nI can't fix it. :(");
	gpuInit();
        printf("\x1b[14;0Htarget: <%03f", 1.0f/60.0f);
        printf("\x1b[7;7H%03f", 0.0f);
        printf("\x1b[8;5H%03f", 0.0f);
        printf("\x1b[9;5H%03f", 0.0f);
        gfxSet3D(true);
	// Initialize the scene
	sceneInit();
	gpuClearBuffers(CLEAR_COLOR);
        u64 start=svcGetSystemTick();
	// Main loop
	while (aptMainLoop())
	{
                u64 finish=svcGetSystemTick();
		gspWaitForVBlank();  // Synchronize with the start of VBlank
		gfxSwapBuffersGpu(); // Swap the framebuffers so that the frame that we rendered last frame is now visible
		hidScanInput();      // Read the user input
		printf("\x1b[15;0H%03f", ((float)(finish-start))/268123468.8f);
                start=svcGetSystemTick();
                
		// Respond to user input
		u32 kDown = hidKeysHeld();
		if (kDown & KEY_START)
			break; // break in order to return to hbmenu
                float slider=CONFIG_3D_SLIDERSTATE;
                slider*=0.12f;
                //input: change this later for analog stuff
                {
                    coord first_copy(first);
                    float amount=1.0f;
                    
                    if(kDown & KEY_SELECT) {
                        m4x4_identity(&view2);
                        printf("\x1b[7;7H%03f", 0.0f);
                        printf("\x1b[8;5H%03f", 0.0f);
                        printf("\x1b[9;5H%03f", 0.0f);
                        first.x=first.y=first.z=first.w=8.0f;
                        first.theta=first.phi=first.psi=0.0f;
                    }
                    if(kDown & KEY_X)
                        amount=0.1f;
                    if(kDown & KEY_DUP)
                        second.phi-=amount;
                    if(kDown & KEY_DDOWN)
                        second.phi+=amount;
                    if(kDown & KEY_DRIGHT)
                        second.theta-=amount;
                    if(kDown & KEY_DLEFT)
                        second.theta+=amount;
                    
                    float amount2=0.1f;
                    float amount3=1.0f;
                    if(kDown & KEY_X) {
                        amount2=0.01f;
                        amount3=0.1f;
                    }
                                        
                    if(kDown & KEY_CSTICK_UP)
                        first.psi+=amount3;
                    if(kDown & KEY_CSTICK_DOWN)
                        first.psi-=amount3;
                    if(kDown & KEY_CSTICK_RIGHT)
                        first.theta+=amount3;
                    if(kDown & KEY_CSTICK_LEFT)
                        first.theta-=amount3;
                    if(kDown & KEY_ZR)
                        first.phi+=amount3;
                    if(kDown & KEY_ZL)
                        first.phi-=amount3;
                    
                    /*if(kDown & KEY_CPAD_RIGHT)
                        first.x+=amount2;
                    if(kDown & KEY_CPAD_LEFT)
                        first.x-=amount2;
                    if(kDown & KEY_CPAD_DOWN)
                        first.w+=amount2;
                    if(kDown & KEY_CPAD_UP)
                        first.w-=amount2;
                    if(kDown & KEY_L)
                        first.z+=amount2;
                    if(kDown & KEY_R)
                        first.z-=amount2;
                    if(kDown & KEY_B)
                        first.y+=amount2;
                    if(kDown & KEY_A)
                        first.y-=amount2;*/
                    {
                        float x,z,w;
                        x=z=w=0;
                        if(kDown & KEY_CPAD_RIGHT)
                            x+=amount2;
                        if(kDown & KEY_CPAD_LEFT)
                            x-=amount2;
                        if(kDown & KEY_CPAD_DOWN)
                            w+=amount2;
                        if(kDown & KEY_CPAD_UP)
                            w-=amount2;
                        if(kDown & KEY_L)
                            z+=amount2;
                        if(kDown & KEY_R)
                            z-=amount2;
                        if(kDown & KEY_B)
                            first.y+=amount2;
                        if(kDown & KEY_A)
                            first.y-=amount2;
                        float rtheta=degToRad(first.theta);
                        float rphi=degToRad(first.phi);
                        float rpsi=degToRad(first.psi);
                        first.x+=x*cosf(rphi)*cosf(rpsi)*cosf(rtheta)+z*cosf(rphi)*cosf(rpsi)*sinf(rtheta)+w*cosf(rphi)*sinf(rpsi);
                        first.z+=z*cosf(rphi)*cosf(rpsi)*cosf(rtheta)+w*cosf(rphi)*cosf(rpsi)*sinf(rtheta)+x*cosf(rphi)*sinf(rpsi);
                        first.w+=w*cosf(rphi)*cosf(rpsi)*cosf(rtheta)+x*cosf(rphi)*cosf(rpsi)*sinf(rtheta)+z*cosf(rphi)*sinf(rpsi);
                    }
                    
                    
                    //clamping stuff
                    if(second.phi>85.0f) second.phi=85.0f;
                    if(second.phi<-85.0f) second.phi=-85.0f;
                    while(second.theta<0.0f) second.theta+=360.0f;
                    while(second.theta>360.0f) second.theta-=360.0f;
                    
                    if(first.phi>85.0f) first.phi=85.0f;
                    if(first.phi<-85.0f) first.phi=-85.0f;
                    if(first.psi>85.0f) first.psi=85.0f;
                    if(first.psi<-85.0f) first.psi=-85.0f;
                    while(first.theta<0.0f) first.theta+=360.0f;
                    while(first.theta>360.0f) first.theta-=360.0f;
                    
                    printf("\x1b[26;0H%i",9);
                    float temp[]={first.x, first.y, first.z, first.w};
                    
                    for(int a=0; a<num_boxes; a++) {
                        bool outside=false;
                        for(int b=0; b<4; b++) {
                            if((temp[b] < boxes[a*8+b]) || (temp[b] > boxes[a*8+b+4])) {
                                outside=true;
                                break;
                            }
                        }
                        if(!outside) {
                            printf("\x1b[26;0H%i",a);
                            first=first_copy;
                            break;
                        }
                    }
                    
                    //display coord stuff:
                    printf("\x1b[0;2H%03f", first.x);
                    printf("\x1b[1;2H%03f", first.y);
                    printf("\x1b[2;2H%03f", first.z);
                    printf("\x1b[3;2H%03f", first.w);
                    printf("\x1b[4;6H%03f", first.theta);
                    printf("\x1b[5;4H%03f", first.phi);
                    printf("\x1b[6;4H%03f", first.psi);
                }
                //set up view matrices
                
                //do rotation stuff
                {
                    m5x5_identity(&view1);
                    float a[4], b[]={first.x, first.y, first.z, first.w}, c[]={0.0f, 1.0f, 0.0f, 0.0f}, d[]={0.0f, 0.0f, 0.0f, 1.0f};
                    float rtheta=degToRad(first.theta);
                    float rphi=degToRad(first.phi);
                    float rpsi=degToRad(first.psi);
                    a[0]=cosf(rphi)*cosf(rpsi)*sinf(rtheta)+b[0];
                    a[1]=sinf(rphi)+b[1];
                    a[2]=cosf(rphi)*cosf(rpsi)*cosf(rtheta)+b[2];
                    a[3]=cosf(rphi)*sinf(rpsi)+b[3]; //i hope this is right...
                    m5x5_orient(&view1, a, b, c, d);                
                }
                
                //m4x4_identity(&view2);
                if(second.theta!=0.0f||second.phi!=0.0f)
                { //so: I should REALLY orthonormalize this eventually
                    float a[3], b[]={0.0f, 0.0f, 0.0f}, c[]={0.0f, 1.0f, 0.0f};
                    float rtheta=degToRad(second.theta);
                    float rphi=degToRad(second.phi);
                    a[0]=cosf(rphi)*sinf(rtheta);
                    a[1]=sinf(rphi);
                    a[2]=cosf(rphi)*cosf(rtheta);
                    m4x4_translate(&view2, 0, -first.y, first.z, false);
                    m4x4_orient(&view2, a, b, c);
                    m4x4_translate(&view2, 0, first.y, -first.z, false);
                    printf("\x1b[7;7H%03f", acosf(view2.r[0].x)*180.0f/M_PI);
                    printf("\x1b[8;5H%03f", acosf(view2.r[1].y)*180.0f/M_PI);
                    printf("\x1b[9;5H%03f", acosf(view2.r[2].z)*180.0f/M_PI);
                    //m4x4_rotate_y(&view2, degToRad(second.theta), false);
                    //m4x4_rotate_x(&view2, degToRad(second.phi), false);
                    second.theta=second.phi=0.0f;
                }
                
                
		// Render the scene
		gpuFrameBegin();
		sceneRender();
		gpuFrameEnd(slider, &proj2, CLEAR_COLOR);
		gpuClearBuffers(CLEAR_COLOR);

		// Flush the framebuffers out of the data cache (not necessary with pure GPU rendering)
		//gfxFlushBuffers();
	}

	// Deinitialize the scene
	sceneExit();

	// Deinitialize graphics
	gpuExit();
        free(vertex_list);
        free(boxes);
	gfxExit();
	return 0;
}
