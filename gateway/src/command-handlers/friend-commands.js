const BaseCommandHandler = require('./base-command-handler');

// FriendSVC配置
const FRIEND_SVC_HOST = 'localhost';
const FRIEND_SVC_PORT = 50054;
const FRIEND_SVC_BASE_URL = `http://${FRIEND_SVC_HOST}:${FRIEND_SVC_PORT}`;

/**
 * 好友命令处理器
 * 处理好友相关命令：add_friend, delete_friend, query_friend, search_users
 */
class FriendCommandHandler extends BaseCommandHandler {
  constructor() {
    super();
    this.serviceName = 'FriendSVC';
  }

  /**
   * 处理添加好友命令
   * @param {object} data - 好友数据
   * @param {object} socket - TCP socket对象
   */
  async handleAddFriend(data, socket) {
    const command = 'add_friend';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'friend_id']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/add', data);
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
   * 处理删除好友命令
   * @param {object} data - 好友数据
   * @param {object} socket - TCP socket对象
   */
  async handleDeleteFriend(data, socket) {
    const command = 'delete_friend';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'friend_id']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/delete', data);
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
   * 处理查询好友命令
   * @param {object} data - 查询数据
   * @param {object} socket - TCP socket对象
   */
  async handleQueryFriend(data, socket) {
    const command = 'query_friend';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/query', data);
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
   * 处理搜索用户命令
   * @param {object} data - 搜索数据
   * @param {object} socket - TCP socket对象
   */
  async handleSearchUsers(data, socket) {
    const command = 'search_users';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['keyword']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/search', data);
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
   * 处理查询共同好友命令
   * @param {object} data - 查询数据
   * @param {object} socket - TCP socket对象
   */
  async handleQueryCommonFriends(data, socket) {
    const command = 'query_common_friends';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'friend_id', 'service_id']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/common', data);
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
   * 处理查询跨服务好友推荐命令
   * @param {object} data - 查询数据
   * @param {object} socket - TCP socket对象
   */
  async handleQueryCrossServiceFriends(data, socket) {
    const command = 'query_cross_service_friends';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      // 验证必需字段
      this.validateRequiredFields(data, ['user_id', 'current_service_id', 'target_service_id']);

      const response = await this.sendRequest(FRIEND_SVC_BASE_URL, '/cross-service', data);
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
      case 'add_friend':
        await this.handleAddFriend(data, socket);
        break;
      case 'delete_friend':
        await this.handleDeleteFriend(data, socket);
        break;
      case 'query_friend':
        await this.handleQueryFriend(data, socket);
        break;
      case 'search_users':
        await this.handleSearchUsers(data, socket);
        break;
      case 'query_common_friends':
        await this.handleQueryCommonFriends(data, socket);
        break;
      case 'query_cross_service_friends':
        await this.handleQueryCrossServiceFriends(data, socket);
        break;
      default:
        throw new Error(`Unknown friend command: ${command}`);
    }
  }
}

module.exports = FriendCommandHandler;