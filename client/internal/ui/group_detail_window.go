package ui

import (
	"fmt"
	"log"
	"time"

	//"strconv"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// 角色类型常量
const (
	RoleMember = 0 // 普通成员
	RoleAdmin  = 1 // 管理员
	RoleOwner  = 2 // 群主
	RoleGuest  = 3 // 访客
)

// GroupDetailWindow 智能群组详情窗口
type GroupDetailWindow struct {
	window          fyne.Window
	netManager      *network.NetworkManager
	userID          int
	groupID         int
	serviceID       int
	groupInfo       map[string]interface{}
	userRole        int
	permissionLevel int

	// UI组件
	mainContainer *fyne.Container
	tabContainer  *container.AppTabs
	infoTab       *container.TabItem
	membersTab    *container.TabItem
	settingsTab   *container.TabItem
	advancedTab   *container.TabItem

	// 群组信息组件
	groupNameLabel   *widget.Label
	groupDescLabel   *widget.Label
	memberCountLabel *widget.Label
	creatorLabel     *widget.Label

	// 成员列表组件
	membersList *widget.List
	members     []map[string]interface{}

	// 设置组件
	settingsContainer *fyne.Container

	// 高级功能组件
	advancedContainer *fyne.Container
}

// NewGroupDetailWindow 创建新的群组详情窗口
func NewGroupDetailWindow(netManager *network.NetworkManager, userID, groupID, serviceID int) *GroupDetailWindow {
	// 创建新窗口
	window := fyne.CurrentApp().NewWindow("群组详情")
	window.Resize(fyne.NewSize(800, 600))

	gdw := &GroupDetailWindow{
		window:     window,
		netManager: netManager,
		userID:     userID,
		groupID:    groupID,
		serviceID:  serviceID,
		members:    []map[string]interface{}{},
	}

	// 初始化UI
	gdw.setupUI()

	// 加载数据
	go gdw.loadGroupData()

	// 窗口关闭事件
	window.SetOnClosed(func() {
		log.Printf("群组详情窗口关闭: GroupID=%d", groupID)
	})

	return gdw
}

// Show 显示窗口
func (gdw *GroupDetailWindow) Show() {
	gdw.window.Show()
}

// setupUI 设置用户界面
func (gdw *GroupDetailWindow) setupUI() {
	// 创建标签页容器
	gdw.tabContainer = container.NewAppTabs()

	// 创建各个标签页
	gdw.createInfoTab()
	gdw.createMembersTab()
	gdw.createSettingsTab()
	gdw.createAdvancedTab()

	// 创建主容器
	gdw.mainContainer = container.NewVBox(
		widget.NewCard("", "", gdw.tabContainer),
	)

	// 设置窗口内容
	gdw.window.SetContent(gdw.mainContainer)
}

// createInfoTab 创建群组信息标签页
func (gdw *GroupDetailWindow) createInfoTab() {
	// 创建信息显示组件
	gdw.groupNameLabel = widget.NewLabel("群组名称: 加载中...")
	gdw.groupDescLabel = widget.NewLabel("群组描述: 加载中...")
	gdw.memberCountLabel = widget.NewLabel("成员数量: 加载中...")
	gdw.creatorLabel = widget.NewLabel("创建者: 加载中...")

	// 创建操作按钮
	joinButton := widget.NewButton("加入群组", func() {
		gdw.handleJoinGroup()
	})

	quitButton := widget.NewButton("退出群组", func() {
		gdw.handleQuitGroup()
	})

	// 创建按钮容器
	buttonContainer := container.NewHBox(joinButton, quitButton)

	// 创建信息容器
	infoContainer := container.NewVBox(
		gdw.groupNameLabel,
		gdw.groupDescLabel,
		gdw.memberCountLabel,
		gdw.creatorLabel,
		widget.NewSeparator(),
		buttonContainer,
	)

	// 添加到标签页
	gdw.infoTab = container.NewTabItem("群组信息", infoContainer)
	gdw.tabContainer.Append(gdw.infoTab)
}

// createMembersTab 创建成员列表标签页
func (gdw *GroupDetailWindow) createMembersTab() {
	// 创建成员列表
	gdw.membersList = widget.NewList(
		func() int {
			return len(gdw.members)
		},
		func() fyne.CanvasObject {
			return container.NewHBox(
				widget.NewLabel("用户名"),
				widget.NewLabel("角色"),
				widget.NewButton("操作", nil),
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(gdw.members) {
				return
			}

			member := gdw.members[id]
			container := obj.(*fyne.Container)

			// 更新用户名
			nameLabel := container.Objects[0].(*widget.Label)
			if username, ok := member["username"].(string); ok {
				nameLabel.SetText(username)
			}

			// 更新角色
			roleLabel := container.Objects[1].(*widget.Label)
			if roleType, ok := member["role_type"].(float64); ok {
				roleLabel.SetText(gdw.getRoleText(int(roleType)))
			}

			// 更新操作按钮
			actionButton := container.Objects[2].(*widget.Button)
			actionButton.OnTapped = func() {
				gdw.showMemberActions(member)
			}
		},
	)

	// 创建刷新按钮
	refreshButton := widget.NewButton("刷新成员", func() {
		go gdw.loadMembers()
	})

	// 创建成员容器
	membersContainer := container.NewBorder(
		refreshButton,   // top
		nil,             // bottom
		nil,             // left
		nil,             // right
		gdw.membersList, // center
	)

	// 添加到标签页
	gdw.membersTab = container.NewTabItem("成员管理", membersContainer)
	gdw.tabContainer.Append(gdw.membersTab)
}

// createSettingsTab 创建设置标签页
func (gdw *GroupDetailWindow) createSettingsTab() {
	// 创建设置表单
	nameEntry := widget.NewEntry()
	nameEntry.SetPlaceHolder("群组名称")

	descEntry := widget.NewMultiLineEntry()
	descEntry.SetPlaceHolder("群组描述")

	// 创建保存按钮
	saveButton := widget.NewButton("保存设置", func() {
		gdw.saveGroupSettings(nameEntry.Text, descEntry.Text)
	})

	// 创建设置容器
	gdw.settingsContainer = container.NewVBox(
		widget.NewForm(
			widget.NewFormItem("群组名称", nameEntry),
			widget.NewFormItem("群组描述", descEntry),
		),
		saveButton,
	)

	// 添加到标签页
	gdw.settingsTab = container.NewTabItem("群组设置", gdw.settingsContainer)
	gdw.tabContainer.Append(gdw.settingsTab)
}

// createAdvancedTab 创建高级功能标签页
func (gdw *GroupDetailWindow) createAdvancedTab() {
	// 创建类型转换按钮
	changeTypeButton := widget.NewButton("变换群组类型", func() {
		gdw.showChangeTypeDialog()
	})

	// 创建权限管理按钮
	permissionButton := widget.NewButton("权限管理", func() {
		gdw.showPermissionDialog()
	})

	// 创建高级容器
	gdw.advancedContainer = container.NewVBox(
		widget.NewCard("群组管理", "", container.NewVBox(
			changeTypeButton,
			permissionButton,
		)),
	)

	// 添加到标签页
	gdw.advancedTab = container.NewTabItem("高级功能", gdw.advancedContainer)
	gdw.tabContainer.Append(gdw.advancedTab)
}

// loadGroupData 加载群组数据
func (gdw *GroupDetailWindow) loadGroupData() {
	// 首先获取用户角色
	gdw.loadUserRole()

	// 然后加载群组设置
	gdw.loadGroupSettings()

	// 加载成员列表
	gdw.loadMembers()
}

// loadUserRole 加载用户角色 - 修复版本
func (gdw *GroupDetailWindow) loadUserRole() {
	log.Printf("开始获取用户角色: GroupID=%d, UserID=%d, ServiceID=%d", gdw.groupID, gdw.userID, gdw.serviceID)

	result, err := gdw.netManager.GetCurrentUserRole(gdw.groupID, gdw.userID, gdw.serviceID)
	if err != nil {
		log.Printf("获取用户角色失败: %v", err)
		gdw.userRole = RoleGuest
		fyne.Do(func() {
			gdw.loadRoleBasedInterface()
		})
		return
	}

	// 添加调试日志
	log.Printf("获取用户角色响应: %+v", result)

	// 解析角色 - 简化逻辑，直接使用服务器返回的角色
	gdw.userRole = RoleGuest // 默认设为访客

	if code, ok := result["code"].(float64); ok && code == 0 {
		// 成功响应
		if data, ok := result["data"].(map[string]interface{}); ok {
			if roleType, ok := data["role_type"].(float64); ok {
				gdw.userRole = int(roleType)
				log.Printf("成功获取用户角色: %d", gdw.userRole)
			} else {
				log.Printf("角色数据中没有role_type字段，使用默认角色")
				// 如果服务器返回了其他格式的角色信息，尝试其他字段
				if roleType, ok := data["role"].(float64); ok {
					gdw.userRole = int(roleType)
					log.Printf("从role字段获取用户角色: %d", gdw.userRole)
				}
			}
		} else {
			log.Printf("响应中没有data字段，使用默认角色")
		}
	} else {
		log.Printf("响应码不为0: %.0f", code)
	}

	log.Printf("最终用户角色: %d", gdw.userRole)

	// 更新界面
	fyne.Do(func() {
		gdw.loadRoleBasedInterface()
	})
}

// loadGroupSettings 加载群组设置
func (gdw *GroupDetailWindow) loadGroupSettings() {
	result, err := gdw.netManager.GetGroupSettings(gdw.groupID, gdw.serviceID)
	if err != nil {
		log.Printf("获取群组设置失败: %v", err)
		return
	}

	if code, ok := result["code"].(float64); ok && code == 0 {
		if data, ok := result["data"].(map[string]interface{}); ok {
			gdw.groupInfo = data

			// 更新UI
			fyne.Do(func() {
				gdw.updateGroupInfo()
			})
		}
	}
}

// loadMembers 加载成员列表
func (gdw *GroupDetailWindow) loadMembers() {
	result, err := gdw.netManager.GetGroupMembers(gdw.groupID, gdw.serviceID)
	if err != nil {
		log.Printf("获取成员列表失败: %v", err)
		return
	}

	if code, ok := result["code"].(float64); ok && code == 0 {
		if dataObj, ok := result["data"].(map[string]interface{}); ok {
			if membersArray, ok := dataObj["members"].([]interface{}); ok {
				gdw.members = make([]map[string]interface{}, len(membersArray))
				for i, member := range membersArray {
					if memberMap, ok := member.(map[string]interface{}); ok {
						gdw.members[i] = memberMap
					}
				}

				// 更新UI
				fyne.Do(func() {
					gdw.membersList.Refresh()
					// 更新群组信息显示，包括成员数量
					gdw.updateGroupInfo()
				})
			}
		}
	}
}

// loadRoleBasedInterface 根据角色加载界面
func (gdw *GroupDetailWindow) loadRoleBasedInterface() {
	// 根据角色显示/隐藏标签页
	switch gdw.userRole {
	case RoleOwner:
		// 群主：显示所有功能
		gdw.showAllTabs()
		gdw.window.SetTitle(fmt.Sprintf("群组详情 - 群主"))

	case RoleAdmin:
		// 管理员：显示部分管理功能
		gdw.showAdminTabs()
		gdw.window.SetTitle(fmt.Sprintf("群组详情 - 管理员"))

	case RoleMember:
		// 普通成员：只显示基本信息
		gdw.showMemberTabs()
		gdw.window.SetTitle(fmt.Sprintf("群组详情 - 成员"))

	case RoleGuest:
		// 访客：显示基本信息和加入选项
		gdw.showGuestTabs()
		gdw.window.SetTitle(fmt.Sprintf("群组详情 - 访客"))
	}
}

// showAllTabs 显示所有标签页（群主）
func (gdw *GroupDetailWindow) showAllTabs() {
	// 确保所有标签页都可见
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)

	gdw.tabContainer.Append(gdw.infoTab)
	gdw.tabContainer.Append(gdw.membersTab)
	gdw.tabContainer.Append(gdw.settingsTab)
	gdw.tabContainer.Append(gdw.advancedTab)
}

// showAdminTabs 显示管理员标签页
func (gdw *GroupDetailWindow) showAdminTabs() {
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)

	gdw.tabContainer.Append(gdw.infoTab)
	gdw.tabContainer.Append(gdw.membersTab)
	gdw.tabContainer.Append(gdw.settingsTab)
	// 管理员不显示高级功能
}

