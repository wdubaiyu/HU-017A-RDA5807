# hU-017A

RDA5807收音机

1. 在[淘宝](https://detail.tmall.com/item.htm?id=701902156484)买的收音机套件自己焊接后发现有bug。

2. 收音机换台后，收音机音量恢复默认大小。

3. 在github和其他网站找找了资料，参考代码自己写了这份代码。


### 功能描述

1. 第一次开机**自动搜台**并保存到列表中
2. F+ ，F- 短按**切换电台**（列表）
3. F+ ，F- 长按手动搜台，不会加入列入中
4. V+ ，V- 短按**调整音量**
5. 先长按下V+ 然后长按V- 切换**省电模式**（功能描述：一定时间后数码管熄灭） 	数码管显示L为打开省电模式，H为关闭省电模式
6. 先长按下V+ 然后长按F- 显示当前电台**RSSI**
7. 先长按下V+ 然后长按F+ 开启**自动搜台**模式   屏幕显示**5.n06** 其中06是**收音阈值**（snr：0~15） V+ V- 进行调节。`snr越大收到台越少`。 	F- **确认搜台**，F+ **取消搜台**
8. 长按V- 切换静音

### 刷写注意事项

1. 使用ttl连接GND、RXD、TXS
2. stc-isp 软件设置，参考images里面的图。时钟频率一定要设置**27.000MHx**
3. 点击打开程序选择固件（build目录下的HU-017A.hex）后，点击**下载/编程**。
4. 然后关闭再打开收音机。

代码参考：
    [HZ1213825](https://github.com/HZ1213825/HAL_STM32F4_RDA5807M) 
    [B站](https://www.bilibili.com/video/BV1Mb411e7re)

**仅做学习和研究使用，不可作为商业用途。**
