第六阶段：简化版增强群组功能设计
6.1 数据库表结构扩展（仅新增必需表）
6.1.1 新增群组设置表 (GroupSettings)
sql
CREATE TABLE GroupSettings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_id INTEGER NOT NULL,
    service_id INTEGER NOT NULL,
    group_type INTEGER NOT NULL, -- 0: QQ群, 1: 微信群, 2: 微博超话
    join_method INTEGER NOT NULL, -- 0: 申请加入, 1: 推荐加入, 2: 自由加入
    allow_subgroups INTEGER DEFAULT 0, -- 是否允许子群
    admin_system INTEGER DEFAULT 0, -- 管理员制度: 0-无, 1-简单, 2-完整
    max_members INTEGER DEFAULT 500,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES Groups(group_id),
    FOREIGN KEY (service_id) REFERENCES Services(id),
    UNIQUE(group_id, service_id)
);
6.1.2 新增群组成员角色表 (GroupMemberRoles)
sql
CREATE TABLE GroupMemberRoles (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    group_id INTEGER NOT NULL,
    user_id INTEGER NOT NULL,
    service_id INTEGER NOT NULL,
    role_type INTEGER NOT NULL, -- 0: 普通成员, 1: 管理员, 2: 群主/话题主持人
    assigned_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (group_id) REFERENCES Groups(group_id),
    FOREIGN KEY (user_id) REFERENCES Users(id),
    FOREIGN KEY (service_id) REFERENCES Services(id),
    UNIQUE(group_id, user_id, service_id)
);
6.2 新增接口设计（完整请求/响应参数）
6.2.1 群组设置管理接口
6.2.1.1 获取群组设置
URL: POST /group_settings

请求参数:

json
{
    "group_id": 1001,
    "service_id": 1
}
响应参数:

json
{
    "code": 0,
    "message": "success",
    "data": {
        "group_type": 0,
        "join_method": 0,
        "allow_subgroups": 1,
        "admin_system": 2,
        "max_members": 500
    }
}
6.2.1.2 更新群组设置
URL: POST /update_group_settings

请求参数:

json
{
    "group_id": 1001,
    "service_id": 1,
    "group_type": 0,
    "join_method": 0,
    "allow_subgroups": 1,
    "admin_system": 2,
    "max_members": 500
}
响应参数:

json
{
    "code": 0,
    "message": "success"
}
6.2.1.3 动态变换群组类型
URL: POST /change_group_type

请求参数:

json
{
    "group_id": 1001,
    "service_id": 1,
    "target_service_id": 2
}
响应参数:

json
{
    "code": 0,
    "message": "success",
    "data": {
        "new_group_type": 1,
        "new_join_method": 1,
        "new_allow_subgroups": 0
    }
}
6.2.2 群组成员角色管理接口
6.2.2.1 设置成员角色
URL: POST /set_member_role

请求参数:

json
{
    "group_id": 1001,
    "user_id": 123,
    "service_id": 1,
    "role_type": 1
}
响应参数:

json
{
    "code": 0,
    "message": "success"
}
6.2.2.2 获取群组成员角色
URL: POST /get_member_roles

请求参数:

json
{
    "group_id": 1001,
    "service_id": 1
}
响应参数:

json
{
    "code": 0,
    "message": "success",
    "data": {
        "members": [
            {
                "user_id": 123,
                "role_type": 2,
                "username": "user123",
                "nickname": "群主"
            },
            {
                "user_id": 124,
                "role_type": 1,
                "username": "user124",
                "nickname": "管理员"
            }
        ]
    }
}
6.2.3 特色功能接口
6.2.3.1 QQ群申请加入
URL: POST /apply_join_group

请求参数:

json
{
    "group_id": 1001,
    "user_id": 123,
    "service_id": 1,
    "apply_reason": "希望加入群组学习交流"
}
响应参数:

json
{
    "code": 0,
    "message": "申请已提交，等待审核"
}
6.2.3.2 微信群邀请加入
URL: POST /invite_join_group

请求参数:

json
{
    "group_id": 1001,
    "inviter_id": 123,
    "invitee_id": 124,
    "service_id": 2
}
响应参数:

json
{
    "code": 0,
    "message": "邀请已发送"
}
6.2.3.3 微博超话自由加入
URL: POST /free_join_topic

请求参数:

json
{
    "group_id": 1001,
    "user_id": 123,
    "service_id": 3
}
响应参数:

json
{
    "code": 0,
    "message": "成功加入超话"
}
6.3 新增命令字定义
命令字	描述	对应接口
get_group_settings	获取群组设置	/group_settings
update_group_settings	更新群组设置	/update_group_settings
change_group_type	动态变换群组类型	/change_group_type
set_member_role	设置成员角色	/set_member_role
get_member_roles	获取成员角色	/get_member_roles
apply_join_group	QQ群申请加入	/apply_join_group
invite_join_group	微信群邀请加入	/invite_join_group
free_join_topic	微博超话自由加入	/free_join_topic

