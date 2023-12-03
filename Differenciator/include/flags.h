#ifndef FLAGS_H
    #define FLAGS_H

    #include "cmd_input.h"

    extern CmdLineArg DIFFERENCIATOR_SOURCE_FILE;

    extern char * SOURCE_FILE_NAME;

    extern char * * cmd_input;
    extern CmdLineArg * FLAGS[];
    extern size_t FLAGS_ARRAY_SIZE;

    void show_error_message(const char * program_name);
    void set_differenciator_source_file_name_flag(void);

#endif // FLAGS_H
