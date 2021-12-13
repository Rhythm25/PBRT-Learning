#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "quaternion.h"
#include "ray.h"
/*#define MachineEpsilon (std::numeric_limits<double>::epsilon() * 0.5)

inline double gamma(int n) {
    return (n * MachineEpsilon) / (1 - n * MachineEpsilon);
}*/
inline double Radians(double deg) { return (PI / 180) * deg; }
using namespace std;
struct Matrix4x4 {
    // Matrix4x4 Public Methods
    Matrix4x4() {
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.f;
        m[0][1] = m[0][2] = m[0][3] = m[1][0] = m[1][2] = m[1][3] = m[2][0] =
            m[2][1] = m[2][3] = m[3][0] = m[3][1] = m[3][2] = 0.f;
    }
    Matrix4x4(double mat[4][4]);
    Matrix4x4(double t00, double t01, double t02, double t03, double t10, double t11,
        double t12, double t13, double t20, double t21, double t22, double t23,
        double t30, double t31, double t32, double t33);
    bool operator==(const Matrix4x4& m2) const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (m[i][j] != m2.m[i][j]) return false;
        return true;
    }
    bool operator!=(const Matrix4x4& m2) const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                if (m[i][j] != m2.m[i][j]) return true;
        return false;
    }
    friend Matrix4x4 Transpose(const Matrix4x4&);
    void Print(FILE* f) const {
        fprintf(f, "[ ");
        for (int i = 0; i < 4; ++i) {
            fprintf(f, "  [ ");
            for (int j = 0; j < 4; ++j) {
                fprintf(f, "%f", m[i][j]);
                if (j != 3) fprintf(f, ", ");
            }
            fprintf(f, " ]\n");
        }
        fprintf(f, " ] ");
    }
    static Matrix4x4 Mul(const Matrix4x4& m1, const Matrix4x4& m2) {
        Matrix4x4 r;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                r.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] +
                m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
        return r;
    }
    friend Matrix4x4 Inverse(const Matrix4x4&);

    friend std::ostream& operator<<(std::ostream& os, const Matrix4x4& m) {
        // clang-format off
        os << "[ [ %f, %f, %f, %f ] "
            "[ %f, %f, %f, %f ] "
            "[ %f, %f, %f, %f ] "
            "[ %f, %f, %f, %f ] ]",
            m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3],
            m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3],
            m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3],
            m.m[3][0], m.m[3][1], m.m[3][2], m.m[3][3];
        // clang-format on
        return os;
    }

    double m[4][4];
};

