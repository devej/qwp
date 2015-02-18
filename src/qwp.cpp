//============================================================================
// Name        : qwp.cpp
// Author      : devej
// Version     :
// Copyright   :
//============================================================================



#if __cplusplus < 201103L
	#include <bits/c++0x_warning.h>
#else
	#define cpp11
#endif


#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <vector>
#include <string.h>	//for memset
#include <chrono>
#include "row.h"


typedef std::chrono::high_resolution_clock::time_point	time_point_t;


void MakeRows( uint64_t x, uint64_t max, Row& row );

typedef std::vector< Row > Rows;

Rows rows;
uint64_t Gcount;
uint64_t Gindex;
uint64_t matches;
uint64_t Width;
uint64_t Height;



void signal_callback_handler( int signum )
{
	fprintf( stdout, "Caught signal %d, shutting down\n", signum );
	fflush( stdout );
	exit( signum );
}



int main( int argc, char**argv )
{
	time_point_t start = std::chrono::high_resolution_clock::now();

	// expect two dimensions as input e.g to solve W(32,10), "qwp 32 10"

	if( argc != 3 ) {
		fprintf( stderr, "Please input two dimensions as width & height: w( W H )\n"
				"\te.g.\'qwp 9 3\'\n"
				"\te.g.\'qwp 32 10\'\n");
		return 1;
	}


	// Register signals and signal handlers (for CTRL+C & other signal) handling
	signal( SIGINT, signal_callback_handler );
	signal( SIGTERM, signal_callback_handler );
#if defined(WIN32) || defined(WINDOWS)
	signal( SIGKILL, signal_callback_handler );	// only on linux
#endif


	Width = atol( argv[1] );
	Height = atol( argv[2] );
	//fprintf( stdout, "Solving for W(%lu,%lu)\n", Width, Height );


	// handle simple cases
	if( 0 >= Height ) {
		fprintf( stdout, "W( %lu, %lu ) = 0\n", Width, Height );
		return 0;
	} else if( 5 > Width ) {
		fprintf( stdout, "widths < 5 not supported\n" );
		return 1;
	}


	Row r;
	matches = 0;
	Gcount = 0;
	Gindex = 0;

	MakeRows( 0, Width, r );
	const uint64_t rows_size = rows.size();	// cache this value since I'll use it a couple times
	//fprintf( stdout, "Ran %lu calculations, resulting in %lu rows\n", Gcount, rows.size() );
	
	// handle another simple case
	if( 1 == Height ) {
		fprintf( stdout, "W( %lu, %lu ) = %lu\n", Width, Height, rows_size );
		return 0;
	}


	std::vector< uint64_t > last_mult( rows_size );


	// Calculate & store each row's matching rows (its "others")
	// i.e. How many other rows are compatible w/ each row?
	for( uint64_t x = 0; x < rows_size; ++x )
	{
		//fprintf( stdout, "Row %lu -> ", x );
		for( uint64_t y = 0; y < rows_size; ++y )
		{
			// no need to compare to myself, I align with me
			if( x == y )
				continue;

			if( !rows[x].alignswith( rows[y] ) ) {
				//rows[x].others_.push_back( &rows[y] );
				rows[x].others2_.push_back( y );
				//fprintf( stdout, "%lu ", y );
			}
		}

		// set 1st layer of "multipliers" while we're iterating (save iterating again later)
		last_mult[x] = rows[x].others2_.size();
		//fprintf( stdout, "\n" );
	}


	std::vector< uint64_t > this_mult( rows_size );
	const size_t sz = rows_size * sizeof this_mult[0];
	Height -= 2;


	// calculate the possibilities for each subsequent layer
	for( uint64_t h = 0; h < Height; ++h )
	{
		//std::vector< uint64_t > this_mult( rows_size );

		//fprintf( stdout, "H=%lu\n", h );
		for( uint64_t x = 0; x < rows_size; ++x )
		{
			const uint64_t osz = rows[x].others2_.size();

			for( uint64_t o = 0; o < osz; ++o )
			{
				//fprintf( stdout, "\tthis_mult[ row[%lu]->%lu ] += %lu",
				//		x, rows[x].others_[o]->index_, last_mult[x] );
				this_mult[ rows[x].others2_[o] ] += last_mult[x];
				//fprintf( stdout, " == %lu\n", this_mult[ rows[x].others_[o]->index_ ] );
			}
		}

		this_mult.swap( last_mult );
		//std::fill( this_mult.begin(), this_mult.end(), 0 );	// really.. almost just as fast
		memset( &this_mult[0], 0, sz );
	}



	// sum the possibilities
#ifdef cpp11
	for( auto m: last_mult )
		matches += m;
#else
	std::vector< uint64_t >::iterator i = last_mult.begin();
	std::vector< uint64_t >::iterator end = last_mult.end();
	for( ; i != end; ++i )
		matches += *i;
#endif


	time_point_t stop = std::chrono::high_resolution_clock::now();
	const uint64_t msecs = std::chrono::duration_cast< std::chrono::milliseconds >( stop - start ).count();

	fprintf( stdout, "w( %lu, %lu ) == %lu (%lu msecs)\n", Width, Height, matches, msecs );
	// w( 9, 3 )   ==  8
	// w( 9, 4 )   == 10
	// w( 9, 5 )   == 14
	// w( 32, 10 ) == 806844323190414 :: 806,844,323,190,414
	return 0;
}


//
//  Makes all the possible rows for a given width
//
void MakeRows( uint64_t x, uint64_t max_width, Row& row )
{
	// for each position (x), try a 2brick & a 3brick, until we reach or exceed the max
	++Gcount;

	// make a copy for each branch
	Row r2 = row;
	Row r3 = row;

	// 2brick
	Brick b2(2);
	uint64_t w2 = r2.AddBrick( b2, max_width );
	if( w2 )
		MakeRows( w2, max_width, r2 );

	// 3brick
	Brick b3(3);
	uint64_t w3 = r3.AddBrick( b3, max_width );
	if( w3 )
		MakeRows( w3, max_width, r3 );

	// If we can't add either, we have maxed out.
	// See if our width == the max.
	// If so, save this complete row.
	if( !w2 && !w3 )
	{
		if( row.GetWidth() == max_width ) {
			row.makeseams();
			row.index_ = Gindex++;	// set each row's cardinal index as we add it
			rows.push_back( row );
		}
	}
}

