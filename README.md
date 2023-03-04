# LagCompensationDemo

Developed with Unreal Engine 5.1.0  

# 工程说明

---  

本工程以射击游戏为模版，做了个延迟补偿的实现演示，如果不知道什么是延迟补偿，建议观看下边的科普篇，想要理解工程源码所实现的算法，可以看硬核篇。  

# 延迟补偿讲解视频

* * *  

科普篇：  [科普向：竞技游戏的延迟与延迟补偿]( https://www.bilibili.com/video/BV1Jb411Q7CH)

硬核篇：  [硬核向：延迟补偿具体实现算法](https://www.bilibili.com/video/BV1cM4y1d7DR)

# 资源

---  

已打包好的游戏  

链接：[百度网盘 请输入提取码](https://pan.baidu.com/s/1vl0MwzFtMGsPAzvghBVNYQ) 提取码：w7ak  

源码：https://github.com/gdxwkzw/LagCompensationDemo.git  

# 使用说明

---  

## 打包好的游戏

### 操控方法

WASD移动，左键开枪，按住左Alt键出现光标，松开左Alt键光标消失  

### 控制台

~键呼出控制台  

常用命令：  

open 127.0.0.1 连接本地服务器  

open ip:端口 连接专属服务器  

exit 退出游戏   

### 使用本地服务器测试

以下内容改编自[在虚幻引擎中设置专用服务器 | 虚幻引擎5.1文档](https://docs.unrealengine.com/5.1/zh-CN/setting-up-dedicated-servers-in-unreal-engine/)  

1. 打开 **Server** 文件夹，然后找到 **[ProjectName]Server.exe**。这里，假设它叫做TestProjectServer.exe。  
   [![DedicatedServerExecutable2.png](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerExecutable2.jpg)](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerExecutable2.png)  

2. 创建 **Server.exe** 的快捷方式。  
   [![DedicatedServerShortcut.png](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerShortcut.jpg)](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerShortcut.png)  

3. **右键点击** **服务器快捷方式**，然后打开其 **属性**。在其 **目标** 字段的末尾添加 **"-log"**，然后点击 **应用（Apply）**。  
   [![ShortcutProperties.png](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/ShortcutProperties.jpg)](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/ShortcutProperties.png)  
   
   这样，当我们运行专用服务器时，它将在命令提示符工具中显示日志。  

4. **双击** **快捷方式**，启动专用服务器。将打开 **命令提示符** 窗口，显示输出日志。如果服务器启动成功，你会在底部看到说明启动关卡所需时间的输出。  
   [![DedicatedServerCommandPrompt.png](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerCommandPrompt.jpg)](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerCommandPrompt.png)  

5. 转到 **Client** 文件夹，然后双击 **TestProject.exe**。这会在新窗口中启动游戏，并且直接自动跳转到预备地图。可以按~键，然后输入open 127.0.0.1并回车，这样会跳转到服务器的地图。  

6. 使用 **ALT+Tab** 离开游戏窗口，然后运行 **TestProject.exe**，启动游戏的第二个实例。然后继续按上述输入open指令，跳到服务器的地图，如果成功，你将在你启动的窗口中看到玩家。你还可以检查服务器日志，以便查看两个玩家连接到服务器的确认信息。  

![DedicatedServerRunning.png](https://docs.unrealengine.com/5.1/Images/making-interactive-experiences/network-multiplayer/network-programming/DedicatedServers/DedicatedServerRunning.jpg)  

祝贺你！本地服务器连接成功！  

### 使用专用服务器测试

一块钱开服务器测游戏攻略：[百度网盘 请输入提取码](https://pan.baidu.com/s/1rLOQm9E2SEK7bvVLZQuQew) 提取码：7x6a   

---  

## 源码

### 核心代码位置

延迟补偿算法实现的代码都写在LagCompensationComponent.h和LagCompensationComponent.cpp里，挂在角色上，代码上LagCompensationCharacter.h中有定义，也可从Content/Blueprints/BP_LagCompensationCharacter的组件栏里看到，调用在Weapon.cpp的ServerFireWithLagCompensation()这个RPC函数中。  

### 调节延迟

打开 工程目录/Config/DefaultEngine.ini   

最下面有一行  

PktLag = 500  

可修改这个值来改变测试的延迟
