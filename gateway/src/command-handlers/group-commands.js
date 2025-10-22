const BaseCommandHandler = require('./base-command-handler');

// GroupSVC配置
const GROUP_SVC_HOST = 'localhost';
const GROUP_SVC_PORT = 50055;
const GROUP_SVC_BASE_URL = `http://${GROUP_SVC_HOST}:${GROUP_SVC_PORT}`;

/**
 * 群组命令处理器
 * 处理群组相关命令：基础命令 + 第六阶段新增的增强功能
 */
class GroupCommandHandler extends BaseCommandHandler {
  constructor() {
    super();
    this.serviceName = 'GroupSVC';
  }

  // ========== 基础群组命令 ==========

  /**
   * 处理加入群组命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleJoinGroup(data, socket) {
    const command = 'join_group';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['user_id', 'group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/join', data);
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
   * 处理退出群组命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleQuitGroup(data, socket) {
    const command = 'quit_group';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['user_id', 'group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/quit', data);
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
   * 处理查询群成员命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleQueryGroupMembers(data, socket) {
    const command = 'query_group_members';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/members', data);
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
   * 处理创建群组命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleCreateGroup(data, socket) {
    const command = 'create_group';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['creator_id', 'group_name']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/create', data);
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
   * 处理获取用户群组列表命令
   * @param {object} data - 用户数据
   * @param {object} socket - TCP socket对象
   */
  async handleGetUserGroups(data, socket) {
    const command = 'get_user_groups';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['user_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/list', data);
      this.log('info', `${command} response from ${this.serviceName}`, response.data);
      
      const responseMessage = this.buildResponse(command, response.data);
      socket.write(`${responseMessage}\n`);
    } catch (error) {
      this.log('error', `Error in ${command} request`, error.message);
      const errorResponse = this.buildErrorResponse(command, error, 400);
      socket.write(`${errorResponse}\n`);
    }
  }

  // ========== 第六阶段新增的增强功能命令 ==========

  /**
   * 处理获取群组设置命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleGetGroupSettings(data, socket) {
    const command = 'get_group_settings';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/group_settings', data);
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
   * 处理更新群组设置命令
   * @param {object} data - 群组设置数据
   * @param {object} socket - TCP socket对象
   */
  async handleUpdateGroupSettings(data, socket) {
    const command = 'update_group_settings';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/update_group_settings', data);
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
 * 处理更改群组类型命令
 * @param {object} data - 群组类型数据
 * @param {object} socket - TCP socket对象
 */
async handleChangeGroupType(data, socket) {
  const command = 'change_group_type';
  this.log('info', `Processing ${command} command with data`, data);

  try {
    this.validateRequiredFields(data, ['group_id', 'service_id', 'target_service_id']);
    
    // 直接传递字段，无需映射
    const requestData = {
      group_id: data.group_id,
      service_id: data.service_id, // 原服务ID
      target_service_id: data.target_service_id
    };
    
    const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/change_group_type', requestData);
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
   * 处理设置成员角色命令
   * @param {object} data - 成员角色数据
   * @param {object} socket - TCP socket对象
   */
  async handleSetMemberRole(data, socket) {
    const command = 'set_member_role';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['group_id', 'user_id', 'service_id', 'role_type']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/set_member_role', data);
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
   * 处理获取成员角色命令
   * @param {object} data - 群组数据
   * @param {object} socket - TCP socket对象
   */
  async handleGetMemberRoles(data, socket) {
    const command = 'get_member_roles';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/get_member_roles', data);
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
   * 处理申请加入群组命令
   * @param {object} data - 申请数据
   * @param {object} socket - TCP socket对象
   */
  async handleApplyJoinGroup(data, socket) {
    const command = 'apply_join_group';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['user_id', 'group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/apply_join_group', data);
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
   * 处理邀请加入群组命令
   * @param {object} data - 邀请数据
   * @param {object} socket - TCP socket对象
   */
  async handleInviteJoinGroup(data, socket) {
    const command = 'invite_join_group';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['inviter_id', 'invitee_id', 'group_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/invite_join_group', data);
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
   * 处理自由加入话题命令
   * @param {object} data - 话题数据
   * @param {object} socket - TCP socket对象
   */
  async handleFreeJoinTopic(data, socket) {
    const command = 'free_join_topic';
    this.log('info', `Processing ${command} command with data`, data);

    try {
      this.validateRequiredFields(data, ['user_id', 'topic_id']);
      const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/free_join_topic', data);
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
 * 处理获取当前用户角色命令
 * @param {object} data - 请求数据
 * @param {object} socket - TCP socket对象
 */
async handleGetCurrentUserRole(data, socket) {
  const command = 'get_current_user_role';
  this.log('info', `Processing ${command} command with data`, data);

  try {
    this.validateRequiredFields(data, ['group_id', 'user_id']);
    const response = await this.sendRequest(GROUP_SVC_BASE_URL, '/get_current_user_role', data);
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
      // 基础群组命令
      case 'join_group':
        await this.handleJoinGroup(data, socket);
        break;
      case 'quit_group':
        await this.handleQuitGroup(data, socket);
        break;
      case 'query_group_members':
        await this.handleQueryGroupMembers(data, socket);
        break;
      case 'create_group':
        await this.handleCreateGroup(data, socket);
        break;
      case 'get_user_groups':
        await this.handleGetUserGroups(data, socket);
        break;
      
      // 第六阶段新增的增强功能命令
      case 'get_group_settings':
        await this.handleGetGroupSettings(data, socket);
        break;
      case 'update_group_settings':
        await this.handleUpdateGroupSettings(data, socket);
        break;
      case 'change_group_type':
        await this.handleChangeGroupType(data, socket);
        break;
      case 'set_member_role':
        await this.handleSetMemberRole(data, socket);
        break;
      case 'get_member_roles':
        await this.handleGetMemberRoles(data, socket);
        break;
      case 'apply_join_group':
        await this.handleApplyJoinGroup(data, socket);
        break;
      case 'invite_join_group':
        await this.handleInviteJoinGroup(data, socket);
        break;
      case 'free_join_topic':
        await this.handleFreeJoinTopic(data, socket);
        break;
      case 'get_current_user_role':
        await this.handleGetCurrentUserRole(data, socket);
        break;
      
      default:
        throw new Error(`Unknown group command: ${command}`);
    }
  }
}

module.exports = GroupCommandHandler;