/////////////////////////////////////////////////////////////////////////
/// \file my_assert.h
/////////////////////////////////////////////////////////////////////////
#ifndef NDEBUG
    #ifndef     MY_ASSERT_H
        #define MY_ASSERT_H


        #include <stdlib.h>


        /////////////////////////////////////////////////////////////////////////
        /// @brief Function for output assert error and exit.
        /// @param[in] expr Expression.
        /// @param[in] expr_string Expression in the form of string.
        /// @param[in] date Date of error.
        /// @param[in] line Error line.
        /// @param[in] file Error file.
        /////////////////////////////////////////////////////////////////////////
        void my_func_assert(const bool expr, const char * expr_string, const char * date,
                            const int line, const char * file);
                            
        /////////////////////////////////////////////////////////////////////////
        /// \brief My modified version of assert().
        /////////////////////////////////////////////////////////////////////////
        #define MY_ASSERT(X) my_func_assert(X, #X, __DATE__, __LINE__, __FILE__)   

    #endif // MY_ASSERT_H
#else // NDEBUG
    
    #define NDEBUG

#endif // NDEBUG

