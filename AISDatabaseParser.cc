
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2003 OPeNDAP, Inc.
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

#include "config_dap.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "AISDatabaseParser.h"
#include "util.h"
#include "debug.h"

static const not_used char *states[] = {
    "START",
    "FINISH",
    "AIS",
    "ENTRY",
    "PRIMARY",
    "ANCILLARY",
    "UNKNOWN",
    "ERROR"
};

/** @name SAX Parser Callbacks

    These methods are declared static in the class header. This gives them C
    linkage which allows them to be used as callbacks by the SAX parser
    engine. */
//@{

/** Initialize the SAX parser state object. This object is passed to each
    callback as a void pointer.
    @param state The SAX parser state. */
void 
AISDatabaseParser::aisStartDocument(AISParserState *state) 
{
    state->state = PARSER_START;
    state->unknown_depth = 0;
    state->prev_state = PARSER_UNKNOWN;
    state->error_msg = "";

    DBG2(cerr << "Parser state: " << states[state->state] << endl);
}

/** Clean up after finishing a parse.
    @param state The SAX parser state. */
void 
AISDatabaseParser::aisEndDocument(AISParserState *state) 
{
    DBG2(cerr << "Ending state == " << states[state->state] << endl);

    if (state->unknown_depth != 0) {
	AISDatabaseParser::aisFatalError(state, "The document contained unbalanced tags.");

        DBG(cerr << "unknown_depth != 0 (" << state->unknown_depth << ")"
	    << endl);
    }
}

/** Process a start element tag. Because the AIS DTD uses attributes and
    because libxml2 does not validate those, we do attribute validation here.
    Values pulled from the attributes are recorded in <code>state</code> for
    later use in aisEndElement.
    @param state The SAX parser state. */
void 
AISDatabaseParser::aisStartElement(AISParserState *state, const char *name, 
				   const char **attrs)
{
    switch (state->state) {
      case PARSER_START:
	if (strcmp(name, "ais") != 0) {
            DBG(cerr << "Expecting ais.  Got " << name << endl);
	}
	state->state = AIS;
	break;

      case PARSER_FINISH:
	break;

      case AIS:
        if (strcmp(name, "entry") == 0) {
	    state->prev_state = state->state;
	    state->state = ENTRY;
        } 
	else {
            state->prev_state = state->state;
            state->state = PARSER_UNKNOWN;
            state->unknown_depth++;
        }
        break;

      case ENTRY:
	if (strcmp(name, "primary") == 0) {
	    state->prev_state = state->state;
	    state->state = PRIMARY;

#if 0
	    if (attrs && (strcmp(attrs[0], "url") == 0))
		state->primary = attrs[1];
	    else {
		AISDatabaseParser::aisFatalError(state, "Required attribute 'url' missing from element 'primary'.");
		break;
	    }
#endif

	    if (attrs) {
		if (strcmp(attrs[0], "url") == 0) {
		    state->regexp = false;
		    state->primary = attrs[1];
		}
		else if (strcmp(attrs[0], "regexp") == 0) {
		    state->regexp = true;
		    state->primary = attrs[1];
		}
	    }
	    else {
		AISDatabaseParser::aisFatalError(state, "Required attribute 'url' or 'regexp' missing from element 'primary'.");
		break;
	    }
	}
	else if (strcmp(name, "ancillary") == 0) {
	    state->prev_state = state->state;
	    state->state = ANCILLARY;

	    string url = "";	// set defaults, MUST have url
	    string rule = "overwrite";
	    for (int i = 0; attrs && attrs[i] != 0; i = i + 2) {
		if (strcmp(attrs[i], "url") == 0)
		    url = attrs[i+1];
		else if (strcmp(attrs[i], "rule") == 0)
		    rule = attrs[i+1];
	    }

	    // If this parser validated the XML, these tests would be
	    // unnecessary. 
	    if (url == "") {
		AISDatabaseParser::aisFatalError(state, "Required attribute 'url' missing from element 'ancillary'.");
		break;
	    }

	    if (rule != "overwrite" && rule != "replace" && rule != "fallback") {
		string msg = string("Optional attribute 'rule' in element 'ancillary' has a bad value: ") + rule + "\nIt should be one of 'overwrite', 'replace' or 'fallback'.";
		AISDatabaseParser::aisFatalError(state, msg.c_str());
		break;
	    }

	    Resource r(url, rule);
	    state->rv.push_back(r);
	}
	else {
            state->prev_state = state->state;
            state->state = PARSER_UNKNOWN;
            state->unknown_depth++;
        }
        break;

      case PRIMARY:
	break;

      case ANCILLARY:
	break;

      case PARSER_UNKNOWN:
	state->unknown_depth++;
	break;

      case PARSER_ERROR:
	break;
    }

    DBG2(cerr << "Start element " << name << " (state " 
	 << states[state->state] << ")" << endl);
}

