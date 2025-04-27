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

For more examples look into the `examples` directory in source code

## Use-cases

* API testing
* Scripting HTTP requests

## Comparison with other tools

* `insomnia` — hxxp does not require an account to work
* `hoppscotch` — hxxp does not lag, works with any headers (even with `accept-encoding`), can be interpolated with commands and can be synced without any account
* `curl` — hxxp is merely a limited interface to libcurl
* `hurl` — hxxp has less functionality and doesn't provide a testing framework with testing DSL
