#ifndef STRINGS_H
    #define STRINGS_H

    #include <stdio.h>

    void show_text(const char * text5, const size_t size);
    size_t get_strings_num(char * buffer);
    void get_pointers(const char * buffer, const char * asm_strs[], const size_t strings_num);
    void show_strings(const char * const * strings, const size_t strings_num);
    const char * skip_word(const char * buffer);
    const char * next_word(const char * buffer);
    char * skip_spaces(char * buffer);
    char * skip_no_spaces(char * buffer);
    char * make_file_extension(char * target, const char * file_name, const char * extension);
    size_t get_input(char * buffer, size_t buffer_size);

#endif // STRINGS_H
