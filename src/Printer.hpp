#pragma once

#include "Utils.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifndef mmin
#define mmin(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef mmax
#define mmax(a,b) (((a) > (b)) ? (a) : (b))
#endif

const int MAXPRINTSTRING = ( 2 * Utils::MAXNAMELEN) + 5;

// Print some number of spaces
void Spaces (int maxLength, int printedSoFar);

struct DataAttrInfo;
class Tuple;

class Printer {
public:
	// Constructor.  Takes as arguments an array of attributes along with
	// the length of the array.
	Printer (const DataAttrInfo *attributes, const int attrCount);
	Printer (const Tuple& t);

	~Printer ( );

	void PrintHeader (std::ostream &c) const;

	// Two flavors for the Print routine.  The first takes a char* to the
	// data and is useful when the data corresponds to a single record in
	// a table -- since in this situation you can just send in the
	// RecData.  The second will be useful in the QL layer.
	void Print (std::ostream &c, const char * const data);
	void Print (std::ostream &c, const void * const data[]);
	void Print (std::ostream &c, const Tuple& t);

	void PrintFooter (std::ostream &c) const;

private:
	void Init (const DataAttrInfo *attributes_, const int attrCount_);

private:
	DataAttrInfo *attributes;
	int attrCount;

	// An array of strings for the header information
	char **psHeader;
	// Number of spaces between each attribute
	int *spaces;

	// The number of tuples printed
	int iCount;
};


//
// printer.cc
//

// This file contains the interface for the Printer class and some
// functions that will be used by both the SM and QL components.

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "printer.hpp"

using namespace std;

//
// void Spaces(int maxLength, int printedSoFar)
//
// This method will output some spaces so that print entry will align everythin
// nice and neat.
//
void Spaces (int maxLength, int printedSoFar) {
	for ( int i = printedSoFar; i < maxLength; i++ )
		std::cout << " ";
}

//
// ------------------------------------------------------------------------------
//
Printer::Printer (const Tuple& t) {
	Init (t.GetAttributes ( ), t.GetAttrCount ( ));
}

//
// Printer
//
// This class handles the printing of tuples.
//
//  DataAttrInfo - describes all of the attributes. Defined
//      within sm.h
//  attrCount - the number of attributes
//
Printer::Printer (const DataAttrInfo *attributes_, const int attrCount_) {
	Init (attributes_, attrCount_);
}

void Printer::Init (const DataAttrInfo *attributes_, const int attrCount_) {
	attrCount = attrCount_;
	attributes = new DataAttrInfo[attrCount];

	for ( int i = 0; i < attrCount; i++ ) {
		attributes[i] = attributes_[i];
		// std::cout << "Printer::init i, offset " << attributes[i].offset << endl;
	}
	// Number of tuples printed
	iCount = 0;

	// Figure out what the header information will look like.  Normally,
	// we can just use the attribute name, but if that appears more than
	// once, then we should use "relation.attribute".

	// this line broke when using CC
	// changing to use malloc and free instead of new and delete
	// psHeader = (char **) new (char *)[attrCount];
	psHeader = ( char** ) malloc (attrCount * sizeof (char*));

	// Also figure out the number of spaces between each attribute
	spaces = new int[attrCount];

	for ( int i = 0; i < attrCount; i++ ) {
		// Try to find the attribute in another column
		int bFound = 0;
		psHeader[i] = new char[MAXPRINTSTRING];
		memset (psHeader[i], 0, MAXPRINTSTRING);

		for ( int j = 0; j < attrCount; j++ )
			if ( j != i &&
				strcmp (attributes[i].attrName,
						attributes[j].attrName) == 0 ) {
				bFound = 1;
				break;
			}

		if ( bFound )
			sprintf_s (psHeader[i], MAXPRINTSTRING,"%s.%s",
					 attributes[i].relName, attributes[i].attrName);
		else
			strcpy_s (psHeader[i], MAXPRINTSTRING, attributes[i].attrName);

		if ( attributes[i].attrType == STRING )
			spaces[i] = mmin (attributes[i].attrLength, MAXPRINTSTRING);
		else
			spaces[i] = mmax (12, strlen (psHeader[i]));

		// We must subtract out those characters that will be for the
		// header.
		spaces[i] -= strlen (psHeader[i]);

		// If there are negative (or zero) spaces, then insert a single
		// space.
		if ( spaces[i] < 1 ) {
			// The psHeader will give us the space we need
			spaces[i] = 0;
			strcat_s (psHeader[i], MAXPRINTSTRING," ");
		}
	}
}


//
// Destructor
//
Printer::~Printer ( ) {
	for ( int i = 0; i < attrCount; i++ )
		delete[] psHeader[i];

	delete[] spaces;
	//delete [] psHeader;
	free (psHeader);
	delete[] attributes;
}

//
// PrintHeader
//
void Printer::PrintHeader (std::ostream &c) const {
	int dashes = 0;
	int iLen;
	int i, j;

	for ( i = 0; i < attrCount; i++ ) {
		// Print out the header information name
		c << psHeader[i];
		iLen = strlen (psHeader[i]);
		dashes += iLen;

		for ( j = 0; j < spaces[i]; j++ )
			c << " ";

		dashes += spaces[i];
	}

	c << "\n";
	for ( i = 0; i < dashes; i++ ) c << "-";
	c << "\n";
}

