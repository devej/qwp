/*
 * row.h
 *      Author: devej
 */

#ifndef ROW_H_
#define ROW_H_

#include <vector>
#include <stdlib.h>
#include <cstdint>
#include <numeric>



struct Brick
{
    Brick( uint64_t width ) : width_(width) {}
    const uint64_t width_;
};


// A Row is a collection of Bricks

class Row
{
public:
    std::vector< uint64_t > seams_;     // the seams that the bricks make
    std::vector< uint64_t > others_;    // all the row #s that do not align with this row

    uint64_t                width_;     // cache the row width so it doesn't need re-calculating all the time
    const uint64_t          max_width_;


    Row( uint64_t max ) : width_(0), max_width_(max) {
        others_.reserve( 2048 );    // swag
        seams_.reserve( 64 );
    }


    uint64_t AddBrick( const Brick& b )
    {
        // add a new brick if possible
        // return the new width or 0 if not possible to add
        if( (width_ + b.width_) > max_width_ )
            return 0;

        if( width_ )
            seams_.emplace_back( width_ );

        width_ += b.width_;
        return width_;
    }


    bool AlignsWith( const Row& rhs, uint64_t& calcs ) const
    {
        // Compare each of my seams to the other guy's seams
        // If I find a match, I align.

        auto rhsseam = rhs.seams_.begin();
        const auto rhsend = rhs.seams_.end();

        for( auto& myseam : seams_ )
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


    //
    // A slightly different method of doing the same thing as AlignsWith.
    // It's a little faster.
    //
    bool AlignsWith2( const Row& rhs, uint64_t& calcs ) const
    {
        // Compare each of my seams to the other guy's seams
        // If I find a match, I align.

        // optimization. ~50% of first seams will be the same (either a 2 or a 3)
        if( seams_[0] == rhs.seams_[0] )
            return true;

        // optimization? ~25% of second seams will be the same (either a 4/5 or 5/6)
        if( seams_[1] == rhs.seams_[1] )
            return true;

        const uint64_t mysz = seams_.size();
        const uint64_t rhssz = rhs.seams_.size();
        uint64_t y = 1;

        for( uint64_t x = 1; x < mysz; ++x )
        {
            for( ; y < rhssz; ++y )
            {
                ++calcs;

                if( seams_[x] == rhs.seams_[y] )
                    return true;

                if( seams_[x] < rhs.seams_[y] ) // need to search the rest?
                    break;      // nope, break out of 'y' loop
            }
        }

        return false;
    }
};


#endif /* ROW_H_ */
