# web hook

|[![License][license:badge]](/LICENSE)|[![Issues][issues:badge]][issues]|
|:------|------------------------------------------------:|
|[![Stargazers][stargazers:badge]][stargazers]|[![Pulls][pulls:badge]][pulls]|
|[![Contributors][contributors:badge]][contributors]|[![Commit Activity][commit-activity:badge]][commit-activity]|

![Alt](https://repobeats.axiom.co/api/embed/f81550002329b0c0414378aebb0dbac1d17d1013.svg "Repobeats analytics image")

## Introduction

This repository is a web hook on Windows/Linux/MacOS. It is a simple web hook that can be used to execute commands / show pages on the server.

## Sample
> hook.json
```json
{
    "hooks": [
        {
            "command": "echo -n \"Hello\"",
            "method": "GET",
            "name": "hi",
            "path": "/hi",
            "result": {
                "content": "<h1>{{&command_output}} {{&app}}</h1>",
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
