#pragma once

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
        if (pInstance == nullptr)
        {
            pInstance = new T;
        }
        return pInstance;
    }
};