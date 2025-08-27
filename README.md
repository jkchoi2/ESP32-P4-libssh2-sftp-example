# ESP32-P4 snmp-example

!screenshot/test-mib.png

이 예제는 ESP32-P4보드에 Hellow World예제에 snmp를 포팅한 예제입니다. (대부분의 esp32보드에서도 정상동작할것 입니다. )
이 예제는 ethernet을 이용한 snmp 예제입니다.

2개의 소스를 참조하였고, 둘다 프로젝트 예제가 없어서 빌드할수가 없었습니다.
https://github.com/leandroadonis86/esp32_simple_snmpv2c_agent/tree/master
https://github.com/ubirch/ubirch-esp32-snmp-agent/tree/master

PC에 iReasoning MIB Browser를 이용하여 테스트 하였고, main.c에서 led toggle을 시키고, snmp로 값을 읽어보면 0/1로 바뀌는것을 확인 하였습니다.
 
## build후 size

We will get back to you as soon as possible.
```
                           Memory Type Usage Summary
┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┓
┃ Memory Type/Section ┃ Used [bytes] ┃ Used [%] ┃ Remain [bytes] ┃ Total [bytes] ┃
┡━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━╇━━━━━━━━━━╇━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━┩
│ Flash               │       415050 │          │                │               │
│    .text            │       329102 │          │                │               │
│    .rodata          │        85456 │          │                │               │
│    .appdesc         │          256 │          │                │               │
│    .init_array      │          236 │          │                │               │
│ DIRAM               │        83337 │    14.46 │         493127 │        576464 │
│    .text            │        65196 │    11.31 │                │               │
│    .bss             │         9484 │     1.65 │                │               │
│    .data            │         8657 │      1.5 │                │               │
│ HP core RAM         │          134 │     1.64 │           8058 │          8192 │
│    .text            │           74 │      0.9 │                │               │
│    .data            │           60 │     0.73 │                │               │
│ LP RAM              │           24 │     0.07 │          32744 │         32768 │
│    .rtc_reserved    │           24 │     0.07 │                │               │
└─────────────────────┴──────────────┴──────────┴────────────────┴───────────────┘
Total image size: 488801 bytes (.bin may be padded larger)

```
