package ui

import (
	"fmt"
	"log"
	"strings"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/layout"
	"fyne.io/fyne/v2/widget"

	"fyne-im/network"
)

// FriendList 好友列表组件
type FriendList struct {
	container   *fyne.Container
	netManager  *network.NetworkManager
	userID      int
	serviceID   int
	window      fyne.Window
	friendsList *widget.List
	friends     []network.Friend
	onRefresh   func()
}

// NewFriendList 创建新的好友列表组件
func NewFriendList(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) *FriendList {
	fl := &FriendList{
		window:     window,
		netManager: netManager,
		userID:     userID,
		serviceID:  serviceID,
		friends:    []network.Friend{},
	}

	// 创建好友列表
	fl.friendsList = widget.NewList(
		func() int {
			return len(fl.friends)
		},
		func() fyne.CanvasObject {
			// 创建好友项模板
			avatar := widget.NewLabel("👤")
			avatar.TextStyle = fyne.TextStyle{Bold: true}

			nameLabel := widget.NewLabel("好友昵称")
			nameLabel.TextStyle = fyne.TextStyle{Bold: true}

			remarkLabel := widget.NewLabel("备注")
			remarkLabel.Resize(fyne.NewSize(100, 20))

			timeLabel := widget.NewLabel("添加时间")
			timeLabel.Resize(fyne.NewSize(80, 20))

			return container.NewHBox(
				avatar,
				container.NewVBox(nameLabel, remarkLabel),
				layout.NewSpacer(),
				timeLabel,
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(fl.friends) {
				return
			}

			friend := fl.friends[id]
			hbox := obj.(*fyne.Container)

			// 更新头像
			avatar := hbox.Objects[0].(*widget.Label)
			avatar.SetText("👤")

			// 更新名称和备注
			vbox := hbox.Objects[1].(*fyne.Container)
			nameLabel := vbox.Objects[0].(*widget.Label)
			remarkLabel := vbox.Objects[1].(*widget.Label)

			nameLabel.SetText(friend.Nickname)
			if friend.Remark != "" {
				remarkLabel.SetText(fmt.Sprintf("备注: %s", friend.Remark))
			} else {
				remarkLabel.SetText("无备注")
			}

			// 更新时间
			timeLabel := hbox.Objects[3].(*widget.Label)
			timeLabel.SetText(friend.AddTime)
		},
	)

	// 添加右键菜单
	fl.friendsList.OnSelected = func(id widget.ListItemID) {
		if id >= len(fl.friends) {
			return
		}

		friend := fl.friends[id]

		// 创建右键菜单
		deleteItem := fyne.NewMenuItem("删除好友", func() {
			fl.deleteFriend(friend)
		})

		editRemarkItem := fyne.NewMenuItem("修改备注", func() {
			fl.editRemark(friend)
		})

		menu := fyne.NewMenu("", deleteItem, editRemarkItem)
		widget.NewPopUpMenu(menu, fl.window.Canvas()).ShowAtPosition(fyne.CurrentApp().Driver().AbsolutePositionForObject(fl.friendsList))
	}

	// 创建刷新按钮
	refreshButton := widget.NewButton("刷新", func() {
		fl.refreshFriends()
	})

	// 创建添加好友按钮
	addFriendButton := widget.NewButton("添加好友", func() {
		fl.showAddFriendDialog()
	})

	// 创建按钮栏
	buttonBar := container.NewHBox(refreshButton, addFriendButton)

	// 创建主容器
	fl.container = container.NewBorder(
		buttonBar,      // top
		nil,            // bottom
		nil,            // left
		nil,            // right
		fl.friendsList, // center
	)

	// 初始加载好友列表
	fl.refreshFriends()

	return fl
}

// GetContainer 获取组件容器
func (fl *FriendList) GetContainer() *fyne.Container {
	return fl.container
}

// refreshFriends 刷新好友列表
func (fl *FriendList) refreshFriends() {
	go func() {
		result, err := fl.netManager.QueryFriends(fl.userID, fl.serviceID)
		fyne.Do(func() {
			if err != nil {
				dialog.ShowError(fmt.Errorf("获取好友列表失败: %v", err), fl.window)
				return
			}

			// 检查响应状态码
			code, ok := result["code"].(float64)
			if !ok || code != 0 {
				message := "未知错误"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf("获取好友列表失败: %s", message), fl.window)
				return
			}

			// 解析好友列表
			var friends []network.Friend
			if data, ok := result["data"].(map[string]interface{}); ok {
				if friendsData, ok := data["friends"].([]interface{}); ok {
					for _, friendData := range friendsData {
						if friendMap, ok := friendData.(map[string]interface{}); ok {
							friend := network.Friend{}
							if id, ok := friendMap["friend_id"].(float64); ok {
								friend.FriendID = int(id)
							}
							if nickname, ok := friendMap["nickname"].(string); ok {
								friend.Nickname = nickname
							}
							if username, ok := friendMap["username"].(string); ok {
								friend.Username = username
							}
							if remark, ok := friendMap["remark"].(string); ok {
								friend.Remark = remark
							}
							if addTime, ok := friendMap["add_time"].(string); ok {
								friend.AddTime = addTime
							}
							friends = append(friends, friend)
						}
					}
				}
			}

			fl.friends = friends
			fl.friendsList.Refresh()
			if fl.onRefresh != nil {
				fl.onRefresh()
			}
		})
	}()
}

