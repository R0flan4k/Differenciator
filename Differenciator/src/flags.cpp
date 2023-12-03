#include <stdio.h>
#include <string.h>

#include "cmd_input.h"
#include "my_assert.h"
#include "flags.h"

char * SOURCE_FILE_NAME = NULL;
char * * cmd_input = NULL;

CmdLineArg DIFFERENCIATOR_SOURCE_FILE = {
    .name =          "--source",
    .num_of_param =  1,
    .flag_function = set_differenciator_source_file_name_flag,
    .argc_number =   0,
    .help =          "--source *file name*",
    .is_mandatory = false,
};

CmdLineArg * FLAGS[] = {&DIFFERENCIATOR_SOURCE_FILE};
size_t FLAGS_ARRAY_SIZE = sizeof(FLAGS) / sizeof(FLAGS[0]);


void show_error_message(const char * program_name)
{
    printf("Error. Please, use %s %s\n", program_name, DIFFERENCIATOR_SOURCE_FILE.help);
}

void set_differenciator_source_file_name_flag()
{
    SOURCE_FILE_NAME = cmd_input[DIFFERENCIATOR_SOURCE_FILE.argc_number + 1];
}