// showMemberTabs 显示成员标签页
func (gdw *GroupDetailWindow) showMemberTabs() {
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)

	gdw.tabContainer.Append(gdw.infoTab)
	gdw.tabContainer.Append(gdw.membersTab)
	// 普通成员不显示设置和高级功能
}

// showGuestTabs 显示访客标签页
func (gdw *GroupDetailWindow) showGuestTabs() {
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)
	gdw.tabContainer.RemoveIndex(0)

	gdw.tabContainer.Append(gdw.infoTab)
	// 访客只显示基本信息
}

// updateGroupInfo 更新群组信息显示
func (gdw *GroupDetailWindow) updateGroupInfo() {
	if gdw.groupInfo == nil {
		return
	}

	// 更新群组名称
	if name, ok := gdw.groupInfo["group_name"].(string); ok {
		gdw.groupNameLabel.SetText(fmt.Sprintf("群组名称: %s", name))
	}

	// 更新群组描述
	if desc, ok := gdw.groupInfo["description"].(string); ok {
		gdw.groupDescLabel.SetText(fmt.Sprintf("群组描述: %s", desc))
	}

	// 更新成员数量
	gdw.memberCountLabel.SetText(fmt.Sprintf("成员数量: %d", len(gdw.members)))

	// 更新创建者
	if creator, ok := gdw.groupInfo["creator_name"].(string); ok {
		gdw.creatorLabel.SetText(fmt.Sprintf("创建者: %s", creator))
	}
}

