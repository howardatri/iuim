const net = require('net');
const { CommandRouter, middlewares } = require('./routes/command-router');

// 网关配置
const PORT = 8233;

// 初始化命令路由器
const router = new CommandRouter();

// 添加中间件
router.use(middlewares.requestLogger);
router.use(middlewares.performance);

// 创建TCP服务器
const server = net.createServer((socket) => {
  console.log('Client connected');

  socket.on('data', async (data) => {
    const message = data.toString().trim();
    console.log(`Received: ${message}`);

    try {
      // 使用路由系统处理命令
      await router.route(message, socket);
    } catch (error) {
      // 全局错误处理中间件
      console.error(`[GLOBAL ERROR] Unhandled error in command processing:`, {
        error: error.message,
        stack: error.stack,
        message: message
      });
      
      // 发送通用错误响应
      try {
        socket.write(JSON.stringify({
          code: 500,
          message: 'Internal server error',
          error: 'An unexpected error occurred while processing your request'
        }) + '\n');
      } catch (writeError) {
        console.error(`[CRITICAL] Failed to send error response:`, writeError.message);
      }
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