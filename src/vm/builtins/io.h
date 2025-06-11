#ifndef __IO_H
#define __IO_H

#include "../object.h"

MEObject* me_io_print(MEObject* self, MEObject** args);
MEObject* me_io_input(MEObject* self, MEObject** args);
MEObject* me_io_open(MEObject* self, MEObject** args);
MEObject* me_io_close(MEObject* self, MEObject** args);
MEObject* me_io_read(MEObject* self, MEObject** args);
MEObject* me_io_write(MEObject* self, MEObject** args);
MEObject* me_io_flush(MEObject* self, MEObject** args);

#endif