// getRoleText 获取角色文本
func (gdw *GroupDetailWindow) getRoleText(roleType int) string {
	switch roleType {
	case RoleOwner:
		return "群主"
	case RoleAdmin:
		return "管理员"
	case RoleMember:
		return "成员"
	case RoleGuest:
		return "访客"
	default:
		return "未知"
	}
}

// handleJoinGroup 处理加入群组
func (gdw *GroupDetailWindow) handleJoinGroup() {
	// 根据服务类型选择不同的加入方式
	switch gdw.serviceID {
	case 1: // QQ群申请加入
		gdw.showApplyJoinDialog()
	case 2: // 微信群邀请加入（需要邀请人）
		gdw.showInviteJoinDialog()
	case 3: // 微博超话自由加入
		gdw.handleFreeJoin()
	default:
		// 使用默认的加入方式
		gdw.handleDefaultJoin()
	}
}

// showApplyJoinDialog 显示申请加入对话框
func (gdw *GroupDetailWindow) showApplyJoinDialog() {
	reasonEntry := widget.NewMultiLineEntry()
	reasonEntry.SetPlaceHolder("请输入申请理由...")

	dialog.ShowForm("申请加入群组", "申请", "取消", []*widget.FormItem{
		widget.NewFormItem("申请理由", reasonEntry),
	}, func(confirm bool) {
		if confirm {
			go func() {
				result, err := gdw.netManager.ApplyJoinGroup(gdw.groupID, gdw.userID, gdw.serviceID, reasonEntry.Text)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("申请失败: %v", err), gdw.window)
						return
					}

					if code, ok := result["code"].(float64); ok && code == 0 {
						dialog.ShowInformation("申请成功", "您的申请已提交，请等待管理员审核", gdw.window)
					} else {
						message := "申请失败"
						if msg, ok := result["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf(message), gdw.window)
					}
				})
			}()
		}
	}, gdw.window)
}

