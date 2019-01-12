#ifndef __SKIN__

#include "wxholtz.hpp"

Dimensions get_skin_dimensions()
{
  Dimensions ret(field_removed_xpm);

  ret.field_packed_height = 38;
  ret.field_width = 45;
  ret.field_x = 3;

  return ret;
}

#endif