// deleteFriend 删除好友
func (fl *FriendList) deleteFriend(friend network.Friend) {
	confirm := dialog.NewConfirm(
		"确认删除",
		fmt.Sprintf("确定要删除好友 %s 吗？", friend.Nickname),
		func(confirmed bool) {
			if confirmed {
				go func() {
					result, err := fl.netManager.DeleteFriend(fl.userID, friend.FriendID, fl.serviceID)
					fyne.Do(func() {
						if err != nil {
							dialog.ShowError(fmt.Errorf("删除好友失败: %v", err), fl.window)
							return
						}

						// 检查响应状态码
						code, ok := result["code"].(float64)
						if !ok || code != 0 {
							message := "未知错误"
							if msg, ok := result["message"].(string); ok {
								message = msg
							}
							dialog.ShowError(fmt.Errorf("删除好友失败: %s", message), fl.window)
							return
						}

						dialog.ShowInformation("成功", "好友删除成功", fl.window)
						fl.refreshFriends()
					})
				}()
			}
		},
		fl.window,
	)
	confirm.Show()
}

// editRemark 修改备注
func (fl *FriendList) editRemark(friend network.Friend) {
	remarkEntry := widget.NewEntry()
	remarkEntry.SetText(friend.Remark)
	remarkEntry.SetPlaceHolder("请输入新的备注")

	form := &widget.Form{
		Items: []*widget.FormItem{
			{Text: "备注", Widget: remarkEntry},
		},
		OnSubmit: func() {
			newRemark := strings.TrimSpace(remarkEntry.Text)
			go func() {
				// 先删除再添加来更新备注
				result1, err := fl.netManager.DeleteFriend(fl.userID, friend.FriendID, fl.serviceID)
				if err != nil {
					fyne.Do(func() {
						dialog.ShowError(fmt.Errorf("更新备注失败: %v", err), fl.window)
					})
					return
				}

				// 检查删除响应状态码
				code, ok := result1["code"].(float64)
				if !ok || code != 0 {
					message := "未知错误"
					if msg, ok := result1["message"].(string); ok {
						message = msg
					}
					fyne.Do(func() {
						dialog.ShowError(fmt.Errorf("更新备注失败: %s", message), fl.window)
					})
					return
				}

				result2, err := fl.netManager.AddFriend(fl.userID, friend.FriendID, fl.serviceID, newRemark)
				fyne.Do(func() {
					if err != nil {
						dialog.ShowError(fmt.Errorf("更新备注失败: %v", err), fl.window)
						return
					}

					// 检查添加响应状态码
					code, ok := result2["code"].(float64)
					if !ok || code != 0 {
						message := "未知错误"
						if msg, ok := result2["message"].(string); ok {
							message = msg
						}
						dialog.ShowError(fmt.Errorf("更新备注失败: %s", message), fl.window)
						return
					}

					dialog.ShowInformation("成功", "备注修改成功", fl.window)
					fl.refreshFriends()
				})
			}()
		},
	}

	dialog.ShowForm("修改备注", "确定", "取消", form.Items, func(confirmed bool) {
		if confirmed {
			form.OnSubmit()
		}
	}, fl.window)
}

