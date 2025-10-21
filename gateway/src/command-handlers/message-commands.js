const BaseCommandHandler = require('./base-command-handler');

// MsgSVC配置
const MSG_SVC_HOST = 'localhost';
const MSG_SVC_PORT = 50052;
const MSG_SVC_BASE_URL = `http://${MSG_SVC_HOST}:${MSG_SVC_PORT}`;

/**
 * 消息命令处理器
 * 处理消息相关命令：send_message, get_history
 */
class MessageCommandHandler extends BaseCommandHandler {
  constructor() {
    super();
    this.serviceName = 'MsgSVC';
  }

  /**
   * 处理发送消息命令
   * @param {object} data - 消息数据
   * @param {object} socket - TCP socket对象
   */
  async handleSendMessage(data, socket) {
    const command = 'send_message';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['sender_id', 'receiver_id', 'content']);

      const response = await this.sendRequest(MSG_SVC_BASE_URL, '/send', data);
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
   * 处理获取历史消息命令
   * @param {object} data - 历史消息查询数据
   * @param {object} socket - TCP socket对象
   */
  async handleGetHistory(data, socket) {
    const command = 'get_history';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id']);

      const response = await this.sendRequest(MSG_SVC_BASE_URL, '/history', data);
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
      case 'send_message':
        await this.handleSendMessage(data, socket);
        break;
      case 'get_history':
        await this.handleGetHistory(data, socket);
        break;
      default:
        throw new Error(`Unknown message command: ${command}`);
    }
  }
}

module.exports = MessageCommandHandler;