/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Don't edit this file!  It is auto-generated by frameworks/rs/api/generate.sh.

/*
 * rs_matrix.rsh: Matrix Functions
 *
 * These functions let you manipulate square matrices of rank 2x2, 3x3, and 4x4.
 * They are particularly useful for graphical transformations and are compatible
 * with OpenGL.
 *
 * We use a zero-based index for rows and columns.  E.g. the last element of a
 * rs_matrix4x4 is found at (3, 3).
 *
 * RenderScript uses column-major matrices and column-based vectors.  Transforming
 * a vector is done by postmultiplying the vector, e.g. (matrix * vector),
 * as provided by rsMatrixMultiply().
 *
 * To create a transformation matrix that performs two transformations at once,
 * multiply the two source matrices, with the first transformation as the right
 * argument.  E.g. to create a transformation matrix that applies the
 * transformation s1 followed by s2, call rsMatrixLoadMultiply(&amp;combined, &amp;s2, &amp;s1).
 * This derives from s2 * (s1 * v), which is (s2 * s1) * v.
 *
 * We have two style of functions to create transformation matrices:
 * rsMatrixLoadTransformation and rsMatrixTransformation.  The former
 * style simply stores the transformation matrix in the first argument.  The latter
 * modifies a pre-existing transformation matrix so that the new transformation
 * happens first.  E.g. if you call rsMatrixTranslate() on a matrix that already
 * does a scaling, the resulting matrix when applied to a vector will first do the
 * translation then the scaling.
 */

#ifndef RENDERSCRIPT_RS_MATRIX_RSH
#define RENDERSCRIPT_RS_MATRIX_RSH

#include "rs_vector_math.rsh"

/*
 * rsExtractFrustumPlanes: Compute frustum planes
 *
 * Computes 6 frustum planes from the view projection matrix
 *
 * Parameters:
 *   viewProj: Matrix to extract planes from.
 *   left: Left plane.
 *   right: Right plane.
 *   top: Top plane.
 *   bottom: Bottom plane.
 *   near: Near plane.
 *   far: Far plane.
 */
static inline void __attribute__((always_inline, overloadable))
    rsExtractFrustumPlanes(const rs_matrix4x4* viewProj, float4* left, float4* right, float4* top,
                           float4* bottom, float4* near, float4* far) {
    // x y z w = a b c d in the plane equation
    left->x = viewProj->m[3] + viewProj->m[0];
    left->y = viewProj->m[7] + viewProj->m[4];
    left->z = viewProj->m[11] + viewProj->m[8];
    left->w = viewProj->m[15] + viewProj->m[12];

    right->x = viewProj->m[3] - viewProj->m[0];
    right->y = viewProj->m[7] - viewProj->m[4];
    right->z = viewProj->m[11] - viewProj->m[8];
    right->w = viewProj->m[15] - viewProj->m[12];

    top->x = viewProj->m[3] - viewProj->m[1];
    top->y = viewProj->m[7] - viewProj->m[5];
    top->z = viewProj->m[11] - viewProj->m[9];
    top->w = viewProj->m[15] - viewProj->m[13];

    bottom->x = viewProj->m[3] + viewProj->m[1];
    bottom->y = viewProj->m[7] + viewProj->m[5];
    bottom->z = viewProj->m[11] + viewProj->m[9];
    bottom->w = viewProj->m[15] + viewProj->m[13];

    near->x = viewProj->m[3] + viewProj->m[2];
    near->y = viewProj->m[7] + viewProj->m[6];
    near->z = viewProj->m[11] + viewProj->m[10];
    near->w = viewProj->m[15] + viewProj->m[14];

    far->x = viewProj->m[3] - viewProj->m[2];
    far->y = viewProj->m[7] - viewProj->m[6];
    far->z = viewProj->m[11] - viewProj->m[10];
    far->w = viewProj->m[15] - viewProj->m[14];

    float len = length(left->xyz);
    *left /= len;
    len = length(right->xyz);
    *right /= len;
    len = length(top->xyz);
    *top /= len;
    len = length(bottom->xyz);
    *bottom /= len;
    len = length(near->xyz);
    *near /= len;
    len = length(far->xyz);
    *far /= len;
}

/*
 * rsIsSphereInFrustum: Checks if a sphere is within the frustum planes
 *
 * Returns true if the sphere is within the 6 frustum planes.
 *
 * Parameters:
 *   sphere: float4 representing the sphere.
 *   left: Left plane.
 *   right: Right plane.
 *   top: Top plane.
 *   bottom: Bottom plane.
 *   near: Near plane.
 *   far: Far plane.
 */
