//============================================================================
// Name        : qwp.cpp
// Author      : devej
//============================================================================


//
// My first iteration of this tried to do it both with and without c++11.
// I've  abandoned that approach, but left this bit here for science.
//
#if __cplusplus < 201103L
    #include <bits/c++0x_warning.h>
    #warning "not using c++11 - good luck"
#else
    #define cpp11
    #pragma message "using c++ 11"
#endif


#include <cstdint>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>     // to catch and handle system signals
#include <vector>
#include <array>
#include <string.h>     // awkwardly for memset
#include <chrono>       // so we can time this thing
#include "row.h"


typedef std::chrono::high_resolution_clock::time_point	time_point_t;


// reduce the number of args passed to MakeRows with this struct
// doesn't really help performance of MakeRows much
struct MakeRowContext {
    MakeRowContext( uint64_t x, uint64_t m, Row& r ) : x_(x), max_width_(m), row_(r) {}
    const uint64_t x_;
    const uint64_t max_width_;
    Row& row_;
};

void MakeRows( const MakeRowContext& mrc  );
//void MakeRows( uint64_t x, uint64_t max, Row& row );

typedef std::vector< Row > Rows;

Rows rows;
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
        fprintf( stdout, "widths < 5 not supported\n" );    // randomly
        return 1;
    }


    Row r;
    matches = 0;
    MakeRows( MakeRowContext{ 0, Width, r } );
    const auto rows_size = rows.size();	// cache this value since I'll use it a couple times

    // midpoint timestamp
    const auto MakeRowStop = std::chrono::high_resolution_clock::now();
    const auto MakeRowTime = std::chrono::duration_cast< std::chrono::milliseconds >( MakeRowStop - start ).count();


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
        for( uint64_t y = 0; y < rows_size; ++y )
        {
            // no need to compare to myself, I align with me
            if( x == y )
                continue;

            if( !rows[x].alignswith( rows[y] ) ) {
                rows[x].others_.push_back( y );
            }
        }

        // set 1st layer of "multipliers" while we're iterating (save iterating again later)
        last_mult[x] = rows[x].others_.size();
    }


    std::vector< uint64_t > this_mult( rows_size );
    const auto sz = rows_size * sizeof this_mult[0];
    //Height -= 2;  // since we already calculated first row and it's others

    // midpoint timestamp
    const auto MakeOthersStop = std::chrono::high_resolution_clock::now();
    const auto MakeOthersTime = std::chrono::duration_cast< std::chrono::milliseconds >( MakeOthersStop - MakeRowStop ).count();

    // calculate the possibilities for each subsequent layer
    for( uint64_t h = 0; h < (Height-2); ++h )
    {
        for( uint64_t x = 0; x < rows_size; ++x )
        {
            const auto osz = rows[x].others_.size();

            for( uint64_t o = 0; o < osz; ++o )
            {
                this_mult[ rows[x].others_[o] ] += last_mult[x];
            }
        }

        this_mult.swap( last_mult );
        //std::fill( this_mult.begin(), this_mult.end(), 0 );	// really.. almost just as fast
        memset( &this_mult[0], 0, sz );
    }



    // sum the possibilities
    for( auto m: last_mult )
        matches += m;


    const auto CalcStop = std::chrono::high_resolution_clock::now();
    const auto CalcTime = std::chrono::duration_cast< std::chrono::milliseconds >( CalcStop - MakeOthersStop ).count();

    //const auto TotalStop = std::chrono::high_resolution_clock::now();
    const auto TotalTime = std::chrono::duration_cast< std::chrono::milliseconds >( CalcStop - start ).count();

    fprintf( stdout, "w( %lu, %lu ) == %lu (%lu msecs)\n", Width, (Height+2), matches, TotalTime );
    fprintf( stdout, "   MakeRows  : %lu\n", MakeRowTime );
    fprintf( stdout, "   MakeOthers: %lu\n", MakeOthersTime );
    fprintf( stdout, "   Calc      : %lu\n", CalcTime );
    // w( 9, 3 )   ==  8
    // w( 9, 4 )   == 10
    // w( 9, 5 )   == 14
    // w( 32, 10 ) == 806844323190414 :: 806,844,323,190,414
    return 0;
}


//
//  Makes all the possible rows for a given width
//
//void MakeRows( uint64_t x, uint64_t max_width, Row& row )
void MakeRows( const MakeRowContext& mrc )
{
    // for each position (x), try a 2brick & a 3brick, until we reach or exceed the max

    // make a copy for each branch
    Row r2 = mrc.row_;  //row;
    Row r3 = mrc.row_;  //row;

    // 2brick
    Brick b2(2);
    const uint64_t w2 = r2.AddBrick( b2, mrc.max_width_ );
    if( w2 )
        MakeRows( MakeRowContext{w2, mrc.max_width_, r2} );

    // 3brick
    Brick b3(3);
    const uint64_t w3 = r3.AddBrick( b3, mrc.max_width_ );
    if( w3 )
        MakeRows( MakeRowContext{w3, mrc.max_width_, r3} );

    // If we can't add either, we have maxed out.
    // If our width == the max, save this complete row.
    // Only make seems and save complete rows.
    if( !w2 && !w3 )
    {
        if( mrc.row_.width_ == mrc.max_width_ ) {
            mrc.row_.makeseams();
            rows.push_back( mrc.row_ );
        }
    }
}

