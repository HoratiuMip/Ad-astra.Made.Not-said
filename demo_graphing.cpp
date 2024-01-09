#define _9T_UNIQUE_SURFACE
#define _9T_ECHO
#include "C:\\Hackerman\\Git\\For_Academical_Purposes\\_9T.cpp"
using namespace _9T;


using FunctionX  = System2Node::Function;
using Collection = System2Node::Collection;
using FunctionXY = std::function< double( double, double ) >;



struct Example {
    std::string   name;
    std::string   equation;
    FunctionX     solution;
    FunctionXY    fxy;
    double        a;
    double        b;
} examples[] = {
    {
        name:     "Example_0",
        equation: "",
        solution: [] ( double x ) { return pow( 2.71828, x ); },
        fxy:      [] ( double x, double y ) { return y; }
    },
    {
        name:     "Example_1",
        equation: "",
        solution: [] ( double x ) { return x * x; },
        fxy:      [] ( double x, double y ) { return 2.0 * x; }
    },
    {
        name:     "Example_2",
        equation: "",
        solution: [] ( double x ) { return 2.0 * atan( 1.0 / pow( 2.71828, 1.0 - x ) ); },
        fxy:      [] ( double x, double y ) { return sin( y ); }
    },
    {
        name:     "Example_3",
        equation: "",
        solution: [] ( double x ) { return x * x * x; },
        fxy:      [] ( double x, double y ) { return 3.0 * x * x; }
    },
    {
        name:     "Example_4",
        equation: "",
        solution: [] ( double x ) { return pow( 2.71828, x / 2.0 ) - x - 2.0; },
        fxy:      [] ( double x, double y ) { return ( x + y ) / 2.0; }
    }
};



void load_examples( System2& system ) {
    std::shared_ptr< SolidBrush2 > brush{
        new SolidBrush2{ 
            system.viewport().renderer(), 
            { 1.0, 0.26, 0.0, 1.0 }, 
            3.0 
        }
    };

    for( auto& ex : examples )
        system.insert( {
            ex.name,
            ex.solution
        } ).first->second.give_brush( brush );

    system.begin()->second.uplink();
}



double int_of( FunctionX f, double a, double b ) {
    constexpr size_t steps = 20;

    double h = ( b - a ) / steps;

    double res = f( a ) + f( b );

    for( a += h; a < b; a+= h )
        res += 2 * f( a );

    return res *= h / 2;
}

double fact( double n ) {
    double res = 1;

    for( double i = 1; i <= n; ++i )
        res *= i;

    return res;
}

Collection accumulate( 
    size_t idx,
    size_t steps,
    size_t known,
    size_t consider,
    double a,
    double b 
) {
    Collection acc{};

    auto&  ex       = examples[ idx ];
    double h        = ( b - a ) / steps;
    size_t at       = 1;
    size_t crt_cons = 1;


    auto coeff_of = [ & ] ( size_t n ) -> double {
        auto prod_n = [ & ] ( double x ) -> double {
            double p = 1.0;

            for( size_t i = 0; i < crt_cons; ++i ) {
                if( i == n ) continue;

                p *= ( x + i );
            }

            return p;
        };

        return ( n % 2 ? -1.0 : 1.0 ) 
               / 
               ( fact( n ) * fact( crt_cons - n - 1 ) ) 
               * 
               int_of( prod_n, 0.0, 1.0 );
    };


    for( ; at <= known; ++at ) {
        double x = a + h * ( at - 1 );
        
        acc.emplace_back( x, ex.solution( x ) );
    }

    for( ; at <= steps; ++at ) {
        auto [ x, y ] = acc.back();

        crt_cons = std::min( at - 1, consider );

        double deriv = 0.0;

        for( size_t n = 0; n < crt_cons; ++n ) {
            auto [ nx, ny ] = *( acc.end() - 1 - n );

            deriv += coeff_of( n ) * ex.fxy( nx, ny );
        }

        acc.emplace_back(
            x + h,
            y + h * deriv
        );
    }

    return acc;
}



