
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2002,2003 OPeNDAP, Inc.
// Author: James Gallagher <jgallagher@opendap.org>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.
 
// (c) COPYRIGHT URI/MIT 1994-1999
// Please read the full copyright statement in the file COPYRIGHT_URI.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>

// Interface for PassiveArray type. 
//
// pwest 11/04/03

#ifndef _passivearray_h
#define _passivearray_h 1

#ifndef __POWERPC__
#ifdef __GNUG__
#pragma interface
#endif
#endif

#include "Array.h"
#include "dods-datatypes.h"

/** @brief Holds a 32-bit signed integer. 

    @see BaseType
    */

class PassiveArray: public Array {
private:
    dods_byte *_byte_val ;
    dods_int16 *_int16_val ;
    dods_int32 *_int32_val ;
    dods_uint16 *_uint16_val ;
    dods_uint32 *_uint32_val ;
    dods_float32 *_float32_val ;
    dods_float64 *_float64_val ;
    string *_str_val ;

public:
    PassiveArray(const string &n = "", BaseType *v = 0);

    PassiveArray(const PassiveArray &copy_from);

    PassiveArray &operator=(const PassiveArray &rhs);

    virtual ~PassiveArray();

    virtual BaseType *ptr_duplicate();

    virtual bool read( const string &dataset ) ;

    virtual bool set_value( dods_byte *val, int sz ) ;
    virtual bool set_value( dods_int16 *val, int sz ) ;
    virtual bool set_value( dods_int32 *val, int sz ) ;
    virtual bool set_value( dods_uint16 *val, int sz ) ;
    virtual bool set_value( dods_uint32 *val, int sz ) ;
    virtual bool set_value( dods_float32 *val, int sz ) ;
    virtual bool set_value( dods_float64 *val, int sz ) ;
    virtual bool set_value( string *val, int sz ) ;
};

/* 
 * $Log: PassiveArray.h,v $
 * Revision 1.1  2004/07/09 16:34:38  pwest
 * Adding Passive Data Model objects
 *
 */

#endif // _passivearray_h
