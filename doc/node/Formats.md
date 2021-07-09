# Format-types {#node-formats}

Some of the VILLASnode @ref node-types are transports which can exchange arbitrary payloads.
Examples for these node-types are: @ref node-type-mqtt, @ref node-type-amqp and @ref node-type-socket.

The format of the payload for these node-types is configurable.
New formats can be added via plugins.

Currently, the following formats are supported (or planned):

| Type             | Description                                      | Status |
| :--              | :--                                              | :--    |
| `csv`            | @subpage node-format-csv                         | stable |
| `gtnet`          | @subpage node-format-raw                         | stable |
| `iotagent-ul`    | @subpage node-format-fiware-iotagent-ul          | stable |
| `json.kakfa`     | @subpage node-format-json-kafka                  | stable |
| `json`           | @subpage node-format-json                        | stable |
| `protobuf`       | @subpage node-format-protobuf                    | stable |
| `raw`            | @subpage node-format-raw                         | stable |
| `tsv`            | @subpage node-format-tsv                         | stable |
| `value`          | @subpage node-format-value                       | stable |
| `villas.binary`  | @subpage node-format-villas-binary               | stable |
| `villas.human`   | @subpage node-format-villas-human                | stable |

# Configuration {#node-config-format}

To use one of the format-types above, a setting named `format` must be present in the configuration of the respective node instance.
The value of the `format` setting can be eiter a simple string identifying one of the available format-types from the table above.
Alternatively, a JSON dictionary can be used to provide additional configuration settings to the payload format.
Please see below for examples of both approaches.

If the format-type is provided as an argument to a VILLASnode command-line tool (see @ref node-usage), also both variants are supported:

```bash
# Simple
villas signal -f json -v 5 sine

# Advanced
villas signal -f '{ "type": "json", "indent": 4, "precision": 4 }' -v 5 sine
```

Please note, that depending on you system configuration not all format-types might be available.
Use the following command to get a list of all available types on your system: `villas node -h`.

All format-types support the following generic options:

## real_precision (integer) = 17 {#node-config-format-precision}

Output all real numbers with at most n digits of precision. The valid range for this setting is between 0 and 31 (inclusive), and other values result in an undefined behavior.

By default, the precision is 17, to correctly and losslessly encode all IEEE 754 double precision floating point numbers.

## ts_origin (boolean) = true {#node-config-format-ts-origin}

If set, include the origin timestamp in the output.
## sequence (boolean) {#node-config-format-sequence}

If set, include the sequence number in the output.

## data (boolean) {#node-config-format-data}

If set, include the data in the output.

## offset (boolean) {#node-config-format-offset}

If set, include the offset between origin and received timestamp in the output.

# Example

## Simple

```
nodes = {
    udp_json_node = {
        type = "socket"
        layer = "udp"

        format = "json"
        
        ...
    }
}
```

## Advanced

```
nodes = {
    udp_json_node = {
        type = "socket"
        layer = "udp"
        format = {
            "type": "json",

            "precision": 4,
            "indent": 4
        }
        
        ...
    }
}
```


# Abstract format-types

- @subpage node-format-line
  - @subpage node-format-column