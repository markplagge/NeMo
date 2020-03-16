//
// Created by Mark Plagge on 3/12/20.
//

#ifndef SUPERNEMO_SINGLETON_H
#define SUPERNEMO_SINGLETON_H
namespace neuro_os {

#include <stdio.h>
#include <iostream>

    template <class T>
    class Singleton
    {
    public:
        template <typename... Args>
        static
        T* get_instance(Args... args)
        {
            if (!instance_)
            {
                instance_ = new T(std::forward<Args>(args)...);
            }

            return instance_;
        }

        static
        void destroy_instance()
        {
            delete instance_;
            instance_ = nullptr;
        }

    private:
        static T* instance_;
    };

    template <class T> T*  Singleton<T>::instance_ = nullptr;



}
#endif //SUPERNEMO_SINGLETON_H
