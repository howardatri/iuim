package ui

import (
	"fmt"
	"log"
	"strconv"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/layout"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// GroupList 群组列表组件
type GroupList struct {
	container  *fyne.Container
	netManager *network.NetworkManager
	userID     int
	serviceID  int
	groups     []map[string]interface{}
	// 必须包含列表控件和刷新回调机制
	groupsList *widget.List
	onRefresh  func()
	window     fyne.Window
}

// NewGroupList 创建群组列表
func NewGroupList(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) *GroupList {
	gl := &GroupList{
		window:     window,
		netManager: netManager,
		userID:     userID,
		serviceID:  serviceID,
		groups:     []map[string]interface{}{},
	}

	// 创建群组列表 - 使用widget.List控件
	gl.groupsList = widget.NewList(
		func() int {
			return len(gl.groups)
		},
		func() fyne.CanvasObject {
			// 根据服务类型创建不同的模板
			switch gl.serviceID {
			case 1: // QQ服务
				return gl.createQQGroupTemplate()
			case 2: // 微信服务
				return gl.createWeChatGroupTemplate()
			case 3: // 微博服务
				return gl.createWeiboGroupTemplate()
			default:
				return gl.createDefaultGroupTemplate()
			}
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(gl.groups) {
				return
			}

			group := gl.groups[id]
			// 根据服务类型渲染不同的群组项
			switch gl.serviceID {
			case 1: // QQ服务
				gl.renderQQGroupItem(group, obj)
			case 2: // 微信服务
				gl.renderWeChatGroupItem(group, obj)
			case 3: // 微博服务
				gl.renderWeiboGroupItem(group, obj)
			default:
				gl.renderDefaultGroupItem(group, obj)
			}
		},
	)

	// 点击群组处理 - 修改为打开智能详情窗口
	gl.groupsList.OnSelected = func(id widget.ListItemID) {
		if id >= len(gl.groups) {
			return
		}
		group := gl.groups[id]

		// 调用新的点击处理方法
		gl.onGroupItemClick(group)

		// 取消列表的选中状态，避免一直高亮
		gl.groupsList.Unselect(id)
	}

	// 创建刷新按钮
	refreshButton := widget.NewButton("刷新", func() {
		gl.RefreshGroups()
	})

	// 创建按钮栏
	buttonBar := container.NewHBox(refreshButton)

	// 创建主容器
	gl.container = container.NewBorder(
		buttonBar,     // top
		nil,           // bottom
		nil,           // left
		nil,           // right
		gl.groupsList, // center
	)

	// 初始加载群组列表
	gl.RefreshGroups()

	return gl
}

// GetContainer 获取容器
func (gl *GroupList) GetContainer() fyne.CanvasObject {
	return gl.container
}

// RefreshGroups 刷新群组列表 - 完全复制好友列表的refreshFriends()方法架构
func (gl *GroupList) RefreshGroups() {
	log.Printf("Starting RefreshGroups - userID: %d, serviceID: %d", gl.userID, gl.serviceID)
	go func() {
		result, err := gl.netManager.GetUserGroups(gl.userID, gl.serviceID)
		fyne.Do(func() {
			if err != nil {
				log.Printf("Refresh groups failed: %v", err)
				dialog.ShowError(fmt.Errorf("获取群组列表失败: %v", err), gl.window)
				return
			}

			// 添加调试日志
			log.Printf("GetUserGroups response in refresh: %+v", result)

			// 检查响应状态码
			code, ok := result["code"].(float64)
			if !ok || code != 0 {
				message := "未知错误"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf("获取群组列表失败: %s", message), gl.window)
				return
			}

			// 解析群组列表 - 关键修复，双重数据格式解析
			var groups []map[string]interface{}

			// 首先尝试直接解析 data 为数组
			if groupsData, ok := result["data"].([]interface{}); ok {
				log.Printf("Data is directly an array with %d items", len(groupsData))
				for i, groupData := range groupsData {
					if groupMap, ok := groupData.(map[string]interface{}); ok {
						groups = append(groups, groupMap)
						log.Printf("Group %d: %+v", i, groupMap)
					} else {
						log.Printf("Group data %d is not a map: %T", i, groupData)
					}
				}
			} else if data, ok := result["data"].(map[string]interface{}); ok {
				// 如果 data 是对象，尝试从 groups 字段获取
				log.Printf("Data is a map with keys: %v", getMapKeys(data))
				if groupsData, ok := data["groups"].([]interface{}); ok {
					log.Printf("Found groups array with %d items", len(groupsData))
					for i, groupData := range groupsData {
						if groupMap, ok := groupData.(map[string]interface{}); ok {
							groups = append(groups, groupMap)
							log.Printf("Group %d: %+v", i, groupMap)
						}
					}
				} else {
					log.Printf("No groups array found in data map")
				}
			} else {
				log.Printf("Data field is of unexpected type: %T", result["data"])
			}

			log.Printf("Updating groups list from %d to %d groups", len(gl.groups), len(groups))
			gl.groups = groups
			gl.groupsList.Refresh()
			log.Printf("Groups list refreshed, now has %d groups", len(gl.groups))
			if gl.onRefresh != nil {
				log.Printf("Calling external onRefresh callback")
				gl.onRefresh()
			}
		})
	}()
}

