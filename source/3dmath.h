/*
 * Bare-bones simplistic 3D math library
 * This library is common to all libctru GPU examples
 */

#pragma once
#include <string.h>
#include <stdbool.h>
#include <math.h>
//5x5 stuff!
typedef union { struct { float v, w, z, y, x; }; float c[5]; } vector_5f;
typedef struct { vector_5f r[5]; } matrix_5x5;

typedef union { struct { float w, z, y, x; }; float c[4]; } vector_4f;
typedef struct { vector_4f r[4]; } matrix_4x4;

static inline float v5f_dp5(const vector_5f* a, const vector_5f* b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w + a->v*b->v;
}

static inline float v5f_mod5(const vector_5f* a)
{
	return sqrtf(v5f_dp5(a,a));
}

static inline void v5f_norm5(vector_5f* vec)
{
	float m = v5f_mod5(vec);
	if (m == 0.0) return;
	vec->x /= m;
	vec->y /= m;
	vec->z /= m;
	vec->w /= m;
        vec->v /= m;
}

static inline void m5x5_zeros(matrix_5x5* out)
{
	memset(out, 0, sizeof(*out));
}

static inline void m5x5_copy(matrix_5x5* out, const matrix_5x5* in)
{
	memcpy(out, in, sizeof(*out));
}

void m5x5_identity(matrix_5x5* out);
void m5x5_multiply(matrix_5x5* out, const matrix_5x5* a, const matrix_5x5* b);

void m5x5_translate(matrix_5x5* mtx, float x, float y, float z, float w);
void m5x5_scale(matrix_5x5* mtx, float x, float y, float z, float w);

void m5x5_persp(matrix_5x5*, float near, float far);
void m5x5_orient(matrix_5x5*, float[4], float[4], float[4], float[4]);
//4x4 stuff


static inline float v4f_dp4(const vector_4f* a, const vector_4f* b)
{
	return a->x*b->x + a->y*b->y + a->z*b->z + a->w*b->w;
}

static inline float v4f_mod4(const vector_4f* a)
{
	return sqrtf(v4f_dp4(a,a));
}

static inline void v4f_norm4(vector_4f* vec)
{
	float m = v4f_mod4(vec);
	if (m == 0.0) return;
	vec->x /= m;
	vec->y /= m;
	vec->z /= m;
	vec->w /= m;
}

static inline void m4x4_zeros(matrix_4x4* out)
{
	memset(out, 0, sizeof(*out));
}

static inline void m4x4_copy(matrix_4x4* out, const matrix_4x4* in)
{
	memcpy(out, in, sizeof(*out));
}

void m4x4_identity(matrix_4x4* out);
void m4x4_multiply(matrix_4x4* out, const matrix_4x4* a, const matrix_4x4* b);

void m4x4_translate(matrix_4x4* mtx, float x, float y, float z, bool left=true);
void m4x4_scale(matrix_4x4* mtx, float x, float y, float z);

void m4x4_rotate_x(matrix_4x4* mtx, float angle, bool bRightSide);
void m4x4_rotate_y(matrix_4x4* mtx, float angle, bool bRightSide);
void m4x4_rotate_z(matrix_4x4* mtx, float angle, bool bRightSide);

// Special versions of the projection matrices that take the 3DS' screen orientation into account
void m4x4_ortho_tilt(matrix_4x4* mtx, float left, float right, float bottom, float top, float near, float far);
void m4x4_persp_tilt(matrix_4x4* mtx, float near, float far);

void m4x4_orient(matrix_4x4*, float[3], float[3], float[3]);

void to4x4(matrix_4x4* mtx, vector_4f* vec, matrix_5x5* start, bool proj);

void cross(float[4], float[4], float[4], float[4]);

static inline void normalize(float* ptr, int size) {
    float sum=0;
    for(int a=0; a<size; a++)
        sum+=(ptr[a]*ptr[a]);
    if(sum!=0) {
        sum=sqrtf(sum);
        for(int a=0; a<size; a++)
            ptr[a]/=sum;
    }
}

static inline float det(float a[3], float b[3], float c[3]) {
    float ret=a[0]*((b[1]*c[2])-(b[2]*c[1]));
    ret-=a[1]*((b[0]*c[2])-(b[2]*c[0]));
    ret+=a[2]*((b[0]*c[1])-(b[1]*c[0]));
    return ret;
}


static inline float degToRad(float deg) {return deg*M_PI/180.0f;}