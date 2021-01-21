# skillbox-chat-server
Simple Chat Server using Sockets

## Requirements
[uWebSockets](https://github.com/uNetworking/uWebSockets)

## Usage examples with the browser console
**1. New connection**
```
ws = new WebSocket("ws://localhost:8888/");
ws.onmessage = ({data}) => console.log(data);
```

**2. Setting your name**
```
ws.send("SET_NAME::name");
```

**3. Sending direct message**
```
ws.send("DIRECT::11::text message");
```

**3. Sending broadcast message**
```
ws.send("TOALL::text message");
```

***

*Made with C++ Workshop from [Skillbox](https://www.youtube.com/channel/UCHJZFCpwlXV7Sie1dV6pQLw).*
