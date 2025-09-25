package ui

import (
	"encoding/json"
	"fmt"
	"log"

	"fyne-im/network"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"
)

// Service 定义服务数据模型，JSON标签与文档约定一致
type Service struct {
	ID          int    `json:"id"`
	Name        string `json:"name"`
	Description string `json:"description"`
	Activated   bool   `json:"activated"`
}

// ServiceResponse 定义服务查询响应结构
type ServiceResponse struct {
	Code int `json:"code"`
	Data struct {
		Services []Service `json:"services"`
	} `json:"data"`
}

// ServiceHallView 服务大厅视图组件
type ServiceHallView struct {
	container    *fyne.Container
	serviceList  *widget.List
	services     []Service
	netManager   *network.NetworkManager
	userID       int
	window       fyne.Window
	refreshFunc  func()
}

// NewServiceHallView 创建新的服务大厅视图
func NewServiceHallView(window fyne.Window, netManager *network.NetworkManager, userID int) *ServiceHallView {
	view := &ServiceHallView{
		netManager: netManager,
		userID:     userID,
		window:     window,
		services:   []Service{},
	}

	// 创建标题
	title := widget.NewLabel("服务大厅")
	title.TextStyle = fyne.TextStyle{Bold: true}
	title.Alignment = fyne.TextAlignCenter

	// 创建服务列表
	view.serviceList = widget.NewList(
		// 列表长度函数
		func() int {
			return len(view.services)
		},
		// 创建列表项函数
		func() fyne.CanvasObject {
			nameLabel := widget.NewLabel("服务名称")
			nameLabel.TextStyle = fyne.TextStyle{Bold: true}
			
			descLabel := widget.NewLabel("服务描述")
			
			actionButton := widget.NewButton("操作", nil)
			
			return container.NewBorder(
				nil, nil, nil, actionButton,
				container.NewVBox(nameLabel, descLabel),
			)
		},
		// 更新列表项函数
		func(id widget.ListItemID, item fyne.CanvasObject) {
			service := view.services[id]
			
			// 获取容器中的组件
			border := item.(*fyne.Container)
			vbox := border.Objects[0].(*fyne.Container)
			nameLabel := vbox.Objects[0].(*widget.Label)
			descLabel := vbox.Objects[1].(*widget.Label)
			actionButton := border.Objects[1].(*widget.Button)
			
			// 更新标签内容
			nameLabel.SetText(service.Name)
			descLabel.SetText(service.Description)
			
			// 根据服务激活状态设置按钮
			if service.Activated {
				actionButton.SetText("进入管理")
				actionButton.OnTapped = func() {
					dialog.ShowInformation("服务管理", fmt.Sprintf("进入 %s 服务管理界面", service.Name), view.window)
				}
			} else {
				actionButton.SetText("开通服务")
				actionButton.OnTapped = func() {
					// 调用激活服务API
					go func() {
						result, err := view.netManager.ActivateService(view.userID, service.ID)
						if err != nil {
							dialog.ShowError(fmt.Errorf("激活服务失败: %v", err), view.window)
							return
						}
						
						// 检查激活结果
						code, ok := result["code"].(float64)
						if !ok || code != 0 {
							message := "激活服务失败"
							if msg, ok := result["message"].(string); ok {
								message = msg
							}
							dialog.ShowError(fmt.Errorf(message), view.window)
							return
						}
						
						// 激活成功，刷新服务列表
						view.refreshServices()
					}()
				}
			}
		},
	)

	// 创建刷新按钮
	refreshButton := widget.NewButton("刷新", func() {
		view.refreshServices()
	})

	// 组装界面
	view.container = container.NewBorder(
		container.NewVBox(title, widget.NewSeparator()), 
		refreshButton, 
		nil, nil, 
		view.serviceList,
	)

	// 保存刷新函数以便外部调用
	view.refreshFunc = view.refreshServices

	// 初始加载数据
	go view.refreshServices()

	return view
}

// GetContainer 返回视图容器
func (v *ServiceHallView) GetContainer() fyne.CanvasObject {
	return v.container
}

// refreshServices 刷新服务列表
func (v *ServiceHallView) refreshServices() {
	// 调用网络管理器查询用户服务
	result, err := v.netManager.QueryUserServices(v.userID)
	if err != nil {
		dialog.ShowError(fmt.Errorf("获取服务列表失败: %v", err), v.window)
		return
	}

	// 将结果转换为JSON字符串
	jsonData, err := json.Marshal(result)
	if err != nil {
		dialog.ShowError(fmt.Errorf("解析服务数据失败: %v", err), v.window)
		return
	}

	// 解析响应
	var response ServiceResponse
	if err := json.Unmarshal(jsonData, &response); err != nil {
		log.Printf("解析JSON失败: %v, 原始数据: %s", err, string(jsonData))
		dialog.ShowError(fmt.Errorf("解析服务数据失败: %v", err), v.window)
		return
	}

	// 更新服务列表
	v.services = response.Data.Services
	v.serviceList.Refresh()
}