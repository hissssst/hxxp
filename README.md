# hxxp

eXtended HTTP — HTTP requests as template files

## Quick example

```bash
$ echo "GET http://example.com/ HTTP/1.1" > example.hxxp
$ hxxp example.hxxp
```

Or even

```bash
$ echo "GET http://example.com/ HTTP/1.1" | hxxp -
```

For more examples look into the `examples` directory in source code.

Generally speaking, hxxp files contain plain HTTP (with `\n` instead of `\r\n` and a full host URI instead of path) and can be extended with shell interpolation. If you know HTTP, you can use hxxp without any learning curve.

## Use-cases

* API testing
* Scripting HTTP requests

## Comparison with other tools

* `insomnia`, `hoppscotch`, `postman`, `bruno`, etc. — hxxp does not require an account to work, hxxp does not provide laggy UI, hxxp works with any headers (even with `accept-encoding`), can be interpolated with commands, can be synced with plain git, hxxp is not bloated, provides no cloud version and does not require to click the menus to setup a header, hxxp does not provide any cryptic DSL to describe HTTP request.
* `curl` — hxxp is merely a limited interface to libcurl
* `hurl` — hxxp has less functionality and doesn't provide a testing framework with testing DSL
* `httpyac` — hxxp does not provide strange DSL, hxxp works only with HTTP and is much more lightweight
