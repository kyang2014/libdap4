
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
 
// Tests for the util functions in util.cc and escaping.cc

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include "debug.h"
#include "util.h"
#include "escaping.h"

string hexstring(unsigned char val); // originally declared static
string unhexstring(string s);

using namespace CppUnit;
using std::cerr ;
using std::endl ;

class generalUtilTest : public TestFixture {
private:

public: 
    generalUtilTest() {}
    ~generalUtilTest() {}

    void setUp() {
    }

    void tearDown() {
    }

    CPPUNIT_TEST_SUITE(generalUtilTest);

    CPPUNIT_TEST(path_to_filename_test);
    CPPUNIT_TEST(hexstring_test);
    CPPUNIT_TEST(unhexstring_test);
    CPPUNIT_TEST(id2www_test);
    CPPUNIT_TEST(www2id_test);
    CPPUNIT_TEST(ce_string_parse_test);
    CPPUNIT_TEST(escattr_test);
    CPPUNIT_TEST(munge_error_message_test);
    CPPUNIT_TEST(get_tempfile_template_test);

    CPPUNIT_TEST_SUITE_END();

    // Tests for methods
    void path_to_filename_test() {
	CPPUNIT_ASSERT(path_to_filename("/this/is/the/end/my.friend") == "my.friend");
	CPPUNIT_ASSERT(path_to_filename("this.dat") == "this.dat");
	CPPUNIT_ASSERT(path_to_filename("/this.dat") == "this.dat");
	CPPUNIT_ASSERT(path_to_filename("/this.dat/") == "");
    }

    void hexstring_test() {
	CPPUNIT_ASSERT(hexstring('[') == "5b");
	CPPUNIT_ASSERT(hexstring(']') == "5d");
	CPPUNIT_ASSERT(hexstring(' ') == "20");
	CPPUNIT_ASSERT(hexstring('%') == "25");
    }

    void unhexstring_test() {
	CPPUNIT_ASSERT(unhexstring("5b") == "[");
	CPPUNIT_ASSERT(unhexstring("5d") == "]");
	CPPUNIT_ASSERT(unhexstring("20") == " ");
	CPPUNIT_ASSERT(unhexstring("25") == "%");
	CPPUNIT_ASSERT(unhexstring("5B") == "[");
	CPPUNIT_ASSERT(unhexstring("5D") == "]");
    }

    void id2www_test() {
	CPPUNIT_ASSERT(id2www("this") == "this");
	CPPUNIT_ASSERT(id2www("This is a test") == "This%20is%20a%20test");
	CPPUNIT_ASSERT(id2www("This.is") == "This.is");
	CPPUNIT_ASSERT(id2www("This-is") == "This-is");
	CPPUNIT_ASSERT(id2www("This_is") == "This_is");
	CPPUNIT_ASSERT(id2www("This/is") == "This%2fis");
	CPPUNIT_ASSERT(id2www("This%is") == "This%25is");
    }

    void www2id_test() {
	CPPUNIT_ASSERT(www2id("This_is_a_test") == "This_is_a_test");
	CPPUNIT_ASSERT(www2id("This is a test") == "This is a test");
	CPPUNIT_ASSERT(www2id("%5b") == "[");
	CPPUNIT_ASSERT(www2id("%5d") == "]");
	CPPUNIT_ASSERT(www2id("u%5b0%5d") == "u[0]");
	CPPUNIT_ASSERT(www2id("WVC%20Lat") == "WVC Lat");
	CPPUNIT_ASSERT(www2id("Grid.Data%20Fields[20][20]") 
	       == "Grid.Data Fields[20][20]");

	CPPUNIT_ASSERT(www2id("Grid.Data%3aFields[20][20]") 
	       == "Grid.Data:Fields[20][20]");

	CPPUNIT_ASSERT(www2id("Grid%3aData%20Fields%5b20%5d[20]", "%", "%20") 
	       == "Grid:Data%20Fields[20][20]");
    }

    // This is the code in expr.lex that removes emclosing double quotes and
    // %20 sequences from a string. I copied this here because that actual
    // function uses globals and would be hard to test. 7/11/2001 jhrg
    string *store_str(char *text) {
	string *s = new string(www2id(string(text)));

	if (*s->begin() == '\"' && *(s->end()-1) == '\"') {
	    s->erase(s->begin());
	    s->erase(s->end()-1);
	}

	return s;
    }

    void ce_string_parse_test() {
	CPPUNIT_ASSERT(*store_str("testing") == "testing");
	CPPUNIT_ASSERT(*store_str("\"testing\"") == "testing");
	CPPUNIT_ASSERT(*store_str("\"test%20ing\"") == "test ing");
	CPPUNIT_ASSERT(*store_str("test%20ing") == "test ing");
    }

    void escattr_test() {
	// The backslash escapes the double quote; in the returned string the
	// first two backslashes are a single escaped bs, the third bs
	// escapes the double quote.
	CPPUNIT_ASSERT(escattr("this_contains a double quote (\")")
	       == "this_contains a double quote (\\\")");
	CPPUNIT_ASSERT(escattr("this_contains a backslash (\\)")
	       == "this_contains a backslash (\\\\)");
    }

    void munge_error_message_test() {
	CPPUNIT_ASSERT(munge_error_message("An Error") == "\"An Error\"");
	CPPUNIT_ASSERT(munge_error_message("\"An Error\"") == "\"An Error\"");
	CPPUNIT_ASSERT(munge_error_message("An \"E\"rror") == "\"An \\\"E\\\"rror\"");
	CPPUNIT_ASSERT(munge_error_message("An \\\"E\\\"rror") == "\"An \\\"E\\\"rror\"");
    }

    void get_tempfile_template_test() {
	if (putenv("TMPDIR=/tmp") == 0) {
	    DBG2(cerr << "TMPDIR: " << getenv("TMPDIR") << endl);
	    CPPUNIT_ASSERT(strcmp(get_tempfile_template("DODSXXXXXX"),
			  "/tmp/DODSXXXXXX") == 0);
	}
	else
	    cerr << "Did not test setting TMPDIR; no test" << endl;

	if (putenv("TMPDIR=/usr/local/tmp/") == 0)
	    CPPUNIT_ASSERT(strcmp(get_tempfile_template("DODSXXXXXX"),
			  "/usr/local/tmp//DODSXXXXXX") == 0);
	else
	    cerr << "Did not test setting TMPDIR; no test" << endl;

#if defined(P_tmpdir)
	string tmplt = P_tmpdir;
	tmplt.append("/"); tmplt.append("DODSXXXXXX");
	putenv("TMPDIR=");
	CPPUNIT_ASSERT(strcmp(get_tempfile_template("DODSXXXXXX"), tmplt.c_str()) 
	       == 0);
#endif
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(generalUtilTest);

int 
main( int argc, char* argv[] )
{
    CppUnit::TextTestRunner runner;
    runner.addTest( CppUnit::TestFactoryRegistry::getRegistry().makeTest() );

    runner.run();

    return 0;
}


