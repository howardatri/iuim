package network

import (
	"bufio"
	"encoding/json"
	"fmt"
	"net"
	"strings"
)

const (
	GatewayHost = "localhost"
	GatewayPort = "8233"
)

// NetworkManager 处理与网关的网络通信
type NetworkManager struct {
	serverAddr string
}

// NewNetworkManager 创建一个新的网络管理器
func NewNetworkManager() *NetworkManager {
	return &NetworkManager{
		serverAddr: fmt.Sprintf("%s:%s", GatewayHost, GatewayPort),
	}
}

// Register 发送注册请求到网关
func (m *NetworkManager) Register(userInfo map[string]string) (map[string]interface{}, error) {
	// 构建注册命令
	jsonData, err := json.Marshal(userInfo)
	if err != nil {
		return nil, fmt.Errorf("序列化用户数据失败: %w", err)
	}

	cmd := fmt.Sprintf("register %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: register_resp {JSON数据}
	if !strings.HasPrefix(resp, "register_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "register_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// Login 发送登录请求到网关
func (m *NetworkManager) Login(username, password string) (map[string]interface{}, error) {
	// 构建登录命令
	loginData := map[string]string{
		"username": username,
		"password": password,
	}

	jsonData, err := json.Marshal(loginData)
	if err != nil {
		return nil, fmt.Errorf("序列化登录数据失败: %w", err)
	}

	cmd := fmt.Sprintf("login %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: login_resp {JSON数据}
	if !strings.HasPrefix(resp, "login_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "login_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// TestConnection 测试与网关的连接
func (m *NetworkManager) TestConnection() (map[string]interface{}, error) {
	resp, err := m.sendCommand("health_check")
	if err != nil {
		return nil, err
	}

	var result map[string]interface{}
	if err := json.Unmarshal([]byte(resp), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// sendCommand 发送命令到网关并获取响应
func (m *NetworkManager) sendCommand(cmd string) (string, error) {
	// 连接到网关
	conn, err := net.Dial("tcp", m.serverAddr)
	if err != nil {
		return "", fmt.Errorf("连接网关失败: %w", err)
	}
	defer conn.Close()

	// 发送命令
	if _, err := fmt.Fprintf(conn, "%s\n", cmd); err != nil {
		return "", fmt.Errorf("发送命令失败: %w", err)
	}

	// 读取响应
	reader := bufio.NewReader(conn)
	response, err := reader.ReadString('\n')
	if err != nil {
		return "", fmt.Errorf("读取响应失败: %w", err)
	}

	return strings.TrimSpace(response), nil
}