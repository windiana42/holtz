/*
 * wxutil.hpp
 * 
 * utility functions for wx widgets operations
 * 
 * Copyright (c) 2003 by Martin Trautmann (martintrautmann@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#ifndef __HOLTZ_WX_UTIL__
#define __HOLTZ_WX_UTIL__

#include <wx/zipstrm.h>

namespace holtz{
  wxInputStream *get_zip_input_stream(wxString zip_filename, wxString entry_filename);
}
#endif
