//----------------------------------*-C++-*----------------------------------//
/*! 
 * \file   RTT_Format_Reader/CellDataIDs.hh
 * \author B.T. Adams
 * \date   Wed Jun 7 10:33:26 2000
 * \brief  Header file for RTT_Format_Reader/CellDataIDs class.
 * \note   Copyright (C) 2016-2017 Los Alamos National Security, LLC.
 *         All rights reserved.
 */
//---------------------------------------------------------------------------//
// $Id$
//---------------------------------------------------------------------------//

#ifndef __RTT_Format_Reader_CellDataIDs_hh__
#define __RTT_Format_Reader_CellDataIDs_hh__

#include "Dims.hh"
#include "ds++/Assert.hh"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace rtt_RTT_Format_Reader {
/*!
 * \brief Controls parsing, storing, and accessing the data specific to the 
 *        cell data ids block of the mesh file.
 */
class CellDataIDs {
  // typedefs
  typedef std::ifstream ifstream;
  typedef std::string string;
  typedef std::vector<string> vector_str;

  const Dims &dims;
  vector_str names;
  vector_str units;

public:
  CellDataIDs(const Dims &dims_)
      : dims(dims_), names(dims.get_ncell_data()),
        units(dims.get_ncell_data()) {}
  ~CellDataIDs() {}

  void readDataIDs(ifstream &meshfile);

private:
  void readKeyword(ifstream &meshfile);
  void readData(ifstream &meshfile);
  void readEndKeyword(ifstream &meshfile);

public:
  /*!
 * \brief Returns the specified cell_data_id nam.
 * \param id_numb cell_data_id index number.
 * \return The cell_data_id name.
 */
  string get_data_id_name(int id_numb) const {
    Insist(id_numb <= dims.get_ncell_data() - 1,
           "Invalid cell data id number!");
    return names[id_numb];
  }
  /*!
 * \brief Returns the units associated with the specified cell_data_id.
 * \param id_numb cell_data_id index number.
 * \return The cell_data_id units.
 */
  string get_data_id_units(int id_numb) const {
    Insist(id_numb <= dims.get_ncell_data() - 1,
           "Invalid cell data id number!");
    return units[id_numb];
  }
};

} // end namespace rtt_RTT_Format_Reader

#endif // __RTT_Format_Reader_CellDataIDs_hh__

//---------------------------------------------------------------------------//
// end of RTT_Format_Reader/CellDataIDs.hh
//---------------------------------------------------------------------------//