static inline bool __attribute__((always_inline, overloadable))
    rsIsSphereInFrustum(float4* sphere, float4* left, float4* right, float4* top, float4* bottom,
                        float4* near, float4* far) {
    float distToCenter = dot(left->xyz, sphere->xyz) + left->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(right->xyz, sphere->xyz) + right->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(top->xyz, sphere->xyz) + top->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(bottom->xyz, sphere->xyz) + bottom->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(near->xyz, sphere->xyz) + near->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    distToCenter = dot(far->xyz, sphere->xyz) + far->w;
    if (distToCenter < -sphere->w) {
        return false;
    }
    return true;
}

/*
 * rsMatrixGet: Get one element
 *
 * Returns one element of a matrix.
 *
 * Warning: The order of the column and row parameters may be unexpected.
 *
 * Parameters:
 *   m: Matrix to extract the element from.
 *   col: Zero-based column of the element to be extracted.
 *   row: Zero-based row of the element to extracted.
 */
extern float __attribute__((overloadable))
    rsMatrixGet(const rs_matrix4x4* m, uint32_t col, uint32_t row);

extern float __attribute__((overloadable))
    rsMatrixGet(const rs_matrix3x3* m, uint32_t col, uint32_t row);

extern float __attribute__((overloadable))
    rsMatrixGet(const rs_matrix2x2* m, uint32_t col, uint32_t row);

/*
 * rsMatrixInverse: Inverts a matrix in place
 *
 * Returns true if the matrix was successfully inverted.
 *
 * Parameters:
 *   m: Matrix to invert.
 */
extern bool __attribute__((overloadable))
    rsMatrixInverse(rs_matrix4x4* m);

/*
 * rsMatrixInverseTranspose: Inverts and transpose a matrix in place
 *
 * The matrix is first inverted then transposed. Returns true if the matrix was
 * successfully inverted.
 *
 * Parameters:
 *   m: Matrix to modify.
 */
extern bool __attribute__((overloadable))
    rsMatrixInverseTranspose(rs_matrix4x4* m);

/*
 * rsMatrixLoad: Load or copy a matrix
 *
 * Set the elements of a matrix from an array of floats or from another matrix.
 *
 * If loading from an array, the floats should be in row-major order, i.e. the element a
 * row 0, column 0 should be first, followed by the element at
 * row 0, column 1, etc.
 *
 * If loading from a matrix and the source is smaller than the destination, the rest
 * of the destination is filled with elements of the identity matrix.  E.g.
 * loading a rs_matrix2x2 into a rs_matrix4x4 will give:
 *
 * m00 m01 0.0 0.0
 * m10 m11 0.0 0.0
 * 0.0 0.0 1.0 0.0
 * 0.0 0.0 0.0 1.0
 *
 *
 * Parameters:
 *   destination: Matrix to set.
 *   array: Array of values to set the matrix to. These arrays should be 4, 9, or 16 floats long, depending on the matrix size.
 *   source: Source matrix.
 */
extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix4x4* destination, const float* array);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix3x3* destination, const float* array);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix2x2* destination, const float* array);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix4x4* destination, const rs_matrix4x4* source);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix3x3* destination, const rs_matrix3x3* source);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix2x2* destination, const rs_matrix2x2* source);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix4x4* destination, const rs_matrix3x3* source);

extern void __attribute__((overloadable))
    rsMatrixLoad(rs_matrix4x4* destination, const rs_matrix2x2* source);