// SetOnRefresh 设置刷新回调方法
func (gl *GroupList) SetOnRefresh(callback func()) {
	gl.onRefresh = callback
}

// 差异化显示模板创建

// createQQGroupTemplate 创建QQ群组模板
func (gl *GroupList) createQQGroupTemplate() fyne.CanvasObject {
	avatar := widget.NewLabel("👥")
	avatar.TextStyle = fyne.TextStyle{Bold: true}

	nameLabel := widget.NewLabel("群组名称")
	nameLabel.TextStyle = fyne.TextStyle{Bold: true}

	groupIDLabel := widget.NewLabel("群号")
	groupIDLabel.Resize(fyne.NewSize(80, 20))

	adminLabel := widget.NewLabel("管理员")
	adminLabel.Resize(fyne.NewSize(80, 20))

	statusLabel := widget.NewLabel("状态")
	statusLabel.Resize(fyne.NewSize(60, 20))

	return container.NewHBox(
		avatar,
		container.NewVBox(nameLabel, groupIDLabel),
		layout.NewSpacer(),
		container.NewVBox(adminLabel, statusLabel),
	)
}

// createWeChatGroupTemplate 创建微信群组模板
func (gl *GroupList) createWeChatGroupTemplate() fyne.CanvasObject {
	avatar := widget.NewLabel("👥")
	avatar.TextStyle = fyne.TextStyle{Bold: true}

	nameLabel := widget.NewLabel("群组名称")
	nameLabel.TextStyle = fyne.TextStyle{Bold: true}

	membersLabel := widget.NewLabel("成员数量")
	membersLabel.Resize(fyne.NewSize(80, 20))

	descLabel := widget.NewLabel("群描述")
	descLabel.Resize(fyne.NewSize(100, 20))

	return container.NewHBox(
		avatar,
		container.NewVBox(nameLabel, membersLabel),
		layout.NewSpacer(),
		descLabel,
	)
}

// createWeiboGroupTemplate 创建微博群组模板
func (gl *GroupList) createWeiboGroupTemplate() fyne.CanvasObject {
	avatar := widget.NewLabel("🌐")
	avatar.TextStyle = fyne.TextStyle{Bold: true}

	nameLabel := widget.NewLabel("社区名称")
	nameLabel.TextStyle = fyne.TextStyle{Bold: true}

	typeLabel := widget.NewLabel("社区类型")
	typeLabel.Resize(fyne.NewSize(80, 20))

	return container.NewHBox(
		avatar,
		container.NewVBox(nameLabel, typeLabel),
		layout.NewSpacer(),
	)
}

// createDefaultGroupTemplate 创建默认群组模板
func (gl *GroupList) createDefaultGroupTemplate() fyne.CanvasObject {
	avatar := widget.NewLabel("👥")
	avatar.TextStyle = fyne.TextStyle{Bold: true}

	nameLabel := widget.NewLabel("群组名称")
	nameLabel.TextStyle = fyne.TextStyle{Bold: true}

	idLabel := widget.NewLabel("群组ID")
	idLabel.Resize(fyne.NewSize(80, 20))

	return container.NewHBox(
		avatar,
		container.NewVBox(nameLabel, idLabel),
		layout.NewSpacer(),
	)
}

