
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 1996, California Institute of Technology.
// ALL RIGHTS RESERVED.   U.S. Government Sponsorship acknowledged.
//
// Please read the full copyright notice in the file COPYRIGHT
// in this directory.
//
// Author: Todd Karakashian, NASA/Jet Propulsion Laboratory
//         Todd.K.Karakashian@jpl.nasa.gov
//
// $RCSfile: escaping.cc,v $ - Miscellaneous routines for DODS HDF server
//
// $Log: escaping.cc,v $
// Revision 1.11  2000/01/27 06:30:01  jimg
// Resolved conflicts from merge with release-3-1-4
//
// Revision 1.9.6.2  2000/01/27 00:03:05  jimg
// Fixed the return type of string::find.
//
// Revision 1.10  1999/08/23 18:57:46  jimg
// Merged changes from release 3.1.0
//
// Revision 1.9.6.1  1999/08/16 23:32:01  jimg
// Added esc2underscore
//
// Revision 1.9  1999/04/29 02:29:36  jimg
// Merge of no-gnu branch
//
// Revision 1.8  1998/09/10 23:37:11  jehamby
// Forgot to update hexstring() to generate correct high-ASCII escapes.
//
// Revision 1.7  1998/09/10 19:38:03  jehamby
// Update escaping routines to not mangle high-ASCII characters with toascii()
// and to generate a correct escape sequence in octstring() for such characters
// through judicious casting (cast to a unsigned char, then an unsigned int).
//
// Revision 1.6.6.1  1999/02/02 21:57:07  jimg
// String to string version
//
// Revision 1.6  1998/03/19 23:29:47  jimg
// Removed old code (that was surrounded by #if 0 ... #endif).
//
// Revision 1.5  1998/02/05 20:14:02  jimg
// DODS now compiles with gcc 2.8.x
//
// Revision 1.4  1997/02/14 04:18:10  jimg
// Added allowable and escape parameters to id2dods and dods2id so that the
// builtin regexs can be overridden if needed.
// Switched to the `fast compile' mode for the Regex objects.
//
// Revision 1.3  1997/02/14 02:24:44  jimg
// Removed reliance on the dods-hdf code.
// Introduced a const int MAXSTR with value 256. This matches the length of
// ID_MAX in the parser.h header (which I did not include since it defines
// a lot of software that is irrelevant to this module).
//
// Revision 1.2  1997/02/14 02:18:16  jimg
// Added to DODS core
//
// Revision 1.2  1996/10/07 21:15:17  todd
// Changes escape character to % from _.
//
// Revision 1.1  1996/09/24 22:38:16  todd
// Initial revision
//
//
/////////////////////////////////////////////////////////////////////////////

// These two routines are for escaping/unescaping strings that are identifiers
// in DODS
// id2dods() -- escape (using WWW hex codes) non-allowable characters in a
// DODS identifier
// dods2id() -- given an WWW hexcode escaped identifier, restore it
// 
// These two routines are for escaping/unescaping strings storing attribute
// values.  They use traditional octal escapes (\nnn) because they are
// intended to be viewed by a user
// escattr() -- escape (using traditional octal backslash) non-allowable
// characters in the value of a DODS attribute
// unescattr() -- given an octally escaped string, restore it
// 
// These are routines used by the above, not intended to be called directly:
// 
// hexstring()
// unhexstring()
// octstring()
// unoctstring()
// 
// -Todd

#include <ctype.h>
#ifdef __GNUG__
#include <strstream>
#else
#include <sstream>
#endif
#include <iomanip>
#include <string>
#include <Regex.h>

const int MAXSTR = 256;

static string hexstring(unsigned char val) {
    static char buf[MAXSTR];

    ostrstream(buf,MAXSTR) << hex << setw(2) << setfill('0') <<
	(unsigned int)val << ends;

    return (string)buf;
}

static char unhexstring(string s) {
    int val;

    istrstream(s.c_str(),MAXSTR) >> hex >> val;

    return (char)val;
}

static string octstring(unsigned char val) {
    static char buf[MAXSTR];

    ostrstream(buf,MAXSTR) << oct << setw(3) << setfill('0') <<
	(unsigned int)val << ends;

    return (string)buf;
}

static char unoctstring(string s) {
    int val;

    istrstream(s.c_str(),MAXSTR) >> oct >> val;

    return (char)val;
}


