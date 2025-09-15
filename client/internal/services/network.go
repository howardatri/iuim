package services

import (
	"encoding/json"
	"fmt"
	"net"
)

// NetworkService 处理与服务器的网络通信
type NetworkService struct {
	conn     net.Conn
	serverIP string
	port     int
	isConnected bool
}

// NewNetworkService 创建一个新的网络服务
func NewNetworkService(serverIP string, port int) *NetworkService {
	return &NetworkService{
		serverIP: serverIP,
		port:     port,
		isConnected: false,
	}
}

// Connect 连接到服务器
func (n *NetworkService) Connect() error {
	address := fmt.Sprintf("%s:%d", n.serverIP, n.port)
	conn, err := net.Dial("tcp", address)
	if err != nil {
		return err
	}
	n.conn = conn
	n.isConnected = true
	return nil
}

// Disconnect 断开与服务器的连接
func (n *NetworkService) Disconnect() error {
	if n.conn != nil {
		err := n.conn.Close()
		n.isConnected = false
		return err
	}
	return nil
}

// IsConnected 检查是否已连接到服务器
func (n *NetworkService) IsConnected() bool {
	return n.isConnected
}

// SendMessage 发送消息到服务器
func (n *NetworkService) SendMessage(message map[string]interface{}) error {
	if !n.isConnected {
		return fmt.Errorf("not connected to server")
	}
	
	jsonData, err := json.Marshal(message)
	if err != nil {
		return err
	}
	
	_, err = n.conn.Write(jsonData)
	return err
}

// ReceiveMessage 从服务器接收消息
func (n *NetworkService) ReceiveMessage() (map[string]interface{}, error) {
	if !n.isConnected {
		return nil, fmt.Errorf("not connected to server")
	}
	
	buffer := make([]byte, 4096)
	length, err := n.conn.Read(buffer)
	if err != nil {
		return nil, err
	}
	
	var message map[string]interface{}
	err = json.Unmarshal(buffer[:length], &message)
	if err != nil {
		return nil, err
	}
	
	return message, nil
}

// Login 登录到服务器
func (n *NetworkService) Login(userID, password string) (bool, error) {
	message := map[string]interface{}{
		"type":     "login",
		"userID":   userID,
		"password": password,
	}
	
	err := n.SendMessage(message)
	if err != nil {
		return false, err
	}
	
	response, err := n.ReceiveMessage()
	if err != nil {
		return false, err
	}
	
	success, ok := response["success"].(bool)
	if !ok {
		return false, fmt.Errorf("invalid response format")
	}
	
	return success, nil
}

// Logout 从服务器登出
func (n *NetworkService) Logout(userID string) (bool, error) {
	message := map[string]interface{}{
		"type":   "logout",
		"userID": userID,
	}
	
	err := n.SendMessage(message)
	if err != nil {
		return false, err
	}
	
	response, err := n.ReceiveMessage()
	if err != nil {
		return false, err
	}
	
	success, ok := response["success"].(bool)
	if !ok {
		return false, fmt.Errorf("invalid response format")
	}
	
	return success, nil
}