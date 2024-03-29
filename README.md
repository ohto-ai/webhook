# webhook

|[![License][license:badge]](/LICENSE)|[![Contributors][contributors:badge]][contributors]|
|:------|------------------------------------------------:|
|[![Issues][issues:badge]][issues]|[![Closed Issues][closed-issues:badge]][closed-issues]|
|[![PRs][pulls:badge]][pulls]|[![Closed PRs][closed-pulls:badge]][closed-pulls]|
|[![Stargazers][stargazers:badge]][stargazers]|[![Commit Activity][commit-activity:badge]][commit-activity]|

[![Cross-platform CI & Release](https://github.com/Ohto-Ai/webhook/actions/workflows/ci_release.yaml/badge.svg)](https://github.com/Ohto-Ai/webhook/actions/workflows/ci_release.yaml)

![Top Lang][top-lang]


![Alt](https://repobeats.axiom.co/api/embed/f81550002329b0c0414378aebb0dbac1d17d1013.svg "Repobeats analytics image")

## Introduction

This repository is a webhook on Linux, Windows and MacOS.

## Demo

See [here](https://api.ohtoai.top/github/)

## Release

[All](https://github.com/Ohto-Ai/webhook/releases) | [Latest](https://github.com/Ohto-Ai/webhook/releases/latest)

## Build

Ensure you have `cmake` and build tools installed.

Clone the repository and build it with `cmake`.

```bash
git clone https://github.com/Ohto-Ai/webhook.git
# for chinese user, you can use ghproxy
# git clone https://ghproxy.com/https://github.com/Ohto-Ai/webhook.git
cd webhook

cmake -Bbuild
cmake --build build --config Release --target webhook -j8
```

## Install

You can add this source to `source.list`, and run this command `apt update` & `apt install ohtoai-webhook`.
```bash
echo "deb [trusted=yes] https://github.com/Ohto-Ai-Dev/apt-repo/raw/main/ubuntu/ ./" | sudo tee -a /etc/apt/sources.list > /dev/null 
sudo apt update
sudo apt install ohtoai-webhook
```
Sources:  
```bash
# GitHub
deb [trusted=yes] https://github.com/Ohto-Ai-Dev/apt-repo/raw/main/ubuntu/ ./
```

Or you can use python script to download latest version on Linux.
```bash
curl -fsSL https://raw.githubusercontent.com/Ohto-Ai/webhook/master/scripts/install.py | python -
```
Or you can download binary release from [GitHub Release](https://github.com/Ohto-Ai/webhook/releases/latest).

## Usage
See config in `~/.ohtoai/ohtoai-webhook/hook.json`.
It a simple webhook server.

## Sample
hook.json
```json
{
    "hooks": [
        {
            "command": "echo -n \"Hello\"",
            "command_timeout": 8000,
            "method": "GET",
            "name": "hi",
            "path": "/hi",
            "result": {
                "content": [
                    "<h1>{{context.app}} {{context.version}}{{context.commit_hash}}</h1>",
                    "<p>Method: {{request.method}}</p>",
                    "<p>Path: {{request.path}}</p>",
                    "<p>User-Agent: {{request.header.user-agent}}</p>",
                    "<p>Client: {{request.remote_addr}}</p>",
                    "<p>{{command_output}}</p>"
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

## Star History

[![Star History Chart](https://api.star-history.com/svg?repos=Ohto-Ai/webhook&type=Date)](https://star-history.com/#Ohto-Ai/webhook&Date)


[license:badge]: https://img.shields.io/github/license/Ohto-Ai/webhook?logo=github&style=flat-square

[issues:badge]: https://img.shields.io/github/issues/Ohto-Ai/webhook?logo=github&style=flat-square
[issues]: https://github.com/Ohto-Ai/webhook/issues
[closed-issues:badge]: https://img.shields.io/github/issues-closed/Ohto-Ai/webhook?logo=github&style=flat-square
[closed-issues]: https://github.com/Ohto-Ai/webhook/issues?q=is%3Aissue+is%3Aclosed

[stargazers:badge]: https://img.shields.io/github/stars/Ohto-Ai/webhook?logo=github&style=flat-square
[stargazers]: https://github.com/Ohto-Ai/webhook/stargazers

[pulls:badge]: https://img.shields.io/github/issues-pr/Ohto-Ai/webhook?logo=github&color=0088ff&style=flat-square
[pulls]: https://github.com/Ohto-Ai/webhook/pulls
[closed-pulls:badge]: https://img.shields.io/github/issues-pr-closed/Ohto-Ai/webhook?logo=github&color=0088ff&style=flat-square
[closed-pulls]: https://github.com/Ohto-Ai/webhook/pulls?q=is%3Apr+is%3Aclosed

[commit-activity:badge]: https://img.shields.io/github/commit-activity/m/Ohto-Ai/webhook?logo=github&style=flat-square
[commit-activity]: https://github.com/Ohto-Ai/webhook/pulse

[contributors:badge]: https://img.shields.io/github/contributors/Ohto-Ai/webhook?logo=github&style=flat-square
[contributors]: https://github.com/Ohto-Ai/webhook/contributors

[top-lang]: https://img.shields.io/github/languages/top/Ohto-Ai/webhook?logo=github&style=flat-square
