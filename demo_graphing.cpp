#define _9T_UNIQUE_SURFACE
#define _9T_ECHO
#include "C:\\Hackerman\\Git\\For_Academical_Purposes\\_9T.cpp"
using namespace _9T;


using FunctionX  = System2Node::Function;
using Collection = System2Node::Collection;
using FunctionXY = std::function< double( double, double ) >;



struct DivDiff {
public:
    DivDiff() = default;

    DivDiff( const Collection& pts ) {
        _n = pts.size() - 1;

        _diffs.reserve( n * ( n + 1 ) / 2 );

        for( size_t idx = 0; idx < n; ++idx )
            _diffs.emplace_back( 
                ( pts[ idx + 1 ].y - pts[ idx ].y ) 
                / 
                ( pts[ idx + 1 ].x - pts[ idx ].x ) 
            );

         }

private:
    std::vector< double >   _diffs   = {};
    size_t                  _n       = 0;

public:
    double operator () ( size_t row, size_t col ) const {
        return _diffs[ _n * ( _n + 1 ) / 2 - ( _n - col ) * ( _n - col + 1 ) / 2 + row ];
    }

};



struct Example {
    std::string   name;
    std::string   equation;
    FunctionX     solution;
    FunctionXY    fxy;
    double        a;
    double        b;
} examples[] = {
    {
        name:     "Example_1",
        equation: "y' = y",
        solution: [] ( double x ) { return pow( 2.71828, x ); },
        fxy:      [] ( double x, double y ) { return y; },
        a:        -5.0,
        b:        5.0
    },
    {
        name:     "Example_2",
        equation: "",
        solution: [] ( double x ) { return 1.0 / ( 1.0 - x ); },
        fxy:      [] ( double x, double y ) { return std::pow( y, 2 ); },
        a:        -5.0,
        b:        5.0
    }
};



void load_examples( System2& system ) {
    std::shared_ptr< SolidBrush2 > brush{
        new SolidBrush2{ 
            system.viewport().renderer(), 
            { 64.0 / 255.0, 225.0 / 255.0, 225.0 / 255.0, 1.0 }, 
            2.0 
        }
    };

    for( auto& ex : examples )
        system.insert( {
            ex.name,
            ex.solution
        } ).first->second.give_brush( brush );

    system.begin()->second.uplink();
}



double* coeffs[] = {
    ( double[] ){ 1.0 },
    ( double[] ){ 3.0 / 2.0, -1.0 / 2.0 },
    ( double[] ){ 23.0 / 12.0, -16.0 / 12.0, 5.0 / 12.0 }
};

Collection accumulate( 
    size_t idx,
    size_t steps,
    size_t known,
    size_t lookback 
) {
    Collection acc{};

    auto lookback_n = [ & ] ( size_t n, double y ) -> double {
        double res = 0.0;

        res += coeffs[ n ][ 0 ] * y;

        for( size_t idx = 1; idx <= n; ++idx ) {
            res += coeffs[ n ][ idx ] * ( acc.end() - 1 - idx )->y;
        }

        return res;
    };

    auto&  ex  = examples[ idx ];
    double h   = ( ex.b - ex.a ) / steps;
    size_t at  = 1;

    for( ; at <= known; ++at ) {
        double x = ex.a + h * ( at - 1 );
        
        acc.emplace_back( x, ex.solution( x ) );
    }

    for( ; at <= steps; ++at ) {
        auto [ x, y ] = acc.back();

        x += h;

        acc.emplace_back(
            x,
            y + h * lookback_n(
                std::min( at - 1, lookback ),
                ex.fxy( x, y )
            )
        );
    }

    return acc;
}



int main() {
    Surface surf{ "ODEs Linear Multistep", { 0, 0 }, { Env::width(), Env::height() } };

    Renderer2 renderer{ surf };

    Viewport2 viewport{ renderer, Coord<>{ 0, 0 }, { 1000, surf.height() } }; 
    viewport.uplink();

    System2 system{ viewport, 60, 1.0, 12, { 1.0, 1.0, 1.0, 0.8 }, 2.0 };
    system.uplink_auto_roam();


    load_examples( system );


    size_t at       = 0;
    size_t steps    = 30;
    size_t known    = 1;
    size_t lookback = 0;

    system.insert( { "approx", accumulate( at, steps, known, lookback ) } );
    system[ "approx" ].uplink().give_brush( 
        std::make_shared< SolidBrush2 >( renderer, Chroma{ 1.0, 0.0, 0.0, 1.0 }, 2.0 ) 
    );

    std::atomic< size_t > refreshes{ 0 };

    auto note_refresh = [ & ] () -> void {
        refreshes.fetch_add( 1, std::memory_order_relaxed );
    };


    surf.on< SURFACE_EVENT_KEY >( [ & ] ( Key key, KEY_STATE state, auto& trace ) -> void {
        switch( key ) {
            case Key::RIGHT: {
                if( state == KEY_STATE_UP ) break;

                system[ examples[ at ].name ].downlink();

                at = ( at + 1 ) % std::size( examples );

                system[ examples[ at ].name ].uplink();

                note_refresh();
            break; }

            case Key::LEFT: {
                if( state == KEY_STATE_UP ) break;

                system[ examples[ at ].name ].downlink();

                at = ( ( at - 1 ) > std::size( examples ) ) ? std::size( examples ) - 1 : at - 1;

                system[ examples[ at ].name ].uplink();

                note_refresh();
            break; }
        

            case Key::UP: {
                if( state == KEY_STATE_UP ) break;

                steps++;

                note_refresh();
            break; }

            case Key::DOWN: {
                if( state == KEY_STATE_UP ) break;

                steps = ( steps == 1 ) ? 1 : steps - 1;

                note_refresh();
            break; }
        }
    } );
    

    while( !Key::down( Key::ESC ) ) {
        static Clock clk;


        renderer.open().fill( { 30, 31, 34, 1 } );


        viewport.fill( { 43, 45, 49, 1 } );

        system.render( renderer );


        renderer.execute();


        if( refreshes.fetch_add( 0, std::memory_order_relaxed ) == 0 )
            goto L_NO_REFRESH;

        system[ "approx" ].collection() = std::move( accumulate( at, steps, known, lookback ) );


        L_NO_REFRESH: continue;
    }


    return 0;
}
