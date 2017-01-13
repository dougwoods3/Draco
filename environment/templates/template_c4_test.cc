//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   <tpkg>/<class>.cc
 * \author <user>
 * \date   <date>
 * \brief  <start>
 * \note   Copyright (C) 2017 Los Alamos National Security, LLC.
 *         All rights reserved.
 */
//---------------------------------------------------------------------------//

#include "c4/ParallelUnitTest.hh"
#include "ds++/Assert.hh"
#include "ds++/Release.hh"

//---------------------------------------------------------------------------//
// TESTS
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
int main(int argc, char *argv[]) {
    rtt_c4::ParallelUnitTest ut(argc, argv, release);
    try {
        // >>> UNIT TESTS
    }
    UT_EPILOG(ut);
}

//---------------------------------------------------------------------------//
// end of <class>.cc
//---------------------------------------------------------------------------//
