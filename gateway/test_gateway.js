const net = require('net');

// 测试网关功能
function testGateway() {
    const client = new net.Socket();
    
    client.connect(8233, 'localhost', () => {
        console.log('Connected to gateway');
        
        // 测试health_check命令
        console.log('Testing health_check command...');
        client.write('health_check\n');
        
        setTimeout(() => {
            // 测试register命令
            console.log('Testing register command...');
            const registerData = {
                username: 'testuser',
                password: 'testpass',
                email: 'test@example.com'
            };
            client.write(`register ${JSON.stringify(registerData)}\n`);
        }, 1000);
        
        setTimeout(() => {
            // 测试新增的群组命令
            console.log('Testing get_group_settings command...');
            const groupData = {
                group_id: 'test_group_123',
                user_id: 'test_user_456'
            };
            client.write(`get_group_settings ${JSON.stringify(groupData)}\n`);
        }, 2000);
        
        setTimeout(() => {
            console.log('Closing connection...');
            client.destroy();
        }, 3000);
    });
    
    client.on('data', (data) => {
        console.log('Received:', data.toString().trim());
    });
    
    client.on('close', () => {
        console.log('Connection closed');
        process.exit(0);
    });
    
    client.on('error', (err) => {
        console.error('Connection error:', err.message);
        process.exit(1);
    });
}

// 启动测试
console.log('Starting gateway test...');
testGateway();