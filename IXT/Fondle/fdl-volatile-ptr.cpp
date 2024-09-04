/*
*/
#include <IXT/volatile_ptr.hpp>
#include <IXT/comms.hpp>

using namespace IXT;



int main() {
    Echo echo{};

    int n = 9;
    {
        VPtr< int > ptr{ &n };
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Stack \'n\' from raw ptr VPtr: " << *ptr;
    } {
        VPtr< int > ptr{ n };
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Stack \'n\' from raw ref VPtr: " << *ptr;
    }

    int* pn = new int{ 10 };
    {
        VPtr< int > ptr{ pn };
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'pn\' from raw ptr VPtr: " << *ptr;
    } {
        VPtr< int > ptr{ *pn };
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'pn\' from raw ref VPtr: " << *ptr;
    }
    echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'pn\' direct deref after VPtr: " << *pn;

    {
        SPtr< int > spn{ new int{ 11 } };
        {
            VPtr< int > ptr{ spn };
            echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'spn\' from shared ptr VPtr: " << *ptr
                                               << " Ref count: " << ptr.use_count();
        }
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'spn\' direct deref after VPtr: " << *spn
                                           << " Ref count: " << spn.use_count();
    }
    
    delete pn;
    pn = new int{ 12 };
    {
        SPtr< int > sfpn{ pn };
        echo( nullptr, ECHO_LEVEL_INTEL ) << "Heap \'sfpn\' direct deref: " << *sfpn
                                           << " Ref count: " << sfpn.use_count();
    }
    echo( nullptr, ECHO_LEVEL_ERROR ) << "Now deref pn: " << *pn;

}