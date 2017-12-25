/**
 *  Copyright (c) 2015 cell-app.com
 *  All rights reserved.
 *
 *  description: allocate object by ngx_palloc
 *  author: fangming
 *  date: 2015.02.05
 *  
 */

#ifndef _OBJECT_PALLOCTOR_H_
#define _OBJECT_PALLOCTOR_H_

#include <typeinfo>
#include <ngx_http.h>

template <typename T>
class ObjectPallocator {
    public:
        static T* allocate(ngx_pool_t* pool)
        {
            void* buf = ngx_palloc(pool, sizeof(T));
            if (!buf) {
                return NULL;
            }
            T* obj = new(buf) T();
            return obj;
        }
        static void free(T *obj)
        {
            if (obj) {
                obj->~T();
            }
        }
    private:
        ObjectPallocator() {}
        ~ObjectPallocator() {}
};

#endif
