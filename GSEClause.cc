
// -*- C++ -*-

// (c) COPYRIGHT URI/MIT 1999
// Please read the full copyright statement in the file COPYRIGHT.
//
// Authors:
//      jhrg,jimg       James Gallagher (jgallagher@gso.uri.edu)

// The Grid Selection Expression Clause class.

#ifdef _GNUG_
#pragma implementation
#endif

#include "config_dap.h"

static char id[] not_used = {"$Id: GSEClause.cc,v 1.7 2002/06/03 22:21:15 jimg Exp $"};

#include <iostream>
#include <strstream>

#include <assert.h>
#include <Pix.h>

#include "dods-datatypes.h"
#include "Error.h"
#include "InternalErr.h"
#include "debug.h"
#include "GSEClause.h"
#include "parser.h"
#include "gse.tab.h"

int gse_parse(void *arg);
void gse_restart(FILE *in);

// Glue routines declared in gse.lex
void gse_switch_to_buffer(void *new_buffer);
void gse_delete_buffer(void * buffer);
void *gse_string(const char *yy_str);


// Private methods

GSEClause::GSEClause()
{
  throw InternalErr(__FILE__, __LINE__, "default ctor called for GSEClause");
}

GSEClause::GSEClause(const GSEClause &param)
{
  throw InternalErr(__FILE__, __LINE__, "copy ctor called for GSEClause");
}

GSEClause &GSEClause::operator=(GSEClause &rhs)
{
  throw InternalErr(__FILE__, __LINE__, "assigment called for GSEClause");
}

template<class T>
static bool
compare(T elem, relop op, double value)
{
    switch (op) {
      case dods_greater_op:
	return elem > value;
      case dods_greater_equal_op:
	return elem >= value;
      case dods_less_op:
	return elem < value;
      case dods_less_equal_op:
	return elem <= value;
      case dods_equal_op:
	return elem == value;
      case dods_not_equal_op:
	return elem != value;
      case dods_nop_op:
	throw Error(malformed_expr, "Attempt to use NOP in Grid selection.");
      default:
	throw Error(malformed_expr, "Unknown relational operator in Grid selection.");
    }
}

#ifndef WIN32
template<class T>
void
GSEClause::set_map_min_max_value(T min, T max)
{
    DBG(cerr << "Inside set map min max value " << min << ", " << max << endl);
    std::ostrstream oss1;
    oss1 << min << std::ends;
    d_map_min_value = oss1.str();
    oss1.freeze(0);

    std::ostrstream oss2;
    oss2 << max << std::ends;
    d_map_max_value = oss2.str();
    oss2.freeze(0);
}
#endif

#ifndef WIN32
template<class T>
void
GSEClause::set_start_stop()
{
    // Read the byte array, scan, set start and stop.
    T *vals = 0;
    d_map->buf2val((void **)&vals);

    // Set the map's max and min values for use in error messages (it's a lot
    // easier to do here, now, than later... 9/20/2001 jhrg)
    set_map_min_max_value<T>(vals[d_start], vals[d_stop]);

    int i = d_start;
    int end = d_stop;
    while(i <= end && !compare<T>(vals[i], d_op1, d_value1))
	i++;

    d_start = i;

    i = end;
    while(i >= 0 && !compare<T>(vals[i], d_op1, d_value1))
	i--;
    d_stop = i;

    // Every clause must have one operator but the second is optional since
    // the more complex for of a clause is optional.
    if (d_op2 != dods_nop_op) {
	int i = d_start;
	int end = d_stop;
	while(i <= end && !compare<T>(vals[i], d_op2, d_value2))
	    i++;

	d_start = i;

	i = end;
	while(i >= 0 && !compare<T>(vals[i], d_op2, d_value2))
	    i--;

	d_stop = i;
    }
}
#endif

