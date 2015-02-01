//---------------------------------------------------------------------------------
//
//  LIBICC++  
//  Copyright Marco Oman 2015
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
#include <iostream>

#include "test_registrar.h"


int main(int argc, char *argv[])
{
    void (*test)(void) = nullptr;
    size_t done = 0;

    try
    {
        for (int i = 1; i < argc; i++)
            if (argv[i][0] != '-')
            {
                if (funct_registrar_t::get(argv[i], test))
                {
                    test();
                    done++;
                }
            }
        // no specific test required, run all of them
        if (test == nullptr)
            for (funct_registrar_t::entry_t entry : funct_registrar_t::entries())
            {
                std::cout << "Running " << entry.first << std::endl;
                entry.second();
                done++;
            }
    }
    catch (std::exception &exc)
    {
        std::cout << "Exception:" << exc.what() << std::endl ;
    }
    std::cout << "Done " << done << " tests" << std::endl;
    return 0;
}