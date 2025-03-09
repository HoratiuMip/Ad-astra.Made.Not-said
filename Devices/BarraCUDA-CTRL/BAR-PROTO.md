# The `BAR STREAM/BURST` protocol 
This is an `ISO/OSI PRESENTATION LAYER ( STACK LAYER 6 )` protocol which assures stable and reliable interpretation of both sent and received byte streams. It is a volatile wrapper around user-defined send/receive methods, providing flexible choices regarding its operation procedures.

> Initially developed to provide a "`plug-n-play`" behaviour for a wireless joystick controller, it has now became a solid `PRESENTATION` protocol capable of exchanging data quickly, detecting any protocol breaches and working with memory swiftly.

> All of this whilst operating under the "`trust-no-one`" restriction. That is, any data coming from outside cannot break the byte interpretation.

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
