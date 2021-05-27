#pragma once
#include <mutex>

// �̰߳�ȫ��ʵ�����ࣨ����ֻ�ܱ�֤�̼߳�Ψһ�����������������ݳ�Ա�ķ��ʣ���Ҫ�Լ�ά����
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