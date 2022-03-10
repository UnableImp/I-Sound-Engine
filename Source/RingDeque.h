//
// Created by zacke on 3/8/2022.
//

#ifndef I_SOUND_ENGINE_RINGDEQUE_H
#define I_SOUND_ENGINE_RINGDEQUE_H

template<typename T>
class RingDeque
{
public:
    RingDeque(int size) : flag(size - 1), frontIndex(0), backIndex(0), sizeIndex(0)
    {
        buffer = new T[size]();
    }

    RingDeque(int size, int startingSize) : flag(size - 1), frontIndex(startingSize), sizeIndex(startingSize), backIndex(0)
    {
        buffer = new T[size]();
    }

    ~RingDeque()
    {
        delete [] buffer;
    }

    T& operator[](int index)
    {
        return buffer[(frontIndex - index) & flag];
    }

    void push_front(T item)
    {
        ++frontIndex;
        ++sizeIndex;
        frontIndex &= flag;
        buffer[frontIndex] = item;
    }

    void push_back(T item)
    {
        --backIndex;
        ++sizeIndex;
        backIndex &= flag;
        buffer[backIndex] = item;
    }

    T front() const
    {
        return buffer[frontIndex];
    }

    T back() const
    {
        return buffer[backIndex];
    }

    void pop_front()
    {
        --frontIndex;
        --sizeIndex;
        frontIndex &= flag;
    }

    void pop_back()
    {
        ++backIndex;
        --sizeIndex;
        backIndex &= flag;
    }

    int size() const
    {
        return sizeIndex;
    }

private:
    T* buffer;
    int frontIndex;
    int backIndex;
    int sizeIndex;
    int flag;
};

#endif //I_SOUND_ENGINE_RINGDEQUE_H
