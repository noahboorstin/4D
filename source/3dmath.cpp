#include "3dmath.h"
#include <stdio.h>

void m5x5_identity(matrix_5x5* out)
{
	m5x5_zeros(out);
	out->r[0].x = out->r[1].y = out->r[2].z = out->r[3].w = out->r[4].v = 1.0f;
}

void m5x5_multiply(matrix_5x5* out, const matrix_5x5* a, const matrix_5x5* b)
{
	int i, j;
	for (i = 0; i < 5; i ++)
		for (j = 0; j < 5; j ++)
			out->r[j].c[i] = a->r[j].x*b->r[0].c[i] + a->r[j].y*b->r[1].c[i] + a->r[j].z*b->r[2].c[i] + a->r[j].w*b->r[3].c[i] + a->r[j].v*b->r[4].c[i];
}

void m5x5_translate(matrix_5x5* mtx, float x, float y, float z, float w)
{
	matrix_5x5 tm, om;

	m5x5_identity(&tm);
	tm.r[0].v = x;
	tm.r[1].v = y;
	tm.r[2].v = z;
        tm.r[3].v = w;
	m5x5_multiply(&om, mtx, &tm);
	m5x5_copy(mtx, &om);
}

void m5x5_scale(matrix_5x5* mtx, float x, float y, float z, float w)
{
	int i;
	for (i = 0; i < 5; i ++)
	{
		mtx->r[i].x *= x;
		mtx->r[i].y *= y;
		mtx->r[i].z *= z;
                mtx->r[i].w *= w;
	}
}


void m4x4_identity(matrix_4x4* out)
{
	m4x4_zeros(out);
	out->r[0].x = out->r[1].y = out->r[2].z = out->r[3].w = 1.0f;
}

void m4x4_multiply(matrix_4x4* out, const matrix_4x4* a, const matrix_4x4* b)
{
	int i, j;
	for (i = 0; i < 4; i ++)
		for (j = 0; j < 4; j ++)
			out->r[j].c[i] = a->r[j].x*b->r[0].c[i] + a->r[j].y*b->r[1].c[i] + a->r[j].z*b->r[2].c[i] + a->r[j].w*b->r[3].c[i];
}