int main() {
    Surface surf{ "Linear Multistep Method", { 0, 0 }, { Env::width(), Env::height() } };

    Renderer2 renderer{ surf };

    Viewport2 system_viewport{ renderer, Coord<>{ 0, 0 }, { 1000, surf.height() } }; 
    system_viewport.uplink();

    Viewport2 sliders_viewport{ renderer, Coord<>{ 1000, 0 }, { surf.width() - 1000, surf.height() } };
    sliders_viewport.uplink();

    System2 system{ system_viewport, 60, 1.0, 12, { 1.0, 1.0, 1.0, 0.8 }, 2.0 };
    system.uplink_auto_roam();


    load_examples( system );


    size_t at       = 0;
    size_t steps    = 10;
    size_t known    = 1;
    size_t consider = 1;
    double start    = 0.0;
    double range    = 10.0;

    system.insert( { "approx", accumulate( at, steps, known, consider, start, start + range ) } );
    system[ "approx" ].uplink().give_brush( 
        std::make_shared< SolidBrush2 >( renderer, Chroma{ 0.0, 1.0, 0.26, 1.0 }, 3.0 ) 
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
        }
    } );


    struct Slider {
        Clust2   clst   = {};
        size_t   divs   = 1;
        double   acc    = 0;

        void render( Renderer2& renderer ) {
            static LinearBrush2 brush{
                renderer, 
                Vec2{ 0, -renderer.surface().height() / 2.0 },
                Vec2{ 0, renderer.surface().height() / 2.0 },
                std::array< std::pair< Chroma, float >, 2 >{ {
                    { Chroma{ 0, 255, 247, 255 }, 0.0f },
                    { Chroma{ 255, 0, 230, 255 }, 1.0f }
                } },
                3.0
            };

            clst.render( renderer, brush );
        }
    } sliders[] = {
        {
            clst: Clust2{ 
                Vec2{ sliders_viewport.origin().x - 170, -320 }, 
                ( Vec2[] ){ { -60, 60 }, { 60, 30 }, { 60, -30 }, { -60, -60 } } 
            },
            divs: 100
        },
        {
            clst: Clust2{ 
                Vec2{ sliders_viewport.origin().x - 30, -320 },
                ( Vec2[] ){ { -60, 30 }, { 60, 30 }, { 60, -30 }, { -60, -30 } }
            },
            divs: 5
        },
        {
            clst: Clust2{ sliders[ 0 ].clst }.spin_at( 180 ).relocate_at( { sliders_viewport.origin().x + 110, -320 } ),
            divs: 5
        },
        {
            clst: Clust2{ 
                Vec2{ sliders[ 2 ].clst.extreme( HEADING_EAST ).x + 30, 0 },
                ( Vec2[] ){ { -10, 60 }, { 10, 60 }, { 10, -60 }, { -10, -60 } }
            },
            divs: 20
        },
        {
            clst: Clust2{ sliders[ 3 ].clst }.relocate_at( { sliders[ 3 ].clst.origin().x + 40, 0 } ),
            divs: 20
        }
    };

    Slider* selected_slider = nullptr;

    sliders_viewport.on< SURFACE_EVENT_KEY >( [ & ] ( Key key, KEY_STATE state, auto& trace ) -> void {
        if( key != Key::LMB ) return;

        switch( state ) {
            case KEY_STATE_DOWN: {
                for( auto& s : sliders )
                    if( s.clst.contains( surf.vec() ) ) {
                        selected_slider = &s;
                        return;
                    }
                
                selected_slider = nullptr;
            break; }

            case KEY_STATE_UP: {    
                if( selected_slider ) {
                    std::cout
                    << "Example:  " << at << " | " << examples[ at ].equation << '\n'
                    << "Steps:    " << steps << '\n'
                    << "Known:    " << known << '\n'
                    << "Consider: " << consider << '\n'
                    << "Start:    " << start << '\n'
                    << "Range:    " << range << "\n\n";   
                }

                selected_slider = nullptr;
            break; }
        }
    } );

    sliders_viewport.on< SURFACE_EVENT_MOUSE >( [ & ] ( Vec2 vec, Vec2 lvec, auto& trace ) -> void {
        if( !selected_slider ) return;

        double& y = selected_slider->clst.origin_ref().y;

        y = std::clamp( y + ( vec - lvec ).y, -320.0, 320.0 );

        auto nrm_y = [ &y ] () -> double { return ( y + 320.0 ) / 640.0; };

        switch( selected_slider - sliders ) {
            case 0: {
                steps = 10 + nrm_y() * 490;

                goto L_REFRESH_REQUESTED;
            break; }

            case 1: {
                known = 1 + nrm_y() * 9;

                goto L_REFRESH_REQUESTED;
            break; }

            case 2: {
                consider = 1 + nrm_y() * 9;

                goto L_REFRESH_REQUESTED;
            break; }

            case 3: {
                start = nrm_y() * 20.0 - 10.0;

                goto L_REFRESH_REQUESTED;
            break; }

            case 4: {
                range = nrm_y() * 20.0;

                goto L_REFRESH_REQUESTED;
            break; }
        }

        return;

        L_REFRESH_REQUESTED:

        note_refresh();
    } );


   
    while( !Key::down( Key::ESC ) ) {
        static Clock clk;


        renderer.open();


        system_viewport.fill( { 43, 45, 49, 255 } );
        sliders_viewport.fill( { 30, 31, 34, 255 } );

        system.render( renderer );

        for( auto& s : sliders )
            s.render( renderer );


        renderer.execute();


        if( refreshes.fetch_add( 0, std::memory_order_relaxed ) == 0 )
            goto L_NO_REFRESH;

        system[ "approx" ].collection() = accumulate( at, steps, known, consider, start, start + range );

        refreshes.fetch_sub( 1, std::memory_order_relaxed );

        L_NO_REFRESH: continue;
    }


    return 0;
}
