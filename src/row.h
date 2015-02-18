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
	uint64_t				index_;
	std::vector< Brick >	bricks_;
	std::set< uint64_t >	seams_;
	std::vector< uint64_t >	others_;

	// cache the row width so it doesn't need re-calculating all the time
	uint64_t				width_;


	Row() : index_(0), width_(0) {
		others_.reserve( 2048 );
	}


	uint64_t AddBrick( const Brick& b, uint64_t max_width )
	{
		if( (width_ + b.width_) > max_width )
			return 0;

		bricks_.push_back( b );
		width_ += b.width_;
		return width_;
	}


	uint64_t GetWidth()
	{
		return width_;
	}


	void print()
	{
#ifdef cpp11
		for( Brick brick: bricks_ )
			fprintf( stdout, "%lu ", brick.width_ );
#else
		std::vector< Brick >::iterator i = bricks_.begin();
		std::vector< Brick >::iterator end = bricks_.end();
		for( ; i != end; ++i )
			fprintf( stdout, "%lu ", i->width_ );
#endif
		
		fprintf( stdout, "\n" );
	}


	bool alignswith( Row& rhs )
	{
		// Compare each of my seams to the other guy's seams
		// If I find a match, I align.
//#ifdef cpp11
//		// optimization: check first brick - ~50% of the time it will be the same
//		if( *seams_.begin() == *rhs.seams_.begin() )
//			return true;
//
//		auto e = rhs.seams_.end();
//
//		for( auto seam : seams_ )
//		{
//			if( e != rhs.seams_.find( seam ) )
//				return true;
//		}
//#else
		// I think the cpp98 style is a little faster
		std::set< uint64_t >::iterator i = seams_.begin();
		std::set< uint64_t >::iterator end = seams_.end();
		std::set< uint64_t >::iterator e = rhs.seams_.end();

		// optimization: check first brick - ~50% of the time it will be the same
		if( *i == *rhs.seams_.begin() )
			return true;

		++i;

		for( ; i != end; ++i )
		{
			if( e != rhs.seams_.find( *i ) )
				return true;

		}
//#endif

		return false;
	}


	void makeseams()
	{
		// calculate my seams
		//seams_.clear();
		uint64_t c = 0;
#ifdef cpp11
		auto i = bricks_.begin();
		auto end = bricks_.end() - 1;	// exclude the last entry
#else
		std::vector< Brick >::iterator i = bricks_.begin();
		std::vector< Brick >::iterator end = bricks_.end() - 1;	// exclude the last entry
#endif

		for( ; i < end; ++i )
		{
			c += i->width_;
			seams_.insert( c );
		}
	}
};


#endif /* ROW_H_ */
