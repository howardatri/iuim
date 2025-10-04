package ui

import (
	"fmt"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/container"
	"fyne.io/fyne/v2/dialog"
	"fyne.io/fyne/v2/widget"
	
	"fyne-im/network"
)

// Service 定义服务数据模型
type Service struct {
	ServiceID   int
	ServiceName string
	Description string
	Activated   bool
}

// ServiceCard 封装了单个服务卡片的所有UI组件和数据
type ServiceCard struct {
	card         *widget.Card
	statusLabel  *widget.Label
	actionButton *widget.Button
	serviceData  Service // 存储静态服务数据
}

// ServiceHallView 服务大厅视图组件
type ServiceHallView struct {
	container   *fyne.Container
	serviceGrid *fyne.Container
	window      fyne.Window
	cards       []*ServiceCard // 用于管理所有卡片
	netManager  *network.NetworkManager // 网络管理器
	userID      int // 用户ID
}

// NewServiceHallView 创建新的服务大厅视图
func NewServiceHallView(window fyne.Window, netManager *network.NetworkManager, userID int) *ServiceHallView {
	view := &ServiceHallView{
		window:     window,
		cards:      []*ServiceCard{},
		netManager: netManager,
		userID:     userID,
	}

	// 创建标题
	title := widget.NewLabel("服务大厅")
	title.TextStyle = fyne.TextStyle{Bold: true}
	title.Alignment = fyne.TextAlignCenter

	// 创建服务网格容器
	view.serviceGrid = container.NewGridWithColumns(2)

	// 硬编码服务列表
	services := []Service{
		{
			ServiceID:   1,
			ServiceName: "QQ",
			Description: "腾讯即时通讯服务",
			Activated:   true,
		},
		{
			ServiceID:   2,
			ServiceName: "微信",
			Description: "微信社交平台",
			Activated:   false,
		},
		{
			ServiceID:   3,
			ServiceName: "微博",
			Description: "新浪微博社交媒体",
			Activated:   false,
		},
		{
			ServiceID:   4,
			ServiceName: "添加服务",
			Description: "添加更多服务",
			Activated:   false,
		},
	}

	// 为每个服务创建卡片
	for _, s := range services {
		// 创建状态标签
		statusLabel := widget.NewLabel("状态：未知")
		
		// 创建操作按钮
		var actionButton *widget.Button

		if s.ServiceName == "添加服务" {
			// 特殊样式的添加服务按钮
			actionButton = widget.NewButton("添加", func() {
				dialog.ShowInformation("添加服务", "即将添加新服务", window)
			})
		} else {
			// 所有其他服务都使用"查询状态"按钮
			actionButton = widget.NewButton("查询状态", func() {
				// 暂时留空
			})
		}

		// 创建卡片内容
		content := container.NewVBox(
			statusLabel,
			container.NewCenter(actionButton),
		)

		// 创建卡片
		card := widget.NewCard(
			s.ServiceName,
			s.Description,
			content,
		)

		// 创建ServiceCard实例
		serviceCard := &ServiceCard{
			card:         card,
			statusLabel:  statusLabel,
			actionButton: actionButton,
			serviceData:  s,
		}

		// 将卡片添加到网格
		view.serviceGrid.Add(card)
		
		// 将ServiceCard实例添加到管理切片
		view.cards = append(view.cards, serviceCard)
		
		// 为非"添加服务"的卡片设置查询状态按钮的回调
		if s.ServiceName != "添加服务" {
			// 使用局部变量捕获当前卡片，避免闭包陷阱
			card := serviceCard
			card.actionButton.OnTapped = func() {
				view.queryCardStatus(card)
			}
		}
	}

	// 组装界面
	view.container = container.NewBorder(
		container.NewVBox(title, widget.NewSeparator()),
		nil,
		nil, nil,
		view.serviceGrid,
	)

	return view
}

// GetContainer 返回视图容器
func (v *ServiceHallView) GetContainer() fyne.CanvasObject {
	return v.container
}

