# The `Warp Joint` Protocol
An `ISO/OSI PRESENTATION LAYER ( STACK LAYER 6 )` protocol which assures stable and reliable interpretation of both sent and received byte streams. It is a volatile wrapper around user-defined send/receive methods, providing flexible choices regarding its operation modes and procedures.

> Initially developed to provide a "`plug-n-play`" behaviour for a wireless joystick controller, it has now became a solid `PRESENTATION` protocol capable of exchanging data quickly, detecting any protocol breaches and working with memory swiftly.

> ![wjp_sketch](https://github.com/user-attachments/assets/254d060f-0311-48b5-aeb7-8849671d29bf)

The examples and samples in this manual are from the `BarraCUDA` controller's on-board/driver source code/sketches, since it was there when they were written.

## General structure
`WJP` is a packet-oriented protocol. Each packet it sends/receives shall begin with a `16`byte header.
> ![wjp_head](https://github.com/user-attachments/assets/fcd0acd3-d72b-42a9-bbb7-6b6904871024)

> - `PROTO_SIG` - `3`bytes - always equal to, in this order, '`W`', '`J`', '`P`' - present at each header begin in order to confirm the incoming byte alignment.
> - `OP` - `1`byte - the operation of the respective packet.
> - `_RESERVED` - `2`bytes - reserved for future, or user-defined, use.
> - `SEQUENCE` - `2`bytes - depending on the type of operation, this field is either a quasi-UId`(1)` of the packet, or an index.
> - `_RESERVED` - `4`bytes - reserved for future, or user-defined, use.
> - `SIZE` - `4`bytes - the byte count of the packet, `NOT` including the size of the header.

`(1)` - having the precision of `16`bits, the packet quasi-UIds shall repeat themselves after some time. However, since these UIds are used for acknowledging operations, having more than `60,000` packets waiting for `ACK`s is not considered a nominal scenario.

## Quick samples
Sample #`1`: Ping.
> ![wjp_ping](https://github.com/user-attachments/assets/c1df81c8-adea-4293-bb54-7f22112d58a6)
> "Hello there" <-> "General Kenobi".

Sample #`2`: Quick Get.
> ![wjp_qget](https://github.com/user-attachments/assets/195db847-154e-477f-b61d-67518fc5936c)
> Laptop asking for the data labeled "BITNA_CRT" ( the status LED's current RGB ). 

Sample #`3`: Indexed Burst.
> ![wjp_iburst](https://github.com/user-attachments/assets/5c923090-cde3-4b75-9165-77c8238d75c5)
> The controller constantly sending its state ( joysticks, switches, gyro, etc ).

## User bind checklist
Everything the user needs to configure/"bind" in order for `WJP` to work properly. Here is just the checklist, each point detailed later in this manual.

> The `WJP_DEVICE`'s `begin()` function check that all the mandatory bindings were configured properly.

> - `SEQUENCE ACQUISITION` function - mandatory - shall be called by the `WJP` at every operation packet transmission.
> ```cpp
> this->WJP_DEVICE::bind_seq_acq( [ this ] () -> int16_t { return _wjp_seq.fetch_add( 1, std::memory_order_relaxed ); } );
> ```

> - `SEND/RECEIVE WRAPPER` functions - mandatory - the functions called by the `WJP` to send/receive data.
> ```cpp
> this->WJP_DEVICE::bind_srwrap( WJP_SRWRAP{
>   send: [ this ] WJP_SEND_LAMBDA { return this->BTH_SOCKET::itr_send( src, sz, flags ); },
>   recv: [ this ] WJP_RECV_LAMBDA { return this->BTH_SOCKET::itr_recv( dst, sz, flags ); }
> } );
> ```

> - `QUICK GET/SET` table - optional - the table describing where are located, and how to set, different data structures, such as the status LED, labeled "BITNA_CRT".
> ```cpp
> WJP_QGSTBL_ENTRY PROTO_QGSTBL_ENTRIES[ 1 ] = {
>  { 
>  str_id: "BITNA_CRT", 
>  sz: 1, 
>  WJP_QGSTBL_READ_ONLY, 
>  qset_func: nullptr,
>  qget_func: nullptr, 
>  src: &BITNA._crt
>  }
> };
> ```
> ```cpp
> device.bind_qgstbl( WJP_QGSTBL{ 
>   entries: PROTO_QGSTBL_ENTRIES, 
>   size: sizeof( PROTO_QGSTBL_ENTRIES ) / sizeof( WJP_QGSTBL_ENTRY ) 
> } );
> ```

> - `INDEXED BURST` table - optional - the table describing where to write the incoming burst data stream, such as the controller's state, nicknamed "dynamic".
> ```cpp
> WJP_IBRSTBL_ENTRY _brstbl_entry = { dst: &dynamic, sz: sizeof( dynamic ) };
> ```
> ```cpp
> this->WJP_DEVICE::bind_brstbl( WJP_IBRSTBL{
>   entries: &_brstbl_entry,
>   size: 1
> } );
> ```

## WJP_DEVICE framework structure
The structure used to communicate using the `WJP`. After the user binded everything required by this structure, the following functions are available.

```cpp
int resolve_recv( WJP_RESOLVE_RECV_INFO* info );
```
> This function is used to read `ONE` incoming header and execute the required procedures.

> Returns:
> - `<return value>` - the number of received bytes during the processing of the current header, including the size of the header. Negative for errors and `0` for connection reset.
> - `info->recv_head` - the received header.
> - `info->sent_count` - the number of sent bytes during the processing of the current header.
> - `info->err` - a `WJPErr_` enumerator. `WJPErr_None` if no error occured.
> - `info->nakr` - the reason for which the framework decided to `NAK` an incoming operation.