/** Process an end element tag. This is where values are added to the
    AISResources object that's held by <code>state</code>.
    @param state The SAX parser state. */
void 
AISDatabaseParser::aisEndElement(AISParserState *state, const char *name) 
{
    DBG2(cerr << "End element " << name << " (state " << states[state->state] 
	 << ")" << endl);

    switch (state->state) {
      case AIS:
	state->prev_state = state->state;
	state->state = PARSER_FINISH;
	break;

      case ENTRY:
	state->prev_state = state->state;
	state->state = AIS;

	// record 'primary' and 'rv'
	if (state->regexp)
	    state->ais->add_regexp_resource(state->primary, state->rv);
	else
	    state->ais->add_url_resource(state->primary, state->rv);

	// empty rv for the next set of ancillary resources.
	state->rv.erase(state->rv.begin(), state->rv.end());
	break;

      case PRIMARY:
	state->prev_state = state->state;
	state->state = ENTRY;
	break;

      case ANCILLARY:
	state->prev_state = state->state;
	state->state = ENTRY;
	break;

      case PARSER_UNKNOWN:
	// Leave the state and prev_state alone.
	state->unknown_depth--;
	break;

      case PARSER_ERROR:
	break;

      default:
	break;
    }
}

/** Handle the standard XML entities.
    @param state The SAX parser state. 
    @param name The XML entity. */
xmlEntityPtr
AISDatabaseParser::aisGetEntity(AISParserState *state, const xmlChar *name) 
{
    return xmlGetPredefinedEntity(name);
}

/** Process an XML warning. This is treated as a fatal error since there's no
    easy way for libdap++ to signal warning to users.
    @param state The SAX parser state. 
    @param msg A printf-style format string. */
void 
AISDatabaseParser::aisWarning(AISParserState *state, const char *msg, ...) 
{
    va_list args;

    state->state = PARSER_ERROR;

    va_start(args, msg);
    char str[1024];
    vsnprintf(str, 1024, msg, args);
    va_end(args);

    int line = getLineNumber(state->ctxt);
    state->error_msg += "At line: " + long_to_string(line) + ": ";
    state->error_msg += string(str) + string("\n");
}

/** Process an XML error. This is treated as a fatal error since there's no
    easy way for libdap++ to signal warning to users.
    @param state The SAX parser state. 
    @param msg A printf-style format string. */
void 
AISDatabaseParser::aisError(AISParserState *state, const char *msg, ...)
{
    va_list args;

    state->state = PARSER_ERROR;

    va_start(args, msg);
    char str[1024];
    vsnprintf(str, 1024, msg, args);
    va_end(args);

    int line = getLineNumber(state->ctxt);
    state->error_msg += "At line: " + long_to_string(line) + ": ";
    state->error_msg += string(str) + string("\n");
}

/** Process an XML fatal error. 
    @param state The SAX parser state. 
    @param msg A printf-style format string. */
void 
AISDatabaseParser::aisFatalError(AISParserState *state, const char *msg, ...)
{
    va_list args;

    state->state = PARSER_ERROR;

    va_start(args, msg);
    char str[1024];
    vsnprintf(str, 1024, msg, args);
    va_end(args);

    int line = getLineNumber(state->ctxt);
    state->error_msg += "At line: " + long_to_string(line) + ": ";
    state->error_msg += string(str) + string("\n");
}

