# web hook

|[![License][license:badge]](/LICENSE)|[![Issues][issues:badge]][issues]|
|:------|------------------------------------------------:|
|[![Stargazers][stargazers:badge]][stargazers]|[![Pulls][pulls:badge]][pulls]|
|[![Contributors][contributors:badge]][contributors]|[![Commit Activity][commit-activity:badge]][commit-activity]|

![Alt](https://repobeats.axiom.co/api/embed/f81550002329b0c0414378aebb0dbac1d17d1013.svg "Repobeats analytics image")

## Introduction

This repository is a web hook on Linux. It is a simple web hook that can be used to execute commands / show pages on the server.

## Demo

See [here](https://api.ohtoai.top/github/)

## Release

[All](https://github.com/Ohto-Ai/webhook/releases) | [Latest](https://github.com/Ohto-Ai/webhook/releases/latest)

### v0.2.1
[Ubuntu 18.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-18.04_v0.2.1)
| [Ubuntu 20.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-20.04_v0.2.1)
| [Ubuntu 22.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-22.04_v0.2.1)

### v0.2.0
[Ubuntu 18.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-18.04_v0.2.0)
| [Ubuntu 20.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-20.04_v0.2.0)
| [Ubuntu 22.04](https://github.com/Ohto-Ai/webhook/releases/download/v0.2.1/webhook_ubuntu-22.04_v0.2.0)

## Sample
> hook.json
```json
{
    "hooks": [
        {
            "async_exec": false,
            "command": "echo -n \"Hello\"",
            "command_timeout": 3000,
            "method": "GET",
            "name": "hi",
            "path": "/hi",
            "result": {
                "content": [
                    "<h1>{{&app}} {{&version}}{{#hash}}({{.}}){{/hash}}</h1>",
                    "<p>Method: {{&request.method}}</p>",
                    "<p>Path: {{&request.path}}</p>",
                    "<p>User-Agent: {{#request.header}}user-agent{{/request.header}}</p>",
                    "<p>Client: {{request.remote_addr}}</p>",
                    "<p>{{&response.command_output}}</p>"
                ],
                "type": "text/html"
            }
        }
    ],
    "listen": {
        "auth": {
            "password": "",
            "path": "/",
            "username": ""
        },
        "host": "localhost",
        "port": 8080,
        "prefix": "/api"
    },
    "log": {
        "console_level": "info",
        "file_level": "info",
        "file_path": "webhook.log",
        "global_level": "info"
    }
}
```

[license:badge]: https://img.shields.io/github/license/Ohto-Ai/webhook?logo=github&style=flat-square

[issues:badge]: https://img.shields.io/github/issues/Ohto-Ai/webhook?logo=github&style=flat-square
[issues]: https://github.com/Ohto-Ai/webhook/issues

[stargazers:badge]: https://img.shields.io/github/stars/Ohto-Ai/webhook?logo=github&style=flat-square
[stargazers]: https://github.com/Ohto-Ai/webhook/stargazers

[pulls:badge]: https://img.shields.io/github/issues-pr/Ohto-Ai/webhook?logo=github&color=0088ff&style=flat-square
[pulls]: https://github.com/Ohto-Ai/webhook/pulls

[commit-activity:badge]: https://img.shields.io/github/commit-activity/m/Ohto-Ai/webhook?logo=github&style=flat-square
[commit-activity]: https://github.com/Ohto-Ai/webhook/pulse

[contributors:badge]: https://img.shields.io/github/contributors/Ohto-Ai/webhook?logo=github&style=flat-square
[contributors]: https://github.com/Ohto-Ai/webhook/contributors