6.4 简化版GroupDBManager类
cpp
// include/utils/group_db_manager.h
class GroupDBManager {
public:
    static GroupDBManager& getInstance();
    
    // 群组设置管理
    bool getGroupSettings(int groupId, int serviceId, std::string& jsonResult);
    bool updateGroupSettings(int groupId, int serviceId, int groupType, int joinMethod, 
                           int allowSubgroups, int adminSystem, int maxMembers);
    bool changeGroupType(int groupId, int sourceServiceId, int targetServiceId);
    
    // 成员角色管理
    bool setMemberRole(int groupId, int userId, int serviceId, int roleType);
    bool getMemberRoles(int groupId, int serviceId, std::string& jsonResult);
    
    // 服务特定加入方式
    bool applyJoinGroup(int groupId, int userId, int serviceId, const std::string& reason);
    bool inviteJoinGroup(int groupId, int inviterId, int inviteeId, int serviceId);
    bool freeJoinTopic(int groupId, int userId, int serviceId);

private:
    GroupDBManager();
    ~GroupDBManager();
    
    sqlite3* db_;
    std::string dbPath_;
    
    bool initialize();
    bool createEnhancedGroupTables();
    
    // 设置服务默认配置
    void setServiceDefaultSettings(int serviceId, int& groupType, int& joinMethod, 
                                 int& allowSubgroups, int& adminSystem, int& maxMembers);
};
6.5 简化开发步骤（2周完成）
6.5.1 第一阶段：数据库和核心类实现（3-4天）
创建GroupDBManager类

实现两张新增表（GroupSettings、GroupMemberRoles）

实现基本的设置管理和角色管理功能

为现有群组自动创建默认设置

6.5.2 第二阶段：接口实现和差异化特性（4-5天）
在GroupSVC中添加所有新接口

实现三种服务的差异化加入方式

实现动态群组类型变换

更新网关路由配置

6.5.3 第三阶段：测试和集成（2-3天）
基础功能测试

服务差异化特性验证

与客户端集成测试

6.6 服务默认配置
cpp
// 在GroupDBManager中实现服务默认配置
void GroupDBManager::setServiceDefaultSettings(int serviceId, int& groupType, int& joinMethod, 
                                             int& allowSubgroups, int& adminSystem, int& maxMembers) {
    switch(serviceId) {
        case 1: // QQ
            groupType = 0;
            joinMethod = 0; // 申请加入
            allowSubgroups = 1; // 允许子群
            adminSystem = 2; // 完整管理员制度
            maxMembers = 1000;
            break;
        case 2: // 微信
            groupType = 1;
            joinMethod = 1; // 推荐加入
            allowSubgroups = 0; // 不允许子群
            adminSystem = 0; // 无管理员制度
            maxMembers = 500;
            break;
        case 3: // 微博
            groupType = 2;
            joinMethod = 2; // 自由加入
            allowSubgroups = 0; // 不允许子群
            adminSystem = 1; // 简单管理
            maxMembers = 10000;
            break;
        default:
            // 默认使用QQ配置
            groupType = 0;
            joinMethod = 0;
            allowSubgroups = 1;
            adminSystem = 2;
            maxMembers = 500;
    }
}
6.7 核心差异化特性实现
6.7.1 加入方式控制
cpp
bool GroupDBManager::applyJoinGroup(int groupId, int userId, int serviceId, const std::string& reason) {
    // 检查是否为QQ群（只有QQ群允许申请加入）
    int currentJoinMethod = 0;
    // 从数据库获取当前群的join_method
    // 如果不是申请加入方式，返回错误
    
    // 实现申请加入逻辑
    return true;
}
6.7.2 动态类型变换
cpp
bool GroupDBManager::changeGroupType(int groupId, int sourceServiceId, int targetServiceId) {
    // 获取当前群设置
    // 根据targetServiceId计算新的默认设置
    // 更新群组设置表
    // 保持群成员不变
    return true;
}
这个简化版设计：

去除了数据迁移步骤 - 新群组自动获得默认设置

所有接口都有完整响应参数 - 确保前后端协作顺畅

开发时间缩短到2周 - 适合您的时间限制

保持核心差异化特性 - 满足课程设计要求

表结构简化 - 只保留必需字段

重点实现三个核心差异化：

QQ群：申请加入 + 管理员制度

微信群：邀请加入 + 简单管理

微博超话：自由加入 + 话题模式

这样可以在有限时间内完成核心功能，展示面向对象的设计思想。