// showInviteJoinDialog 显示邀请加入对话框
func (gdw *GroupDetailWindow) showInviteJoinDialog() {
	dialog.ShowInformation("邀请加入", "微信群需要群成员邀请才能加入，请联系群成员邀请您", gdw.window)
}

// handleFreeJoin 处理自由加入
func (gdw *GroupDetailWindow) handleFreeJoin() {
	dialog.ShowConfirm("加入超话", "确定要加入这个超话吗？", func(confirm bool) {
		if confirm {
			go func() {
				result, err := gdw.netManager.FreeJoinTopic(gdw.groupID, gdw.userID, gdw.serviceID)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("加入失败: %v", err), gdw.window)
						return
					}

					if code, ok := result["code"].(float64); ok && code == 0 {
						dialog.ShowInformation("加入成功", "您已成功加入超话", gdw.window)
						// 重新加载用户角色
						go gdw.loadUserRole()
					} else {
						message := "加入失败"
						if msg, ok := result["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf(message), gdw.window)
					}
				})
			}()
		}
	}, gdw.window)
}

// handleDefaultJoin 处理默认加入
func (gdw *GroupDetailWindow) handleDefaultJoin() {
	dialog.ShowConfirm("加入群组", "确定要加入这个群组吗？", func(confirm bool) {
		if confirm {
			go func() {
				result, err := gdw.netManager.JoinGroup(gdw.userID, gdw.groupID, gdw.serviceID, 1)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("加入失败: %v", err), gdw.window)
						return
					}

					if code, ok := result["code"].(float64); ok && code == 0 {
						dialog.ShowInformation("加入成功", "您已成功加入群组", gdw.window)
						// 重新加载用户角色
						go gdw.loadUserRole()
					} else {
						message := "加入失败"
						if msg, ok := result["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf(message), gdw.window)
					}
				})
			}()
		}
	}, gdw.window)
}