// 差异化显示渲染

// renderQQGroupItem 渲染QQ群组项
func (gl *GroupList) renderQQGroupItem(group map[string]interface{}, obj fyne.CanvasObject) {
	hbox := obj.(*fyne.Container)

	// 更新头像
	avatar := hbox.Objects[0].(*widget.Label)
	avatar.SetText("👥")

	// 更新名称和群号
	vbox := hbox.Objects[1].(*fyne.Container)
	nameLabel := vbox.Objects[0].(*widget.Label)
	groupIDLabel := vbox.Objects[1].(*widget.Label)

	if name, ok := group["group_name"].(string); ok {
		nameLabel.SetText(name)
	} else {
		nameLabel.SetText("未知群组")
	}

	if id, ok := group["group_id"].(float64); ok {
		groupIDLabel.SetText(fmt.Sprintf("群号: %.0f", id))
	} else {
		groupIDLabel.SetText("群号: 未知")
	}

	// 更新管理员和状态
	rightVbox := hbox.Objects[3].(*fyne.Container)
	adminLabel := rightVbox.Objects[0].(*widget.Label)
	statusLabel := rightVbox.Objects[1].(*widget.Label)

	if creator, ok := group["creator_id"].(float64); ok {
		adminLabel.SetText(fmt.Sprintf("管理员: %.0f", creator))
	} else {
		adminLabel.SetText("管理员: 未知")
	}

	statusLabel.SetText("申请状态: 已加入")
}

// renderWeChatGroupItem 渲染微信群组项
func (gl *GroupList) renderWeChatGroupItem(group map[string]interface{}, obj fyne.CanvasObject) {
	hbox := obj.(*fyne.Container)

	// 更新头像
	avatar := hbox.Objects[0].(*widget.Label)
	avatar.SetText("👥")

	// 更新名称和成员数量
	vbox := hbox.Objects[1].(*fyne.Container)
	nameLabel := vbox.Objects[0].(*widget.Label)
	membersLabel := vbox.Objects[1].(*widget.Label)

	if name, ok := group["group_name"].(string); ok {
		nameLabel.SetText(name)
	} else {
		nameLabel.SetText("未知群组")
	}

	// 显示实际的成员数量
	if memberCount, ok := group["member_count"].(float64); ok {
		membersLabel.SetText(fmt.Sprintf("成员: %d人", int(memberCount)))
	} else {
		membersLabel.SetText("成员: 查看详情")
	}

	// 更新描述
	descLabel := hbox.Objects[3].(*widget.Label)
	if desc, ok := group["description"].(string); ok && desc != "" {
		descLabel.SetText(desc)
	} else {
		descLabel.SetText("无描述")
	}
}

// renderWeiboGroupItem 渲染微博群组项
func (gl *GroupList) renderWeiboGroupItem(group map[string]interface{}, obj fyne.CanvasObject) {
	hbox := obj.(*fyne.Container)

	// 更新头像
	avatar := hbox.Objects[0].(*widget.Label)
	avatar.SetText("🌐")

	// 更新名称和类型
	vbox := hbox.Objects[1].(*fyne.Container)
	nameLabel := vbox.Objects[0].(*widget.Label)
	typeLabel := vbox.Objects[1].(*widget.Label)

	if name, ok := group["group_name"].(string); ok {
		nameLabel.SetText(name)
	} else {
		nameLabel.SetText("未知社区")
	}

	typeLabel.SetText("类型: 微博社区")
}

// renderDefaultGroupItem 渲染默认群组项
func (gl *GroupList) renderDefaultGroupItem(group map[string]interface{}, obj fyne.CanvasObject) {
	hbox := obj.(*fyne.Container)

	// 更新头像
	avatar := hbox.Objects[0].(*widget.Label)
	avatar.SetText("👥")

	// 更新名称和ID
	vbox := hbox.Objects[1].(*fyne.Container)
	nameLabel := vbox.Objects[0].(*widget.Label)
	idLabel := vbox.Objects[1].(*widget.Label)

	if name, ok := group["group_name"].(string); ok {
		nameLabel.SetText(name)
	} else {
		nameLabel.SetText("未知群组")
	}

	if id, ok := group["group_id"].(float64); ok {
		idLabel.SetText(fmt.Sprintf("ID: %.0f", id))
	} else {
		idLabel.SetText("ID: 未知")
	}
}

