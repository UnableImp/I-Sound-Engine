//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_IVECTOR3_H
#define I_SOUND_ENGINE_IVECTOR3_H

#include <cmath>
#include <iostream>
#include <algorithm>

struct IVector3
{
    float x,y,z;

    friend std::ostream& operator<<(std::ostream& os, const IVector3& vec);

    IVector3 operator-(const IVector3& rhs) const
    {
        IVector3 out;
        out.x = x - rhs.x;
        out.y = y - rhs.y;
        out.z = z - rhs.z;
        return out;
    }

    IVector3 operator+(const IVector3& rhs) const
    {
        IVector3 out;
        out.x = x + rhs.x;
        out.y = y + rhs.y;
        out.z = z + rhs.z;
        return out;
    }

    IVector3 operator*(float rhs) const
    {
        IVector3 out;
        out.x = x * rhs;
        out.y = y * rhs;
        out.z = z * rhs;
        return out;
    }

    static IVector3 Cross(const IVector3& lhs, const IVector3& rhs)
    {
        IVector3 out;
        out.x = (lhs.y * rhs.z) - (lhs.z * rhs.y);
        out.y = (lhs.z * rhs.x) - (lhs.x * rhs.z);
        out.z = (lhs.x * rhs.y) - (lhs.y * rhs.x);
        return out;
    }

    /*!
     * Caclulates the normalized vector squared
     * @return squard normalized vector
     */
    IVector3 sqrtNormalized() const
    {
        IVector3 out;
        float length = (x * x) + (y * y) + (z * z);
        out.x = x / length;
        out.y = y / length;
        out.z = z / length;
        return out;
    }

    IVector3 Normalized() const
    {
        IVector3 out;
        float length = std::sqrt((x * x) + (y * y) + (z * z));
        out.x = x / length;
        out.y = y / length;
        out.z = z / length;
        return out;
    }

    static float sqrtDistance(const IVector3& lhs, const IVector3& rhs)
    {
        return ((lhs.x - rhs.x) * (lhs.x - rhs.x)) +
               ((lhs.y - rhs.y) * (lhs.y - rhs.y)) +
               ((lhs.z - rhs.z) * (lhs.z - rhs.z)) ;

    }

    static float Distance(const IVector3& lhs, const IVector3& rhs)
    {
        return std::sqrt(((lhs.x - rhs.x) * (lhs.x - rhs.x)) +
                         ((lhs.y - rhs.y) * (lhs.y - rhs.y)) +
                         ((lhs.z - rhs.z) * (lhs.z - rhs.z)));

    }

    static float Dot(const IVector3& lhs, const IVector3& rhs)
    {
        return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
    }

    float Mag() const
    {
        return std::sqrt((x * x) + (y * y) + (z * z));
    }

    static float Angle(const IVector3& lhs, const IVector3& rhs)
    {
//        float mag = lhs.Mag() * rhs.Mag();
//        float dot = IVector3::Dot(lhs, rhs);
//        float out = dot / mag;
//        float angle = std::acos(out);
//        return angle;
        return std::acos(std::clamp(IVector3::Dot(lhs, rhs) / (lhs.Mag() * rhs.Mag()), -1.0f, 1.0f));
    }

};

inline std::ostream& operator<<(std::ostream& os, const IVector3& vec)
{
    os << "[" << vec.x << ", " << vec.y << ", " << vec.z << "]";
    return os;
}

#endif //I_SOUND_ENGINE_IVECTOR3_H