void
GSEClause::compute_indices()
{
#ifdef WIN32
    //  Allows us to get around short-comming with MS Visual C++ 6.0
    //  templates
    char dummy;

    switch (d_map->var()->type()) {
      case dods_byte_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_int16_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_uint16_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_int32_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_uint32_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_float32_c:
	set_start_stop((char *)(&dummy));
	break;
      case dods_float64_c:
	set_start_stop((char *)(&dummy));
	break;
    default:
	throw Error(malformed_expr, 
             "Grid selection using non-numeric map vectors is not supported");
    }
#else
    switch (d_map->var()->type()) {
      case dods_byte_c:
	set_start_stop<dods_byte>();
	break;
      case dods_int16_c:
	set_start_stop<dods_int16>();
	break;
      case dods_uint16_c:
	set_start_stop<dods_uint16>();
	break;
      case dods_int32_c:
	set_start_stop<dods_int32>();
	break;
      case dods_uint32_c:
	set_start_stop<dods_uint32>();
	break;
      case dods_float32_c:
	set_start_stop<dods_float32>();
	break;
      case dods_float64_c:
	set_start_stop<dods_float64>();
	break;
    default:
	throw Error(malformed_expr, 
             "Grid selection using non-numeric map vectors is not supported");
    }
#endif // WIN32

}

// Public methods

GSEClause::GSEClause(Grid *grid, const string &map, const double value,
		     const relop op) 
    : d_map(0),
      d_value1(value), d_value2(0), d_op1(op), d_op2(dods_nop_op),
      d_map_min_value(""), d_map_max_value("")
{
    d_map = dynamic_cast<Array *>(grid->var(map));
    DBG(cerr << d_map->toString());
    // Initialize the start and stop indices.
    Pix p = d_map->first_dim();
    d_start = d_map->dimension_start(p);
    d_stop = d_map->dimension_stop(p);

    compute_indices();
}

GSEClause::GSEClause(Grid *grid, const string &map, const double value1,
		     const relop op1, const double value2, const relop op2) 
    : d_map(0),
      d_value1(value1), d_value2(value2), d_op1(op1), d_op2(op2),
      d_map_min_value(""), d_map_max_value("")
{
    d_map = dynamic_cast<Array *>(grid->var(map));
    DBG(cerr << d_map->toString());
    // Initialize the start and stop indices.
    Pix p = d_map->first_dim();
    d_start = d_map->dimension_start(p);
    d_stop = d_map->dimension_stop(p);

    compute_indices();
}

GSEClause::GSEClause(Grid *grid, const string &expr)
{
}

GSEClause::GSEClause(Grid *grid, char *expr)
{
}

bool
GSEClause::OK() const
{
    if (!d_map)
	return false;
    
    // More ...

    return true;
}

Array *
GSEClause::get_map() const
{
    return d_map;
}

void
GSEClause::set_map(Array *map)
{
    d_map = map;
}

string
GSEClause::get_map_name() const
{
    return d_map->name();
}

int
GSEClause::get_start() const
{
    return d_start;
}

void
GSEClause::set_start(int start)
{
    d_start = start;
}

int
GSEClause::get_stop() const
{
    DBG(cerr << "Returning stop index value of: " << d_stop << endl);
    return d_stop;
}

void
GSEClause::set_stop(int stop)
{
    d_stop = stop;
}

string
GSEClause::get_map_min_value() const
{
    return d_map_min_value;
}

string
GSEClause::get_map_max_value() const
{
    return d_map_max_value;
}

// $Log: GSEClause.cc,v $
// Revision 1.7  2002/06/03 22:21:15  jimg
// Merged with release-3-2-9
//
// Revision 1.5.4.2  2001/10/30 06:55:45  rmorris
// Win32 porting changes.  Brings core win32 port up-to-date.
//
// Revision 1.6  2001/09/28 17:50:07  jimg
// Merged with 3.2.7.
//
// Revision 1.5.4.1  2001/09/25 20:33:02  jimg
// Changes/Fixes associated with fixes to grid() (see ce_functions.cc).
//
// Revision 1.5  2000/09/22 02:17:20  jimg
// Rearranged source files so that the CVS logs appear at the end rather than
// the start. Also made the ifdef guard symbols use the same naming scheme and
// wrapped headers included in other headers in those guard symbols (to cut
// down on extraneous file processing - See Lakos).
//
// Revision 1.4  2000/06/07 18:06:59  jimg
// Merged the pc port branch
//
// Revision 1.3.20.1  2000/06/02 18:21:27  rmorris
// Mod's for port to Win32.
//
// Revision 1.3  1999/04/29 02:29:30  jimg
// Merge of no-gnu branch
//
// Revision 1.2  1999/03/24 23:37:14  jimg
// Added support for the Int16, UInt16 and Float32 types
//
// Revision 1.1  1999/01/21 02:07:43  jimg
// Created
//

