//----------------------------------*-C++-*----------------------------------//
/*!
 * \file   viz/Ensight_Translator.t.hh
 * \author Thomas M. Evans
 * \date   Fri Jan 21 16:36:10 2000
 * \brief  Ensight_Translator template definitions.
 * \note   Copyright (C) 2016-2017 Los Alamos National Security, LLC.
 *         All rights reserved.
 */
//---------------------------------------------------------------------------//
// $Id$
//---------------------------------------------------------------------------//

#include <map>
#include <sstream>

namespace rtt_viz {

//---------------------------------------------------------------------------//
// CONSTRUCTOR
//---------------------------------------------------------------------------//
/*!
 * \brief Constructor for Ensight_Translator.
 *
 * This constructor builds an Ensight_Translator.  The behavior of the
 * (existing) ensight dump files is controlled by the overwrite parameter.
 * If this is true, any existing ensight dumps (with the same problem name)
 * will be overwritten.  If overwrite is false then the ensight dumps are
 * appended.  The ensight case files are parsed to get the dump times if
 * overwrite is false.
 *
 * \param prefix std_string giving the name of the problem
 * \param gd_wpath directory where dumps are stored
 * \param vdata_names string field containing vertex data names
 * \param cdata_names string field containing cell data names
 * \param overwrite bool that controls whether an existing ensight
 * directory is to be appended to or overwritten.  If true, overwrites the
 * existing ensight directory.  If false, and the ensight directory exists,
 * the case file is appended to.  In either case, if the ensight directory
 * does not exist it is created.  The default for overwrite is false.
 * \param static_geom optional input that if true, geometry is assumed
 * the same across all calls to Ensight_Translator::ensight_dump.
 * \param binary If true, geometry and variable data files are output in
 * binary format.
 *
 * NOTE: If appending data (\a overwrite is false), then \a binary must
 * be the same value as the first ensight dump.  This class does NOT check
 * for this potential error (yes, it's possible to check and is left for a
 * future exercise).
 */
template <typename SSF>
Ensight_Translator::Ensight_Translator(
    const std_string &prefix, const std_string &gd_wpath,
    const SSF &vdata_names, const SSF &cdata_names, const bool overwrite,
    const bool static_geom, const bool binary)
    : d_static_geom(static_geom), d_binary(binary), d_dump_dir(gd_wpath),
      d_num_cell_types(0), d_cell_names(), d_vrtx_cnt(0), d_cell_type_index(),
      d_dump_times(), d_prefix(), d_vdata_names(vdata_names),
      d_cdata_names(cdata_names), d_case_filename(), d_geo_dir(),
      d_vdata_dirs(), d_cdata_dirs(), d_geom_out(), d_cell_out(),
      d_vertex_out() {
  Require(d_dump_times.empty());
  create_filenames(prefix);

  bool graphics_continue = false; // default behavior

  if (!overwrite) {
    // then try to parse the case file.  Case files are always ascii.

    std::ifstream casefile(d_case_filename.c_str());

    if (casefile) {
      // then case file exists, so parse the dump times
      std_string key("number of steps:");
      std_string line;
      int num_steps = 0;

      for (;;) {
        std::getline(casefile, line);
        Insist(casefile.good(),
               "Error getting number of steps from case file!");
        if (line.find(key) == 0) {
          std::istringstream ss(line.substr(key.size()));
          ss >> num_steps;
          //std::cout << "FOUND " << num_steps
          //      << " STEPS " << std::endl;
          break;
        }
      }

      // read next three lines and discard
      std::getline(casefile, line);
      std::getline(casefile, line);
      std::getline(casefile, line);
      Insist(casefile.good(), "Error reading case file!");

      // read the dump_times

      d_dump_times.resize(num_steps);

      for (int i = 0; i < num_steps; ++i) {
        casefile >> d_dump_times[i];
        Insist(casefile.good(), "Error reading dump_times from case file!");
        //std::cout << "   STEP " << i
        //        << " TIME " << d_dump_times[i] << std::endl;
      }

      casefile.close();
      graphics_continue = true;
    }
  }

  initialize(graphics_continue);
}

//---------------------------------------------------------------------------//
// ENSIGHT DUMP PUBLIC INTERFACES
//---------------------------------------------------------------------------//
/*!
 * \brief Do an Ensight dump to disk.
 *
 * Performs an ensight dump in the directory specified in
 * Ensight_Translator::Ensight_Translator().
 *
 * \param icycle time cycle number associated with this dump
 *
 * \param time elapsed problem time
 *
 * \param dt current problem timestep
 *
 * \param ipar IVF field of pointers to vertices.  Dimensioned
 * [0:ncells-1, 0:n_local_vertices_per_cell-1], where
 * n_local_vertices_per_cell is the number of vertices that make up the cell.
 * ipar(i,j) maps the jth+1 vertex number, in the ith+1 cell, to Ensight's
 * "vertex number."  The "vertex number" is in [1:nvertices], so that for
 * example, the corresponding x-coordinate is pt_coor(ipar(i,j)-1, 0).
 *
 * \param iel_type ISF field of Ensight_Cell_Types.  Dimensioned
 * [0:ncells-1].  Each cell in the problem must be associated with a
 * Ensight_Cell_Types enumeration object.
 *
 * \param cell_rgn_index ISF field of region identifiers.  Dimensioned
 * [0:ncells-1].  This matches a region index to each cell in the problem.
 *
 * \param pt_coor FVF field of vertex coordinates. pt_coor is
 * dimensioned [0:nvertices-1, 0:ndim-1].  For each vertex point give the
 * value in the appropriate dimension.
 *
 * \param vrtx_data FVF field of vertex data.  vrtx_data is
 * dimensioned [0:nvertices-1, 0:number of vertex data fields - 1].  The
 * ordering of the second index must match the vdata_names field input
 * argument to Ensight_Translator::Ensight_Translator().  The ordering of the
 * first index must match the vertex ordering from pt_coor.
 *
 * \param cell_data FVF field of cell data.  cell_data is
 * dimensioned [0:ncells-1, 0:number of cell data fields - 1].  The ordering
 * of the second index must match the cdata_names field input argument
 * to Ensight_Translator::Ensight_Translator().  The ordering of the first
 * index must match the cell ordering from ipar.
 *
 * \param rgn_numbers ISF field of unique region ids.  This has dimensions of
 * the number of unique values found in the cell_rgn_index field.
 *
 * \param rgn_name SSF field of unique region names.  This has the same
 * dimensions and ordering as rgn_numbers.  In summary, rgn_numbers gives a
 * list of the unique region ids in the problem and rgn_name gives a list of
 * the names associated with each region id.
 *
 * \sa \ref Ensight_Translator_strings "Ensight_Translator class" for
 * restrictions on name strings.
 *
 * \sa \ref Ensight_Translator_description "Ensight_Translator class" for
 * information on templated field types.
 *
 * \sa Examples page for more details about how to do Ensight dumps.
 */
template <typename ISF, typename IVF, typename SSF, typename FVF>
void Ensight_Translator::ensight_dump(
    int icycle, double time, double dt, const IVF &ipar_in, const ISF &iel_type,
    const ISF &cell_rgn_index, const FVF &pt_coor_in, const FVF &vrtx_data_in,
    const FVF &cell_data_in, const ISF &rgn_numbers, const SSF &rgn_name) {
  using rtt_viz::Viz_Traits;
  using std::string;
  using std::vector;
  using std::find;

  // >>> PREPARE DATA TO SET ENSIGHT OUTPUT

  // load traits for vector field types
  Viz_Traits<IVF> ipar(ipar_in);
  Viz_Traits<FVF> pt_coor(pt_coor_in);
  Viz_Traits<FVF> vrtx_data(vrtx_data_in);
  Viz_Traits<FVF> cell_data(cell_data_in);

  // define sizes used throughout
  size_t ncells = ipar.nrows();
  size_t nvertices = pt_coor.nrows();
  Remember(size_t nrgn = rgn_name.size(););

  // Check sizes of all data.
  Require(iel_type.size() == ncells);
  Require(cell_rgn_index.size() == ncells);
  Require(cell_data.nrows() == ncells || cell_data.nrows() == 0);
  Require(vrtx_data.nrows() == nvertices || vrtx_data.nrows() == 0);
  Require(rgn_numbers.size() == nrgn);

  // create the parts list
  vector<int>::const_iterator find_location_c;
  vector<int>::iterator find_location;
  vector<int> parts_list;

  for (size_t i = 0; i < ncells; ++i) {
    find_location =
        find(parts_list.begin(), parts_list.end(), cell_rgn_index[i]);

    if (find_location == parts_list.end())
      parts_list.push_back(cell_rgn_index[i]);
  }

  // store the number of parts
  size_t nparts = parts_list.size();

  // create the parts names
  vector<string> part_names;

  for (size_t i = 0; i < nparts; ++i) {
    find_location_c =
        find(rgn_numbers.begin(), rgn_numbers.end(), parts_list[i]);

    if (find_location_c != rgn_numbers.end()) {
      int index = find_location_c - rgn_numbers.begin();
      part_names.push_back(rgn_name[index]);
    } else if (find_location_c == rgn_numbers.end()) {
      Insist(0, "Didn't supply a region name!");
    }
  }

  Insist(parts_list.size() == part_names.size(), "Mismatch on part size!");
  Insist(rgn_name.size() == parts_list.size(), "Mismatch on region size!");

  // create the cells that make up each part

  // vertices_of_part[ipart] is the set of vertex indices that make up part
  // ipart.
  vec_set_int vertices_of_part(nparts);

  // cells_of_type[ipart][itype][i] is the cell index of the i'th cell of
  // type itype in part ipart.
  sf3_int cells_of_type(nparts);
  for (size_t i = 0; i < nparts; ++i)
    cells_of_type[i].resize(d_num_cell_types);

  // Initialize cells_of_type and vertices_of_part.

  for (size_t i = 0; i < ncells; ++i) {
    find_location =
        find(parts_list.begin(), parts_list.end(), cell_rgn_index[i]);

    Check(find_location != parts_list.end());
    Check(iel_type[i] < d_num_cell_types);

    int ipart = find_location - parts_list.begin();

    cells_of_type[ipart][iel_type[i]].push_back(i);

    int n_local_vertices = d_vrtx_cnt[iel_type[i]];

    for (int iv = 0; iv < n_local_vertices; ++iv)
      vertices_of_part[ipart].insert(ipar(i, iv) - 1);
  }

  // Form global cell and vertex indices.  These are the same as their local
  // index, in this case.
  sf_int g_cell_indices(ncells);
  sf_int g_vrtx_indices(nvertices);

  for (size_t i = 0; i < ncells; ++i)
    g_cell_indices[i] = i;

  for (size_t i = 0; i < nvertices; ++i)
    g_vrtx_indices[i] = i;

  // >>> WRITE OUT DATA TO DIRECTORIES

  open(icycle, time, dt);

  for (size_t ipart = 0; ipart < part_names.size(); ipart++) {
    // Load vertices_of_part into a vector.
    set_int &v = vertices_of_part[ipart];
    sf_int vertices;

    for (set_const_iterator iv = v.begin(); iv != v.end(); ++iv)
      vertices.push_back(*iv);

    // write the geometry data
    write_geom(ipart + 1, part_names[ipart], ipar, pt_coor,
               cells_of_type[ipart], vertices, g_vrtx_indices, g_cell_indices);

    // write the vertex data
    write_vrtx_data(ipart + 1, vrtx_data, vertices);

    // write out the cell data
    write_cell_data(ipart + 1, cell_data, cells_of_type[ipart]);
  }

  close();
}

//---------------------------------------------------------------------------//
/*!
 * \brief Write ensight data for a single part.
 *
 * open() must be called before calling this function.
 *
 * Writes data for a single part (or "region," in ensight_dump parlance).
 *
 * \param part_num A part number to be used by Ensight.  Must be positive.
 *
 * \param part_name A name for the part.
 *
 * \param ipar See ensight_dump().
 *
 * \param iel_type See ensight_dump().
 *
 * \param pt_coor See ensight_dump().
 *
 * \param vrtx_data See ensight_dump().
 *
 * \param cell_data See ensight_dump().
 *
 * \param g_vrtx_indices.  Global vertex indices.  These are used by Ensight
 * as integer labels for each vertex.  Specifically, let i access the i'th
 * value in \a vrtx_data.  Then \a g_vrtx_indices[i] gives the "global index"
 * (or label index) for i.  This is referred to as "global" because \a
 * g_vrtx_indices can be used to map each processor's local indices to global
 * index space.
 *
 * \param g_cell_indices. Global cell indices.  Analogous to \a
 * g_vrtx_indices, but for cell indices.
 *
 * \sa \ref Ensight_Translator_strings "Ensight_Translator class" for
 * restrictions on name strings.
 *
 * \sa \ref Ensight_Translator_description "Ensight_Translator class" for
 * information on templated field types.
 *
 * \sa Examples page for more details about how to do Ensight dumps.
 */
template <typename ISF, typename IVF, typename FVF>
void Ensight_Translator::write_part(int part_num, const std_string &part_name,
                                    const IVF &ipar_in, const ISF &iel_type,
                                    const FVF &pt_coor_in,
                                    const FVF &vrtx_data_in,
                                    const FVF &cell_data_in,
                                    const ISF &g_vrtx_indices,
                                    const ISF &g_cell_indices) {
  Require(part_num > 0);

  using rtt_viz::Viz_Traits;
  using std::string;
  using std::vector;
  using std::find;

  // load traits for vector field types
  Viz_Traits<IVF> ipar(ipar_in);
  Viz_Traits<FVF> pt_coor(pt_coor_in);
  Viz_Traits<FVF> vrtx_data(vrtx_data_in);
  Viz_Traits<FVF> cell_data(cell_data_in);

  // define sizes used throughout
  size_t ncells = ipar.nrows();
  size_t nvertices = pt_coor.nrows();

  // Check sizes of all data.
  Require(iel_type.size() == ncells);
  Require(cell_data.nrows() == ncells || cell_data.nrows() == 0);
  Require(vrtx_data.nrows() == nvertices || vrtx_data.nrows() == 0);
  Require(g_vrtx_indices.size() == nvertices);
  Require(g_cell_indices.size() == ncells);

  // cells_of_type[itype][i] is the cell index of the i'th cell of
  // type itype.
  sf2_int cells_of_type(d_num_cell_types);

  for (size_t i = 0; i < ncells; ++i) {
    Check(iel_type[i] < d_num_cell_types);
    cells_of_type[iel_type[i]].push_back(i);
  }

  // All vertices are output in this case.
  sf_int vertices(nvertices);
  for (size_t i = 0; i < nvertices; ++i)
    vertices[i] = i;

  // >>> WRITE OUT DATA TO DIRECTORIES

  // write the geometry data
  write_geom(part_num, part_name, ipar, pt_coor, cells_of_type, vertices,
             g_vrtx_indices, g_cell_indices);

  // write the vertex data
  write_vrtx_data(part_num, vrtx_data, vertices);

  // write out the cell data
  write_cell_data(part_num, cell_data, cells_of_type);
}

//---------------------------------------------------------------------------//
// ENSIGHT DATA OUTPUT FUNCTIONS (PRIVATE)
//---------------------------------------------------------------------------//
/*!
 * \brief Write out data to ensight geometry file.
 */
template <typename IVF, typename FVF, typename ISF>
void Ensight_Translator::write_geom(const int part_num,
                                    const std_string &part_name,
                                    const rtt_viz::Viz_Traits<IVF> &ipar,
                                    const rtt_viz::Viz_Traits<FVF> &pt_coor,
                                    const sf2_int &cells_of_type,
                                    const sf_int &vertices,
                                    const ISF &g_vrtx_indices,
                                    const ISF &g_cell_indices) {
  // Return if the geometry is static and we've already dumped the
  // geometry.
  if (d_static_geom && d_dump_times.size() > 1) {
    return;
  }

  Insist(d_geom_out.is_open(),
         "Geometry file not open.  Must call open() before write_part().");

  size_t ndim = pt_coor.ncols(0);
  size_t nvertices = vertices.size();

  // output part number and names
  d_geom_out << "part" << endl;
  d_geom_out << part_num << endl;
  d_geom_out << part_name << endl;
  d_geom_out << "coordinates" << endl;
  d_geom_out << nvertices << endl; // #vertices in this part

  // output the global vertex indices and form ens_vertex.
  // Enight demands that vertices be numbered from 1 to the number of
  // vertices *for this part* (nvertices).  Argghhh.
  // ens_vertex maps our local vertex index to a vertex in [1,nvertices].

  std::map<int, int> ens_vertex;
  for (size_t i = 0; i < nvertices; ++i) {
    d_geom_out << g_vrtx_indices[vertices[i]] << endl;

    // add 1 because ipar and Ensight demand indices that start at 1.
    ens_vertex[vertices[i] + 1] = i + 1;
  }

  // output the coordinates
  for (size_t idim = 0; idim < ndim; idim++)
    for (size_t i = 0; i < nvertices; ++i)
      d_geom_out << pt_coor(vertices[i], idim) << endl;

  // ensight expects coordinates for three dimensions, so fill any
  // remaining dimensions with zeroes
  double zero = 0.0;
  for (size_t idim = ndim; idim < 3; idim++)
    for (size_t i = 0; i < nvertices; ++i)
      d_geom_out << zero << endl;

  // for each cell type, dump the local vertex indices for each cell.
  for (int type = 0; type < d_num_cell_types; type++) {
    const sf_int &c = cells_of_type[type];
    const size_t num_elem = c.size();

    if (num_elem > 0) {
      d_geom_out << d_cell_names[type] << endl;
      d_geom_out << num_elem << endl;

      for (size_t i = 0; i < num_elem; ++i)
        d_geom_out << g_cell_indices[c[i]] << endl;

      for (size_t i = 0; i < num_elem; ++i) {
        Check(static_cast<int>(ipar.ncols(c[i])) == d_vrtx_cnt[type]);
        for (int j = 0; j < d_vrtx_cnt[type]; j++)
          d_geom_out << ens_vertex[ipar(c[i], j)];
        d_geom_out << endl;
      }
    }
  } // done looping over cell types
}

//---------------------------------------------------------------------------//
/*!
 * \brief Write out data to ensight vertex data.
 */
template <typename FVF>
void Ensight_Translator::write_vrtx_data(
    const int part_num, const rtt_viz::Viz_Traits<FVF> &vrtx_data,
    const sf_int &vertices) {
  if (vrtx_data.nrows() == 0)
    return;

  size_t nvertices = vertices.size();
  size_t ndata = vrtx_data.ncols(0);

  std::string err = "Vertex data files not open."
                    "  Must call open() before write_part().";
  Insist(d_vertex_out.size() == static_cast<size_t>(ndata), err.c_str());

  // loop over all vertex data fields and write out data for each field
  for (size_t nvd = 0; nvd < ndata; nvd++) {
    Ensight_Stream &vout = *d_vertex_out[nvd];

    Insist(vout.is_open(), err.c_str());

    vout << "part" << endl;
    vout << part_num << endl;
    vout << "coordinates" << endl;

    for (size_t i = 0; i < nvertices; ++i)
      vout << vrtx_data(vertices[i], nvd) << endl;
  }
}

//---------------------------------------------------------------------------//
/*!
 * \brief Write out data to ensight cell data.
 */
template <typename FVF>
void Ensight_Translator::write_cell_data(
    const int part_num, const rtt_viz::Viz_Traits<FVF> &cell_data,
    const sf2_int &cells_of_type) {
  if (cell_data.nrows() == 0)
    return;

  int ndata = cell_data.ncols(0);

  std::string err = "Cell data files not open."
                    "  Must call open() before write_part().";

  Insist(d_cell_out.size() == static_cast<size_t>(ndata), err.c_str());

  // loop over all cell data fields and write out data for each field
  for (int ncd = 0; ncd < ndata; ncd++) {
    Ensight_Stream &cellout = *d_cell_out[ncd];

    Insist(cellout.is_open(), err.c_str());

    cellout << "part" << endl;
    cellout << part_num << endl;

    // loop over ensight cell types
    for (int type = 0; type < d_num_cell_types; type++) {
      const sf_int &c = cells_of_type[type];

      size_t num_elem = c.size();

      // print out data if there are cells of this type
      if (num_elem > 0) {
        // printout cell-type name
        cellout << d_cell_names[type] << endl;

        // print out data
        for (size_t i = 0; i < num_elem; ++i)
          cellout << cell_data(c[i], ncd) << endl;
      }
    }
  }
}

} // end of rtt_viz

//---------------------------------------------------------------------------//
// end of viz/Ensight_Translator.t.hh
//---------------------------------------------------------------------------//
