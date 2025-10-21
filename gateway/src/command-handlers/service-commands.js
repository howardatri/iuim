const BaseCommandHandler = require('./base-command-handler');

// ServiceSVC配置
const SERVICE_SVC_HOST = 'localhost';
const SERVICE_SVC_PORT = 50056;
const SERVICE_SVC_BASE_URL = `http://${SERVICE_SVC_HOST}:${SERVICE_SVC_PORT}`;

/**
 * 服务管理命令处理器
 * 处理服务管理相关命令：activate_service, deactivate_service, query_user_services
 */
class ServiceCommandHandler extends BaseCommandHandler {
  constructor() {
    super();
    this.serviceName = 'ServiceSVC';
  }

  /**
   * 处理激活服务命令
   * @param {object} data - 服务数据
   * @param {object} socket - TCP socket对象
   */
  async handleActivateService(data, socket) {
    const command = 'activate_service';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'service_id']);

      const response = await this.sendRequest(SERVICE_SVC_BASE_URL, '/activate', data);
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
   * 处理停用服务命令
   * @param {object} data - 服务数据
   * @param {object} socket - TCP socket对象
   */
  async handleDeactivateService(data, socket) {
    const command = 'deactivate_service';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'service_id']);

      const response = await this.sendRequest(SERVICE_SVC_BASE_URL, '/deactivate', data);
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
   * 处理查询用户服务命令
   * @param {object} data - 用户数据
   * @param {object} socket - TCP socket对象
   */
  async handleQueryUserServices(data, socket) {
    const command = 'query_user_services';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id']);

      const response = await this.sendRequest(SERVICE_SVC_BASE_URL, '/query_user_services', data);
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
      case 'activate_service':
        await this.handleActivateService(data, socket);
        break;
      case 'deactivate_service':
        await this.handleDeactivateService(data, socket);
        break;
      case 'query_user_services':
        await this.handleQueryUserServices(data, socket);
        break;
      default:
        throw new Error(`Unknown service command: ${command}`);
    }
  }
}

module.exports = ServiceCommandHandler;