// 群组操作方法

// quitGroup 退出群组
func (gl *GroupList) quitGroup(group map[string]interface{}) {
	groupName := "未知群组"
	if name, ok := group["group_name"].(string); ok {
		groupName = name
	}

	dialog.ShowConfirm("确认退出", fmt.Sprintf("确定要退出群组 '%s' 吗？", groupName), func(confirmed bool) {
		if !confirmed {
			return
		}

		if groupID, ok := group["group_id"].(float64); ok {
			go func() {
				result, err := gl.netManager.QuitGroup(gl.userID, int(groupID), gl.serviceID)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("退出群组失败: %v", err), gl.window)
						return
					}

					// 检查响应状态
					if code, ok := result["code"].(float64); ok && code != 0 {
						message := "未知错误"
						if msg, ok := result["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf("退出群组失败: %s", message), gl.window)
						return
					}

					dialog.ShowInformation("成功", "退出群组成功！", gl.window)
					gl.RefreshGroups() // 刷新列表
				})
			}()
		} else {
			dialog.ShowError(fmt.Errorf("无效的群组ID"), gl.window)
		}
	}, gl.window)
}

// showGroupMembers 显示群组成员
func (gl *GroupList) showGroupMembers(group map[string]interface{}) {
	if groupID, ok := group["group_id"].(float64); ok {
		go func() {
			result, err := gl.netManager.GetGroupMembers(int(groupID), gl.serviceID)
			fyne.Do(func() {
				if err != nil {
					dialog.ShowError(fmt.Errorf("获取群组成员失败: %v", err), gl.window)
					return
				}

				// 检查响应状态
				if code, ok := result["code"].(float64); ok && code != 0 {
					message := "未知错误"
					if msg, ok := result["message"].(string); ok {
						message = msg
					}
					dialog.ShowError(fmt.Errorf("获取群组成员失败: %s", message), gl.window)
					return
				}

				// 解析成员列表 - 修复数据格式解析
				var membersText string
				var members []interface{}

				// 首先检查data是否为对象格式
				if dataObj, ok := result["data"].(map[string]interface{}); ok {
					// 从data对象中获取members数组
					if membersData, ok := dataObj["members"].([]interface{}); ok {
						members = membersData
					}
				} else if dataArray, ok := result["data"].([]interface{}); ok {
					// 如果data直接是数组格式
					members = dataArray
				}

				// 构建成员列表文本
				if len(members) == 0 {
					membersText = "该群组暂无成员"
				} else {
					membersText = fmt.Sprintf("群组成员 (共%d人):\n\n", len(members))
					for i, memberData := range members {
						if member, ok := memberData.(map[string]interface{}); ok {
							userID := "未知"
							if id, ok := member["user_id"].(float64); ok {
								userID = strconv.Itoa(int(id))
							}
							joinTime := "未知时间"
							if time, ok := member["join_time"].(string); ok {
								joinTime = time
							}
							membersText += fmt.Sprintf("%d. 用户ID: %s, 加入时间: %s\n", i+1, userID, joinTime)
						}
					}
				}

				// 如果仍然无法解析，显示调试信息
				if len(members) == 0 && membersText == "" {
					log.Printf("Failed to parse members data. Raw data: %+v", result["data"])
					membersText = "无法解析成员列表数据"
				}

				// 显示成员列表对话框
				groupName := "未知群组"
				if name, ok := group["group_name"].(string); ok {
					groupName = name
				}

				dialog.ShowInformation(fmt.Sprintf("群组成员 - %s", groupName), membersText, gl.window)
			})
		}()
	} else {
		dialog.ShowError(fmt.Errorf("无效的群组ID"), gl.window)
	}
}

// ==================== 第六阶段增强功能：智能群组详情窗口 ====================

