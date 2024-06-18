# Binary Protocol

The binary protocol is a byte-based encoding of the user settings protocol. All numbers are encoded
as little endian.

The binary protocol supports the following commands:

- GET - get a short setting description
- GET_FULL - get a full setting description
- LIST - get a short setting description for each settings
- LIST_FULL - get a full setting description for each settings
- SET - set a setting value
- SET_DEFAULT - set a setting default value
- RESTORE - set all settings to their default values

## GET (0x01)

A valid get command is encoded as [1 byte command (0x01), 2 byte setting ID]. For example, to get
the settings with id 13, the command is `010D00`.

A response to a valid get command is: [2 byte setting id, setting key as a null terminated string, 1
byte [setting type](../../include/user_settings_types.h), 1 byte value length (LEN), LEN bytes
value].

If the value of the settings is not set, LEN will be zero and no value is encoded.

Some Examples:

- a u8 setting with ID 7, key `s7` and value 7 is encoded as: `0700733700010107`
- a u8 setting with ID 7, key `s7` and no value is encoded as: `07007337000100`

## GET FULL (0x02)

A valid get full command is encoded as [1 byte command (0x02), 2 byte setting ID]. For example, to
get the settings with id 13, the command is `020D00`.

A response to a valid get command is the same as for GET with the following additional fields: [...,
1 byte default length (DEFAULT_LEN), DEFAULT_LEN bytes default value, 1 byte maximum setting
length].

If the default value of the settings is not set, DEFAULT_LEN will be zero and no default value is
encoded.

Some Examples:

- a u8 setting with ID 7 , key `s7`, value 7, default value 15 and max length 1 is encoded as:
  `0700733700010107010F01`
- a u8 setting with ID 7 , key `s7`, value 7, no default value and max length 1 is encoded as:
  `07007337000101070001`
- a u8 setting with ID 7 , key `s7`, no value, no default value and max length 1 is encoded as:
  `070073370001000001`

## LIST (0x03)

A valid list command is encoded as `03`.

Each setting is encoded separately as specified in the GET command.

## LIST FULL (0x04)

A valid list command is encoded as `04`.

Each setting is encoded separately as specified in the GET FULL command.

## SET (0x05)

A valid set command is encoded as [1 byte command (0x05), 2 byte setting ID, 1 byte value length,
LEN bytes value]. For example, to set setting with id 13 to the value 7 (the setting is type u8),
the command is `050D000107`.

## SET DEFAULT (0x06)

A valid set default command is encoded as [1 byte command (0x06), 2 byte setting ID, 1 byte default
value length, LEN bytes default value]. For example, to set the default value of setting with id 13
to 10 (the setting is type u8), the command is `060D00010A`.

## RESTORE (0x07)

A valid restore command is encoded as `07`.

TODO: maybe list some and list some full should be removed? We should implement support for multiple
commands in a single buffer in the executor instead.

## LIST SOME (0x08)

A valid list some command is encoded as [1 byte command (0x08), 1 byte number of settings (N), N * 2
byte setting ID].

Each setting is encoded separately as specified in the GET command.

## LIST SOME FULL (0x09)

A valid list some full command is encoded as [1 byte command (0x09), 1 byte number of settings (N),
N * 2 byte setting ID].

Each setting is encoded separately as specified in the GET FULL command.

## Additional examples

The following list gives a settings description (in text), its short (GET) and full (GET FULL)
encoding. Bellow that are the SET and SET_DEFAULT commands required to set the specific values of
that setting.