// showAddFriendDialog 显示添加好友对话框
func (fl *FriendList) showAddFriendDialog() {
	addDialog := NewAddFriendDialog(fl.window, fl.netManager, fl.userID, fl.serviceID, func() {
		fl.refreshFriends()
	})
	addDialog.Show()
}

// SetOnRefresh 设置刷新回调
func (fl *FriendList) SetOnRefresh(callback func()) {
	fl.onRefresh = callback
}

// AddFriendDialog 添加好友对话框
type AddFriendDialog struct {
	dialog      dialog.Dialog
	window      fyne.Window
	netManager  *network.NetworkManager
	userID      int
	serviceID   int
	onSuccess   func()
	searchEntry *widget.Entry
	resultsList *widget.List
	remarkEntry *widget.Entry
	users       []network.User
}

// NewAddFriendDialog 创建添加好友对话框
func NewAddFriendDialog(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int, onSuccess func()) *AddFriendDialog {
	afd := &AddFriendDialog{
		window:     window,
		netManager: netManager,
		userID:     userID,
		serviceID:  serviceID,
		onSuccess:  onSuccess,
		users:      []network.User{},
	}

	// 创建搜索框
	afd.searchEntry = widget.NewEntry()
	afd.searchEntry.SetPlaceHolder("输入用户名或昵称搜索")
	afd.searchEntry.OnChanged = func(text string) {
		if strings.TrimSpace(text) != "" {
			afd.searchUsers(strings.TrimSpace(text))
		} else {
			afd.users = []network.User{}
			afd.resultsList.Refresh()
		}
	}

	// 创建搜索结果列表
	afd.resultsList = widget.NewList(
		func() int {
			return len(afd.users)
		},
		func() fyne.CanvasObject {
			avatar := widget.NewLabel("👤")
			nameLabel := widget.NewLabel("用户名")
			nicknameLabel := widget.NewLabel("昵称")
			emailLabel := widget.NewLabel("邮箱")
			addButton := widget.NewButton("添加", nil)

			return container.NewHBox(
				avatar,
				container.NewVBox(nameLabel, nicknameLabel, emailLabel),
				layout.NewSpacer(),
				addButton,
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(afd.users) {
				return
			}

			user := afd.users[id]
			hbox := obj.(*fyne.Container)

			// 更新用户信息
			vbox := hbox.Objects[1].(*fyne.Container)
			nameLabel := vbox.Objects[0].(*widget.Label)
			nicknameLabel := vbox.Objects[1].(*widget.Label)
			emailLabel := vbox.Objects[2].(*widget.Label)

			nameLabel.SetText(fmt.Sprintf("用户名: %s", user.Username))
			nicknameLabel.SetText(fmt.Sprintf("昵称: %s", user.Nickname))
			emailLabel.SetText(fmt.Sprintf("邮箱: %s", user.Email))

			// 更新添加按钮
			addButton := hbox.Objects[3].(*widget.Button)
			addButton.OnTapped = func() {
				afd.addFriend(user)
			}
		},
	)

	// 创建备注输入框
	afd.remarkEntry = widget.NewEntry()
	afd.remarkEntry.SetPlaceHolder("可选：输入备注信息")

	// 创建对话框内容
	content := container.NewVBox(
		widget.NewLabel("搜索用户:"),
		afd.searchEntry,
		widget.NewSeparator(),
		widget.NewLabel("搜索结果:"),
		container.NewScroll(afd.resultsList),
		widget.NewSeparator(),
		widget.NewLabel("备注:"),
		afd.remarkEntry,
	)

	// 设置内容大小
	content.Resize(fyne.NewSize(500, 400))

	// 创建对话框
	afd.dialog = dialog.NewCustom("添加好友", "关闭", content, window)

	return afd
}

// Show 显示对话框
func (afd *AddFriendDialog) Show() {
	afd.dialog.Show()
}

// searchUsers 搜索用户
func (afd *AddFriendDialog) searchUsers(keyword string) {
	go func() {
		// 调用网络接口搜索用户
		result, err := afd.netManager.SearchUsers(keyword)
		
		// 使用 fyne.Do 在主线程中更新UI
		fyne.Do(func() {
			if err != nil {
				// 网络请求失败
				log.Printf("网络请求失败: %s", err.Error())
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			// 检查响应状态码
			code, ok := result["code"].(float64)
			if !ok {
				log.Printf("服务器响应格式错误")
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			if code != 0 {
				// 服务端错误
				message := "搜索失败"
				if msg, exists := result["message"].(string); exists {
					message = msg
				}
				log.Printf("搜索失败: %s (错误码: %.0f)", message, code)
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			// 解析用户数据
			data, exists := result["data"]
			if !exists {
				log.Printf("服务器响应数据格式错误")
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			usersData, ok := data.(map[string]interface{})
			if !ok {
				log.Printf("用户数据格式错误")
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			usersArray, exists := usersData["users"]
			if !exists {
				log.Printf("未找到用户数据")
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			usersList, ok := usersArray.([]interface{})
			if !ok {
				log.Printf("用户列表格式错误")
				afd.users = []network.User{}
				afd.resultsList.Refresh()
				return
			}

			// 清空之前的搜索结果
			afd.users = []network.User{}

			// 解析每个用户
			for _, userInterface := range usersList {
				userMap, ok := userInterface.(map[string]interface{})
				if !ok {
					continue
				}

				user := network.User{}
				
				// 安全地提取用户信息
				if id, ok := userMap["user_id"].(float64); ok {
					user.UserID = int(id)
				}
				if username, ok := userMap["username"].(string); ok {
					user.Username = username
				}
				if nickname, ok := userMap["nickname"].(string); ok {
					user.Nickname = nickname
				}
				if email, ok := userMap["email"].(string); ok {
					user.Email = email
				}

				// 只添加有效的用户数据
				if user.Username != "" {
					afd.users = append(afd.users, user)
				}
			}

			// 刷新列表显示
			afd.resultsList.Refresh()
		})
	}()
}

// addFriend 添加好友
func (afd *AddFriendDialog) addFriend(user network.User) {
	remark := strings.TrimSpace(afd.remarkEntry.Text)

	go func() {
		result, err := afd.netManager.AddFriend(afd.userID, user.UserID, afd.serviceID, remark)
		fyne.Do(func() {
			if err != nil {
				dialog.ShowError(fmt.Errorf("添加好友失败: %v", err), afd.window)
				return
			}

			// 检查响应状态码
			code, ok := result["code"].(float64)
			if !ok || code != 0 {
				message := "未知错误"
				if msg, ok := result["message"].(string); ok {
					message = msg
				}
				dialog.ShowError(fmt.Errorf("添加好友失败: %s", message), afd.window)
				return
			}

			dialog.ShowInformation("成功", fmt.Sprintf("成功添加好友: %s", user.Nickname), afd.window)
			afd.dialog.Hide()
			if afd.onSuccess != nil {
				afd.onSuccess()
			}
		})
	}()
}

// UserSearch 用户搜索组件
type UserSearch struct {
	container   *fyne.Container
	netManager  *network.NetworkManager
	userID      int
	serviceID   int
	window      fyne.Window
	searchEntry *widget.Entry
	resultsList *widget.List
	users       []network.User
	onAddFriend func(user network.User)
}

// NewUserSearch 创建用户搜索组件
func NewUserSearch(window fyne.Window, netManager *network.NetworkManager, userID, serviceID int) *UserSearch {
	us := &UserSearch{
		window:     window,
		netManager: netManager,
		userID:     userID,
		serviceID:  serviceID,
		users:      []network.User{},
	}

	// 创建搜索框
	us.searchEntry = widget.NewEntry()
	us.searchEntry.SetPlaceHolder("输入用户名或昵称搜索")
	us.searchEntry.OnChanged = func(text string) {
		if strings.TrimSpace(text) != "" {
			us.searchUsers(strings.TrimSpace(text))
		} else {
			us.users = []network.User{}
			us.resultsList.Refresh()
		}
	}

	// 创建搜索结果列表
	us.resultsList = widget.NewList(
		func() int {
			return len(us.users)
		},
		func() fyne.CanvasObject {
			avatar := widget.NewLabel("👤")
			nameLabel := widget.NewLabel("用户名")
			nicknameLabel := widget.NewLabel("昵称")
			addButton := widget.NewButton("添加好友", nil)

			return container.NewHBox(
				avatar,
				container.NewVBox(nameLabel, nicknameLabel),
				layout.NewSpacer(),
				addButton,
			)
		},
		func(id widget.ListItemID, obj fyne.CanvasObject) {
			if id >= len(us.users) {
				return
			}

			user := us.users[id]
			hbox := obj.(*fyne.Container)

			// 更新用户信息
			vbox := hbox.Objects[1].(*fyne.Container)
			nameLabel := vbox.Objects[0].(*widget.Label)
			nicknameLabel := vbox.Objects[1].(*widget.Label)

			nameLabel.SetText(user.Username)
			nicknameLabel.SetText(user.Nickname)

			// 更新添加按钮
			addButton := hbox.Objects[3].(*widget.Button)
			addButton.OnTapped = func() {
				if us.onAddFriend != nil {
					us.onAddFriend(user)
				}
			}
		},
	)

	// 创建主容器
	us.container = container.NewVBox(
		widget.NewLabel("搜索用户"),
		us.searchEntry,
		widget.NewSeparator(),
		container.NewScroll(us.resultsList),
	)

	return us
}

// GetContainer 获取组件容器
func (us *UserSearch) GetContainer() *fyne.Container {
	return us.container
}

// searchUsers 搜索用户
func (us *UserSearch) searchUsers(keyword string) {
	go func() {
		result, err := us.netManager.SearchUsers(keyword)
		fyne.Do(func() {
			if err != nil {
				log.Printf("搜索用户失败: %v", err)
				us.users = []network.User{}
				us.resultsList.Refresh()
				return
			}

			// 检查响应状态码
			code, ok := result["code"].(float64)
			if !ok || code != 0 {
				log.Printf("搜索用户失败: 状态码错误")
				us.users = []network.User{}
				us.resultsList.Refresh()
				return
			}

			// 解析用户列表
			var users []network.User
			if data, ok := result["data"].(map[string]interface{}); ok {
				if usersData, ok := data["users"].([]interface{}); ok {
					for _, userData := range usersData {
						if userMap, ok := userData.(map[string]interface{}); ok {
							user := network.User{}
							if id, ok := userMap["user_id"].(float64); ok {
								user.UserID = int(id)
							}
							if username, ok := userMap["username"].(string); ok {
								user.Username = username
							}
							if nickname, ok := userMap["nickname"].(string); ok {
								user.Nickname = nickname
							}
							if email, ok := userMap["email"].(string); ok {
								user.Email = email
							}
							users = append(users, user)
						}
					}
				}
			}

			us.users = users
			us.resultsList.Refresh()
		})
	}()
}

// SetOnAddFriend 设置添加好友回调
func (us *UserSearch) SetOnAddFriend(callback func(user network.User)) {
	us.onAddFriend = callback
}