#pragma once
/**
 * @file: OSp/petri_net.hpp
 * @brief: 
 * @details
 * @authors: Vatca "Mipsan" Tudor-Horatiu
 */

#include <A113/BRp/descriptor.hpp>


namespace A113 { namespace OSp {


class PetriNet_Sink;
class PetriNet_Drain;
class PetriNet_Token;


class PetriNet_Sink { friend class PetriNet_Executor;
public:
    struct Config {
        int exec_tok_lim   = -0x1;
    } config;

A113_PROTECTED:
    std::list< PetriNet_Drain* >   _drains   = {};

public:
    virtual RESULT when_token( PetriNet_Token* tok_, int tok_sexecc ) {
        return 0x0;
    }

};

class PetriNet_Drain { friend class PetriNet_Executor;
public:
    struct Config {
        int engaged :1;
    } config;

A113_PROTECTED:
    std::list< PetriNet_Sink* >   _sinks   = {};

public:
    virtual RESULT assert_token( PetriNet_Token* tok_ ) = 0;

};

class PetriNet_Token { friend class PetriNet_Executor;
public:
    PetriNet_Token() = default;

public:
    struct Config {
        int   _sexecc   = 0x0;
    } config;

A113_PROTECTED:
    PetriNet_Sink*   _sink     = nullptr;

public:
    virtual RESULT when_drained( PetriNet_Drain* drn_, RESULT drn_res_ ) {
        return 0x0;
    }

    virtual HPtr< PetriNet_Token > split( PetriNet_Sink* snk_ ) {
        return nullptr;
    }

};

class PetriNet_Executor {
public:
    typedef   double   dt_t;

A113_PROTECTED:
    std::list< HPtr< PetriNet_Token > >   _tokens   = {};
    std::list< HPtr< PetriNet_Sink > >    _sinks    = {};
    std::list< HPtr< PetriNet_Drain > >   _drains   = {};

public:
    RESULT impulse( dt_t dt_ );

};


} };