// handleQuitGroup 处理退出群组
func (gdw *GroupDetailWindow) handleQuitGroup() {
	dialog.ShowConfirm("退出群组", "确定要退出这个群组吗？", func(confirm bool) {
		if confirm {
			go func() {
				result, err := gdw.netManager.QuitGroup(gdw.userID, gdw.groupID, gdw.serviceID)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("退出失败: %v", err), gdw.window)
						return
					}

					if code, ok := result["code"].(float64); ok && code == 0 {
						dialog.ShowInformation("退出成功", "您已成功退出群组", gdw.window)
						gdw.window.Close()
					} else {
						message := "退出失败"
						if msg, ok := result["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf(message), gdw.window)
					}
				})
			}()
		}
	}, gdw.window)
}

// showMemberActions 显示成员操作菜单
func (gdw *GroupDetailWindow) showMemberActions(member map[string]interface{}) {
	memberID, _ := member["user_id"].(float64)
	memberName, _ := member["username"].(string)
	currentRole, _ := member["role_type"].(float64)

	// 如果是自己，显示个人信息
	if int(memberID) == gdw.userID {
		gdw.showPersonalInfo(member)
		return
	}

	// 只有管理员和群主才能操作其他成员
	if gdw.userRole != RoleOwner && gdw.userRole != RoleAdmin {
		gdw.showMemberInfo(member)
		return
	}

	// 群主可以操作所有人，管理员只能操作普通成员
	if gdw.userRole == RoleAdmin && int(currentRole) <= RoleAdmin {
		dialog.ShowInformation("权限不足", "管理员无法操作群主或其他管理员", gdw.window)
		return
	}

	var buttons []fyne.CanvasObject

	// 添加角色设置按钮
	if gdw.userRole == RoleOwner {
		// 群主可以设置任何角色
		if int(currentRole) != RoleAdmin {
			buttons = append(buttons, widget.NewButton("🛡️ 设为管理员", func() {
				gdw.confirmSetMemberRole(int(memberID), memberName, RoleAdmin, "管理员")
			}))
		}
		if int(currentRole) != RoleMember {
			buttons = append(buttons, widget.NewButton("👤 设为普通成员", func() {
				gdw.confirmSetMemberRole(int(memberID), memberName, RoleMember, "普通成员")
			}))
		}
	} else if gdw.userRole == RoleAdmin {
		// 管理员只能将普通成员设为管理员
		if int(currentRole) == RoleMember {
			buttons = append(buttons, widget.NewButton("🛡️ 设为管理员", func() {
				gdw.confirmSetMemberRole(int(memberID), memberName, RoleAdmin, "管理员")
			}))
		}
	}

	// TODO: 添加移除成员功能（需要服务器端支持）
	// 暂时注释掉移除成员功能，因为服务器端没有相应的API
	/*
		// 添加移除成员按钮（只有群主可以移除管理员，管理员只能移除普通成员）
		canRemove := false
		if gdw.userRole == RoleOwner {
			canRemove = true
		} else if gdw.userRole == RoleAdmin && int(currentRole) == RoleMember {
			canRemove = true
		}

		if canRemove {
			buttons = append(buttons, widget.NewButton("❌ 移除成员", func() {
				gdw.confirmRemoveMember(int(memberID), memberName)
			}))
		}
	*/

	// 添加查看详情按钮
	buttons = append(buttons, widget.NewButton("ℹ️ 查看详情", func() {
		gdw.showMemberInfo(member)
	}))

	content := container.NewVBox(
		widget.NewCard("成员操作", fmt.Sprintf("成员: %s (%s)", memberName, gdw.getRoleText(int(currentRole))),
			container.NewVBox(buttons...)),
	)

	dialog.ShowCustom("成员管理", "关闭", content, gdw.window)
}

