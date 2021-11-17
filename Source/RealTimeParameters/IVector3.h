//
// Created by zack on 11/11/21.
//

#ifndef I_SOUND_ENGINE_IVECTOR3_H
#define I_SOUND_ENGINE_IVECTOR3_H

struct IVector3
{
    float x,y,z;
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
};



#endif //I_SOUND_ENGINE_IVECTOR3_H
