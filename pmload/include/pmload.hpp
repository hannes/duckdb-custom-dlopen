#pragma once


namespace pmload {

    static void *my_dlsym(void *lib, const char *name, bool add_underscore = true, bool recurse = true);
    static void *my_dlopen(const char *file, int flags);


    } // namespace pmload
