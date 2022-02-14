//
// Created by zack on 2/9/22.
//

#ifndef I_SOUND_ENGINE_RINGBUFFER_H
#define I_SOUND_ENGINE_RINGBUFFER_H

template<typename T>
class RingBuffer
{
public:
    RingBuffer(int size) : pos(0), count(size), flag(size - 1)
    {
        buffer = new T[size]();
    }

    ~RingBuffer()
    {
        delete [] buffer;
    }

    void put(T item)
    {
        ++pos;
        pos &= flag;
        buffer[pos] = item;
    }

    void set(int index, T item)
    {
        int toSet = pos - index;

        toSet &= flag;

        buffer[toSet] = item;
    }

    T get(int index)
    {
        int toGet = pos - index;

        toGet &= flag;

        return buffer[toGet];
    }

private:
    T* buffer;
    int pos;
    int count;
    int flag;
};

#endif //I_SOUND_ENGINE_RINGBUFFER_H
