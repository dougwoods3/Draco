//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   diagnostics/test/tstProcmon.cc
 * \author Kelly Thompson
 * \date   Monday, Apr 22, 2013, 13:59 pm
 * \brief  Procmon test.
 * \note   Copyright (C) 2013 Los Alamos National Security, LLC.
 *         All rights reserved.
 */
//---------------------------------------------------------------------------//
//! \version $Id: tstProcmon.cc 5830 2011-05-05 19:43:43Z kellyt $
//---------------------------------------------------------------------------//

#include "../Procmon.hh"
#include "c4/ParallelUnitTest.hh"
#include "ds++/Release.hh"
#include <sstream>

using namespace std;
using namespace rtt_diagnostics;

#define PASSMSG(A) ut.passes(A)
#define FAILMSG(A) ut.failure(A)
#define ITFAILS    ut.failure( __LINE__ )

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

void tst_procmon_basic( rtt_dsxx::UnitTest & ut )
{
    std::cout << "Running tst_procmon_basic(ut)..." << std::endl;

    double vmrss_orig(-1.0);
    double vmrss_bigger(-1.0);
    double vmrss_end(-1.0);

    // Run the function and ensure the output has some required fields.
    {
        std::ostringstream msg;
        procmon_resource_print("tst_procmon_use_01",msg);
        std::string line=msg.str();
        std::vector<std::string> tokens = tokenize( line, " \t" );
        if( tokens[4]  != std::string("VmSize") ) ITFAILS;
        if( tokens[8]  != std::string("VmRss")  ) ITFAILS;
        if( tokens[11] != std::string("kB")     ) ITFAILS;

        // Save the VmRSS size
        vmrss_orig = atof( tokens[10].c_str() );
    }

    // Allocate some memory, so we can show that VmRSS increases:    
    size_t const N(100000000);
    double *vdbl = new double[N];
    for( size_t i=0; i<N; ++i )
        vdbl[i] = 3.1415;

    {
        std::ostringstream msg;
        procmon_resource_print("tst_procmon_use_02",msg);
        std::string line=msg.str();
        std::vector<std::string> tokens = tokenize( line, " \t" );

        // Save the VmRSS size
        vmrss_bigger = atof( tokens[10].c_str() );
    }

    // We should be using more memory.
    if( vmrss_bigger < vmrss_orig ) ITFAILS;
    
    for( size_t i=0; i<N; ++i )
        vdbl[i] = 42.5150;
    
    delete [] vdbl;

    // Check that the memory use has decreased:
    {
        std::ostringstream msg;
        procmon_resource_print("tst_procmon_use_03",msg);
        std::string line=msg.str();
        std::vector<std::string> tokens = tokenize( line, " \t" );

        // Save the VmRSS size
        vmrss_end = atof( tokens[10].c_str() );
    }

    if( vmrss_bigger < vmrss_end ) ITFAILS;
    
    if( ut.numFails==0 )
        PASSMSG("No failures in tst_procmon_basic.");
    return;
}
//---------------------------------------------------------------------------//

void tst_procmon_macro( rtt_dsxx::UnitTest & ut )
{
    std::cout << "\nRunning tst_procmon_macro(ut)..." << std::endl;
    std::ostringstream msg;

    // if USE_PROCMON is OFF, this will do nothing.  I'm not sure how to test
    // this. 
    PROCMON_REPORT("tst_procmon_use_02");
    
    if( ut.numFails==0 )
        PASSMSG("No failures in tst_procmon_macro.");
    return;
}

//---------------------------------------------------------------------------//

int main(int argc, char *argv[])
{
    rtt_c4::ParallelUnitTest ut( argc, argv, rtt_dsxx::release );
    try
    {
        // >>> UNIT TESTS
        tst_procmon_basic(ut);
        tst_procmon_macro(ut);
    }
    UT_EPILOG(ut);
}   

//---------------------------------------------------------------------------//
// end of tstProcmon.cc
//---------------------------------------------------------------------------//