/*
 * rsMatrixLoadFrustum: Load a frustum projection matrix
 *
 * Constructs a frustum projection matrix, transforming the box identified by
 * the six clipping planes left, right, bottom, top, near, far.
 *
 * To apply this projection to a vector, multiply the vector by the created
 * matrix using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to set.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadFrustum(rs_matrix4x4* m, float left, float right, float bottom, float top,
                        float near, float far);

/*
 * rsMatrixLoadIdentity: Load identity matrix
 *
 * Set the elements of a matrix to the identity matrix.
 *
 * Parameters:
 *   m: Matrix to set.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadIdentity(rs_matrix4x4* m);

extern void __attribute__((overloadable))
    rsMatrixLoadIdentity(rs_matrix3x3* m);

extern void __attribute__((overloadable))
    rsMatrixLoadIdentity(rs_matrix2x2* m);

/*
 * rsMatrixLoadMultiply: Multiply two matrices
 *
 * Sets m to the matrix product of lhs * rhs.
 *
 * To combine two 4x4 transformaton matrices, multiply the second transformation matrix
 * by the first transformation matrix.  E.g. to create a transformation matrix that applies
 * the transformation s1 followed by s2, call rsMatrixLoadMultiply(&amp;combined, &amp;s2, &amp;s1).
 *
 * Warning: Prior to version 21, storing the result back into right matrix is not supported and
 * will result in undefined behavior.  Use rsMatrixMulitply instead.   E.g. instead of doing
 * rsMatrixLoadMultiply (&amp;m2r, &amp;m2r, &amp;m2l), use rsMatrixMultiply (&amp;m2r, &amp;m2l).
 * rsMatrixLoadMultiply (&amp;m2l, &amp;m2r, &amp;m2l) works as expected.
 *
 * Parameters:
 *   m: Matrix to set.
 *   lhs: Left matrix of the product.
 *   rhs: Right matrix of the product.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadMultiply(rs_matrix4x4* m, const rs_matrix4x4* lhs, const rs_matrix4x4* rhs);

extern void __attribute__((overloadable))
    rsMatrixLoadMultiply(rs_matrix3x3* m, const rs_matrix3x3* lhs, const rs_matrix3x3* rhs);

extern void __attribute__((overloadable))
    rsMatrixLoadMultiply(rs_matrix2x2* m, const rs_matrix2x2* lhs, const rs_matrix2x2* rhs);

/*
 * rsMatrixLoadOrtho: Load an orthographic projection matrix
 *
 * Constructs an orthographic projection matrix, transforming the box identified by the
 * six clipping planes left, right, bottom, top, near, far into a unit cube
 * with a corner at (-1, -1, -1) and the opposite at (1, 1, 1).
 *
 * To apply this projection to a vector, multiply the vector by the created matrix
 * using rsMatrixMultiply().
 *
 * See https://en.wikipedia.org/wiki/Orthographic_projection .
 *
 * Parameters:
 *   m: Matrix to set.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadOrtho(rs_matrix4x4* m, float left, float right, float bottom, float top, float near,
                      float far);

/*
 * rsMatrixLoadPerspective: Load a perspective projection matrix
 *
 * Constructs a perspective projection matrix, assuming a symmetrical field of view.
 *
 * To apply this projection to a vector, multiply the vector by the created matrix
 * using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to set.
 *   fovy: Field of view, in degrees along the Y axis.
 *   aspect: Ratio of x / y.
 *   near: Near clipping plane.
 *   far: Far clipping plane.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadPerspective(rs_matrix4x4* m, float fovy, float aspect, float near, float far);

/*
 * rsMatrixLoadRotate: Load a rotation matrix
 *
 * This function creates a rotation matrix.  The axis of rotation is the (x, y, z) vector.
 *
 * To rotate a vector, multiply the vector by the created matrix using rsMatrixMultiply().
 *
 * See http://en.wikipedia.org/wiki/Rotation_matrix .
 *
 * Parameters:
 *   m: Matrix to set.
 *   rot: How much rotation to do, in degrees.
 *   x: X component of the vector that is the axis of rotation.
 *   y: Y component of the vector that is the axis of rotation.
 *   z: Z component of the vector that is the axis of rotation.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadRotate(rs_matrix4x4* m, float rot, float x, float y, float z);

/*
 * rsMatrixLoadScale: Load a scaling matrix
 *
 * This function creates a scaling matrix, where each component of a vector is multiplied
 * by a number.  This number can be negative.
 *
 * To scale a vector, multiply the vector by the created matrix using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to set.
 *   x: Multiple to scale the x components by.
 *   y: Multiple to scale the y components by.
 *   z: Multiple to scale the z components by.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadScale(rs_matrix4x4* m, float x, float y, float z);

/*
 * rsMatrixLoadTranslate: Load a translation matrix
 *
 * This function creates a translation matrix, where a number is added to each element of
 * a vector.
 *
 * To translate a vector, multiply the vector by the created matrix using
 * rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to set.
 *   x: Number to add to each x component.
 *   y: Number to add to each y component.
 *   z: Number to add to each z component.
 */
extern void __attribute__((overloadable))
    rsMatrixLoadTranslate(rs_matrix4x4* m, float x, float y, float z);

/*
 * rsMatrixMultiply: Multiply a matrix by a vector or another matrix
 *
 * For the matrix by matrix variant, sets m to the matrix product m * rhs.
 *
 * When combining two 4x4 transformation matrices using this function, the resulting
 * matrix will correspond to performing the rhs transformation first followed by
 * the original m transformation.
 *
 * For the matrix by vector variant, returns the post-multiplication of the vector
 * by the matrix, ie. m * in.
 *
 * When multiplying a float3 to a rs_matrix4x4, the vector is expanded with (1).
 *
 * When multiplying a float2 to a rs_matrix4x4, the vector is expanded with (0, 1).
 *
 * When multiplying a float2 to a rs_matrix3x3, the vector is expanded with (0).
 *
 * Starting with API 14, this function takes a const matrix as the first argument.
 *
 * Parameters:
 *   m: Left matrix of the product and the matrix to be set.
 *   rhs: Right matrix of the product.
 */