//
// PrintFooter
//
void Printer::PrintFooter (std::ostream &c) const {
	c << "\n";
	c << iCount << " tuple(s).\n";
}

//
// Print
//
//  data - this is an array of void *.  This print routine is used by
//  the QL Layer.
//
//  Unfortunately, this is essentially the same as the other Print
//  routine.
//
void Printer::Print (std::ostream &c, const void * const data[]) {
	char str[MAXPRINTSTRING], strSpace[50];
	int i, a;
	float b;

	// Increment the number of tuples printed
	iCount++;

	for ( i = 0; i<attrCount; i++ ) {
		if ( attributes[i].attrType == STRING ) {
			// We will only print out the first MAXNAME+10 characters of
			// the string value.
			memset (str, 0, MAXPRINTSTRING);

			if ( attributes[i].attrLength>MAXPRINTSTRING ) {
				strcpy_s (str, MAXPRINTSTRING - 1, ( char * ) data[i]);
				str[MAXPRINTSTRING - 3] = '.';
				str[MAXPRINTSTRING - 2] = '.';
				c << str;
				Spaces (MAXPRINTSTRING, strlen (str));
			} else {
				strcpy_s (str, attributes[i].attrLength, ( char * ) data[i]);
				c << str;
				if ( attributes[i].attrLength < ( int ) strlen (psHeader[i]) )
					Spaces (strlen (psHeader[i]), strlen (str));
				else
					Spaces (attributes[i].attrLength, strlen (str));
			}
		}
		if ( attributes[i].attrType == INT ) {
			memcpy (&a, data[i], sizeof (int));
			sprintf_s (strSpace, MAXPRINTSTRING,"%d", a);
			c << a;
			if ( strlen (psHeader[i]) < 12 )
				Spaces (12, strlen (strSpace));
			else
				Spaces (strlen (psHeader[i]), strlen (strSpace));
		}
		if ( attributes[i].attrType == FLOAT ) {
			memcpy (&b, data[i], sizeof (float));
			sprintf_s (strSpace, MAXPRINTSTRING,"%f", b);
			c << strSpace;
			if ( strlen (psHeader[i]) < 12 )
				Spaces (12, strlen (strSpace));
			else
				Spaces (strlen (psHeader[i]), strlen (strSpace));
		}
	}
	c << "\n";
}

void Printer::Print (std::ostream &c, const Tuple& t) {
	const char * data;
	t.GetData (data);
	// std::cout << "Printer::Print(tuple) " << t << endl;
	Print (c, data);
}

//
// Print
//
//  data - the actual data for the tuple to be printed
//
//  The routine tries to make things line up nice, however no
//  attempt is made to keep the tuple constrained to some number of
//  characters.
//
void Printer::Print (std::ostream &c, const char * const data) {
	char str[MAXPRINTSTRING], strSpace[50];
	int i, a;
	float b;

	if ( data == NULL )
		return;

	// Increment the number of tuples printed
	iCount++;

	for ( i = 0; i<attrCount; i++ ) {
		// std::cout << "[Printer::Print i, offset " << attributes[i].offset << "]";

		if ( attributes[i].attrType == STRING ) {
			// We will only print out the first MAXNAME+10 characters of
			// the string value.
			memset (str, 0, MAXPRINTSTRING);

			if ( attributes[i].attrLength>MAXPRINTSTRING ) {
				strcpy_s (str, MAXPRINTSTRING - 1, data + attributes[i].offset);
				str[MAXPRINTSTRING - 3] = '.';
				str[MAXPRINTSTRING - 2] = '.';
				c << str;
				Spaces (MAXPRINTSTRING, strlen (str));
			} else {
				strcpy_s (str, attributes[i].attrLength, data + attributes[i].offset);
				c << str;
				if ( attributes[i].attrLength < ( int ) strlen (psHeader[i]) )
					Spaces (strlen (psHeader[i]), strlen (str));
				else
					Spaces (attributes[i].attrLength, strlen (str));
			}
		}
		if ( attributes[i].attrType == INT ) {
			memcpy (&a, ( data + attributes[i].offset ), sizeof (int));
			sprintf_s (strSpace, MAXPRINTSTRING,"%d", a);
			c << a;
			if ( strlen (psHeader[i]) < 12 )
				Spaces (12, strlen (strSpace));
			else
				Spaces (strlen (psHeader[i]), strlen (strSpace));
		}
		if ( attributes[i].attrType == FLOAT ) {
			memcpy (&b, ( data + attributes[i].offset ), sizeof (float));
			sprintf_s (strSpace, MAXPRINTSTRING, "%f", b);
			c << strSpace;
			if ( strlen (psHeader[i]) < 12 )
				Spaces (12, strlen (strSpace));
			else
				Spaces (strlen (psHeader[i]), strlen (strSpace));
		}
	}
	c << "\n";
}
