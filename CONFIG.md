# Config Options

### These are fields on the server_config struct

1. `not_found_handler`: Function to call when client requests URL without a registered handler
    - Value Type: Function pointer with form `void (*) (Request)` - Function must take Request object.
    - <b>REQUIRED OPTION</b>
