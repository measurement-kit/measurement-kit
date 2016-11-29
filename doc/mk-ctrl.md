# Measurement Kit ConTrol Protocol (MKCTP)

The Measurement Kit Control Protocol (MKCTP) is used by clients to control a running Measurement Kit instance.

The protocol is designed to be:

* Simple to parse

* Simple to serialize

* Human readable

It is a newline delimited protocol and uses JSON as encoding.

## Commands

These are the lists of commands supported by the protocol.

### AUTH

This command is used to authenticate the client to the control port.

Syntax:

```
AUTH password
```

When the password is invalid it will reply with:

```
ERR invalid-password
{}
```

When the password is valid it will reply with:

```
OK
{}
```

### RUN

This command is used to run a specific command of the MK measurement API.

```
RUN nettest_name {'parameter1': 'value1', 'parameter2': 'value2'}
```

For example:

```
RUN web_connectivity {"input": "http://www.google.com", "settings": {"nameserver": "8.8.8.8:53", "dns/resolver": "system", "backend": "https://a.collector.test.ooni.io:4444"}}
```

Upon successful execution of a test it will return the result as:

```
OK
{}
```

Upon failure it will return:

```
ERR some-error
{}
```

## Replies

Every reply returned by the control protocol MUST contain two rows. The first line is the eventual error produced by
the command while the second line is the result encoded as a JSON string.

In case of an error:

```
ERR error-string
{}
```

In case of success:

```
OK
{}
```
