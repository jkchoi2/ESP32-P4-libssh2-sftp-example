# ESP32-P4-libssh2-sftp-example

이 예제는 ESP32-P4보드에 Hellow World예제에 libssh2를 포팅한 예제입니다.
PC에 openSSH 서비스를 실행 하고 테스트 하였고 잘 동작 합니다. 방화벽 설정에서 포트제외 설정 필요함.
ethernet을 사용함.

## build후 size

We will get back to you as soon as possible.
┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┓
┃ Memory Type/Section ┃ Used [bytes] ┃ Used [%] ┃ Remain [bytes] ┃ Total [bytes] ┃
┡━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━╇━━━━━━━━━━╇━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━┩
│ Flash               │       558634 │          │                │               │
│    .text            │       439054 │          │                │               │
│    .rodata          │       119088 │          │                │               │
│    .appdesc         │          256 │          │                │               │
│    .init_array      │          236 │          │                │               │
│ DIRAM               │        85215 │    14.78 │         491249 │        576464 │
│    .text            │        67606 │    11.73 │                │               │
│    .data            │         8885 │     1.54 │                │               │
│    .bss             │         8724 │     1.51 │                │               │
│ HP core RAM         │          134 │     1.64 │           8058 │          8192 │
│    .text            │           74 │      0.9 │                │               │
│    .data            │           60 │     0.73 │                │               │
│ LP RAM              │           24 │     0.07 │          32744 │         32768 │
│    .rtc_reserved    │           24 │     0.07 │                │               │
└─────────────────────┴──────────────┴──────────┴────────────────┴───────────────┘
Total image size: 635023 bytes (.bin may be padded larger)

                            Memory Type Usage Summary
┏━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━┓
┃ Memory Type/Section ┃ Used [bytes] ┃ Used [%] ┃ Remain [bytes] ┃ Total [bytes] ┃
┡━━━━━━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━╇━━━━━━━━━━╇━━━━━━━━━━━━━━━━╇━━━━━━━━━━━━━━━┩
│ Flash               │       597224 │          │                │               │
│    .text            │       475956 │          │                │               │
│    .rodata          │       120776 │          │                │               │
│    .appdesc         │          256 │          │                │               │
│    .init_array      │          236 │          │                │               │
│ DIRAM               │        84705 │    14.69 │         491759 │        576464 │
│    .text            │        66408 │    11.52 │                │               │
│    .bss             │         9412 │     1.63 │                │               │
│    .data            │         8885 │     1.54 │                │               │
│ HP core RAM         │          134 │     1.64 │           8058 │          8192 │
│    .text            │           74 │      0.9 │                │               │
│    .data            │           60 │     0.73 │                │               │
│ LP RAM              │           24 │     0.07 │          32744 │         32768 │
│    .rtc_reserved    │           24 │     0.07 │                │               │
└─────────────────────┴──────────────┴──────────┴────────────────┴───────────────┘
Total image size: 672415 bytes (.bin may be padded larger)
