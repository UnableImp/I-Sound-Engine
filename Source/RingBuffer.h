//
// Created by zack on 2/9/22.
//

#ifndef I_SOUND_ENGINE_RINGBUFFER_H
#define I_SOUND_ENGINE_RINGBUFFER_H

template<typename T>
class RingBuffer
{
public:
    RingBuffer(int size) : pos(0), count(size)
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
        if(pos >= count)
            pos = 0;
        buffer[pos] = item;
    }

    void set(int index, T item)
    {
        int toSet = pos - index;

        if(toSet < 0)
            toSet += count;

        buffer[toSet] = item;
    }

    T get(int index)
    {
        int toGet = pos - index;

        if(toGet < 0)
            toGet += count;

        return buffer[toGet];
    }

private:
    T* buffer;
    int pos;
    int count;
};

#endif //I_SOUND_ENGINE_RINGBUFFER_H