// queryCardStatus 查询卡片服务状态
// queryCardStatus 查询卡片服务状态
func (v *ServiceHallView) queryCardStatus(card *ServiceCard) {
    // 启动一个新的goroutine来执行网络请求
    go func() {
        // 调用网络管理器查询用户服务
        result, err := v.netManager.QueryUserServices(v.userID)
        
        fyne.Do(func() {
            if err != nil {
                dialog.ShowError(err, v.window)
                return
            }
            
            // 检查返回结果是否成功 - 使用 code 字段而不是 success
            code, ok := result["code"].(float64)
            if !ok || code != 0 {
                errMsg, _ := result["message"].(string)
                if errMsg == "" {
                    errMsg = "查询服务失败"
                }
                dialog.ShowError(fmt.Errorf(errMsg), v.window)
                return
            }
            
            // 解析服务列表 - 从 data 字段中获取
            data, ok := result["data"].(map[string]interface{})
            if !ok {
                dialog.ShowError(fmt.Errorf("无法解析响应数据"), v.window)
                return
            }
            
            servicesData, ok := data["services"].([]interface{})
            if !ok {
                dialog.ShowError(fmt.Errorf("无法解析服务列表"), v.window)
                return
            }
            
            // 查找匹配的服务
            for _, serviceData := range servicesData {
                serviceMap, ok := serviceData.(map[string]interface{})
                if !ok {
                    continue
                }
                
                serviceIDFloat, ok := serviceMap["id"].(float64)  // 注意字段名是 "id" 不是 "service_id"
                if !ok {
                    continue
                }
                
                serviceID := int(serviceIDFloat)
                
                // 找到匹配的服务
                if serviceID == card.serviceData.ServiceID {
                    // 创建更新后的服务信息
                    activatedFloat, _ := serviceMap["activated"].(float64)
                    updatedService := Service{
                        ServiceID:   serviceID,
                        ServiceName: card.serviceData.ServiceName, // 保持原有的服务名
                        Description: card.serviceData.Description, // 保持原有的描述
                        Activated:   activatedFloat == 1,          // 注意：数据库返回 0/1，但客户端期望 bool
                    }
                    
                    // 更新卡片UI
                    v.updateCardUI(card, updatedService)
                    break
                }
            }
        })
    }()
}

// updateCardUI 更新卡片UI
func (v *ServiceHallView) updateCardUI(card *ServiceCard, serviceInfo Service) {
    // 更新服务数据
    card.serviceData = serviceInfo
    
    // 根据服务激活状态更新UI
    if serviceInfo.Activated {
        // 更新状态标签
        card.statusLabel.SetText("状态：已开通")
        
        // 更新按钮文本和操作
        card.actionButton.SetText("进入管理")
        card.actionButton.OnTapped = func() {
            // 根据服务ID跳转到对应的服务界面
            serviceView := CreateServiceView(v.window, v.netManager, v.userID, serviceInfo.ServiceID, serviceInfo.ServiceName)
            v.window.SetContent(serviceView)
        }
    } else {
        // 更新状态标签
        card.statusLabel.SetText("状态：未开通")
        
        // 更新按钮文本和操作
        card.actionButton.SetText("开通服务")
        card.actionButton.OnTapped = func() {
            dialog.ShowInformation(
                "开通服务", 
                fmt.Sprintf("即将开通%s服务", serviceInfo.ServiceName), 
                v.window,
            )
			// 启动goroutine调用激活服务接口
    go func() {
        // 调用网络管理器激活服务
        result, err := v.netManager.ActivateService(v.userID, serviceInfo.ServiceID)
        
        // 使用fyne.Do确保UI更新在主线程
        fyne.Do(func() {
            if err != nil {
                dialog.ShowError(err, v.window)
                return
            }
            
            // 检查响应结果
            code, ok := result["code"].(float64)
            if !ok || code != 0 {
                errMsg, _ := result["message"].(string)
                if errMsg == "" {
                    errMsg = "开通服务失败"
                }
                dialog.ShowError(fmt.Errorf(errMsg), v.window)
                return
            }
            
            // 开通成功，显示提示并重新查询状态
            dialog.ShowInformation(
                "开通成功", 
                fmt.Sprintf("%s服务开通成功！", serviceInfo.ServiceName), 
                v.window,
            )
            
            // 重新查询服务状态以更新UI
            v.queryCardStatus(card)
        })
    }()
        }
    }
    
    // 刷新卡片UI
    card.card.Refresh()
}