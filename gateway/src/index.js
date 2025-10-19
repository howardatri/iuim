const net = require('net');
const http = require('http');
const axios = require('axios');

// UserSVC配置
const PORT = 8233;
const USER_SVC_HOST = 'localhost';
const USER_SVC_PORT = 50051;
const USER_SVC_BASE_URL = `http://${USER_SVC_HOST}:${USER_SVC_PORT}`;

// ServiceSVC配置
const SERVICE_SVC_HOST = 'localhost';
const SERVICE_SVC_PORT = 50056;
const SERVICE_SVC_BASE_URL = `http://${SERVICE_SVC_HOST}:${SERVICE_SVC_PORT}`;

// FriendSVC配置
const FRIEND_SVC_HOST = 'localhost';
const FRIEND_SVC_PORT = 50054;
const FRIEND_SVC_BASE_URL = `http://${FRIEND_SVC_HOST}:${FRIEND_SVC_PORT}`;

// MsgSVC配置
const MSG_SVC_HOST = 'localhost';
const MSG_SVC_PORT = 50052;
const MSG_SVC_BASE_URL = `http://${MSG_SVC_HOST}:${MSG_SVC_PORT}`;

// GroupSVC配置
const GROUP_SVC_HOST = 'localhost';
const GROUP_SVC_PORT = 50055;
const GROUP_SVC_BASE_URL = `http://${GROUP_SVC_HOST}:${GROUP_SVC_PORT}`;

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
    } else if (cmd === 'activate_service') {
      // 处理激活服务命令
      try {
        // 获取JSON字符串部分
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const serviceData = JSON.parse(jsonStr);
        
        console.log(`Processing activate_service command with data:`, serviceData);
        
        // 转发到ServiceSVC的activate接口
        axios.post(`${SERVICE_SVC_BASE_URL}/activate`, serviceData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Activate service response from ServiceSVC:`, response.data);
          // 将响应发送回客户端，添加命令前缀
          socket.write(`activate_service_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in activate_service request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to ServiceSVC',
            error: error.message
          };
          
          // 如果服务器返回了错误响应
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`activate_service_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing activate_service command:`, error.message);
        socket.write(`activate_service_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'deactivate_service') {
      // 处理停用服务命令
      try {
        // 获取JSON字符串部分
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const serviceData = JSON.parse(jsonStr);
        
        console.log(`Processing deactivate_service command with data:`, serviceData);
        
        // 转发到ServiceSVC的deactivate接口
        axios.post(`${SERVICE_SVC_BASE_URL}/deactivate`, serviceData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Deactivate service response from ServiceSVC:`, response.data);
          // 将响应发送回客户端，添加命令前缀
          socket.write(`deactivate_service_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in deactivate_service request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to ServiceSVC',
            error: error.message
          };
          
          // 如果服务器返回了错误响应
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`deactivate_service_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing deactivate_service command:`, error.message);
        socket.write(`deactivate_service_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'query_user_services') {
      // 处理查询用户服务命令
      try {
        // 获取JSON字符串部分
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const userData = JSON.parse(jsonStr);
        
        console.log(`Processing query_user_services command with data:`, userData);
        
        // 转发到ServiceSVC的query_user_services接口
        axios.post(`${SERVICE_SVC_BASE_URL}/query_user_services`, userData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Query user services response from ServiceSVC:`, response.data);
          // 将响应发送回客户端，添加命令前缀
          socket.write(`query_user_services_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in query_user_services request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to ServiceSVC',
            error: error.message
          };
          
          // 如果服务器返回了错误响应
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`query_user_services_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing query_user_services command:`, error.message);
        socket.write(`query_user_services_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'add_friend') {
      // 处理添加好友命令
      try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const friendData = JSON.parse(jsonStr);
        
        console.log(`Processing add_friend command with data:`, friendData);
        
        // 转发到FriendSVC的add接口
        axios.post(`${FRIEND_SVC_BASE_URL}/add`, friendData, {
          headers: { 'Content-Type': 'application/json' },
          timeout: 10000  // 添加超时设置
        })
        .then(response => {
          console.log(`Add friend response from FriendSVC:`, response.data);
          socket.write(`add_friend_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in add_friend request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to FriendSVC',
            error: error.message
          };
          
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`add_friend_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing add_friend command:`, error.message);
        socket.write(`add_friend_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'delete_friend') {
      // 处理删除好友命令
      try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const friendData = JSON.parse(jsonStr);
        
        console.log(`Processing delete_friend command with data:`, friendData);
        
        // 转发到FriendSVC的delete接口
        axios.post(`${FRIEND_SVC_BASE_URL}/delete`, friendData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Delete friend response from FriendSVC:`, response.data);
          socket.write(`delete_friend_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in delete_friend request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to FriendSVC',
            error: error.message
          };
          
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`delete_friend_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing delete_friend command:`, error.message);
        socket.write(`delete_friend_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'query_friend') {
      // 处理查询好友命令
      try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const friendData = JSON.parse(jsonStr);
        
        console.log(`Processing query_friend command with data:`, friendData);
        
        // 转发到FriendSVC的query接口
        axios.post(`${FRIEND_SVC_BASE_URL}/query`, friendData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Query friend response from FriendSVC:`, response.data);
          socket.write(`query_friend_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in query_friend request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to FriendSVC',
            error: error.message
          };
          
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`query_friend_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing query_friend command:`, error.message);
        socket.write(`query_friend_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'send_message') {
      // 处理发送消息命令
      try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const messageData = JSON.parse(jsonStr);
        
        console.log(`Processing send_message command with data:`, messageData);
        
        // 转发到MsgSVC的send接口
        axios.post(`${MSG_SVC_BASE_URL}/send`, messageData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Send message response from MsgSVC:`, response.data);
          socket.write(`send_message_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in send_message request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to MsgSVC',
            error: error.message
          };
          
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`send_message_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing send_message command:`, error.message);
        socket.write(`send_message_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    } else if (cmd === 'get_history') {
      // 处理获取历史消息命令
      try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const historyData = JSON.parse(jsonStr);
        
        console.log(`Processing get_history command with data:`, historyData);
        
        // 转发到MsgSVC的history接口
        axios.post(`${MSG_SVC_BASE_URL}/history`, historyData, {
          headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
          console.log(`Get history response from MsgSVC:`, response.data);
          socket.write(`get_history_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
          console.error(`Error in get_history request:`, error.message);
          let errorResponse = {
            code: 500,
            message: 'Error connecting to MsgSVC',
            error: error.message
          };
          
          if (error.response && error.response.data) {
            errorResponse = error.response.data;
          }
          
          socket.write(`get_history_resp ${JSON.stringify(errorResponse)}\n`);
        });
      } catch (error) {
        console.error(`Error parsing get_history command:`, error.message);
        socket.write(`get_history_resp ${JSON.stringify({
          code: 400,
          message: 'Invalid JSON format',
          error: error.message
        })}\n`);
      }
    }else if (cmd === 'search_users') {
    // 处理搜索用户命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const searchData = JSON.parse(jsonStr);
        
        console.log(`Processing search_users command with data:`, searchData);
        
        // 转发到FriendSVC的search接口（端口50054）
        axios.post(`${FRIEND_SVC_BASE_URL}/search`, searchData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Search users response from FriendSVC:`, response.data);
            socket.write(`search_users_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in search_users request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to FriendSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`search_users_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing search_users command:`, error.message);
        socket.write(`search_users_resp ${JSON.stringify({
            code: 400,
            message: 'Invalid JSON format',
            error: error.message
        })}\n`);
    }
    }else if (cmd === 'join_group') {
    // 处理加入群组命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const groupData = JSON.parse(jsonStr);
        
        console.log(`Processing join_group command with data:`, groupData);
        
        // 转发到GroupSVC的join接口
        axios.post(`${GROUP_SVC_BASE_URL}/join`, groupData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Join group response from GroupSVC:`, response.data);
            socket.write(`join_group_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in join_group request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to GroupSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`join_group_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing join_group command:`, error.message);
        socket.write(`join_group_resp ${JSON.stringify({
            code: 400,
            message: 'Invalid JSON format',
            error: error.message
        })}\n`);
    }
} else if (cmd === 'quit_group') {
    // 处理退出群组命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const groupData = JSON.parse(jsonStr);
        
        console.log(`Processing quit_group command with data:`, groupData);
        
        // 转发到GroupSVC的quit接口
        axios.post(`${GROUP_SVC_BASE_URL}/quit`, groupData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Quit group response from GroupSVC:`, response.data);
            socket.write(`quit_group_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in quit_group request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to GroupSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`quit_group_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing quit_group command:`, error.message);
        socket.write(`quit_group_resp ${JSON.stringify({
            code: 400,
            message: 'Invalid JSON format',
            error: error.message
        })}\n`);
    }
} else if (cmd === 'query_group_members') {
    // 处理查询群成员命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const groupData = JSON.parse(jsonStr);
        
        console.log(`Processing query_group_members command with data:`, groupData);
        
        // 转发到GroupSVC的members接口
        axios.post(`${GROUP_SVC_BASE_URL}/members`, groupData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Query group members response from GroupSVC:`, response.data);
            socket.write(`query_group_members_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in query_group_members request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to GroupSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`query_group_members_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing query_group_members command:`, error.message);
        socket.write(`query_group_members_resp ${JSON.stringify({
            code: 400,
            message: 'Invalid JSON format',
            error: error.message
        })}\n`);
    }
} else if (cmd === 'create_group') {
    // 处理创建群组命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const groupData = JSON.parse(jsonStr);
        
        console.log(`Processing create_group command with data:`, groupData);
        
        // 转发到GroupSVC的create接口
        axios.post(`${GROUP_SVC_BASE_URL}/create`, groupData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Create group response from GroupSVC:`, response.data);
            socket.write(`create_group_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in create_group request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to GroupSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`create_group_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing create_group command:`, error.message);
        socket.write(`create_group_resp ${JSON.stringify({
            code: 400,
            message: 'Invalid JSON format',
            error: error.message
        })}\n`);
    }
}else if (cmd === 'get_user_groups') {
    // 处理获取用户群组列表命令
    try {
        const jsonStr = message.substring(message.indexOf(' ') + 1);
        const groupData = JSON.parse(jsonStr);
        
        console.log(`Processing get_user_groups command with data:`, groupData);
        
        // 转发到GroupSVC的list接口
        axios.post(`${GROUP_SVC_BASE_URL}/list`, groupData, {
            headers: { 'Content-Type': 'application/json' }
        })
        .then(response => {
            console.log(`Get user groups response from GroupSVC:`, response.data);
            socket.write(`get_user_groups_resp ${JSON.stringify(response.data)}\n`);
        })
        .catch(error => {
            console.error(`Error in get_user_groups request:`, error.message);
            let errorResponse = {
                code: 500,
                message: 'Error connecting to GroupSVC',
                error: error.message
            };
            
            if (error.response && error.response.data) {
                errorResponse = error.response.data;
            }
            
            socket.write(`get_user_groups_resp ${JSON.stringify(errorResponse)}\n`);
        });
    } catch (error) {
        console.error(`Error parsing get_user_groups command:`, error.message);
        socket.write(`get_user_groups_resp ${JSON.stringify({
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