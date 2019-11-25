
#include "row.h"
#include <cstdint>
#include <iostream>
#include <vector>
#include <string.h>     // awkwardly for memset
#include <chrono>       // so we can time this thing


constexpr uint8_t Width = 32;
constexpr uint8_t Height = 10;

using Rows = std::vector< Row<Width> >;
Rows rows;

void MakeRows( Row<Width>& row  );



int main( int /*argc*/, char**/*argv*/ )
{
    auto CalcStart = std::chrono::system_clock::now();

    Row<Width> r;
    uint64_t matches = 0;
    MakeRows( r );
    const auto rows_size = rows.size();	// cache this value since I'll use it a couple times

    std::vector< uint64_t > last_mult( rows_size );
    std::vector< uint64_t > this_mult( rows_size );

    // take a timestamp
    const auto MakeRowStop = std::chrono::system_clock::now();

    // Calculate & store each row's matching rows (aka, its "others")
    // i.e. How many other rows are compatible w/ each row?

    for( uint64_t x = 0; x < rows_size; ++x )
    {
        for( uint64_t y = 0; y < rows_size; ++y )
        {
            // no need to compare to myself, I align with me
            if( x == y )
                continue;

            if( !rows[x].AlignsWith2( rows[y] ) ) {
                rows[x]._others.emplace_back( y );
            }
        }

        // set 1st layer of "multipliers" while we're iterating (save iterating again later)
        last_mult[x] = rows[x]._others.size();
    }

    // take a timestamp
    const auto MakeOthersStop = std::chrono::system_clock::now();

    const auto sz = rows_size * sizeof this_mult[0];
    //Height -= 2;  // since we already calculated first row and it's others

    // calculate the possibilities for each subsequent layer
    for( uint64_t h = 0; h < (Height-2); ++h )
    {
        for( uint64_t x = 0; x < rows_size; ++x )
        {
            const auto osz = rows[x]._others.size();

            for( uint64_t o = 0; o < osz; ++o )
            {
                this_mult[ rows[x]._others[o] ] += last_mult[x];
            }
        }

        this_mult.swap( last_mult );
        //std::fill( this_mult.begin(), this_mult.end(), 0 );	// really.. almost just as fast
        memset( &this_mult[0], 0, sz );
    }


    // sum the possibilities
    for( const auto m: last_mult )
        matches += m;

    // calcualte some timings
    const auto CalcStop = std::chrono::system_clock::now();
    const auto MakeRowTimeMs = std::chrono::duration_cast< std::chrono::milliseconds >( MakeRowStop - CalcStart ).count();
    const auto MakeOthersTimeMs = std::chrono::duration_cast< std::chrono::milliseconds >( MakeOthersStop - MakeRowStop ).count();
    const auto CalcTimeMs = std::chrono::duration_cast< std::chrono::milliseconds >( CalcStop - MakeOthersStop ).count();
    const auto TotalTimeMs = std::chrono::duration_cast< std::chrono::milliseconds >( CalcStop - CalcStart ).count();

    std::cout << "w( " << static_cast<uint16_t>(Width) << ", " << static_cast<uint16_t>(Height) << " ) = " << matches << " (" << TotalTimeMs << " ms)\n"
        << "   MakeRows ms: " << MakeRowTimeMs << '\n'
        << " MakeOthers ms: " << MakeOthersTimeMs << '\n'
        << "       Calc ms: " << CalcTimeMs << '\n';
    // w( 9, 3 )   ==  8
    // w( 9, 4 )   == 10
    // w( 9, 5 )   == 14
    // w( 32, 10 ) == 806844323190414 :: 806,844,323,190,414
    return 0;
}


//
//  Makes all the possible rows for a given width
//
void MakeRows( Row<Width>& row )
{
    // for each row, try adding 2brick & a 3brick, until we reach or exceed the max

    // make a copy for each branch
    Row<Width> r2( row );  //row;
    Row<Width> r3( row );  //row;

    // 2brick
    const uint8_t w2 = r2.AddBrick<2>();
    if( w2 )
        MakeRows( r2 );

    // 3brick
    const uint8_t w3 = r3.AddBrick<3>();
    if( w3 )
        MakeRows( r3 );

    // If we can't add either, we have maxed out this row.
    // If our width == the max, save this complete row.
    //if( !w2 && !w3 )
    if( !w2  ) // if 2 didn't fit, 3 certainly won't either
    {
        if( row._width == Width ) {
            rows.emplace_back( row );
        }
    }
}
