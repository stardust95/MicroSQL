#pragma once
//
// File:        predicate.h
//


#include "Utils.hpp"
#include <cerrno>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>

class Predicate {
public:
	Predicate ( ) { }
	~Predicate ( ) { }

	Predicate (AttrType   attrTypeIn,
			   int        attrLengthIn,
			   int        attrOffsetIn,
			   CompOp     compOpIn,
			   void       *valueIn) {
		attrType = attrTypeIn;
		attrLength = attrLengthIn;
		attrOffset = attrOffsetIn;
		compOp = compOpIn;
		value = valueIn;
		//pinHint = pinHintIn;
	}

	CompOp initOp ( ) const { return compOp; }
	bool eval (const char *buf, CompOp c) const;
	bool eval (const char *lhsBuf, const char* rhsValue, CompOp c) const;

private:
	AttrType   attrType;
	int        attrLength;
	int        attrOffset;
	CompOp     compOp;
	void*      value;
	//ClientHint pinHint;
};


using namespace std;

bool AlmostEqualRelative (float A, float B, float maxRelativeError = 0.000001) {
	if ( A == B )
		return true;
	float relativeError = fabs (( A - B ) / B);
	if ( relativeError <= maxRelativeError )
		return true;
	return false;
}


bool Predicate::eval (const char *buf, CompOp c) const {
	return this->eval (buf, NULL, c);
}

bool Predicate::eval (const char *buf, const char* rhs, CompOp c) const {
	const void * value_ = rhs;
	if ( rhs == NULL )
		value_ = value;

	if ( c == NO_OP || value_ == NULL ) {
		return true;
	}
	const char * attr = buf + attrOffset;

	// cerr << "Predicate::eval " << *((int *)attr) << " " << *((int *)value_) << endl;

	if ( c == LT_OP ) {
		if ( attrType == INT ) {
			return *( ( int * ) attr ) < *( ( int * ) value_ );
		}
		if ( attrType == FLOAT ) {
			return *( ( float * ) attr ) < *( ( float * ) value_ );
		}
		if ( attrType == STRING ) {
			return strncmp (attr, ( char * ) value_, attrLength) < 0;
		}
	}
	if ( c == GT_OP ) {
		if ( attrType == INT ) {
			return *( ( int * ) attr ) > *( ( int * ) value_ );
		}
		if ( attrType == FLOAT ) {
			return *( ( float * ) attr ) > *( ( float * ) value_ );
		}
		if ( attrType == STRING ) {
			return strncmp (attr, ( char * ) value_, attrLength) > 0;
		}
	}
	if ( c == EQ_OP ) {
		if ( attrType == INT ) {
			return *( ( int * ) attr ) == *( ( int * ) value_ );
		}
		if ( attrType == FLOAT ) {
			return *( ( float * ) attr ) == *( ( float * ) value_ );
			// return AlmostEqualRelative(*((float *)attr), *((float *)value_));
		}
		if ( attrType == STRING ) {
			return strncmp (attr, ( char * ) value_, attrLength) == 0;
		}
	}
	if ( c == LE_OP ) {
		return this->eval (buf, rhs, LT_OP) || this->eval (buf, rhs, EQ_OP);
	}
	if ( c == GE_OP ) {
		return this->eval (buf, rhs, GT_OP) || this->eval (buf, rhs, EQ_OP);
	}
	if ( c == NE_OP ) {
		return !this->eval (buf, rhs, EQ_OP);
	}
	assert ("Bad value for c - should never get here.");
	return true;
}
