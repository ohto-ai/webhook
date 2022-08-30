# web hook

|[![License][license:badge]](/LICENSE)|[![Issues][issues:badge]][issues]|
|:------|------------------------------------------------:|
|[![Stargazers][stargazers:badge]][stargazers]|[![Pulls][pulls:badge]][pulls]|
|[![Contributors][contributors:badge]][contributors]|[![Commit Activity][commit-activity:badge]][commit-activity]|

![Last Modify][last-modify:badge]

## Introduction

This repository is a web hook on Windows/Linux/MacOS. It is a simple web hook that can be used to execute commands / show pages on the server.

## Sample
> hook.json
```cpp
{
    "server":{
        "host":"localhost",
        "port":8003
    },
    "hook": [
        {
            "name": "pull-repo",
            "method": "POST",
            "path": "/api/update",
            "command": "cd ~/workspace/myrepo && git pull --recurse-submodules",
            "result":
            {
                "from": "constant",
                "value": "success",
                "type": "text/plain"
            }
        },
        {
            "name": "hello",
            "method": "GET",
            "path": "/api/hello",
            "command": "echo Hi there, this is pull-hook test page.",
            "result":
            {
                "from": "command",
                "type": "text/plain"
            }
        },
        {
            "name": "hi",
            "method": "GET",
            "path": "/api/hi",
            "result":
            {
                "from": "constant",
                "value": "Hi!!!",
                "type": "text/plain"
            }
        },
        {
            "name": "hi-html",
            "method": "GET",
            "path": "/api/html",
            "result":
            {
                "from": "constant",
                "value": "<!DOCTYPE html><html><body><h1>Hi, this is a html page</h1></body></html>",
                "type": "text/html"
            }
        }
    ]
}
```

[last-modify:badge]: https://img.shields.io/badge/last_modify-2022--08--31_02:11:53-orange.svg?style=flat-square

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
