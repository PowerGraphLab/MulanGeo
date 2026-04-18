/*
 * 4x4 矩阵 — 非内联实现
 */

#include "Mat4.h"

#include <cmath>
#include <algorithm>

namespace MulanGeo::Engine {

// ============================================================
// 逆矩阵（余子式展开法）
// ============================================================

Mat4 Mat4::inverted() const {
    // 基于 MESA GLU 的实现
    double inv[16];
    const double* v = data();

    inv[0]  =  v[5]*v[10]*v[15] - v[5]*v[11]*v[14] - v[9]*v[6]*v[15] + v[9]*v[7]*v[14] + v[13]*v[6]*v[11] - v[13]*v[7]*v[10];
    inv[4]  = -v[4]*v[10]*v[15] + v[4]*v[11]*v[14] + v[8]*v[6]*v[15] - v[8]*v[7]*v[14] - v[12]*v[6]*v[11] + v[12]*v[7]*v[10];
    inv[8]  =  v[4]*v[9]*v[15]  - v[4]*v[11]*v[13] - v[8]*v[5]*v[15] + v[8]*v[7]*v[13] + v[12]*v[5]*v[11] - v[12]*v[7]*v[9];
    inv[12] = -v[4]*v[9]*v[14]  + v[4]*v[10]*v[13] + v[8]*v[5]*v[14] - v[8]*v[6]*v[13] - v[12]*v[5]*v[10] + v[12]*v[6]*v[9];

    inv[1]  = -v[1]*v[10]*v[15] + v[1]*v[11]*v[14] + v[9]*v[2]*v[15] - v[9]*v[3]*v[14] - v[13]*v[2]*v[11] + v[13]*v[3]*v[10];
    inv[5]  =  v[0]*v[10]*v[15] - v[0]*v[11]*v[14] - v[8]*v[2]*v[15] + v[8]*v[3]*v[14] + v[12]*v[2]*v[11] - v[12]*v[3]*v[10];
    inv[9]  = -v[0]*v[9]*v[15]  + v[0]*v[11]*v[13] + v[8]*v[1]*v[15] - v[8]*v[3]*v[13] - v[12]*v[1]*v[11] + v[12]*v[3]*v[9];
    inv[13] =  v[0]*v[9]*v[14]  - v[0]*v[10]*v[13] - v[8]*v[1]*v[14] + v[8]*v[2]*v[13] + v[12]*v[1]*v[10] - v[12]*v[2]*v[9];

    inv[2]  =  v[1]*v[6]*v[15]  - v[1]*v[7]*v[14]  - v[5]*v[2]*v[15] + v[5]*v[3]*v[14] + v[13]*v[2]*v[7]  - v[13]*v[3]*v[6];
    inv[6]  = -v[0]*v[6]*v[15]  + v[0]*v[7]*v[14]  + v[4]*v[2]*v[15] - v[4]*v[3]*v[14] - v[12]*v[2]*v[7]  + v[12]*v[3]*v[6];
    inv[10] =  v[0]*v[5]*v[15]  - v[0]*v[7]*v[13]  - v[4]*v[1]*v[15] + v[4]*v[3]*v[13] + v[12]*v[1]*v[7]  - v[12]*v[3]*v[5];
    inv[14] = -v[0]*v[5]*v[14]  + v[0]*v[6]*v[13]  + v[4]*v[1]*v[14] - v[4]*v[2]*v[13] - v[12]*v[1]*v[6]  + v[12]*v[2]*v[5];

    inv[3]  = -v[1]*v[6]*v[11]  + v[1]*v[7]*v[10]  + v[5]*v[2]*v[11] - v[5]*v[3]*v[10] - v[9]*v[2]*v[7]   + v[9]*v[3]*v[6];
    inv[7]  =  v[0]*v[6]*v[11]  - v[0]*v[7]*v[10]  - v[4]*v[2]*v[11] + v[4]*v[3]*v[10] + v[8]*v[2]*v[7]   - v[8]*v[3]*v[6];
    inv[11] = -v[0]*v[5]*v[11]  + v[0]*v[7]*v[9]   + v[4]*v[1]*v[11] - v[4]*v[3]*v[9]  - v[8]*v[1]*v[7]   + v[8]*v[3]*v[5];
    inv[15] =  v[0]*v[5]*v[10]  - v[0]*v[6]*v[9]   - v[4]*v[1]*v[10] + v[4]*v[2]*v[9]  + v[8]*v[1]*v[6]   - v[8]*v[2]*v[5];

    // 行列式
    double det = v[0]*inv[0] + v[1]*inv[4] + v[2]*inv[8] + v[3]*inv[12];
    if (det == 0) return identity();

    double invDet = 1.0 / det;
    Mat4 result;
    double* r = result.data();
    for (int i = 0; i < 16; ++i)
        r[i] = inv[i] * invDet;
    return result;
}

// ============================================================
// 旋转
// ============================================================

Mat4 Mat4::rotationAxis(const Vec3& axis, double radians) {
    Vec3 n = axis.normalized();
    double c = std::cos(radians);
    double s = std::sin(radians);
    double t = 1.0 - c;

    Mat4 r;
    r.m[0][0] = t*n.x*n.x + c;     r.m[1][0] = t*n.x*n.y - s*n.z; r.m[2][0] = t*n.x*n.z + s*n.y;
    r.m[0][1] = t*n.x*n.y + s*n.z; r.m[1][1] = t*n.y*n.y + c;     r.m[2][1] = t*n.y*n.z - s*n.x;
    r.m[0][2] = t*n.x*n.z - s*n.y; r.m[1][2] = t*n.y*n.z + s*n.x; r.m[2][2] = t*n.z*n.z + c;
    return r;
}

Mat4 Mat4::rotationX(double radians) {
    double c = std::cos(radians), s = std::sin(radians);
    Mat4 r;
    r.m[1][1] = c;  r.m[2][1] = -s;
    r.m[1][2] = s;  r.m[2][2] = c;
    return r;
}

Mat4 Mat4::rotationY(double radians) {
    double c = std::cos(radians), s = std::sin(radians);
    Mat4 r;
    r.m[0][0] = c;  r.m[2][0] = s;
    r.m[0][2] = -s; r.m[2][2] = c;
    return r;
}

Mat4 Mat4::rotationZ(double radians) {
    double c = std::cos(radians), s = std::sin(radians);
    Mat4 r;
    r.m[0][0] = c;  r.m[1][0] = -s;
    r.m[0][1] = s;  r.m[1][1] = c;
    return r;
}

// ============================================================
// 视图 / 投影
// ============================================================

Mat4 Mat4::lookAt(const Vec3& eye, const Vec3& target, const Vec3& up) {
    Vec3 fwd   = (target - eye).normalized();  // z 轴方向
    Vec3 right = Vec3::cross(fwd, up).normalized();
    Vec3 newUp = Vec3::cross(right, fwd);

    Mat4 r;
    r.m[0][0] = right.x;   r.m[1][0] = right.y;   r.m[2][0] = right.z;   r.m[3][0] = -Vec3::dot(right, eye);
    r.m[0][1] = newUp.x;   r.m[1][1] = newUp.y;   r.m[2][1] = newUp.z;   r.m[3][1] = -Vec3::dot(newUp, eye);
    r.m[0][2] = -fwd.x;    r.m[1][2] = -fwd.y;    r.m[2][2] = -fwd.z;    r.m[3][2] =  Vec3::dot(fwd, eye);
    return r;
}

Mat4 Mat4::perspective(double fovY, double aspect, double nearZ, double farZ) {
    double tanHalf = std::tan(fovY * 0.5);
    double range   = farZ - nearZ;

    Mat4 r;
    r.m[0][0] = 1.0 / (aspect * tanHalf);
    r.m[1][1] = 1.0 / tanHalf;
    r.m[2][2] = -(farZ + nearZ) / range;
    r.m[2][3] = -1.0;
    r.m[3][2] = -(2.0 * farZ * nearZ) / range;
    r.m[3][3] = 0.0;
    return r;
}

Mat4 Mat4::ortho(double left, double right, double bottom, double top,
                 double nearZ, double farZ) {
    double rl = right - left;
    double tb = top - bottom;
    double fn = farZ - nearZ;

    Mat4 r;
    r.m[0][0] =  2.0 / rl;
    r.m[1][1] =  2.0 / tb;
    r.m[2][2] = -2.0 / fn;
    r.m[3][0] = -(right + left) / rl;
    r.m[3][1] = -(top + bottom) / tb;
    r.m[3][2] = -(farZ + nearZ) / fn;
    return r;
}

} // namespace MulanGeo::Engine
