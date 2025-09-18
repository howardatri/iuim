const net = require('net');
const http = require('http');

const PORT = 8233;
const USER_SVC_HOST = 'localhost';
const USER_SVC_PORT = 50051;

// 创建TCP服务器
const server = net.createServer((socket) => {
  console.log('Client connected');

  socket.on('data', (data) => {
    const message = data.toString().trim();
    console.log(`Received: ${message}`);

    // 解析命令和参数
    const parts = message.split(' ');
    const cmd = parts[0];
    
    if (cmd === 'health_check') {
      // 转发到UserSVC的health-check接口
      const options = {
        hostname: USER_SVC_HOST,
        port: USER_SVC_PORT,
        path: '/health-check',
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        }
      };

      const req = http.request(options, (res) => {
        let responseData = '';
        
        res.on('data', (chunk) => {
          responseData += chunk;
        });
        
        res.on('end', () => {
          console.log(`Response from UserSVC: ${responseData}`);
          // 将响应发送回客户端
          socket.write(`${responseData}\n`);
        });
      });
      
      req.on('error', (error) => {
        console.error(`Error connecting to UserSVC: ${error.message}`);
        socket.write(JSON.stringify({
          code: 500,
          message: 'Error connecting to UserSVC',
          error: error.message
        }) + '\n');
      });
      
      req.end();
    } else {
      // 未知命令
      socket.write(JSON.stringify({
        code: 400,
        message: 'Unknown command'
      }) + '\n');
    }
  });

  socket.on('end', () => {
    console.log('Client disconnected');
  });

  socket.on('error', (err) => {
    console.error(`Socket error: ${err.message}`);
  });
});

server.listen(PORT, () => {
  console.log(`Gateway listening on port ${PORT}`);
});

server.on('error', (err) => {
  console.error(`Server error: ${err.message}`);
});