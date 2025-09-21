const net = require('net');
const http = require('http');
const axios = require('axios');

const PORT = 8233;
const USER_SVC_HOST = 'localhost';
const USER_SVC_PORT = 50051;
const USER_SVC_BASE_URL = `http://${USER_SVC_HOST}:${USER_SVC_PORT}`;

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
    } else if (cmd === 'register') {
      // 处理注册命令
      try {
        // 获取JSON字符串部分
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const userData = JSON.parse(jsonStr);
        
        console.log(`Processing register command with data:`, userData);
        
        // 转发到UserSVC的register接口
        axios.post(`${USER_SVC_BASE_URL}/register`, userData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Register response from UserSVC:`, response.data);
          // 将响应发送回客户端，添加命令前缀
          socket.write(`register_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in register request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to UserSVC',
            error: error.message
          };
          
          // 如果服务器返回了错误响应
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`register_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing register command:`, error.message);
        socket.write(`register_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'login') {
      // 处理登录命令
      try {
        // 获取JSON字符串部分
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const loginData = JSON.parse(jsonStr);
        
        console.log(`Processing login command with data:`, loginData);
        
        // 转发到UserSVC的login接口
        axios.post(`${USER_SVC_BASE_URL}/login`, loginData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Login response from UserSVC:`, response.data);
          // 将响应发送回客户端，添加命令前缀
          socket.write(`login_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in login request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to UserSVC',
            error: error.message
          };
          
          // 如果服务器返回了错误响应
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`login_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing login command:`, error.message);
        socket.write(`login_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
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