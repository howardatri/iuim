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

// QueryCommonFriends 查询共同好友
func (m *NetworkManager) QueryCommonFriends(userID, friendID, serviceID int) (map[string]interface{}, error) {
	// 构建查询命令
	queryData := map[string]interface{}{
		"user_id":    userID,
		"friend_id":  friendID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询数据失败: %w", err)
	}

	cmd := fmt.Sprintf("query_common_friends %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: query_common_friends_resp {JSON数据}
	if !strings.HasPrefix(resp, "query_common_friends_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "query_common_friends_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	log.Printf("Parsed query_common_friends result: %+v", result)
	return result, nil
}

// QueryCrossServiceFriends 查询跨服务好友推荐
func (m *NetworkManager) QueryCrossServiceFriends(userID, currentServiceID, targetServiceID int) (map[string]interface{}, error) {
	// 构建查询命令
	queryData := map[string]interface{}{
		"user_id":            userID,
		"current_service_id": currentServiceID,
		"target_service_id":  targetServiceID,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询数据失败: %w", err)
	}

	cmd := fmt.Sprintf("query_cross_service_friends %s", string(jsonData))

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: query_cross_service_friends_resp {JSON数据}
	if !strings.HasPrefix(resp, "query_cross_service_friends_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "query_cross_service_friends_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	log.Printf("Parsed query_cross_service_friends result: %+v", result)
	return result, nil
}

// UpdateProfile 更新用户资料
func (m *NetworkManager) UpdateProfile(userID int, profileData map[string]interface{}) (map[string]interface{}, error) {
	// 构建更新资料命令
	updateData := map[string]interface{}{
		"user_id": userID,
	}

	// 合并资料数据
	for key, value := range profileData {
		updateData[key] = value
	}

	jsonData, err := json.Marshal(updateData)
	if err != nil {
		return nil, fmt.Errorf("序列化更新资料数据失败: %w", err)
	}

	cmd := fmt.Sprintf("update_profile %s", string(jsonData))
	log.Printf("发送更新资料命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: update_profile_resp {JSON数据}
	if !strings.HasPrefix(resp, "update_profile_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "update_profile_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// BindWechat 绑定微信账号
func (m *NetworkManager) BindWechat(userID int, wechatID string) (map[string]interface{}, error) {
	// 构建绑定微信命令
	bindData := map[string]interface{}{
		"user_id":   userID,
		"wechat_id": wechatID,
	}

	jsonData, err := json.Marshal(bindData)
	if err != nil {
		return nil, fmt.Errorf("序列化绑定微信数据失败: %w", err)
	}

	cmd := fmt.Sprintf("bind_wechat %s", string(jsonData))
	log.Printf("发送绑定微信命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: bind_wechat_resp {JSON数据}
	if !strings.HasPrefix(resp, "bind_wechat_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "bind_wechat_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// GetProfile 获取用户资料
func (m *NetworkManager) GetProfile(userID int) (map[string]interface{}, error) {
	// 构建获取资料命令
	profileData := map[string]interface{}{
		"user_id": userID,
	}

	jsonData, err := json.Marshal(profileData)
	if err != nil {
		return nil, fmt.Errorf("序列化获取资料数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_profile %s", string(jsonData))
	log.Printf("发送获取资料命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: get_profile_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_profile_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_profile_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// ==================== 第六阶段增强群组功能网络方法 ====================

// GetGroupSettings 获取群组设置
func (m *NetworkManager) GetGroupSettings(groupID, serviceID int) (map[string]interface{}, error) {
	// 构建获取群组设置命令
	settingsData := map[string]interface{}{
		"group_id":   groupID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(settingsData)
	if err != nil {
		return nil, fmt.Errorf("序列化获取群组设置数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_group_settings %s", string(jsonData))
	log.Printf("发送获取群组设置命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: get_group_settings_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_group_settings_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_group_settings_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// UpdateGroupSettings 更新群组设置
func (m *NetworkManager) UpdateGroupSettings(groupID, serviceID int, settings map[string]interface{}) (map[string]interface{}, error) {
	// 构建更新群组设置命令
	updateData := map[string]interface{}{
		"group_id":   groupID,
		"service_id": serviceID,
		"settings":   settings,
	}

	jsonData, err := json.Marshal(updateData)
	if err != nil {
		return nil, fmt.Errorf("序列化更新群组设置数据失败: %w", err)
	}

	cmd := fmt.Sprintf("update_group_settings %s", string(jsonData))
	log.Printf("发送更新群组设置命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: update_group_settings_resp {JSON数据}
	if !strings.HasPrefix(resp, "update_group_settings_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "update_group_settings_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// ChangeGroupType 变换群组类型
func (m *NetworkManager) ChangeGroupType(groupID, sourceServiceID, targetServiceID int) (map[string]interface{}, error) {
	// 构建变换群组类型命令
	changeData := map[string]interface{}{
		"group_id":          groupID,
		"service_id":        sourceServiceID, // 修复：使用service_id而不是source_service_id
		"target_service_id": targetServiceID,
	}

	jsonData, err := json.Marshal(changeData)
	if err != nil {
		return nil, fmt.Errorf("序列化变换群组类型数据失败: %w", err)
	}

	cmd := fmt.Sprintf("change_group_type %s", string(jsonData))
	log.Printf("发送变换群组类型命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: change_group_type_resp {JSON数据}
	if !strings.HasPrefix(resp, "change_group_type_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "change_group_type_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// SetMemberRole 设置成员角色
func (m *NetworkManager) SetMemberRole(groupID, userID, serviceID, roleType int) (map[string]interface{}, error) {
	// 构建设置成员角色命令
	roleData := map[string]interface{}{
		"group_id":   groupID,
		"user_id":    userID,
		"service_id": serviceID,
		"role_type":  roleType,
	}

	jsonData, err := json.Marshal(roleData)
	if err != nil {
		return nil, fmt.Errorf("序列化设置成员角色数据失败: %w", err)
	}

	cmd := fmt.Sprintf("set_member_role %s", string(jsonData))
	log.Printf("发送设置成员角色命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: set_member_role_resp {JSON数据}
	if !strings.HasPrefix(resp, "set_member_role_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "set_member_role_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// GetMemberRoles 获取成员角色列表
func (m *NetworkManager) GetMemberRoles(groupID, serviceID int) (map[string]interface{}, error) {
	// 构建获取成员角色列表命令
	rolesData := map[string]interface{}{
		"group_id":   groupID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(rolesData)
	if err != nil {
		return nil, fmt.Errorf("序列化获取成员角色列表数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_member_roles %s", string(jsonData))
	log.Printf("发送获取成员角色列表命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: get_member_roles_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_member_roles_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_member_roles_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// GetCurrentUserRole 获取当前用户在群组中的角色 - 修复版本
func (m *NetworkManager) GetCurrentUserRole(groupID, userID, serviceID int) (map[string]interface{}, error) {
	// 构建获取成员角色命令
	roleData := map[string]interface{}{
		"group_id":   groupID,
		"user_id":    userID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(roleData)
	if err != nil {
		return nil, fmt.Errorf("序列化获取成员角色数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_current_user_role %s", string(jsonData))
	log.Printf("发送获取当前用户角色命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: get_current_user_role_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_current_user_role_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_current_user_role_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	// 检查响应状态码
	if code, ok := result["code"].(float64); ok && code != 0 {
		message := "未知错误"
		if msg, ok := result["message"].(string); ok {
			message = msg
		}
		return nil, fmt.Errorf("获取成员角色失败: %s", message)
	}

	// 直接使用服务器返回的角色信息，不再尝试从成员列表查找
	log.Printf("成功获取用户角色响应: %+v", result)

	// 返回原始响应，让调用方处理
	return result, nil
}

// ApplyJoinGroup QQ群申请加入
func (m *NetworkManager) ApplyJoinGroup(groupID, userID, serviceID int, reason string) (map[string]interface{}, error) {
	// 构建申请加入群组命令
	applyData := map[string]interface{}{
		"group_id":   groupID,
		"user_id":    userID,
		"service_id": serviceID,
		"reason":     reason,
	}

	jsonData, err := json.Marshal(applyData)
	if err != nil {
		return nil, fmt.Errorf("序列化申请加入群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("apply_join_group %s", string(jsonData))
	log.Printf("发送申请加入群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: apply_join_group_resp {JSON数据}
	if !strings.HasPrefix(resp, "apply_join_group_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "apply_join_group_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// InviteJoinGroup 微信群邀请加入
func (m *NetworkManager) InviteJoinGroup(groupID, inviterID, inviteeID, serviceID int) (map[string]interface{}, error) {
	// 构建邀请加入群组命令
	inviteData := map[string]interface{}{
		"group_id":   groupID,
		"inviter_id": inviterID,
		"invitee_id": inviteeID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(inviteData)
	if err != nil {
		return nil, fmt.Errorf("序列化邀请加入群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("invite_join_group %s", string(jsonData))
	log.Printf("发送邀请加入群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: invite_join_group_resp {JSON数据}
	if !strings.HasPrefix(resp, "invite_join_group_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "invite_join_group_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// FreeJoinTopic 微博超话自由加入
func (m *NetworkManager) FreeJoinTopic(groupID, userID, serviceID int) (map[string]interface{}, error) {
	// 构建自由加入超话命令
	joinData := map[string]interface{}{
		"group_id":   groupID,
		"user_id":    userID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(joinData)
	if err != nil {
		return nil, fmt.Errorf("序列化自由加入超话数据失败: %w", err)
	}

	cmd := fmt.Sprintf("free_join_topic %s", string(jsonData))
	log.Printf("发送自由加入超话命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: free_join_topic_resp {JSON数据}
	if !strings.HasPrefix(resp, "free_join_topic_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "free_join_topic_resp ")
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
	log.Printf("Parsed add_friend result: %+v", result)
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
// SendMessage 发送消息
func (m *NetworkManager) SendMessage(senderID, receiverID, serviceID int, content string) (map[string]interface{}, error) {
	// 构建发送消息命令
	messageData := map[string]interface{}{
		"sender_id":   senderID,
		"receiver_id": receiverID,
		"type":        0, // 私聊类型
		"content":     content,
		"service_id":  serviceID,
	}

	jsonData, err := json.Marshal(messageData)
	if err != nil {
		log.Printf("SendMessage: 序列化消息数据失败 - senderID: %d, receiverID: %d, serviceID: %d, content: %s, error: %v",
			senderID, receiverID, serviceID, content, err)
		return nil, fmt.Errorf("序列化消息数据失败: %w", err)
	}

	cmd := fmt.Sprintf("send_message %s", string(jsonData))
	log.Printf("SendMessage: 发送命令 - %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		log.Printf("SendMessage: 发送命令失败 - senderID: %d, receiverID: %d, serviceID: %d, error: %v",
			senderID, receiverID, serviceID, err)
		return nil, fmt.Errorf("发送消息命令失败: %w", err)
	}

	// 解析响应
	// 响应格式: send_message_resp {JSON数据}
	if !strings.HasPrefix(resp, "send_message_resp ") {
		log.Printf("SendMessage: 无效的响应格式 - senderID: %d, receiverID: %d, serviceID: %d, response: %s",
			senderID, receiverID, serviceID, resp)
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "send_message_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		log.Printf("SendMessage: 解析响应JSON失败 - senderID: %d, receiverID: %d, serviceID: %d, jsonStr: %s, error: %v",
			senderID, receiverID, serviceID, jsonStr, err)
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	log.Printf("SendMessage: 成功 - senderID: %d, receiverID: %d, serviceID: %d, result: %v",
		senderID, receiverID, serviceID, result)
	return result, nil
}

// GetMessageHistory 获取消息历史记录
// This is the corrected version that accepts page and pageSize.
func (m *NetworkManager) GetMessageHistory(userID, targetID, serviceID, page, pageSize int) (map[string]interface{}, error) {
	// 构建获取历史消息命令
	historyData := map[string]interface{}{
		"user_id":    userID,
		"target_id":  targetID,
		"type":       0, // 私聊类型
		"service_id": serviceID,
		"page":       page,     // Use the 'page' parameter
		"page_size":  pageSize, // Use the 'pageSize' parameter
	}

	jsonData, err := json.Marshal(historyData)
	if err != nil {
		log.Printf("GetMessageHistory: 序列化历史消息数据失败 - userID: %d, targetID: %d, serviceID: %d, error: %v",
			userID, targetID, serviceID, err)
		return nil, fmt.Errorf("序列化历史消息数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_history %s", string(jsonData))
	log.Printf("GetMessageHistory: 发送命令 - %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		log.Printf("GetMessageHistory: 发送命令失败 - userID: %d, targetID: %d, serviceID: %d, error: %v",
			userID, targetID, serviceID, err)
		return nil, fmt.Errorf("获取历史消息命令失败: %w", err)
	}

	// 解析响应
	// 响应格式: get_history_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_history_resp ") {
		log.Printf("GetMessageHistory: 无效的响应格式 - userID: %d, targetID: %d, serviceID: %d, response: %s",
			userID, targetID, serviceID, resp)
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_history_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		log.Printf("GetMessageHistory: 解析响应JSON失败 - userID: %d, targetID: %d, serviceID: %d, jsonStr: %s, error: %v",
			userID, targetID, serviceID, jsonStr, err)
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	log.Printf("GetMessageHistory: 成功 - userID: %d, targetID: %d, serviceID: %d, result: %v",
		userID, targetID, serviceID, result)
	return result, nil
}

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

// JoinGroup 加入群组
func (m *NetworkManager) JoinGroup(userID, groupID, serviceID, joinType int) (map[string]interface{}, error) {
	// 构建加入群组命令
	joinData := map[string]interface{}{
		"user_id":    userID,
		"group_id":   groupID,
		"service_id": serviceID,
		"join_type":  joinType,
	}

	jsonData, err := json.Marshal(joinData)
	if err != nil {
		return nil, fmt.Errorf("序列化加入群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("join_group %s", string(jsonData))
	log.Printf("发送加入群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: join_group_resp {JSON数据}
	if !strings.HasPrefix(resp, "join_group_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "join_group_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// QuitGroup 退出群组
func (m *NetworkManager) QuitGroup(userID, groupID, serviceID int) (map[string]interface{}, error) {
	// 构建退出群组命令
	quitData := map[string]interface{}{
		"user_id":    userID,
		"group_id":   groupID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(quitData)
	if err != nil {
		return nil, fmt.Errorf("序列化退出群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("quit_group %s", string(jsonData))
	log.Printf("发送退出群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: quit_group_resp {JSON数据}
	if !strings.HasPrefix(resp, "quit_group_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "quit_group_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// CreateGroup 创建群组
func (m *NetworkManager) CreateGroup(creatorID int, groupName string, serviceID int, description string) (map[string]interface{}, error) {
	// 构建创建群组命令
	createData := map[string]interface{}{
		"creator_id":  creatorID,
		"group_name":  groupName,
		"service_id":  serviceID,
		"description": description,
	}

	jsonData, err := json.Marshal(createData)
	if err != nil {
		return nil, fmt.Errorf("序列化创建群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("create_group %s", string(jsonData))
	log.Printf("发送创建群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: create_group_resp {JSON数据}
	if !strings.HasPrefix(resp, "create_group_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "create_group_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// GetGroupMembers 获取群成员
func (m *NetworkManager) GetGroupMembers(groupID, serviceID int) (map[string]interface{}, error) {
	// 构建查询群成员命令
	queryData := map[string]interface{}{
		"group_id":   groupID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询群成员数据失败: %w", err)
	}

	cmd := fmt.Sprintf("query_group_members %s", string(jsonData))
	log.Printf("发送查询群成员命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: query_group_members_resp {JSON数据}
	if !strings.HasPrefix(resp, "query_group_members_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "query_group_members_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}

// GetUserGroups 获取用户群组列表
func (m *NetworkManager) GetUserGroups(userID, serviceID int) (map[string]interface{}, error) {
	// 构建查询用户群组命令
	queryData := map[string]interface{}{
		"user_id":    userID,
		"service_id": serviceID,
	}

	jsonData, err := json.Marshal(queryData)
	if err != nil {
		return nil, fmt.Errorf("序列化查询用户群组数据失败: %w", err)
	}

	cmd := fmt.Sprintf("get_user_groups %s", string(jsonData))
	log.Printf("发送查询用户群组命令: %s", cmd)

	// 发送命令并获取响应
	resp, err := m.sendCommand(cmd)
	if err != nil {
		return nil, err
	}

	// 解析响应
	// 响应格式: get_user_groups_resp {JSON数据}
	if !strings.HasPrefix(resp, "get_user_groups_resp ") {
		return nil, fmt.Errorf("无效的响应格式: %s", resp)
	}

	jsonStr := strings.TrimPrefix(resp, "get_user_groups_resp ")
	var result map[string]interface{}
	if err := json.Unmarshal([]byte(jsonStr), &result); err != nil {
		return nil, fmt.Errorf("解析响应JSON失败: %w", err)
	}

	return result, nil
}
