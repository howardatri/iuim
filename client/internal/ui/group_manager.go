package ui

import (
	"fmt"
	//"log"
	"strconv"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// GroupManager 群组管理器
type GroupManager struct {
	window     fyne.Window
	netManager *network.NetworkManager
	userID     int
	serviceID  int
	container  *fyne.Container
}

// NewGroupManager 创建群组管理器
func NewGroupManager(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) *GroupManager {
	gm := &GroupManager{
		window:     window,
		netManager: netManager,
		userID:     userID,
		serviceID:  serviceID,
	}

	gm.setupUI()
	return gm
}

// setupUI 设置用户界面
func (gm *GroupManager) setupUI() {
	// 创建群组操作按钮
	createGroupBtn := widget.NewButton("创建群组", func() {
		gm.ShowCreateGroupDialog()
	})

	joinGroupBtn := widget.NewButton("加入群组", func() {
		gm.ShowJoinGroupDialog()
	})

	// 根据服务类型显示不同的按钮
	var serviceSpecificBtns []fyne.CanvasObject
	switch gm.serviceID {
	case 1: // QQ服务
		qqJoinBtn := widget.NewButton("申请加入QQ群", func() {
			gm.ShowJoinGroupDialog()
		})
		serviceSpecificBtns = append(serviceSpecificBtns, qqJoinBtn)
	case 2: // 微信服务
		wechatInviteBtn := widget.NewButton("邀请加入微信群", func() {
			gm.ShowInviteToWeChatGroupDialog()
		})
		serviceSpecificBtns = append(serviceSpecificBtns, wechatInviteBtn)
	case 3: // 微博服务
		weiboJoinBtn := widget.NewButton("申请加入微博社区", func() {
			gm.ShowJoinGroupDialog()
		})
		serviceSpecificBtns = append(serviceSpecificBtns, weiboJoinBtn)
	}

	// 组合所有按钮
	allBtns := []fyne.CanvasObject{createGroupBtn, joinGroupBtn}
	allBtns = append(allBtns, serviceSpecificBtns...)

	gm.container = container.NewVBox(allBtns...)
}

// GetContainer 获取容器
func (gm *GroupManager) GetContainer() fyne.CanvasObject {
	return gm.container
}

// 基础群组操作（封装网络层）

// JoinGroup 加入群组
func (gm *GroupManager) JoinGroup(groupID, joinType int) error {
	result, err := gm.netManager.JoinGroup(gm.userID, groupID, gm.serviceID, joinType)
	if err != nil {
		return fmt.Errorf("网络请求失败: %w", err)
	}

	// 检查响应状态
	if code, ok := result["code"].(float64); ok && code != 0 {
		if message, ok := result["message"].(string); ok {
			return fmt.Errorf("加入群组失败: %s", message)
		}
		return fmt.Errorf("加入群组失败，错误代码: %.0f", code)
	}

	return nil
}

// QuitGroup 退出群组
func (gm *GroupManager) QuitGroup(groupID int) error {
	result, err := gm.netManager.QuitGroup(gm.userID, groupID, gm.serviceID)
	if err != nil {
		return fmt.Errorf("网络请求失败: %w", err)
	}

	// 检查响应状态
	if code, ok := result["code"].(float64); ok && code != 0 {
		if message, ok := result["message"].(string); ok {
			return fmt.Errorf("退出群组失败: %s", message)
		}
		return fmt.Errorf("退出群组失败，错误代码: %.0f", code)
	}

	return nil
}

// CreateGroup 创建群组
func (gm *GroupManager) CreateGroup(groupName, description string) error {
	// 对于微信服务，使用群组名称的哈希值作为群组ID
	var groupID int
	if gm.serviceID == 2 { // 微信服务
		groupID = gm.hashGroupName(groupName)
		fmt.Printf("微信群组创建: 群组名称=%s, 生成的群组ID=%d\n", groupName, groupID)
	} else {
		// 对于其他服务，使用默认的群组ID生成逻辑
		groupID = 0 // 让服务器自动分配
	}

	result, err := gm.netManager.CreateGroup(gm.userID, groupName, gm.serviceID, description)
	if err != nil {
		return fmt.Errorf("网络请求失败: %w", err)
	}

	// 检查响应状态
	if code, ok := result["code"].(float64); ok && code != 0 {
		if message, ok := result["message"].(string); ok {
			return fmt.Errorf("创建群组失败: %s", message)
		}
		return fmt.Errorf("创建群组失败，错误代码: %.0f", code)
	}

	return nil
}

// 差异化群组操作

// ApplyJoinQQGroup QQ群申请加入
func (gm *GroupManager) ApplyJoinQQGroup(groupID int) error {
	return gm.JoinGroup(groupID, 0) // QQ群使用申请加入方式
}

// InviteToWeChatGroup 微信群邀请加入
func (gm *GroupManager) InviteToWeChatGroup(inviteeID, groupID int) error {
	// 这里简化处理，实际应该是邀请其他用户
	return gm.JoinGroup(groupID, 1) // 微信群使用推荐加入方式
}

// JoinWeiboCommunity 微博社区申请加入
func (gm *GroupManager) JoinWeiboCommunity(groupID int) error {
	return gm.JoinGroup(groupID, 0) // 微博社区使用申请加入方式
}

// 界面组件

// ShowJoinGroupDialog 显示加入群组对话框
func (gm *GroupManager) ShowJoinGroupDialog() {
	var groupEntry *widget.Entry
	var labelText string
	var placeholderText string
	var dialogTitle string

	// 根据服务类型设置不同的输入提示
	switch gm.serviceID {
	case 2: // 微信服务
		labelText = "微信群名称"
		placeholderText = "请输入微信群名称"
		dialogTitle = "加入微信群"
	case 1: // QQ服务
		labelText = "QQ群号"
		placeholderText = "请输入QQ群号"
		dialogTitle = "加入QQ群"
	case 3: // 微博服务
		labelText = "微博社区ID"
		placeholderText = "请输入微博社区ID"
		dialogTitle = "加入微博社区"
	default:
		labelText = "群组ID"
		placeholderText = "请输入群组ID"
		dialogTitle = "加入群组"
	}

	groupEntry = widget.NewEntry()
	groupEntry.SetPlaceHolder(placeholderText)

	form := &widget.Form{
		Items: []*widget.FormItem{
			{Text: labelText, Widget: groupEntry},
		},
		OnSubmit: func() {
			groupInput := groupEntry.Text
			if groupInput == "" {
				dialog.ShowError(fmt.Errorf("请输入%s", labelText), gm.window)
				return
			}

			var groupID int
			var err error

			// 根据服务类型处理输入
			if gm.serviceID == 2 { // 微信服务，使用群名称哈希
				groupID = gm.hashGroupName(groupInput)
			} else { // 其他服务，直接解析为数字
				groupID, err = strconv.Atoi(groupInput)
				if err != nil {
					dialog.ShowError(fmt.Errorf("%s必须是数字", labelText), gm.window)
					return
				}
			}

			// 根据服务类型选择加入方式
			var joinType int
			switch gm.serviceID {
			case 1: // QQ服务 - 申请加入
				joinType = 0
			case 2: // 微信服务 - 推荐加入
				joinType = 1
			case 3: // 微博服务 - 申请加入
				joinType = 0
			default:
				joinType = 0
			}

			err = gm.JoinGroup(groupID, joinType)
			if err != nil {
				dialog.ShowError(err, gm.window)
				return
			}

			var successMessage string
			if gm.serviceID == 2 {
				successMessage = fmt.Sprintf("成功加入微信群: %s", groupInput)
			} else {
				successMessage = "加入群组成功！"
			}
			dialog.ShowInformation("成功", successMessage, gm.window)
		},
	}

	dialog.ShowForm(dialogTitle, "确定", "取消", form.Items, func(confirmed bool) {
		if confirmed {
			form.OnSubmit()
		}
	}, gm.window)
}

// ShowCreateGroupDialog 显示创建群组对话框
func (gm *GroupManager) ShowCreateGroupDialog() {
	groupNameEntry := widget.NewEntry()
	groupNameEntry.SetPlaceHolder("请输入群组名称")

	descriptionEntry := widget.NewMultiLineEntry()
	descriptionEntry.SetPlaceHolder("请输入群组描述（可选）")
	descriptionEntry.Resize(fyne.NewSize(300, 100))

	form := &widget.Form{
		Items: []*widget.FormItem{
			{Text: "群组名称", Widget: groupNameEntry},
			{Text: "群组描述", Widget: descriptionEntry},
		},
		OnSubmit: func() {
			groupName := groupNameEntry.Text
			if groupName == "" {
				dialog.ShowError(fmt.Errorf("请输入群组名称"), gm.window)
				return
			}

			description := descriptionEntry.Text

			err := gm.CreateGroup(groupName, description)
			if err != nil {
				dialog.ShowError(err, gm.window)
				return
			}

			dialog.ShowInformation("成功", "创建群组成功！", gm.window)
		},
	}

	dialog.ShowForm("创建群组", "确定", "取消", form.Items, func(confirmed bool) {
		if confirmed {
			form.OnSubmit()
		}
	}, gm.window)
}

// ShowInviteToWeChatGroupDialog 显示微信群邀请对话框
func (gm *GroupManager) ShowInviteToWeChatGroupDialog() {
	groupNameEntry := widget.NewEntry()
	groupNameEntry.SetPlaceHolder("请输入微信群名称")

	usernameEntry := widget.NewEntry()
	usernameEntry.SetPlaceHolder("请输入被邀请用户的用户名")

	form := &widget.Form{
		Items: []*widget.FormItem{
			{Text: "微信群名称", Widget: groupNameEntry},
			{Text: "用户名", Widget: usernameEntry},
		},
		OnSubmit: func() {
			groupName := groupNameEntry.Text
			username := usernameEntry.Text

			if groupName == "" || username == "" {
				dialog.ShowError(fmt.Errorf("请填写所有字段"), gm.window)
				return
			}

			// 首先搜索用户获取用户ID
			searchResult, err := gm.netManager.SearchUsers(username)
			if err != nil {
				dialog.ShowError(fmt.Errorf("搜索用户失败: %v", err), gm.window)
				return
			}

			// 检查搜索结果
			if code, ok := searchResult["code"].(float64); !ok || code != 0 {
				message := "搜索用户失败"
				if msg, ok := searchResult["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf(message), gm.window)
				return
			}

			// 解析用户数据
			var inviteeID int
			if data, ok := searchResult["data"].([]interface{}); ok && len(data) > 0 {
				if user, ok := data[0].(map[string]interface{}); ok {
					if userID, ok := user["user_id"].(float64); ok {
						inviteeID = int(userID)
					} else {
						dialog.ShowError(fmt.Errorf("无法获取用户ID"), gm.window)
						return
					}
				} else {
					dialog.ShowError(fmt.Errorf("用户数据格式错误"), gm.window)
					return
				}
			} else {
				dialog.ShowError(fmt.Errorf("未找到用户: %s", username), gm.window)
				return
			}

			// 对于微信群，我们使用群名称的哈希值作为临时群ID
			// 实际应用中应该有专门的群名称到群ID的映射
			groupID := gm.hashGroupName(groupName)

			err = gm.InviteToWeChatGroup(inviteeID, groupID)
			if err != nil {
				dialog.ShowError(err, gm.window)
				return
			}

			dialog.ShowInformation("成功", fmt.Sprintf("已邀请用户 %s 加入微信群 %s！", username, groupName), gm.window)
		},
	}

	dialog.ShowForm("邀请加入微信群", "确定", "取消", form.Items, func(confirmed bool) {
		if confirmed {
			form.OnSubmit()
		}
	}, gm.window)
}

// hashGroupName 将群名称转换为群ID（简单哈希实现）
func (gm *GroupManager) hashGroupName(groupName string) int {
	hash := 0
	for _, char := range groupName {
		hash = hash*31 + int(char)
	}
	if hash < 0 {
		hash = -hash
	}
	return hash % 1000000 // 限制在6位数以内
}
