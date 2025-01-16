#pragma once

#pragma warning (disable : 4251)

#include <iostream>

// template에는 dllexport를 쓰는게 아니다.
template <typename T>
class List
{
public:
    List()
        : capacity(2)
    { 
        //ReAllocate(2); 
        data = new T[capacity];
        memset(data, 0, sizeof(T) * capacity);
    }

    ~List()
    {
        if (data != nullptr)
        {
            delete[] data;
        }
    }

    void PushBack(const T& value)
    {
        if (size == capacity)
        {
            ReAllocate(capacity * 2);
        }

        data[size] = value;
        size++;
    }

    void PushBack(T&& value)
    {
        if (size == capacity)
        {
            ReAllocate(capacity * 2);
        }

        data[size] = std::move(value);
        size++;
    }

    void Erase(int index)
    {
        // 예외 처리.
        if (index < 0 || index >= size)
        {
            __debugbreak();
        }

        // 삭제한 데이터의 인데스 다음 위치를 한칸 앞으로 모두 이동.
        for (int i = index; i < size - 1; i++)
        {
            data[i] = std::move(data[i + 1]);
        }

        // 삭제 후 크기 줄이기.
        --size;
    }

    int Size() const
    {
        return size;
    }

    int Capacity() const
    {
        return capacity;
    }

    const T& operator[](int index) const
    {
        if (index >= size)
        {
            __debugbreak();
        }

        return data[index];
    }

    T& operator[](int index)
    {
        if (index >= size)
        {
            __debugbreak();
        }

        return data[index];
    }

	T* begin() const
	{
		return data;
	}
	
	T* end() const
	{
		return data + size;
	}

private:
    void ReAllocate(int newCapacity)
    {
        T* newBlock = new T[newCapacity];
        memset(newBlock, 0, sizeof(T) * newCapacity);

        if (newCapacity < size)
        {
            size = newCapacity;
        }

        memcpy(newBlock, data, sizeof(T) * capacity);

        delete[] data;
        data = newBlock;
        capacity = newCapacity;
    }

private:
    T* data = nullptr;
    int size = 0;
    int capacity = 0;
};