// confirmSetMemberRole 确认设置成员角色
func (gdw *GroupDetailWindow) confirmSetMemberRole(memberID int, memberName string, roleType int, roleName string) {
	dialog.ShowConfirm("确认操作",
		fmt.Sprintf("确定要将 %s 设置为 %s 吗？", memberName, roleName),
		func(confirmed bool) {
			if confirmed {
				gdw.setMemberRole(memberID, roleType)
			}
		}, gdw.window)
}

// TODO: 移除成员功能（需要服务器端支持）
/*
// confirmRemoveMember 确认移除成员
func (gdw *GroupDetailWindow) confirmRemoveMember(memberID int, memberName string) {
	dialog.ShowConfirm("确认移除",
		fmt.Sprintf("确定要将 %s 移除出群吗？此操作不可撤销。", memberName),
		func(confirmed bool) {
			if confirmed {
				gdw.removeMember(memberID)
			}
		}, gdw.window)
}

// removeMember 移除成员
func (gdw *GroupDetailWindow) removeMember(memberID int) {
	go func() {
		result, err := gdw.netManager.RemoveMember(gdw.groupID, memberID, gdw.serviceID)
		fyne.Do(func() {
			if err != nil {
				dialog.ShowError(fmt.Errorf("移除成员失败: %v", err), gdw.window)
				return
			}

			if code, ok := result["code"].(float64); ok && code == 0 {
				dialog.ShowInformation("移除成功", "成员已被移除", gdw.window)
				// 重新加载成员列表
				go gdw.loadMembers()
			} else {
				message := "移除失败"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf(message), gdw.window)
			}
		})
	}()
}
*/

// showMemberInfo 显示成员详细信息
func (gdw *GroupDetailWindow) showMemberInfo(member map[string]interface{}) {
	memberName, _ := member["username"].(string)
	memberID, _ := member["user_id"].(float64)
	roleType, _ := member["role_type"].(float64)
	joinTime, _ := member["join_time"].(string)

	info := container.NewVBox(
		widget.NewCard("成员信息", "",
			container.NewVBox(
				widget.NewLabel(fmt.Sprintf("用户名: %s", memberName)),
				widget.NewLabel(fmt.Sprintf("用户ID: %.0f", memberID)),
				widget.NewLabel(fmt.Sprintf("角色: %s", gdw.getRoleText(int(roleType)))),
				widget.NewLabel(fmt.Sprintf("加入时间: %s", joinTime)),
			),
		),
	)

	dialog.ShowCustom("成员详情", "关闭", info, gdw.window)
}

// showPersonalInfo 显示个人信息
func (gdw *GroupDetailWindow) showPersonalInfo(member map[string]interface{}) {
	memberName, _ := member["username"].(string)
	roleType, _ := member["role_type"].(float64)
	joinTime, _ := member["join_time"].(string)

	info := container.NewVBox(
		widget.NewCard("我的信息", "",
			container.NewVBox(
				widget.NewLabel(fmt.Sprintf("用户名: %s", memberName)),
				widget.NewLabel(fmt.Sprintf("我的角色: %s", gdw.getRoleText(int(roleType)))),
				widget.NewLabel(fmt.Sprintf("加入时间: %s", joinTime)),
			),
		),
	)

	dialog.ShowCustom("我的信息", "关闭", info, gdw.window)
}

// setMemberRole 设置成员角色
func (gdw *GroupDetailWindow) setMemberRole(memberID, roleType int) {
	go func() {
		result, err := gdw.netManager.SetMemberRole(gdw.groupID, memberID, gdw.serviceID, roleType)
		fyne.Do(func() {
			if err != nil {
				dialog.ShowError(fmt.Errorf("设置角色失败: %v", err), gdw.window)
				return
			}

			if code, ok := result["code"].(float64); ok && code == 0 {
				dialog.ShowInformation("设置成功", "成员角色已更新", gdw.window)
				// 重新加载成员列表
				go gdw.loadMembers()
			} else {
				message := "设置失败"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf(message), gdw.window)
			}
		})
	}()
}

