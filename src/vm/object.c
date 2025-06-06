#include "object.h"

#include "objects/boolobject.h"

void me_objects_init() {
    // This is an initializer for some type objects that need runtime initialization.
    me_bool_init();
}