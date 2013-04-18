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

#include "config.h"

#include <cppunit/TextTestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#define DODS_DEBUG

#include "D4Attributes.h"
#include "XMLWriter.h"
#include "debug.h"

#include "testFile.h"
#include "test_config.h"

using namespace CppUnit;
using namespace std;
using namespace libdap;

class D4AttributesTest: public TestFixture {
private:
    XMLWriter *xml;
    D4Attributes *attrs;

    D4Attribute a, a2, a3, a4, c, c2;

public:
    D4AttributesTest() {
    }

    ~D4AttributesTest() {
    }

    void setUp() {
        attrs = new D4Attributes;
        xml = new XMLWriter;

        a.set_name("first");
        a.set_type(attr_byte_c);
        a.add_value("1");
        a.add_value("2");

        a2.set_name("second");
        a2.set_type(attr_int32_c);
        a2.add_value("10000");

        c.set_name("container_1");
        c.set_type(attr_container_c);

        a3.set_name("color");
        a3.set_type(attr_str_c);
        vector<string> colors; colors.push_back("red"); colors.push_back("blue");  colors.push_back("green");
        a3.add_value_vector(colors);

        a4 = a2;
        a4.set_name("control");

        c.attributes()->add_attribute(&a3);

        c2.set_name("container_2");
        c2.set_type(attr_container_c);

        c2.attributes()->add_attribute(&a4);
        c2.attributes()->add_attribute(&c);
    }

    void tearDown() {
        delete xml;
        delete attrs;
    }

    // An empty D4Dimensions object prints nothing; the XMLWriter class adds
    // a xml doc preface.
    void test_print_empty() {
        attrs->print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_empty.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    void test_print_single_attribute() {
        a.print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_values_1.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);

    }

    void test_print_1() {
        attrs->add_attribute(&a);

        attrs->print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_1.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }


    void test_print_2() {
        attrs->add_attribute(&a);
        attrs->add_attribute(&a2);

        attrs->print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_2.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    void test_print_3() {
        attrs->add_attribute(&a);
        attrs->add_attribute(&a2);
        attrs->add_attribute(&c);
        attrs->add_attribute(&c2);

        attrs->print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_3.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    void test_print_assignment() {
        attrs->add_attribute(&a);
        attrs->add_attribute(&a2);

        D4Attributes lhs = *attrs;

        lhs.print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_assignment.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    void test_print_assignment_2() {
        attrs->add_attribute(&a);
        attrs->add_attribute(&a2);
        attrs->add_attribute(&c);
        attrs->add_attribute(&c2);

        D4Attributes lhs = *attrs;

        lhs.print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_assignment_2.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    void test_print_copy_ctor() {
        attrs->add_attribute(&a);
        attrs->add_attribute(&a2);

        D4Attributes lhs(*attrs);

        lhs.print_dap4(*xml);
        string doc = xml->get_doc();
        string baseline = readTestBaseline(string(TEST_SRC_DIR) + "/D4-xml/D4Attributes_copy_ctor.xml");
        DBG(cerr << "D4Attributes: " << doc << endl);
        CPPUNIT_ASSERT(doc == baseline);
    }

    CPPUNIT_TEST_SUITE( D4AttributesTest );

        CPPUNIT_TEST(test_print_empty);
        CPPUNIT_TEST(test_print_single_attribute);
        CPPUNIT_TEST(test_print_1);
        CPPUNIT_TEST(test_print_2);
        CPPUNIT_TEST(test_print_3);

        CPPUNIT_TEST(test_print_assignment);
        CPPUNIT_TEST(test_print_assignment_2);

        CPPUNIT_TEST(test_print_copy_ctor);

        CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(D4AttributesTest);

int main(int, char**) {
    CppUnit::TextTestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());

    bool wasSuccessful = runner.run("", false);

    return wasSuccessful ? 0 : 1;
}