void m4x4_translate(matrix_4x4* mtx, float x, float y, float z, bool left=true)
{
	matrix_4x4 tm, om;

	m4x4_identity(&tm);
	tm.r[0].w = x;
	tm.r[1].w = y;
	tm.r[2].w = z;
        if(left)
            m4x4_multiply(&om, mtx, &tm);
        else
            m4x4_multiply(&om, &tm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_scale(matrix_4x4* mtx, float x, float y, float z)
{
	int i;
	for (i = 0; i < 4; i ++)
	{
		mtx->r[i].x *= x;
		mtx->r[i].y *= y;
		mtx->r[i].z *= z;
	}
}

void m4x4_rotate_x(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = 1.0f;
	rm.r[1].y = cosAngle;
	rm.r[1].z = sinAngle;
	rm.r[2].y = -sinAngle;
	rm.r[2].z = cosAngle;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_rotate_y(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = cosAngle;
	rm.r[0].z = sinAngle;
	rm.r[1].y = 1.0f;
	rm.r[2].x = -sinAngle;
	rm.r[2].z = cosAngle;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_rotate_z(matrix_4x4* mtx, float angle, bool bRightSide)
{
	matrix_4x4 rm, om;

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	m4x4_zeros(&rm);
	rm.r[0].x = cosAngle;
	rm.r[0].y = sinAngle;
	rm.r[1].x = -sinAngle;
	rm.r[1].y = cosAngle;
	rm.r[2].z = 1.0f;
	rm.r[3].w = 1.0f;

	if (bRightSide) m4x4_multiply(&om, mtx, &rm);
	else            m4x4_multiply(&om, &rm, mtx);
	m4x4_copy(mtx, &om);
}

void m4x4_ortho_tilt(matrix_4x4* mtx, float left, float right, float bottom, float top, float near, float far)
{
	matrix_4x4 mp;
	m4x4_zeros(&mp);

	// Build standard orthogonal projection matrix
	mp.r[0].x = 2.0f / (right - left);
	mp.r[0].w = (left + right) / (left - right);
	mp.r[1].y = 2.0f / (top - bottom);
	mp.r[1].w = (bottom + top) / (bottom - top);
	mp.r[2].z = 2.0f / (near - far);
	mp.r[2].w = (far + near) / (far - near);
	mp.r[3].w = 1.0f;

	// Fix depth range to [-1, 0]
	matrix_4x4 mp2, mp3;
	m4x4_identity(&mp2);
	mp2.r[2].z = 0.5;
	mp2.r[2].w = -0.5;
	m4x4_multiply(&mp3, &mp2, &mp);

	// Fix the 3DS screens' orientation by swapping the X and Y axis
	m4x4_identity(&mp2);
	mp2.r[0].x = 0.0;
	mp2.r[0].y = 1.0;
	mp2.r[1].x = -1.0; // flipped
	mp2.r[1].y = 0.0;
	m4x4_multiply(mtx, &mp2, &mp3);
}

void m4x4_persp_tilt(matrix_4x4* mtx, float near, float far)
{
        float aspect=240.0f/400.0f;
        float num=-1.0f/512.0; //this should NOT be -1
	m4x4_zeros(mtx);
	// Build standard perspective projection matrix
	mtx->r[0].x = aspect; //near/right;
	mtx->r[1].y = 1; //near/top;
	mtx->r[2].z = (near * num) / (far - near);
	mtx->r[2].w = (far * near * num) / (far - near);
	mtx->r[3].z = -1.0f;

	// Rotate the matrix one quarter of a turn CCW in order to fix the 3DS screens' orientation
	m4x4_rotate_z(mtx, M_PI / 2, false);
}

void m5x5_persp(matrix_5x5* mtx, float near, float far)
{
    m5x5_zeros(mtx);
    mtx->r[0].x = 1.0f;
    mtx->r[1].y = 1.0f;
    mtx->r[2].z = 1.0f;
    mtx->r[3].w = near / (near - far);
    mtx->r[3].v = (far * near) / (near - far); //right?
    mtx->r[4].w = -1.0f;
	// Rotate the matrix one quarter of a turn CCW in order to fix the 3DS screens' orientation
}

void m5x5_orient(matrix_5x5* mtx, float cam[4], float look[4], float up[4], float in[4]){
    float rightDir[4]={1,0,0,0}, upDir[4], lookDir[4], inDir[4];
    for(int a=0; a<4; a++) {
        lookDir[a]=look[a]-cam[a];
        upDir[a]=up[a];
        inDir[a]=in[a];
    }
    normalize(lookDir, 4);
    normalize(upDir, 4);
    
    cross(rightDir, lookDir, inDir, upDir);
    normalize(rightDir, 4);
    cross(upDir, rightDir, lookDir, inDir);
    normalize(upDir, 4); //maybe unnecessary?
    cross(inDir, upDir, rightDir, lookDir);
    normalize(inDir, 4); //probably unnecessary, right?
    matrix_5x5 rot;
    m5x5_identity(&rot);
    for(int a=0; a<4; a++) {
        rot.r[a].c[4]=rightDir[a];
        rot.r[a].c[3]=upDir[a];
        rot.r[a].c[2]=-lookDir[a];
        rot.r[a].c[1]=-inDir[a];
    }
    matrix_5x5 temp;
    m5x5_multiply(&temp, &rot, mtx);
    m5x5_copy(mtx, &temp);
    m5x5_translate(mtx, -look[0], -look[1], -look[2], -look[3]);
}

void cross(float a[4], float b[4], float c[4], float d[4]){
    {
        float x[]={b[1], b[2], b[3]};
        float y[]={c[1], c[2], c[3]};
        float z[]={d[1], d[2], d[3]};
        a[0]=det(x,y,z);
    }
    {
        float x[]={b[0], b[2], b[3]};
        float y[]={c[0], c[2], c[3]};
        float z[]={d[0], d[2], d[3]};
        a[1]=-det(x,y,z);
    }
    {
        float x[]={b[0], b[1], b[3]};
        float y[]={c[0], c[1], c[3]};
        float z[]={d[0], d[1], d[3]};
        a[2]=det(x,y,z);
    }
    {
        float x[]={b[0], b[1], b[2]};
        float y[]={c[0], c[1], c[2]};
        float z[]={d[0], d[1], d[2]};
        a[3]=-det(x,y,z);
    }
}

void m4x4_orient(matrix_4x4* mtx, float cam[3], float look[3], float up[3]) {
    float lookDir[3];
    float upDir[3];
    for(int a=0; a<3; a++) {
        lookDir[a]=look[a]-cam[a];
        upDir[a]=up[a];
    }
    normalize(lookDir, 3);
    normalize(upDir, 3);
    float rightDir[3];
    rightDir[0]=lookDir[1]*upDir[2]-lookDir[2]*upDir[1];
    rightDir[1]=lookDir[2]*upDir[0]-lookDir[0]*upDir[2];
    rightDir[2]=lookDir[0]*upDir[1]-lookDir[1]*upDir[0];
    normalize(rightDir, 3);
    upDir[0]=rightDir[1]*lookDir[2]-rightDir[2]*lookDir[1];
    upDir[1]=rightDir[2]*lookDir[0]-rightDir[0]*lookDir[2];
    upDir[2]=rightDir[0]*lookDir[1]-rightDir[1]*lookDir[0];
    matrix_4x4 rot;
    m4x4_identity(&rot);
    for(int a=0; a<3; a++) {
        rot.r[a].c[3]=rightDir[a];
        rot.r[a].c[2]=upDir[a];
        rot.r[a].c[1]=-lookDir[a];
    }
    matrix_4x4 temp;
    m4x4_multiply(&temp, &rot, mtx);
    m4x4_copy(mtx, &temp);
}

void to4x4(matrix_4x4* mtx, vector_4f* vec, matrix_5x5* start, bool proj)
{
    for(int a=0; a<4; a++) {
        vec->c[3-a]=start->r[a].v;
        for(int b=0; b<4; b++)
            mtx->r[a].c[3-b]=start->r[a].c[4-b];
    }
    if(proj)
        vec->x=start->r[4].w;
}