extern void __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix4x4* m, const rs_matrix4x4* rhs);

extern void __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix3x3* m, const rs_matrix3x3* rhs);

extern void __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix2x2* m, const rs_matrix2x2* rhs);

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix4x4* m, float4 in);
#endif

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix4x4* m, float3 in);
#endif

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix4x4* m, float2 in);
#endif

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float3 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix3x3* m, float3 in);
#endif

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float3 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix3x3* m, float2 in);
#endif

#if !defined(RS_VERSION) || (RS_VERSION <= 13)
extern float2 __attribute__((overloadable))
    rsMatrixMultiply(rs_matrix2x2* m, float2 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix4x4* m, float4 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix4x4* m, float3 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float4 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix4x4* m, float2 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float3 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix3x3* m, float3 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float3 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix3x3* m, float2 in);
#endif

#if (defined(RS_VERSION) && (RS_VERSION >= 14))
extern float2 __attribute__((overloadable))
    rsMatrixMultiply(const rs_matrix2x2* m, float2 in);
#endif

/*
 * rsMatrixRotate: Apply a rotation to a transformation matrix
 *
 * Multiply the matrix m with a rotation matrix.
 *
 * This function modifies a transformation matrix to first do a rotation.  The axis of
 * rotation is the (x, y, z) vector.
 *
 * To apply this combined transformation to a vector, multiply the vector by the created
 * matrix using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to modify.
 *   rot: How much rotation to do, in degrees.
 *   x: X component of the vector that is the axis of rotation.
 *   y: Y component of the vector that is the axis of rotation.
 *   z: Z component of the vector that is the axis of rotation.
 */
extern void __attribute__((overloadable))
    rsMatrixRotate(rs_matrix4x4* m, float rot, float x, float y, float z);

/*
 * rsMatrixScale: Apply a scaling to a transformation matrix
 *
 * Multiply the matrix m with a scaling matrix.
 *
 * This function modifies a transformation matrix to first do a scaling.   When scaling,
 * each component of a vector is multiplied by a number.  This number can be negative.
 *
 * To apply this combined transformation to a vector, multiply the vector by the created
 * matrix using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to modify.
 *   x: Multiple to scale the x components by.
 *   y: Multiple to scale the y components by.
 *   z: Multiple to scale the z components by.
 */
extern void __attribute__((overloadable))
    rsMatrixScale(rs_matrix4x4* m, float x, float y, float z);

/*
 * rsMatrixSet: Set one element
 *
 * Set an element of a matrix.
 *
 * Warning: The order of the column and row parameters may be unexpected.
 *
 * Parameters:
 *   m: Matrix that will be modified.
 *   col: Zero-based column of the element to be set.
 *   row: Zero-based row of the element to be set.
 *   v: Value to set.
 */
extern void __attribute__((overloadable))
    rsMatrixSet(rs_matrix4x4* m, uint32_t col, uint32_t row, float v);

extern void __attribute__((overloadable))
    rsMatrixSet(rs_matrix3x3* m, uint32_t col, uint32_t row, float v);

extern void __attribute__((overloadable))
    rsMatrixSet(rs_matrix2x2* m, uint32_t col, uint32_t row, float v);

/*
 * rsMatrixTranslate: Apply a translation to a transformation matrix
 *
 * Multiply the matrix m with a translation matrix.
 *
 * This function modifies a transformation matrix to first do a translation.  When
 * translating, a number is added to each component of a vector.
 *
 * To apply this combined transformation to a vector, multiply the vector by the
 * created matrix using rsMatrixMultiply().
 *
 * Parameters:
 *   m: Matrix to modify.
 *   x: Number to add to each x component.
 *   y: Number to add to each y component.
 *   z: Number to add to each z component.
 */
extern void __attribute__((overloadable))
    rsMatrixTranslate(rs_matrix4x4* m, float x, float y, float z);

/*
 * rsMatrixTranspose: Transpose a matrix place
 *
 * Transpose the matrix m in place.
 *
 * Parameters:
 *   m: Matrix to transpose.
 */
extern void __attribute__((overloadable))
    rsMatrixTranspose(rs_matrix4x4* m);

extern void __attribute__((overloadable))
    rsMatrixTranspose(rs_matrix3x3* m);

extern void __attribute__((overloadable))
    rsMatrixTranspose(rs_matrix2x2* m);

#endif // RENDERSCRIPT_RS_MATRIX_RSH
