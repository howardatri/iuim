package network

import (
	"bufio"
	"encoding/json"
	"fmt"
	"log"
	"net"
	"strings"
)

// Friend 好友数据结构
type Friend struct {
	FriendID int    `json:"friend_id"`
	Nickname string `json:"nickname"`
	Username string `json:"username"`
	Remark   string `json:"remark"`
	AddTime  string `json:"add_time"`
}

// User 用户数据结构
type User struct {
	UserID   int    `json:"user_id"`
	Username string `json:"username"`
	Nickname string `json:"nickname"`
	Email    string `json:"email"`
}

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

// QueryFriends 查询好友列表
func (m *NetworkManager) QueryFriends(userID, serviceID int) (map[string]interface{}, error) {
	// 构建查询命令
	queryData := map[string]interface{}{
		"user_id":    userID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询数据失败: %w", err)
	}

	cmd := fmt.Sprintf("query_friend %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: query_friend_resp {JSON数据}
	if !strings.HasPrefix(resp, "query_friend_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "query_friend_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// AddFriend 添加好友
func (m *NetworkManager) AddFriend(userID, friendID, serviceID int, remark string) (map[string]interface{}, error) {
	// 构建添加命令
	addData := map[string]interface{}{
		"user_id":    userID,
		"friend_id":  friendID,
		"service_id": serviceID,
		"remark":     remark,
	}

	jsonData, err := json.Marshal(addData)
	if err != nil {
		return nil, fmt.Errorf("序列化添加数据失败: %w", err)
	}

	cmd := fmt.Sprintf("add_friend %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: add_friend_resp {JSON数据}
	if !strings.HasPrefix(resp, "add_friend_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "add_friend_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// DeleteFriend 删除好友
func (m *NetworkManager) DeleteFriend(userID, friendID, serviceID int) (map[string]interface{}, error) {
	// 构建删除命令
	deleteData := map[string]interface{}{
		"user_id":    userID,
		"friend_id":  friendID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(deleteData)
	if err != nil {
		return nil, fmt.Errorf("序列化删除数据失败: %w", err)
	}

	cmd := fmt.Sprintf("delete_friend %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: delete_friend_resp {JSON数据}
	if !strings.HasPrefix(resp, "delete_friend_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "delete_friend_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// SearchUsers 搜索用户
func (m *NetworkManager) SearchUsers(keyword string) (map[string]interface{}, error) {
	// 构建搜索命令
	searchData := map[string]interface{}{
		"keyword": keyword,
	}

	jsonData, err := json.Marshal(searchData)
	if err != nil {
		return nil, fmt.Errorf("序列化搜索数据失败: %w", err)
	}

	cmd := fmt.Sprintf("search_users %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: search_users_resp {JSON数据}
	if !strings.HasPrefix(resp, "search_users_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "search_users_resp ")
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
	log.Printf("即将解析的登录响应JSON字符串: %s", jsonStr) // 新增日志

	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		log.Printf("JSON解析错误: %v", err) // 新增日志
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

// QueryUserServices 查询用户的服务列表
func (m *NetworkManager) QueryUserServices(userId int) (map[string]interface{}, error) {
	// 构建查询命令
	queryData := map[string]interface{}{
		"user_id": userId,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询数据失败: %w", err)
	}

	cmd := fmt.Sprintf("query_user_services %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: query_user_services_resp {JSON数据}
	if !strings.HasPrefix(resp, "query_user_services_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "query_user_services_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// ActivateService 激活用户服务
func (m *NetworkManager) ActivateService(userId int, serviceId int) (map[string]interface{}, error) {
	// 构建激活命令
	activateData := map[string]interface{}{
		"user_id":    userId,
		"service_id": serviceId,
	}

	jsonData, err := json.Marshal(activateData)
	if err != nil {
		return nil, fmt.Errorf("序列化激活数据失败: %w", err)
	}

	cmd := fmt.Sprintf("activate_service %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: activate_service_resp {JSON数据}
	if !strings.HasPrefix(resp, "activate_service_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "activate_service_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// DeactivateService 停用用户服务
func (m *NetworkManager) DeactivateService(userId int, serviceId int) (map[string]interface{}, error) {
	// 构建停用命令
	deactivateData := map[string]interface{}{
		"user_id":    userId,
		"service_id": serviceId,
	}

	jsonData, err := json.Marshal(deactivateData)
	if err != nil {
		return nil, fmt.Errorf("序列化停用数据失败: %w", err)
	}

	cmd := fmt.Sprintf("deactivate_service %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: deactivate_service_resp {JSON数据}
	if !strings.HasPrefix(resp, "deactivate_service_resp ") {
		return nil, fmt.Errorf("无效的响应格式")
	}

	jsonStr := strings.TrimPrefix(resp, "deactivate_service_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
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
	println("sendCommand")
	println(response)
	return strings.TrimSpace(response), nil
}