class Transform
{
public:
    // Transform Public Methods
    Transform() {}
    Transform(const double mat[4][4]) {
        m = Matrix4x4(mat[0][0], mat[0][1], mat[0][2], mat[0][3], mat[1][0],
            mat[1][1], mat[1][2], mat[1][3], mat[2][0], mat[2][1],
            mat[2][2], mat[2][3], mat[3][0], mat[3][1], mat[3][2],
            mat[3][3]);
        mInv = Inverse(m);
    }
    Transform(const Matrix4x4& m) : m(m), mInv(Inverse(m)) {}
    Transform(const Matrix4x4& m, const Matrix4x4& mInv) : m(m), mInv(mInv) {}
    void Print(FILE* f) const;
    friend Transform Inverse(const Transform& t) {
        return Transform(t.mInv, t.m);
    }
    friend Transform Transpose(const Transform& t) {
        return Transform(Transpose(t.m), Transpose(t.mInv));
    }
    bool operator==(const Transform& t) const {
        return t.m == m && t.mInv == mInv;
    }
    bool operator!=(const Transform& t) const {
        return t.m != m || t.mInv != mInv;
    }
    bool operator<(const Transform& t2) const {
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j) {
                if (m.m[i][j] < t2.m.m[i][j]) return true;
                if (m.m[i][j] > t2.m.m[i][j]) return false;
            }
        return false;
    }
    bool IsIdentity() const {
        return (m.m[0][0] == 1.f && m.m[0][1] == 0.f && m.m[0][2] == 0.f &&
            m.m[0][3] == 0.f && m.m[1][0] == 0.f && m.m[1][1] == 1.f &&
            m.m[1][2] == 0.f && m.m[1][3] == 0.f && m.m[2][0] == 0.f &&
            m.m[2][1] == 0.f && m.m[2][2] == 1.f && m.m[2][3] == 0.f &&
            m.m[3][0] == 0.f && m.m[3][1] == 0.f && m.m[3][2] == 0.f &&
            m.m[3][3] == 1.f);
    }
    const Matrix4x4& GetMatrix() const { return m; }
    const Matrix4x4& GetInverseMatrix() const { return mInv; }
    bool HasScale() const {
        double la2 = (*this)(Vector3f(1, 0, 0)).LengthSquared();
        double lb2 = (*this)(Vector3f(0, 1, 0)).LengthSquared();
        double lc2 = (*this)(Vector3f(0, 0, 1)).LengthSquared();
#define NOT_ONE(x) ((x) < .999f || (x) > 1.001f)
        return (NOT_ONE(la2) || NOT_ONE(lb2) || NOT_ONE(lc2));
#undef NOT_ONE
    }
    template <typename T>
    inline Point3<T> operator()(const Point3<T>& p) const;
    template <typename T>
    inline Vector3<T> operator()(const Vector3<T>& v) const;
    template <typename T>
    /*inline Normal3<T> operator()(const Normal3<T>&) const;
    inline Ray operator()(const Ray& r) const;
    inline RayDifferential operator()(const RayDifferential& r) const;
    Bounds3f operator()(const Bounds3f& b) const;*/
    //inline ray operator()(const ray& r) const;

    //ray ConvertRay(const ray& r,double &tmax,double &tmin);
    /*ray ConvertRay(const ray& r, Vector3f* oError,
        Vector3f* dError, double& tmax, double& tmin) const;*/
    inline ray Transform::operator()(const ray& r, Vector3f* oError,
        Vector3f* dError, double& tmax, double& tmin) const;
    Transform operator*(const Transform& t2) const {
        return Transform(Matrix4x4::Mul(m, t2.m), Matrix4x4::Mul(t2.mInv, mInv));
    }
    bool SwapsHandedness() const;
    //SurfaceInteraction operator()(const SurfaceInteraction& si) const;
    template <typename T>
    inline Point3<T> operator()(const Point3<T>& pt,
        Vector3<T>* absError) const;
    template <typename T>
    inline Point3<T> operator()(const Point3<T>& p, const Vector3<T>& pError,
        Vector3<T>* pTransError) const;
    template <typename T>
    inline Vector3<T> operator()(const Vector3<T>& v,
        Vector3<T>* vTransError) const;
    template <typename T>
    inline Vector3<T> operator()(const Vector3<T>& v, const Vector3<T>& vError,
        Vector3<T>* vTransError) const;
    /*inline Ray operator()(const Ray& r, Vector3f* oError,
        Vector3f* dError) const;
    inline Ray operator()(const Ray& r, const Vector3f& oErrorIn,
        const Vector3f& dErrorIn, Vector3f* oErrorOut,
        Vector3f* dErrorOut) const;*/

    friend std::ostream& operator<<(std::ostream& os, const Transform& t) {
        os << "t=" << t.m << ", inv=" << t.mInv;
        return os;
    }

private:
    // Transform Private Data
    Matrix4x4 m, mInv;
    friend class AnimatedTransform;
    friend struct Quaternion;
};

ray ConvertRayTrans(const Transform& trans, const ray& r, Vector3f* oError,
    Vector3f* dError, double& tmax, double& tmin);
Point3f ConvertPTrans(const Transform& trans, const Point3f& p, Vector3f& pError);
Point3f ConvertPTrans(const Transform& trans, const Point3f& p);
Vector3f ConvertVTrans(const Transform& trans, const Vector3f& v, Vector3f& vError);
Vector3f ConvertVTrans(const Transform& trans, const Vector3f& v);
Transform Translate(const Vector3f& delta);
Transform Scale(double x, double y, double z);
Transform RotateX(double theta);
Transform RotateY(double theta);
Transform RotateZ(double theta);
Transform Rotate(double theta, const Vector3f& axis);
Transform LookAt(const Point3f& pos, const Point3f& look, const Vector3f& up);
Transform Orthographic(double znear, double zfar);
Transform Perspective(double fov, double znear, double zfar);
bool SolveLinearSystem2x2(const double A[2][2], const double B[2], double* x0,
    double* x1);

// Transform Inline Functions
template <typename T>
inline Point3<T> Transform::operator()(const Point3<T>& p) const {
    T x = p.x, y = p.y, z = p.z;
    T xp = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
    T yp = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
    T zp = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
    T wp = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
    assert(wp!= 0);
    if (wp == 1)
        return Point3<T>(xp, yp, zp);
    else
        return Point3<T>(xp, yp, zp) / wp;
}

