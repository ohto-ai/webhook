# web hook

## Introduce

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