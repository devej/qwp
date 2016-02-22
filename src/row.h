/*
 * row.h
 *
 *      Author: devej
 */

#ifndef ROW_H_
#define ROW_H_

#include <vector>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>



struct Brick
{
	Brick( uint64_t width ) : width_(width) {}
	const uint64_t width_;
};


// A Row is a collection of Bricks

class Row
{
public:
	std::vector< Brick >    bricks_;    // the bricks in this row
	std::vector< uint64_t > seams_;     // the seams that the bricks make
	std::vector< uint64_t > others_;    // all the other rows that do not align with this row

	uint64_t                width_;     // cache the row width so it doesn't need re-calculating all the time


	Row() : width_(0) {
		others_.reserve( 2048 );    // swag
	}


	uint64_t AddBrick( const Brick& b, uint64_t max_width )
	{
	    // add a new brick if possible
	    // return the new width or 0 if not possible to add
		if( (width_ + b.width_) > max_width )
			return 0;

		bricks_.push_back( b );
		width_ += b.width_;
		return width_;
	}


//	uint64_t GetWidth() const
//	{
//		return width_;
//	}


//	void print()
//	{
//		for( auto brick: bricks_ )
//			fprintf( stdout, "%lu ", brick.width_ );
//
//		fprintf( stdout, "\n" );
//	}


	bool alignswith( const Row& rhs ) const
	{
		// Compare each of my seams to the other guy's seams
		// If I find a match, I align.

	    auto rhsseam = rhs.seams_.begin();
		const auto rhsend = rhs.seams_.end();

		for( auto myseam : seams_ )
		{
		    for( ; rhsseam != rhsend; ++rhsseam )
		    {
                if( myseam == *rhsseam )
                    return true;

                if( myseam < *rhsseam ) // need to search the rest?
                    break;      // nope
		    }
		}

		return false;
	}


	void makeseams()
	{
		// calculate & cache my seams
		uint64_t c = 0;

		auto i = bricks_.begin();
		auto end = bricks_.end() - 1;	// exclude the last brick - it's not a seam

		for( ; i < end; ++i )
		{
			c += i->width_;
			seams_.push_back( c );
		}
	}
};


#endif /* ROW_H_ */