```text
SETTING:     id: 1, key: "enabled", value: /, default: / (TYPE_BOOL)
GET:         01 00 65 6E 61 62 6C 65 64 00 00 00
GET FULL:    01 00 65 6E 61 62 6C 65 64 00 00 00 00 01
SET:         /
SET DEFAULT: /

SETTING:     id: 2, key: "number", value: 69, default: 13 (TYPE_U8)
GET:         02 00 6E 75 6D 62 65 72 00 01 01 45
GET FULL:    02 00 6E 75 6D 62 65 72 00 01 01 45 01 0D 01
SET:         05 02 00 01 45
SET DEFAULT: 06 02 00 01 0D

SETTING:     id: 3, key: "hey", value: /, default: 1337 (TYPE_U16)
GET:         03 00 68 65 79 00 02 00
GET FULL:    03 00 68 65 79 00 02 00 02 39 05 02
SET:         /
SET DEFAULT: 06 03 00 02 39 05

SETTING:     id: 4, key: "yo", value: 7654321, default: 1234567 (TYPE_U32)
GET:         04 00 79 6F 00 03 04 B1 CB 74 00
GET FULL:    04 00 79 6F 00 03 04 B1 CB 74 00 04 87 D6 12 00 04
SET:         05 04 00 04 B1 CB 74 00
SET DEFAULT: 06 04 00 04 87 D6 12 00

SETTING:     id: 5, key: "lets", value: 1, default: / (TYPE_U64)
GET:         05 00 6C 65 74 73 00 04 08 01 00 00 00 00 00 00 00
GET FULL:    05 00 6C 65 74 73 00 04 08 01 00 00 00 00 00 00 00 00 08
SET:         05 05 00 08 01 00 00 00 00 00 00 00
SET DEFAULT: /

SETTING:     id: 6, key: "go", value: -2, default: -1 (TYPE_I8)
GET:         06 00 67 6F 00 05 01 FE
GET FULL:    06 00 67 6F 00 05 01 FE 01 FF 01
SET:         05 06 00 01 FE
SET DEFAULT: 06 06 00 01 FF

SETTING:     id: 7, key: "t7", value: 202, default: 505 (TYPE_I16)
GET:         07 00 74 37 00 06 02 CA 00
GET FULL:    07 00 74 37 00 06 02 CA 00 02 F9 01 02
SET:         05 07 00 02 CA 00
SET DEFAULT: 06 07 00 02 F9 01

SETTING:     id: 8, key: "t8", value: -7654321, default: / (TYPE_I32)
GET:         08 00 74 38 00 07 04 4F 34 8B FF
GET FULL:    08 00 74 38 00 07 04 4F 34 8B FF 00 04
SET:         05 08 00 04 4F 34 8B FF
SET DEFAULT: /

SETTING:     id: 9, key: "ttt", value: 987656789, default: 65432123456 (TYPE_I64)
GET:         09 00 74 74 74 00 08 08 55 72 DE 3A 00 00 00 00
GET FULL:    09 00 74 74 74 00 08 08 55 72 DE 3A 00 00 00 00 08 40 F8 0E 3C 0F 00 00 00 08
SET:         05 09 00 08 55 72 DE 3A 00 00 00 00
SET DEFAULT: 06 09 00 08 40 F8 0E 3C 0F 00 00 00

SETTING:     id: 10, key: "text", value: "apple", default: "banana" (TYPE_STR)
GET:         0A 00 74 65 78 74 00 09 06 61 70 70 6C 65 00
GET FULL:    0A 00 74 65 78 74 00 09 06 61 70 70 6C 65 00 07 62 61 6E 61 6E 61 00 0A
SET:         05 0A 00 06 61 70 70 6C 65 00   (Note that the string is null terminated)
SET DEFAULT: 06 0A 00 07 62 61 6E 61 6E 61 00

SETTING:     id: 11, key: "secret", value: 08090A0B0C0D, default: 010203040506 (TYPE_BYTES)
GET:         0B 00 73 65 63 72 65 74 00 0A 06 08 09 0A 0B 0C 0D
GET FULL:    0B 00 73 65 63 72 65 74 00 0A 06 08 09 0A 0B 0C 0D 06 01 02 03 04 05 06 08
SET:         05 0B 00 06 08 09 0A 0B 0C 0D
SET DEFAULT: 06 0B 00 06 01 02 03 04 05 06
```
