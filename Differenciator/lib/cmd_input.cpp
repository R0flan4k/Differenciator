#include <stdio.h>
#include <string.h>

#include "cmd_input.h"
#include "my_assert.h"
#include "flags.h"

bool check_cmd_input(int argc, char * * argv)
{
    MY_ASSERT(argv != nullptr);

    cmd_input = argv;

    const char * program_name = argv[0];

    for (size_t i = 0; i < FLAGS_ARRAY_SIZE; i++)
    {
        for (int j = 1; j < argc; j++)
        {
            if (strcmp(FLAGS[i]->name, argv[j]) == 0)
            {
                if (argc > j + FLAGS[i]->num_of_param)
                {
                    FLAGS[i]->argc_number = j;

                    FLAGS[i]->flag_function();

                    FLAGS[i]->is_mandatory = true;
                }
                else
                {
                    printf("Error. Please, use %s %s\n", program_name, FLAGS[i]->help);
                    return false;
                }
            }
        }
    }

    for (size_t i = 0; i < FLAGS_ARRAY_SIZE; i++)
    {
        if (!FLAGS[i]->is_mandatory)
        {
            show_error_message(program_name);
            return false;
        }
    }


    return true;
}
