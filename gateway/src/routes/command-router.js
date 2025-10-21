const UserCommandHandler = require('../command-handlers/user-commands');
const FriendCommandHandler = require('../command-handlers/friend-commands');
const GroupCommandHandler = require('../command-handlers/group-commands');
const ServiceCommandHandler = require('../command-handlers/service-commands');
const MessageCommandHandler = require('../command-handlers/message-commands');

/**
 * 中心化命令路由系统
 * 负责解析TCP-Line格式消息，路由到对应的处理器，并提供中间件机制
 */
class CommandRouter {
  constructor() {
    // 初始化命令处理器实例
    this.userHandler = new UserCommandHandler();
    this.friendHandler = new FriendCommandHandler();
    this.groupHandler = new GroupCommandHandler();
    this.serviceHandler = new ServiceCommandHandler();
    this.messageHandler = new MessageCommandHandler();

    // 命令路由映射表
    this.commandRoutes = new Map();
    this.commandAliases = new Map();
    this.middlewares = [];

    // 初始化路由映射
    this.initializeRoutes();
  }

  /**
   * 初始化命令路由映射
   */
  initializeRoutes() {
    // 用户相关命令
    this.registerCommand('health_check', this.userHandler);
    this.registerCommand('register', this.userHandler);
    this.registerCommand('login', this.userHandler);
    
    // 用户资料相关命令
    this.registerCommand('update_profile', this.userHandler);
    this.registerCommand('bind_wechat', this.userHandler);
    this.registerCommand('get_profile', this.userHandler);

    // 好友相关命令
    this.registerCommand('add_friend', this.friendHandler);
    this.registerCommand('delete_friend', this.friendHandler);
    this.registerCommand('query_friend', this.friendHandler);
    this.registerCommand('search_users', this.friendHandler);

    // 群组相关命令（基础 + 增强功能）
    this.registerCommand('join_group', this.groupHandler);
    this.registerCommand('quit_group', this.groupHandler);
    this.registerCommand('query_group_members', this.groupHandler);
    this.registerCommand('create_group', this.groupHandler);
    this.registerCommand('get_user_groups', this.groupHandler);
    
    // 第六阶段新增的群组增强功能
    this.registerCommand('get_group_settings', this.groupHandler);
    this.registerCommand('update_group_settings', this.groupHandler);
    this.registerCommand('change_group_type', this.groupHandler);
    this.registerCommand('set_member_role', this.groupHandler);
    this.registerCommand('get_member_roles', this.groupHandler);
    this.registerCommand('apply_join_group', this.groupHandler);
    this.registerCommand('invite_join_group', this.groupHandler);
    this.registerCommand('free_join_topic', this.groupHandler);
    this.registerCommand('get_current_user_role', this.groupHandler);

    // 服务管理相关命令
    this.registerCommand('activate_service', this.serviceHandler);
    this.registerCommand('deactivate_service', this.serviceHandler);
    this.registerCommand('query_user_services', this.serviceHandler);

    // 消息相关命令
    this.registerCommand('send_message', this.messageHandler);
    this.registerCommand('get_history', this.messageHandler);

    // 设置命令别名（如果需要）
    this.registerAlias('health', 'health_check');
    this.registerAlias('reg', 'register');
    this.registerAlias('signin', 'login');
  }

  /**
   * 注册命令处理器
   * @param {string} command - 命令名
   * @param {BaseCommandHandler} handler - 处理器实例
   */
  registerCommand(command, handler) {
    this.commandRoutes.set(command, handler);
  }

  /**
   * 注册命令别名
   * @param {string} alias - 别名
   * @param {string} originalCommand - 原始命令名
   */
  registerAlias(alias, originalCommand) {
    this.commandAliases.set(alias, originalCommand);
  }

  /**
   * 添加中间件
   * @param {Function} middleware - 中间件函数
   */
  use(middleware) {
    this.middlewares.push(middleware);
  }

  /**
   * 解析TCP-Line格式的消息
   * @param {string} message - 原始消息
   * @returns {object} 解析结果 {command, data, originalMessage}
   */
  parseMessage(message) {
    try {
      const trimmedMessage = message.toString().trim();
      const parts = trimmedMessage.split(' ');
      const command = parts[0];
      
      let data = {};
      if (parts.length > 1) {
        const jsonStr = trimmedMessage.substring(trimmedMessage.indexOf(' ') + 1);
        data = JSON.parse(jsonStr);
      }
      
      return {
        command,
        data,
        originalMessage: trimmedMessage
      };
    } catch (error) {
      throw new Error(`Failed to parse message: ${error.message}`);
    }
  }

  /**
   * 查找命令对应的处理器
   * @param {string} command - 命令名
   * @returns {BaseCommandHandler|null} 处理器实例
   */
  findHandler(command) {
    // 首先检查是否是别名
    const actualCommand = this.commandAliases.get(command) || command;
    
    // 查找对应的处理器
    return this.commandRoutes.get(actualCommand) || null;
  }

