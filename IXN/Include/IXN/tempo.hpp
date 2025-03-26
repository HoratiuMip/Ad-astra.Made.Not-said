#pragma once
/*
*/

#include <IXN/descriptor.hpp>

namespace _ENGINE_NAMESPACE {



enum TICK {
    TICK_NANOS = 0, TICK_MICROS, TICK_MILLIS, TICK_SECS, TICK_MINS, TICK_HRS
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

_ENGINE_PROTECTED:
    std::chrono::high_resolution_clock::time_point   _create     = {};
    std::chrono::high_resolution_clock::time_point   _last_lap   = {};

public:
    template< TICK unit = TICK_SECS >
    double up_time() const {
        using namespace std::chrono;

        return duration< double >( high_resolution_clock::now() - _create ).count() * TICK_MULS[ unit ];
    }

    template< TICK unit = TICK_SECS >
    double peek_lap() const {
        using namespace std::chrono; 

        return duration< double >( high_resolution_clock::now() - _last_lap ).count() * TICK_MULS[ unit ];
    }

    template< TICK unit = TICK_SECS >
    double lap() {
        using namespace std::chrono;

        auto now = high_resolution_clock::now();

        return duration< double >( now - std::exchange( _last_lap, now ) ).count() * TICK_MULS[ unit ];
    }

    template< TICK unit = TICK_SECS >
    double cmpxchg_lap( double floating ) {
        if( this->peek_lap< unit >() < floating )
            return false;
        return this->lap();
    }

_ENGINE_PROTECTED:
    template< TICK unit >
    struct _Map {
        typedef std::conditional_t< unit == TICK_NANOS,  std::chrono::nanoseconds,
                std::conditional_t< unit == TICK_MICROS, std::chrono::microseconds,
                std::conditional_t< unit == TICK_MILLIS, std::chrono::milliseconds,
                std::conditional_t< unit == TICK_SECS,   std::chrono::seconds,
                std::conditional_t< unit == TICK_MINS,   std::chrono::minutes,
                std::conditional_t< unit == TICK_HRS,    std::chrono::hours, 
                void > > > > > > type;
    };

public:
    template< TICK unit = TICK_SECS >
    static auto epoch() {
        using namespace std::chrono;

        return duration_cast< typename _Map< unit >::type >(
            high_resolution_clock::now().time_since_epoch()
        ).count();
    }

};



};