//@}

/** This local variable holds pointers to the callback <i>functions</i> which
    comprise the SAX parser. */
static xmlSAXHandler aisSAXParser = {
    0, // internalSubset 
    0, // isStandalone 
    0, // hasInternalSubset 
    0, // hasExternalSubset 
    0, // resolveEntity 
    (getEntitySAXFunc)AISDatabaseParser::aisGetEntity, // getEntity 
    0, // entityDecl 
    0, // notationDecl 
    0, // attributeDecl 
    0, // elementDecl 
    0, // unparsedEntityDecl 
    0, // setDocumentLocator 
    (startDocumentSAXFunc)AISDatabaseParser::aisStartDocument, // startDocument
    (endDocumentSAXFunc)AISDatabaseParser::aisEndDocument,  // endDocument 
    (startElementSAXFunc)AISDatabaseParser::aisStartElement,  // startElement 
    (endElementSAXFunc)AISDatabaseParser::aisEndElement,  // endElement 
    0, // reference 
    0, // (charactersSAXFunc)gladeCharacters,  characters 
    0, // ignorableWhitespace 
    0, // processingInstruction 
    0, // (commentSAXFunc)gladeComment,  comment 
    (warningSAXFunc)AISDatabaseParser::aisWarning, // warning 
    (errorSAXFunc)AISDatabaseParser::aisError, // error 
    (fatalErrorSAXFunc)AISDatabaseParser::aisFatalError // fatalError 
};

/** Parse an AIS database encoded in XML. The information in the XML document
    is loaded into an instance of AISResources. 
    @param database Read from this XML file.
    @param ais Load information into this instance of AISResources.
    @exception AISDatabaseReadFailed Thrown if the XML document could not be
    read or parsed. */
void
AISDatabaseParser::intern(const string &database, AISResources *ais)
    throw(AISDatabaseReadFailed)
{
    xmlParserCtxtPtr ctxt;
    AISParserState state;
	
    ctxt = xmlCreateFileParserCtxt(database.c_str());
    if (!ctxt) 
	return;

    state.ais = ais;		// dump values here
    state.ctxt = ctxt;		// need ctxt for error messages

    ctxt->sax = &aisSAXParser;
    ctxt->userData = &state;
    ctxt->validate = true;

    xmlParseDocument(ctxt);
	
    // use getLineNumber and getColumnNumber to make the error messages better.
    if (!ctxt->wellFormed) {
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
	throw AISDatabaseReadFailed(string("\nThe database is not a well formed XML document.\n") + state.error_msg);
    }

    if (!ctxt->valid) {
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
	throw AISDatabaseReadFailed(string("\nThe database is not a valid document.\n") + state.error_msg);
    }

    if (state.state == PARSER_ERROR) {
	ctxt->sax = NULL;
	xmlFreeParserCtxt(ctxt);
	throw AISDatabaseReadFailed(string("\nError parsing AIS resources.\n") + state.error_msg);
    }

    ctxt->sax = NULL;
    xmlFreeParserCtxt(ctxt);
}

// $Log: AISDatabaseParser.cc,v $
// Revision 1.6  2003/04/22 19:40:27  jimg
// Merged with 3.3.1.
//
// Revision 1.5  2003/03/12 01:07:34  jimg
// Added regular expressions to the AIS subsystem. In an AIS database (XML)
// it is now possible to list a regular expression in place of an explicit
// URL. The AIS will try to match this Regexp against candidate URLs and
// return the ancillary resources for all those that succeed.
//
// Revision 1.4  2003/02/26 01:27:49  jimg
// Changed the name of the parse() method to intern().
//
// Revision 1.3  2003/02/25 04:18:53  jimg
// Added line numbers to error messages. Improved message formatting a little.
//
// Revision 1.2  2003/02/21 00:14:24  jimg
// Repaired copyright.
//
// Revision 1.1  2003/02/20 22:16:04  jimg
// Added.
//