/** Replace characters that are not allowed in DODS identifiers.
    @param s The string in which to replace characters.
    @param allowable a regular expression describing the set of characters
    that are allowable in a DODS identifier.
    @return The modified identifier. */
string 
id2dods(string s, const string allowable = "[^0-9a-zA-Z_%]") {
    static Regex badregx(allowable.c_str(), 1);
    static const string ESC = "%";

    int index=0, matchlen;
    while ((index = badregx.search(s.c_str(), s.size(), matchlen, index)) != -1)
      s.replace(index, 1, ESC + hexstring(s[index]));

    if (isdigit(s[0]))
	s.insert(0, '_');

    return s;
}

/** Escape non-printable characters in an identifier using the WWW %<hex
    code> notation.
    @param s The DODS identifier to modify.
    @param escape A regular expression that matches the WWW hex code. By
    default "%[0-7][0-9a-fA-F]".
    @return The modified identifier. */
string 
dods2id(string s, const string escape = "%[0-7][0-9a-fA-F]") {
    static Regex escregx(escape.c_str(), 1);

    int index=0, matchlen;
    while ((index = escregx.search(s.c_str(), s.size(), matchlen, index)) != -1)
      s.replace(index, 3, unhexstring(s.substr(index+1,2)));

    return s;
}

/** Return a string that has all the `%<hex digit><hex digit>' sequences
    replaced with underscores (`_').
    @param s The string to transform
    @param escape A regular expression which matches the `%<hd><hd>' pattern.
    By default this is the string "%[0-7][0-9a-fA-F]". Replacing the pattern
    allows the function to be used to map other patterns to an underscore.
    @return The modified string. */
string 
esc2underscore(string s, const string escape = "%[0-7][0-9a-fA-F]") {
    static Regex escregx(escape.c_str(), 1);

    int index=0, matchlen;
    while ((index = escregx.search(s.c_str(), s.size(), matchlen, index)) != -1)
      s.replace(index, matchlen, "_");

    return s;
}

/** Escape non-printable characters and quotes from an HDF attribute.
    @param s The attribute to modify.
    @return The modified attribute. */
string escattr(string s) {
    static Regex nonprintable("[^ !-~]");
    const string ESC = "\\";
    const char QUOTE = '\"';
    const string ESCQUOTE = ESC + QUOTE;

    // escape non-printing characters with octal escape
    int index = 0, matchlen;
    while ( (index = nonprintable.search(s.c_str(), s.size(), matchlen, index)) != -1)
	s.replace(index,1, ESC + octstring(s[index]));

    // escape " with backslash
    string::size_type ind = 0;
    while ( (ind = s.find(QUOTE, ind)) != s.npos) {
	s.replace(ind, 1, ESCQUOTE);
	ind += ESCQUOTE.length();
    }

    return s;
}

/** Un-escape special characters, quotes and backslashes from an HDF attribute.
    <p>
    Note: A regex to match one \ must be defined as: Regex foo = "\\\\";
    because both C++ strings and GNU's Regex also employ \ as an escape
    character! 
    @param s The escaped attribute.
    @return The unescaped attribute. */
string unescattr(string s) {
    static Regex escregx("\\\\[01][0-7][0-7]");  // matches 4 characters
    static Regex escquoteregex("[^\\\\]\\\\\"");  // matches 3 characters
    static Regex escescregex("\\\\\\\\");      // matches 2 characters
    const string ESC = "\\";
    const char QUOTE = '\"';
    const string ESCQUOTE = ESC + QUOTE;

    // unescape any octal-escaped ASCII characters
    int index = 0, matchlen;
    while ( (index = escregx.search(s.c_str(), s.size(), matchlen, index)) != -1) {
	s.replace(index,4, unoctstring(s.substr(index+1,3)));
	index++;
    }

    // unescape any escaped quotes
    index = 0;
    while ( (index = escquoteregex.search(s.c_str(), s.size(), matchlen, index)) != -1) {
	s.replace(index+1,2, QUOTE);
	index++;
    }

    // unescape any escaped backslashes
    index = 0;
    while ( (index = escescregex.search(s.c_str(), s.size(), matchlen, index)) != -1) {
	s.replace(index,2, ESC);
	index++;
    }

    return s;
}