  /**
   * 执行中间件链
   * @param {object} context - 上下文对象
   * @param {number} index - 当前中间件索引
   * @returns {Promise} 执行结果
   */
  async executeMiddlewares(context, index = 0) {
    if (index >= this.middlewares.length) {
      return; // 所有中间件执行完毕
    }

    const middleware = this.middlewares[index];
    await middleware(context, () => this.executeMiddlewares(context, index + 1));
  }

  /**
   * 路由并处理命令
   * @param {string} message - 原始消息
   * @param {object} socket - TCP socket对象
   * @returns {Promise} 处理结果
   */
  async route(message, socket) {
    let context = null;
    
    try {
      // 解析消息
      const { command, data, originalMessage } = this.parseMessage(message);
      
      // 创建上下文对象
      context = {
        command,
        data,
        originalMessage,
        socket,
        startTime: Date.now(),
        handled: false
      };

      // 执行中间件
      await this.executeMiddlewares(context);

      // 如果中间件已经处理了请求，直接返回
      if (context.handled) {
        return;
      }

      // 查找处理器
      const handler = this.findHandler(command);
      
      if (!handler) {
        throw new Error(`Unknown command: ${command}`);
      }

      // 执行命令处理
      await handler.handleCommand(command, data, socket);
      
      // 记录成功处理的日志
      this.log('info', `Command ${command} processed successfully`, {
        processingTime: Date.now() - context.startTime
      });

    } catch (error) {
      // 统一错误处理
      this.handleError(error, context, socket);
    }
  }

  /**
   * 统一错误处理
   * @param {Error} error - 错误对象
   * @param {object} context - 上下文对象
   * @param {object} socket - TCP socket对象
   */
  handleError(error, context, socket) {
    const command = context ? context.command : 'unknown';
    
    this.log('error', `Error processing command ${command}`, {
      error: error.message,
      stack: error.stack,
      processingTime: context ? Date.now() - context.startTime : 0
    });

    // 构建错误响应
    const errorResponse = {
      code: 400,
      message: error.message || 'Command processing failed',
      error: error.message
    };

    // 发送错误响应
    try {
      socket.write(`${JSON.stringify(errorResponse)}\n`);
    } catch (writeError) {
      this.log('error', 'Failed to send error response', writeError.message);
    }
  }

  /**
   * 记录日志
   * @param {string} level - 日志级别
   * @param {string} message - 日志消息
   * @param {object} data - 附加数据
   */
  log(level, message, data = null) {
    const timestamp = new Date().toISOString();
    const logMessage = data 
      ? `[${timestamp}] ${level.toUpperCase()}: ${message} - ${JSON.stringify(data)}`
      : `[${timestamp}] ${level.toUpperCase()}: ${message}`;
    
    if (level === 'error') {
      console.error(logMessage);
    } else {
      console.log(logMessage);
    }
  }

  /**
   * 获取所有注册的命令列表
   * @returns {Array} 命令列表
   */
  getRegisteredCommands() {
    return Array.from(this.commandRoutes.keys());
  }

  /**
   * 获取所有注册的别名列表
   * @returns {Array} 别名列表
   */
  getRegisteredAliases() {
    return Array.from(this.commandAliases.keys());
  }

  /**
   * 获取路由统计信息
   * @returns {object} 统计信息
   */
  getStats() {
    return {
      totalCommands: this.commandRoutes.size,
      totalAliases: this.commandAliases.size,
      totalMiddlewares: this.middlewares.length,
      handlers: {
        user: this.userHandler.constructor.name,
        friend: this.friendHandler.constructor.name,
        group: this.groupHandler.constructor.name,
        service: this.serviceHandler.constructor.name,
        message: this.messageHandler.constructor.name
      }
    };
  }
}

// 创建默认的中间件

/**
 * 请求日志中间件
 * @param {object} context - 上下文对象
 * @param {Function} next - 下一个中间件
 */
const requestLoggerMiddleware = async (context, next) => {
  console.log(`[${new Date().toISOString()}] Incoming command: ${context.command}`);
  await next();
};

/**
 * 性能监控中间件
 * @param {object} context - 上下文对象
 * @param {Function} next - 下一个中间件
 */
const performanceMiddleware = async (context, next) => {
  const startTime = Date.now();
  await next();
  const processingTime = Date.now() - startTime;
  
  if (processingTime > 1000) { // 超过1秒的请求记录警告
    console.warn(`[PERFORMANCE] Command ${context.command} took ${processingTime}ms to process`);
  }
};

module.exports = {
  CommandRouter,
  middlewares: {
    requestLogger: requestLoggerMiddleware,
    performance: performanceMiddleware
  }
};