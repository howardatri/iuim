const BaseCommandHandler = require('./base-command-handler');

// UserSVC配置
const USER_SVC_HOST = 'localhost';
const USER_SVC_PORT = 50051;
const USER_SVC_BASE_URL = `http://${USER_SVC_HOST}:${USER_SVC_PORT}`;

/**
 * 用户命令处理器
 * 处理用户相关命令：register, login, health_check
 */
class UserCommandHandler extends BaseCommandHandler {
  constructor() {
    super();
    this.serviceName = 'UserSVC';
  }

  /**
   * 处理健康检查命令
   * @param {object} data - 命令数据
   * @param {object} socket - TCP socket对象
   */
  async handleHealthCheck(data, socket) {
    const command = 'health_check';
    this.log('info', `Processing ${command} command`);

    try {
      const response = await this.sendRequest(USER_SVC_BASE_URL, '/health-check', {});
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      // 直接发送响应数据，不添加命令前缀（保持原有格式）
      socket.write(`${JSON.stringify(response.data)}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理用户注册命令
   * @param {object} data - 用户注册数据
   * @param {object} socket - TCP socket对象
   */
  async handleRegister(data, socket) {
    const command = 'register';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['username', 'password']);

      const response = await this.sendRequest(USER_SVC_BASE_URL, '/register', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理用户登录命令
   * @param {object} data - 用户登录数据
   * @param {object} socket - TCP socket对象
   */
  async handleLogin(data, socket) {
    const command = 'login';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['username', 'password']);

      const response = await this.sendRequest(USER_SVC_BASE_URL, '/login', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理更新用户资料命令
   * @param {object} data - 用户资料数据
   * @param {object} socket - TCP socket对象
   */
  async handleUpdateProfile(data, socket) {
    const command = 'update_profile';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id']);

      const response = await this.sendRequest(USER_SVC_BASE_URL, '/update_profile', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理绑定微信命令
   * @param {object} data - 微信绑定数据
   * @param {object} socket - TCP socket对象
   */
  async handleBindWechat(data, socket) {
    const command = 'bind_wechat';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'wechat_id']);

      const response = await this.sendRequest(USER_SVC_BASE_URL, '/bind_wechat', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理获取用户资料命令
   * @param {object} data - 获取资料数据
   * @param {object} socket - TCP socket对象
   */
  async handleGetProfile(data, socket) {
    const command = 'get_profile';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id']);

      const response = await this.sendRequest(USER_SVC_BASE_URL, '/get_profile', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  /**
   * 处理命令的主入口
   * @param {string} command - 命令名
   * @param {object} data - 命令数据
   * @param {object} socket - TCP socket对象
   */
  async handleCommand(command, data, socket) {
    switch (command) {
      case 'health_check':
        await this.handleHealthCheck(data, socket);
        break;
      case 'register':
        await this.handleRegister(data, socket);
        break;
      case 'login':
        await this.handleLogin(data, socket);
        break;
      case 'update_profile':
        await this.handleUpdateProfile(data, socket);
        break;
      case 'bind_wechat':
        await this.handleBindWechat(data, socket);
        break;
      case 'get_profile':
        await this.handleGetProfile(data, socket);
        break;
      default:
        throw new Error(`Unknown user command: ${command}`);
    }
  }
}

module.exports = UserCommandHandler;