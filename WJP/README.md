# The `BAR STREAM/BURST` protocol 
This is an `ISO/OSI PRESENTATION LAYER ( STACK LAYER 6 )` protocol which assures stable and reliable interpretation of both sent and received byte streams. It is a volatile wrapper around user-defined send/receive methods, providing flexible choices regarding its operation procedures.

> Initially developed to provide a "`plug-n-play`" behaviour for a wireless joystick controller, it has now became a solid `PRESENTATION` protocol capable of exchanging data quickly, detecting any protocol breaches and working with memory swiftly.

> All of this whilst operating under the "`trust-no-one`" restriction. That is, any data coming from outside cannot break the byte interpretation.

The examples and samples in this manual are from the `BarraCUDA` controller's on-board/driver source code/sketches, since it was there when they were written.

## General structure
`BAR` is a packet-oriented protocol. Each packet it sends/receives shall begin with a `12`byte header.
> ![bar-general-structure](https://github.com/user-attachments/assets/63df0587-3602-431f-bd08-2d87b4dbd89c)

> - `PROTO_SIG` - `3`bytes - always equal to, in this order, '`B`', '`A`', '`R`' - present at each header begin in order to confirm the incoming byte alignment.
> - `OP` - `1`byte - the operation of the respective packet.
> - `RESERVED` - `2`bytes - reserved for future, or user-defined, use.
> - `SEQUENCE` - `2`bytes - depending on the type of operation, this field is either a quasi-UId`(1)` of the packet, or an index.
> - `SIZE` - `4`bytes - the byte count of the packet, `NOT` including the size of the header.

`(1)` - having the precision of `16`bits, the packet quasi-UIds shall repeat themselves after some time. However, since these UIds are used for acknowledging operations, having more than `60,000` packets waiting for `ACK`s is not considered a real scenario.

## Quick samples
Sample #`1`: Ping.
> ![bar-ping-sample](https://github.com/user-attachments/assets/a9c90066-944d-4e4b-90c2-82fbca9991df)
> "Hello there" <-> "General Kenobi".

Sample #`2`: Get.
> ![bar-get-sample](https://github.com/user-attachments/assets/b323f31c-9d3d-4407-abca-34339d62ff75)
> Laptop asking for the data labeled "BITNA_CRT" ( the status LED's current RGB ). 

Sample #`3`: Burst.
> ![bar-burst-sample](https://github.com/user-attachments/assets/b3f045cb-3976-482d-b136-008cfbacd1ac)
> The controller constantly sending its state ( joysticks, switches, gyro, etc ).

## User bind checklist
Everything the user needs to configure/"bind" in order for `BAR` to work properly. Here is just the checklist, each point detailed later in this manual.
> - `SEQUENCE ACQUISITION` function - mandatory - shall be called by the framework at every operation packet transmission.
> ```cpp
> this->BAR_PROTO_STREAM::bind_seq_acq( [ this ] () -> int16_t { return _bar_seq.fetch_add( 1, std::memory_order_relaxed ); } );
> ```

> - `SEND/RECEIVE WRAPPER` functions - mandatory - the functions called by the framework to send/receive data.
> ```cpp
> this->BAR_PROTO_STREAM::bind_srwrap( BAR_PROTO_SRWRAP{
>   send: [ this ] BAR_PROTO_STREAM_SEND_LAMBDA { return this->BTH_SOCKET::itr_send( src, sz, flags ); },
>   recv: [ this ] BAR_PROTO_STREAM_RECV_LAMBDA { return this->BTH_SOCKET::itr_recv( dst, sz, flags ); }
> } );
> ```

> - `GET/SET` table - optional - the table describing where are located, and how to set, different data structures, such as the status LED, labeled "BITNA_CRT".
> ```cpp
> BAR_PROTO_GSTBL_ENTRY PROTO_GSTBL_ENTRIES[ 1 ] = {
>  { 
>   str_id: "BITNA_CRT", 
>   src: &BITNA._crt, 
>   sz: 1, 
>   BAR_PROTO_GSTBL_READ_ONLY, 
>   set: nullptr 
>  }
> };
> ```
> ```cpp
> stream.bind_gstbl( BAR_PROTO_GSTBL{ 
>   entries: PROTO_GSTBL_ENTRIES, 
>   size: sizeof( PROTO_GSTBL_ENTRIES ) / sizeof( BAR_PROTO_GSTBL_ENTRY ) 
> } );
> ```

> - `BURST` table - optional - the table describing where to write the incoming burst data stream, such as the controller's state, nicknamed "dynamic".
> ```cpp
> BAR_PROTO_BRSTBL_ENTRY _brstbl_entry = { dst: &dynamic, sz: sizeof( dynamic ) };
> ```
> ```cpp
> this->BAR_PROTO_STREAM::bind_brstbl( BAR_PROTO_BRSTBL{
>   entries: &_brstbl_entry,
>   size: 1
> } );
> ```

## BAR_PROTO_STREAM framework structure
The structure used to communicate using the `BAR` protocol. After the user binded everything required by this structure, the following functions are available.

```cpp
int resolve_recv( BAR_PROTO_STREAM_RESOLVE_RECV_INFO* info );
```
> This function is used to read `ONE` incoming header and execute the required procedures.

> Returns:
> - `<return value>` - the number of received bytes during the processing of the current header, including the size of the header. Negative for errors and `0` for connection reset.
> - `info->recv_head` - the received header.
> - `info->send` - the numver of sent bytes during the processing of the current header.
> - `info->err` - a `BAR_PROTO_STREAM_ERR` enumerator.
> - `info->nakr` - the reason for which the framework decided to `NAK` an incoming operation.

