#pragma once

#include <vector>
#include <cstdint>


// A Row is a collection of Bricks
// But all we need to save is the seams they create when placed

template< uint8_t MaxWidth >
class Row
{
public:
    std::vector< uint8_t >  _seams;     // the seams that the bricks make
    std::vector< uint64_t > _others;    // all the row #s that do not align with this row
    uint8_t                 _width = 0; // cache the row width so it doesn't need re-calculating all the time


    Row() {
        _others.reserve( 2048 );    // swag
        _seams.reserve( (MaxWidth/2) +1 );
    }

    Row( const Row& ) = default;

    // specialized below
    template< uint8_t BrickWidth >
    inline uint8_t AddBrick()
    {
        // add a new brick if possible
        // return the new width or 0 if not possible to add
        if( (_width + BrickWidth) > MaxWidth )
            return 0;

        if( _width )
            _seams.emplace_back( _width );

        _width += BrickWidth;
        return _width;
    }

/*
    bool AlignsWith( const Row& rhs, uint64_t& calcs ) const
    {
        // Compare each of my seams to the other guy's seams
        // If I find a match, I align.

        auto rhsseam = rhs._seams.begin();
        const auto rhsend = rhs._seams.end();

        for( auto& myseam : _seams )
        {
            for( ; rhsseam != rhsend; ++rhsseam )
            {
                ++calcs;

                if( myseam == *rhsseam )
                    return true;

                if( myseam < *rhsseam ) // need to search the rest?
                    break;      // nope
            }
        }

        return false;
    }
*/

    //
    // A slightly different method of doing the same thing as AlignsWith.
    // It's a little faster.
    //
    bool AlignsWith2( const Row& rhs ) const
    {
        // Compare each of my seams to the other guy's seams
        // If I find a match, I align.

        // optimization. ~50% of first seams will be the same (either a 2 or a 3)
        if( _seams[0] == rhs._seams[0] )
            return true;

        // optimization? ~25% of second seams will be the same (either a 4/5 or 5/6)
        if( _seams[1] == rhs._seams[1] )
            return true;

        const auto mysz = _seams.size();
        const auto rhssz = rhs._seams.size();
        std::size_t y = 1;

        for( uint64_t x = 1; x < mysz; ++x )
        {
            for( ; y < rhssz; ++y )
            {
                if( _seams[x] == rhs._seams[y] )
                    return true;

                if( _seams[x] < rhs._seams[y] ) // need to search the rest?
                    break;      // nope, break out of 'y' loop
            }
        }

        return false;
    }
};


    // //
    // // AddBrick specializations
    // //   Turns out it's not really worth the extra code
    // //
    // template<>
    // inline uint64_t Row::AddBrick<2>()
    // {
    //     // add a new brick if possible
    //     // return the new width or 0 if not possible to add
    //     if( (_width + 2) > _max_width )
    //         return 0;

    //     if( _width )
    //         _seams.emplace_back( _width );

    //     _width += 2;
    //     return _width;
    // }

    // template<>
    // inline uint64_t Row::AddBrick<3>()
    // {
    //     // add a new brick if possible
    //     // return the new width or 0 if not possible to add
    //     if( (_width + 3) > _max_width )
    //         return 0;

    //     if( _width )
    //         _seams.emplace_back( _width );

    //     _width += 3;
    //     return _width;
    // }
