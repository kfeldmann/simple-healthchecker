# Simple Healthchecker

_**This is just getting started. It is not actually useful yet!**_

The goal is to provide a minimal, lightweight http client to use for
healthchecking of applications inside Docker containers. To be
compatible with Docker, the exit code for failures is always 1.

Dependencies:
- Only a C library (musl on Alpine, for example)

Usage:

```
hc server port host-header path
```

Where:
- `server` is the name or ip of the service endpoint
- `port` is the port on which the service is listening
- `host-header` is the value to include in the http host header.
  This might be the same as or different from the `server` value
- `path` is the URI to request, such as `/` or `/directory/file.ext`

Dockerfile Example:
```
HEALTHCHECK --interval=30s --timeout=3s \
  CMD /sbin/hc localhost 8080 api.mysite.dom /
```