// onGroupItemClick 处理群组项点击事件
func (gl *GroupList) onGroupItemClick(group map[string]interface{}) {
	// 获取群组ID
	groupID, ok := group["group_id"].(float64)
	if !ok {
		log.Printf("无效的群组ID: %+v", group)
		dialog.ShowError(fmt.Errorf("无效的群组ID"), gl.window)
		return
	}

	log.Printf("点击群组: ID=%d, ServiceID=%d, UserID=%d", int(groupID), gl.serviceID, gl.userID)

	// 显示加载状态
	loadingDialog := dialog.NewInformation("加载中", "正在获取群组信息和用户角色...", gl.window)
	loadingDialog.Show()

	// 在后台获取用户角色，然后打开详情窗口
	go func() {
		// 获取当前用户在该群组中的角色
		roleResult, err := gl.netManager.GetCurrentUserRole(int(groupID), gl.userID, gl.serviceID)

		fyne.Do(func() {
			loadingDialog.Hide()

			if err != nil {
				log.Printf("获取用户角色失败: %v", err)
				// 即使获取角色失败，也可以以访客身份打开窗口
				gl.openGroupDetailWindow(int(groupID), gl.serviceID)
				return
			}

			// 添加调试日志
			log.Printf("群组列表点击时获取的用户角色响应: %+v", roleResult)

			// 解析角色信息
			userRole := 4 // 默认为访客
			if code, ok := roleResult["code"].(float64); ok && code == 0 {
				if data, ok := roleResult["data"].(map[string]interface{}); ok {
					if roleType, ok := data["role_type"].(float64); ok {
						userRole = int(roleType)
					}
				}
			}

			log.Printf("群组列表点击时解析的用户角色: %d", userRole)

			// 根据角色添加视觉反馈
			gl.addRoleVisualFeedback(group, userRole)

			// 打开智能详情窗口
			gl.openGroupDetailWindow(int(groupID), gl.serviceID)
		})
	}()
}

// openGroupDetailWindow 打开群组详情窗口
func (gl *GroupList) openGroupDetailWindow(groupID, serviceID int) {
	log.Printf("打开群组详情窗口: GroupID=%d, ServiceID=%d", groupID, serviceID)

	// 创建新的群组详情窗口
	detailWindow := NewGroupDetailWindow(gl.netManager, gl.userID, groupID, serviceID)

	// 显示窗口
	detailWindow.Show()

	log.Printf("群组详情窗口已打开")
}

// addRoleVisualFeedback 根据用户角色添加视觉反馈
func (gl *GroupList) addRoleVisualFeedback(group map[string]interface{}, userRole int) {
	// 这里可以根据角色为群组项添加不同的视觉标识
	// 由于Fyne的限制，我们通过日志记录角色信息
	roleName := gl.getRoleDisplayName(userRole)
	groupName := "未知群组"
	if name, ok := group["group_name"].(string); ok {
		groupName = name
	}

	log.Printf("群组 '%s' 中的角色: %s", groupName, roleName)

	// 可以在这里实现更复杂的视觉反馈，比如：
	// - 为群主显示金色边框或皇冠图标
	// - 为管理员显示银色边框或盾牌图标
	// - 为普通成员显示默认样式
	// 由于当前的列表渲染机制限制，暂时通过日志记录
}

// getRoleDisplayName 获取角色显示名称
func (gl *GroupList) getRoleDisplayName(roleType int) string {
	switch roleType {
	case 1:
		return "群主 👑"
	case 2:
		return "管理员 🛡️"
	case 3:
		return "成员"
	case 4:
		return "访客"
	default:
		return "未知角色"
	}
}

// 保留原有的右键菜单功能作为备用
func (gl *GroupList) showGroupContextMenu(group map[string]interface{}) {
	// 创建右键菜单
	quitItem := fyne.NewMenuItem("退出群组", func() {
		gl.quitGroup(group)
	})

	membersItem := fyne.NewMenuItem("查看成员", func() {
		gl.showGroupMembers(group)
	})

	detailItem := fyne.NewMenuItem("群组详情", func() {
		gl.onGroupItemClick(group)
	})

	menu := fyne.NewMenu("", detailItem, membersItem, quitItem)
	widget.NewPopUpMenu(menu, gl.window.Canvas()).ShowAtPosition(fyne.CurrentApp().Driver().AbsolutePositionForObject(gl.groupsList))
}
