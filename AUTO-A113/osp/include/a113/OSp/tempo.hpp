#pragma once
/**
 * @file: OSp/tempo.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <a113/osp/core.hpp>

namespace a113 {

enum Tick_ {
    Tick_ns = 0, Tick_us, Tick_ms, Tick_s, Tick_m, Tick_h
};

inline constexpr double TICK_MULS[] = { 
    1'000'000'000.0, 
    1'000'000.0, 
    1'000.0,
    1.0, 
    1.0 / 60.0,
    1.0 / 3600.0
};


struct ticker_lap_epoch_init_t{};

class Ticker {
public:
    Ticker()
    : _create{ std::chrono::high_resolution_clock::now() },
      _last_lap{ std::chrono::high_resolution_clock::now() }
    {}

    Ticker( [[maybe_unused]]ticker_lap_epoch_init_t )
    : _create{ std::chrono::high_resolution_clock::now() },
      _last_lap{}
    {}

_A113_PROTECTED:
    std::chrono::high_resolution_clock::time_point   _create     = {};
    std::chrono::high_resolution_clock::time_point   _last_lap   = {};

public:
    template< Tick_ _UNIT_ = Tick_s > double up_time( void ) const {
        using namespace std::chrono;
        return duration< double >( high_resolution_clock::now() - _create ).count() * TICK_MULS[ _UNIT_ ];
    }

    template< Tick_ _UNIT_ = Tick_s > double peek_lap( void ) const {
        using namespace std::chrono; 
        return duration< double >( high_resolution_clock::now() - _last_lap ).count() * TICK_MULS[ _UNIT_ ];
    }

    template< Tick_ _UNIT_ = Tick_s > double lap( void ) {
        using namespace std::chrono;
        auto now = high_resolution_clock::now();
        return duration< double >( now - std::exchange( _last_lap, now ) ).count() * TICK_MULS[ _UNIT_ ];
    }

    template< Tick_ _UNIT_ = Tick_s >
    double cmpxchg_lap( double value_ ) {
        if( this->peek_lap< _UNIT_ >() < value_ )return false;
        return this->lap();
    }

_A113_PROTECTED:
    template< Tick_ _UNIT_ >
    struct _Map {
        typedef std::conditional_t< _UNIT_ == Tick_ns,  std::chrono::nanoseconds,
                std::conditional_t< _UNIT_ == Tick_us, std::chrono::microseconds,
                std::conditional_t< _UNIT_ == Tick_ms, std::chrono::milliseconds,
                std::conditional_t< _UNIT_ == Tick_s,   std::chrono::seconds,
                std::conditional_t< _UNIT_ == Tick_m,   std::chrono::minutes,
                std::conditional_t< _UNIT_ == Tick_h,    std::chrono::hours, 
                void > > > > > > type;
    };

public:
    template< Tick_ _UNIT_ = Tick_s > static auto epoch( void ) {
        using namespace std::chrono;
        return duration_cast< typename _Map< _UNIT_ >::type >(
            high_resolution_clock::now().time_since_epoch()
        ).count();
    }

};



};
