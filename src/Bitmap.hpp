#pragma once
#include "Utils.hpp"

#include <cmath>
#include <cstring>
#include <cassert>


class Bitmap {
public:
	Bitmap (size_t numBits);
	Bitmap (char * buf, size_t numBits); //deserialize from buf
	~Bitmap ( );

	void set (unsigned int bitNumber);
	void set ( ); // set all bits to 1
	void reset (unsigned int bitNumber);
	void reset ( ); // set all bits to 0
	bool test (unsigned int bitNumber) const;

	int numChars ( ) const; // return size of char buffer to hold bitmap
	int to_char_buf (char *, size_t len) const; //serialize content to char buffer
	int getSize ( ) const { return size; }
private:
	unsigned int size;
	char * buffer;
};

std::ostream& operator <<(std::ostream & os, const Bitmap& b);


Bitmap::Bitmap (size_t numBits) : size (numBits) {
	buffer = new char[this->numChars ( )];
	// zero out to avoid valgrind warnings.
	memset (( void* ) buffer, 0, this->numChars ( ));
	this->reset ( );
}

Bitmap::Bitmap (char * buf, size_t numBits) : size (numBits) {
	buffer = new char[this->numChars ( )];
	memcpy (buffer, buf, this->numChars ( ));
}

int Bitmap::to_char_buf (char * b, size_t len) const //copy content to char buffer -
{
	assert (b != NULL && len == this->numChars ( ));
	memcpy (( void* ) b, buffer, len);
	return 0;
}

Bitmap::~Bitmap ( ) {
	delete[] buffer;
}

int Bitmap::numChars ( ) const {
	int numChars = ( size / 8 );
	if ( ( size % 8 ) != 0 )
		numChars++;
	return numChars;
}

void Bitmap::reset ( ) {
	for ( unsigned int i = 0; i < size; i++ ) {
		Bitmap::reset (i);
	}
}

void Bitmap::reset (unsigned int bitNumber) {
	assert (bitNumber <= ( size - 1 ));
	int byte = bitNumber / 8;
	int offset = bitNumber % 8;

	buffer[byte] &= ~( 1 << offset );
}

void Bitmap::set (unsigned int bitNumber) {
	assert (bitNumber <= size - 1);
	int byte = bitNumber / 8;
	int offset = bitNumber % 8;

	buffer[byte] |= ( 1 << offset );
}

void Bitmap::set ( ) {
	for ( unsigned int i = 0; i < size; i++ ) {
		Bitmap::set (i);
	}
}

bool Bitmap::test (unsigned int bitNumber) const {
	assert (bitNumber <= size - 1);
	int byte = bitNumber / 8;
	int offset = bitNumber % 8;

	return (buffer[byte] & ( 1 << offset )) != 0;
}


std::ostream& operator <<(std::ostream & os, const Bitmap& b) {
	os << "[";
	for ( int i = 0; i < b.getSize ( ); i++ ) {
		if ( i % 8 == 0 && i != 0 )
			os << ".";
		os << ( b.test (i) ? 1 : 0 );
	}
	os << "]";
	return os;
}