template <typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& v) const {
    T x = v.x, y = v.y, z = v.z;
    return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
        m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
        m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

/*inline ray Transform::operator()(const ray& r) const {
    Vector3f oError;
    Point3f o = (*this)(r.origin(), &oError);
    Vector3f d = (*this)(r.direction());
    // Offset ray origin to edge of error bounds and compute _tMax_
    double lengthSquared = d.LengthSquared();
    double tMax = r.tMax;
    if (lengthSquared > 0) {
        double dt = Dot(Abs(d), oError) / lengthSquared;
        o += d * dt;
        tMax -= dt;
    }
    return Ray(o, d, tMax, r.time);
}*/
/*template <typename T>
ray Transform::ConvertRay(const ray& r, double& tmax, double& tmin) {
    Vector3f oError;
    Point3f o = (*this)(r.origin(), &oError);
    Vector3f d = (*this)(r.direction());
    // Offset ray origin to edge of error bounds and compute _tMax_
    double lengthSquared = d.LengthSquared();
    double tMax = tmax;
    if (lengthSquared > 0) {
        double dt = Dot(Abs(d), oError) / lengthSquared;
        o += d * dt;
        tMax -= dt;
    }
    tmax = tMax;
    return ray(o, d, r.time);
}*/

/*template <typename T>
inline ray Transform::operator()(const ray& r, Vector3f* oError,
    Vector3f* dError, double& tmax, double& tmin) const {
    Point3f o = (*this)(r.origin(), oError);
    Vector3f d = (*this)(r.direction(), dError);
    double tMax = tmax;
    Float lengthSquared = d.LengthSquared();
    if (lengthSquared > 0) {
        Float dt = Dot(Abs(d), *oError) / lengthSquared;
        o += d * dt;
        //        tMax -= dt;
    }
    return ray(o, d, r.time);

}*/
/*ray Transform::ConvertRay(const ray& r, Vector3f* oError,
    Vector3f* dError, double& tmax, double& tmin) const{
    Point3f o = (*this)(r.origin(), oError);
    Vector3f d = (*this)(r.direction(), dError);
    double tMax = tmax;
    Float lengthSquared = d.LengthSquared();
    if (lengthSquared > 0) {
        Float dt = Dot(Abs(d), *oError) / lengthSquared;
        o += d * dt;
        //        tMax -= dt;
    }
    return ray(o, d, r.time);
}*/

/*template <typename T>
inline Normal3<T> Transform::operator()(const Normal3<T>& n) const {
    T x = n.x, y = n.y, z = n.z;
    return Normal3<T>(mInv.m[0][0] * x + mInv.m[1][0] * y + mInv.m[2][0] * z,
        mInv.m[0][1] * x + mInv.m[1][1] * y + mInv.m[2][1] * z,
        mInv.m[0][2] * x + mInv.m[1][2] * y + mInv.m[2][2] * z);
}

inline Ray Transform::operator()(const Ray& r) const {
    Vector3f oError;
    Point3f o = (*this)(r.o, &oError);
    Vector3f d = (*this)(r.d);
    // Offset ray origin to edge of error bounds and compute _tMax_
    double lengthSquared = d.LengthSquared();
    double tMax = r.tMax;
    if (lengthSquared > 0) {
        double dt = Dot(Abs(d), oError) / lengthSquared;
        o += d * dt;
        tMax -= dt;
    }
    return Ray(o, d, r.time);
}

inline RayDifferential Transform::operator()(const RayDifferential& r) const {
    Ray tr = (*this)(Ray(r));
    RayDifferential ret(tr.o, tr.d, tr.tMax, tr.time);
    ret.hasDifferentials = r.hasDifferentials;
    ret.rxOrigin = (*this)(r.rxOrigin);
    ret.ryOrigin = (*this)(r.ryOrigin);
    ret.rxDirection = (*this)(r.rxDirection);
    ret.ryDirection = (*this)(r.ryDirection);
    return ret;
}
*/
template <typename T>
inline Point3<T> Transform::operator()(const Point3<T>& p,
    Vector3<T>* pError) const {
    T x = p.x, y = p.y, z = p.z;
    // Compute transformed coordinates from point _pt_
    T xp = (m.m[0][0] * x + m.m[0][1] * y) + (m.m[0][2] * z + m.m[0][3]);
    T yp = (m.m[1][0] * x + m.m[1][1] * y) + (m.m[1][2] * z + m.m[1][3]);
    T zp = (m.m[2][0] * x + m.m[2][1] * y) + (m.m[2][2] * z + m.m[2][3]);
    T wp = (m.m[3][0] * x + m.m[3][1] * y) + (m.m[3][2] * z + m.m[3][3]);

    // Compute absolute error for transformed point
    T xAbsSum = (std::abs(m.m[0][0] * x) + std::abs(m.m[0][1] * y) +
        std::abs(m.m[0][2] * z) + std::abs(m.m[0][3]));
    T yAbsSum = (std::abs(m.m[1][0] * x) + std::abs(m.m[1][1] * y) +
        std::abs(m.m[1][2] * z) + std::abs(m.m[1][3]));
    T zAbsSum = (std::abs(m.m[2][0] * x) + std::abs(m.m[2][1] * y) +
        std::abs(m.m[2][2] * z) + std::abs(m.m[2][3]));
    *pError = gamma(3) * Vector3<T>(xAbsSum, yAbsSum, zAbsSum);
    assert(wp!= 0);
    if (wp == 1)
        return Point3<T>(xp, yp, zp);
    else
        return Point3<T>(xp, yp, zp) / wp;
}

template <typename T>
inline Point3<T> Transform::operator()(const Point3<T>& pt,
    const Vector3<T>& ptError,
    Vector3<T>* absError) const {
    T x = pt.x, y = pt.y, z = pt.z;
    T xp = (m.m[0][0] * x + m.m[0][1] * y) + (m.m[0][2] * z + m.m[0][3]);
    T yp = (m.m[1][0] * x + m.m[1][1] * y) + (m.m[1][2] * z + m.m[1][3]);
    T zp = (m.m[2][0] * x + m.m[2][1] * y) + (m.m[2][2] * z + m.m[2][3]);
    T wp = (m.m[3][0] * x + m.m[3][1] * y) + (m.m[3][2] * z + m.m[3][3]);
    absError->x =
        (gamma(3) + (T)1) *
        (std::abs(m.m[0][0]) * ptError.x + std::abs(m.m[0][1]) * ptError.y +
            std::abs(m.m[0][2]) * ptError.z) +
        gamma(3) * (std::abs(m.m[0][0] * x) + std::abs(m.m[0][1] * y) +
            std::abs(m.m[0][2] * z) + std::abs(m.m[0][3]));
    absError->y =
        (gamma(3) + (T)1) *
        (std::abs(m.m[1][0]) * ptError.x + std::abs(m.m[1][1]) * ptError.y +
            std::abs(m.m[1][2]) * ptError.z) +
        gamma(3) * (std::abs(m.m[1][0] * x) + std::abs(m.m[1][1] * y) +
            std::abs(m.m[1][2] * z) + std::abs(m.m[1][3]));
    absError->z =
        (gamma(3) + (T)1) *
        (std::abs(m.m[2][0]) * ptError.x + std::abs(m.m[2][1]) * ptError.y +
            std::abs(m.m[2][2]) * ptError.z) +
        gamma(3) * (std::abs(m.m[2][0] * x) + std::abs(m.m[2][1] * y) +
            std::abs(m.m[2][2] * z) + std::abs(m.m[2][3]));
    assert(wp!= 0);
    if (wp == 1.)
        return Point3<T>(xp, yp, zp);
    else
        return Point3<T>(xp, yp, zp) / wp;
}

template <typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& v,
    Vector3<T>* absError) const {
    T x = v.x, y = v.y, z = v.z;
    absError->x =
        gamma(3) * (std::abs(m.m[0][0] * v.x) + std::abs(m.m[0][1] * v.y) +
            std::abs(m.m[0][2] * v.z));
    absError->y =
        gamma(3) * (std::abs(m.m[1][0] * v.x) + std::abs(m.m[1][1] * v.y) +
            std::abs(m.m[1][2] * v.z));
    absError->z =
        gamma(3) * (std::abs(m.m[2][0] * v.x) + std::abs(m.m[2][1] * v.y) +
            std::abs(m.m[2][2] * v.z));
    return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
        m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
        m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

template <typename T>
inline Vector3<T> Transform::operator()(const Vector3<T>& v,
    const Vector3<T>& vError,
    Vector3<T>* absError) const {
    T x = v.x, y = v.y, z = v.z;
    absError->x =
        (gamma(3) + (T)1) *
        (std::abs(m.m[0][0]) * vError.x + std::abs(m.m[0][1]) * vError.y +
            std::abs(m.m[0][2]) * vError.z) +
        gamma(3) * (std::abs(m.m[0][0] * v.x) + std::abs(m.m[0][1] * v.y) +
            std::abs(m.m[0][2] * v.z));
    absError->y =
        (gamma(3) + (T)1) *
        (std::abs(m.m[1][0]) * vError.x + std::abs(m.m[1][1]) * vError.y +
            std::abs(m.m[1][2]) * vError.z) +
        gamma(3) * (std::abs(m.m[1][0] * v.x) + std::abs(m.m[1][1] * v.y) +
            std::abs(m.m[1][2] * v.z));
    absError->z =
        (gamma(3) + (T)1) *
        (std::abs(m.m[2][0]) * vError.x + std::abs(m.m[2][1]) * vError.y +
            std::abs(m.m[2][2]) * vError.z) +
        gamma(3) * (std::abs(m.m[2][0] * v.x) + std::abs(m.m[2][1] * v.y) +
            std::abs(m.m[2][2] * v.z));
    return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
        m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
        m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
}

/*inline Ray Transform::operator()(const Ray& r, Vector3f* oError,
    Vector3f* dError) const {
    Point3f o = (*this)(r.o, oError);
    Vector3f d = (*this)(r.d, dError);
    double tMax = r.tMax;
    double lengthSquared = d.LengthSquared();
    if (lengthSquared > 0) {
        double dt = Dot(Abs(d), *oError) / lengthSquared;
        o += d * dt;
        //        tMax -= dt;
    }
    return Ray(o, d, tMax, r.time);
}

inline Ray Transform::operator()(const Ray& r, const Vector3f& oErrorIn,
    const Vector3f& dErrorIn, Vector3f* oErrorOut,
    Vector3f* dErrorOut) const {
    Point3f o = (*this)(r.o, oErrorIn, oErrorOut);
    Vector3f d = (*this)(r.d, dErrorIn, dErrorOut);
    double tMax = r.tMax;
    double lengthSquared = d.LengthSquared();
    if (lengthSquared > 0) {
        double dt = Dot(Abs(d), *oErrorOut) / lengthSquared;
        o += d * dt;
        //        tMax -= dt;
    }
    return Ray(o, d, tMax, r.time);
}*/

// AnimatedTransform Declarations
/*class AnimatedTransform {
public:
    // AnimatedTransform Public Methods
    AnimatedTransform(const Transform* startTransform, double startTime,
        const Transform* endTransform, double endTime);
    static void Decompose(const Matrix4x4& m, Vector3f* T, Quaternion* R,
        Matrix4x4* S);
    void Interpolate(double time, Transform* t) const;
    //Ray operator()(const Ray& r) const;
    //RayDifferential operator()(const RayDifferential& r) const;
    Point3f operator()(double time, const Point3f& p) const;
    Vector3f operator()(double time, const Vector3f& v) const;
    bool HasScale() const {
        return startTransform->HasScale() || endTransform->HasScale();
    }
    Bounds3f MotionBounds(const Bounds3f& b) const;
    Bounds3f BoundPointMotion(const Point3f& p) const;

private:
    // AnimatedTransform Private Data
    const Transform* startTransform, * endTransform;
    const double startTime, endTime;
    const bool actuallyAnimated;
    Vector3f T[2];
    Quaternion R[2];
    Matrix4x4 S[2];
    bool hasRotation;
    struct DerivativeTerm {
        DerivativeTerm() {}
        DerivativeTerm(double c, double x, double y, double z)
            : kc(c), kx(x), ky(y), kz(z) {}
        double kc, kx, ky, kz;
        double Eval(const Point3f& p) const {
            return kc + kx * p.x + ky * p.y + kz * p.z;
        }
    };
    DerivativeTerm c1[3], c2[3], c3[3], c4[3], c5[3];
};*/

class Interval {
public:
    // Interval Public Methods
    Interval(double v) : low(v), high(v) {}
    Interval(double v0, double v1)
        : low(std::min(v0, v1)), high(std::max(v0, v1)) {}
    Interval operator+(const Interval& i) const {
        return Interval(low + i.low, high + i.high);
    }
    Interval operator-(const Interval& i) const {
        return Interval(low - i.high, high - i.low);
    }
    Interval operator*(const Interval& i) const {
        return Interval(std::min(std::min(low * i.low, high * i.low),
            std::min(low * i.high, high * i.high)),
            std::max(std::max(low * i.low, high * i.low),
                std::max(low * i.high, high * i.high)));
    }
    double low, high;
};




#endif