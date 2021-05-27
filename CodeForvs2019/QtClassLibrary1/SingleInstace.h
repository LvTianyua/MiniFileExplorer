#pragma once
#include <mutex>

// 线程安全单实例基类（基类只能保证线程间唯一单例，子类其他数据成员的访问，需要自己维护）
template<class T>
class CSingleInstace
{
private:
    CSingleInstace(const CSingleInstace&) = delete;
    CSingleInstace& operator=(const CSingleInstace&) = delete;

protected:
    CSingleInstace() = default;
    virtual ~CSingleInstace() = default; 

public:
    static T* GetInstance()
    {
        static T* pInstance = nullptr;
        static std::mutex mutex;
        if (pInstance == nullptr)
        {
            std::lock_guard<std::mutex> gMutex(mutex);
            if (pInstance == nullptr)
            {
                pInstance = new T;
            }
        }
        return pInstance;
    }
};