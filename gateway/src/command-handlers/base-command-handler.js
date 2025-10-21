const axios = require('axios');

/**
 * 命令处理器基类
 * 提供统一的处理方法和错误处理机制
 */
class BaseCommandHandler {
  constructor() {
    this.timeout = 10000; // 默认超时时间
  }

  /**
   * 解析TCP-Line格式的消息
   * @param {string} message - 原始消息
   * @returns {object} 解析结果 {command, data}
   */
  parseMessage(message) {
    try {
      const parts = message.split(' ');
      const command = parts[0];
      
      if (parts.length < 2) {
        return { command, data: {} };
      }
      
      const jsonStr = message.substring(message.indexOf(' ') + 1);
      const data = JSON.parse(jsonStr);
      
      return { command, data };
    } catch (error) {
      throw new Error(`Invalid message format: ${error.message}`);
    }
  }

  /**
   * 发送HTTP请求到微服务
   * @param {string} baseUrl - 微服务基础URL
   * @param {string} endpoint - 接口端点
   * @param {object} data - 请求数据
   * @returns {Promise} axios响应
   */
  async sendRequest(baseUrl, endpoint, data) {
    try {
      const response = await axios.post(`${baseUrl}${endpoint}`, data, {
        headers: { 'Content-Type': 'application/json' },
        timeout: this.timeout
      });
      return response;
    } catch (error) {
      // 重新抛出错误，保持原有的错误信息
      throw error;
    }
  }

  /**
   * 构建响应消息
   * @param {string} command - 原始命令
   * @param {object} responseData - 响应数据
   * @returns {string} 格式化的响应消息
   */
  buildResponse(command, responseData) {
    return `${command}_resp ${JSON.stringify(responseData)}`;
  }

  /**
   * 构建错误响应
   * @param {string} command - 原始命令
   * @param {Error} error - 错误对象
   * @param {number} defaultCode - 默认错误码
   * @returns {string} 格式化的错误响应
   */
  buildErrorResponse(command, error, defaultCode = 500) {
    let errorResponse = {
      code: defaultCode,
      message: error.message || 'Internal server error',
      error: error.message
    };

    // 如果是axios错误且有响应数据，使用服务器返回的错误信息
    if (error.response && error.response.data) {
      errorResponse = error.response.data;
    }

    return this.buildResponse(command, errorResponse);
  }

  /**
   * 记录日志
   * @param {string} level - 日志级别 (info, error)
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
   * 验证必需参数
   * @param {object} data - 数据对象
   * @param {Array} requiredFields - 必需字段数组
   * @throws {Error} 如果缺少必需字段
   */
  validateRequiredFields(data, requiredFields) {
    const missingFields = requiredFields.filter(field => 
      data[field] === undefined || data[field] === null || data[field] === ''
    );
    
    if (missingFields.length > 0) {
      throw new Error(`Missing required fields: ${missingFields.join(', ')}`);
    }
  }

  /**
   * 处理命令的通用方法
   * 子类应该重写此方法
   * @param {string} command - 命令名
   * @param {object} data - 命令数据
   * @param {object} socket - TCP socket对象
   * @returns {Promise} 处理结果
   */
  async handleCommand(command, data, socket) {
    throw new Error('handleCommand method must be implemented by subclass');
  }
}

module.exports = BaseCommandHandler;