// saveGroupSettings 保存群组设置
func (gdw *GroupDetailWindow) saveGroupSettings(name, description string) {
	settings := map[string]interface{}{
		"group_name":  name,
		"description": description,
	}

	go func() {
		result, err := gdw.netManager.UpdateGroupSettings(gdw.groupID, gdw.serviceID, settings)
		fyne.Do(func() {
			if err != nil {
				dialog.ShowError(fmt.Errorf("保存设置失败: %v", err), gdw.window)
				return
			}

			if code, ok := result["code"].(float64); ok && code == 0 {
				dialog.ShowInformation("保存成功", "群组设置已更新", gdw.window)
				// 重新加载群组设置
				go gdw.loadGroupSettings()
			} else {
				message := "保存失败"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf(message), gdw.window)
			}
		})
	}()
}

// showChangeTypeDialog 显示变换类型对话框 - 修复版本
func (gdw *GroupDetailWindow) showChangeTypeDialog() {
	serviceSelect := widget.NewSelect([]string{"QQ群", "微信群", "微博超话"}, nil)
	serviceSelect.SetSelected("QQ群")

	dialog.ShowForm("变换群组类型", "确定", "取消", []*widget.FormItem{
		widget.NewFormItem("目标类型", serviceSelect),
	}, func(confirm bool) {
		if confirm {
			targetServiceID := 1
			switch serviceSelect.Selected {
			case "微信群":
				targetServiceID = 2
			case "微博超话":
				targetServiceID = 3
			}

			// 显示确认对话框
			dialog.ShowConfirm("确认转换",
				fmt.Sprintf("确定要将群组从 %s 转换为 %s 吗？\n\n转换后：\n- 当前窗口将关闭\n- 群组将从当前服务列表中消失\n- 请在目标服务中查看该群组",
					gdw.getServiceName(gdw.serviceID), serviceSelect.Selected),
				func(confirmed bool) {
					if confirmed {
						go func() {
							result, err := gdw.netManager.ChangeGroupType(gdw.groupID, gdw.serviceID, targetServiceID)
							fyne.Do(func() {
								if err != nil {
									dialog.ShowError(fmt.Errorf("变换类型失败: %v", err), gdw.window)
									return
								}

								if code, ok := result["code"].(float64); ok && code == 0 {
									dialog.ShowInformation("转换成功",
										fmt.Sprintf("群组已成功转换为 %s！\n\n当前窗口将关闭，请在 %s 服务中查看该群组。",
											serviceSelect.Selected, serviceSelect.Selected),
										gdw.window)

									// 延迟关闭窗口，让用户看到成功消息
									go func() {
										time.Sleep(2 * time.Second)
										fyne.Do(func() {
											gdw.window.Close()
										})
									}()
								} else {
									message := "变换失败"
									if msg, ok := result["message"].(string); ok {
										message = msg
									}
									dialog.ShowError(fmt.Errorf(message), gdw.window)
								}
							})
						}()
					}
				}, gdw.window)
		}
	}, gdw.window)
}

// getServiceName 获取服务名称
func (gdw *GroupDetailWindow) getServiceName(serviceID int) string {
	switch serviceID {
	case 1:
		return "QQ群"
	case 2:
		return "微信群"
	case 3:
		return "微博超话"
	default:
		return "未知服务"
	}
}

// showPermissionDialog 显示权限管理对话框
func (gdw *GroupDetailWindow) showPermissionDialog() {
	content := container.NewVBox(
		widget.NewLabel("权限管理功能"),
		widget.NewLabel("此功能用于管理群组的各种权限设置"),
		widget.NewButton("刷新权限", func() {
			go gdw.loadUserRole()
		}),
	)

	dialog.ShowCustom("权限管理", "关闭", content, gdw.window)
}

// Close 关闭窗口
func (gdw *GroupDetailWindow) Close() {
	gdw.window.Close()
}

// RequestFocus 请求窗口焦点
func (gdw *GroupDetailWindow) RequestFocus() {
	gdw.window.RequestFocus()
}

// SetCloseCallback 设置窗口关闭回调
func (gdw *GroupDetailWindow) SetCloseCallback(callback func()) {
	gdw.window.SetOnClosed(callback)
}
