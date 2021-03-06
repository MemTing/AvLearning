## RTP协议

对于协议设计，都有头部  + 数据区域

RTP报文由两部分组成，报头和有效载荷

### RTP头部

固定12字节

V:   RTP协议版本号，占2位。版本号不对会导致区分不出来。

P:  填充标志，占1位。如果P=1,在报文尾部填充一个或多个8位组。主要是有一些加密算法，需要固定长度的数据，这个时候就会填充。比如只有15个字节，需要16字节，就要填充。大多数情况，都是0。

X: 扩展标志，占1位，如果X=1,在RTP报头后跟一个扩展报头。平时用得不多，webrtc有用。

​	也就是，RTP头部+扩展区域+数据区域（有效载荷）。

CC:  CSRC计数器，占4位，指示CSRC标识符的个数。一般情况都是0。

M: 标记，占1位，不同的码流，含义不同。对视频，标志一帧数据结束。对于音频，标志一帧开始。

PT: 有效载荷类型，占7位，用于说明RTP报文中有效载荷的类型。动态荷载类型（96代表H264），荷载类型（比如96）代表什么要看sdp协商，96和H264没有必然关系。

序列号：占16位，用于标识发送的RTP报文的序列号，每发送一个报文，序列号增加1。接收端可以根据序列号检	测报文丢失情况，重新排序报文，恢复数据。使用UDP方式发送RTP报文，会有乱序，设一个缓存，从缓存排序	后的队列里面读取数据。

时戳：占32位，反映了RTP报文的第一个8位组的采样时刻。接收者使用时戳来计算延迟和延迟抖动，进行同步控	制。timestamp

同步信源标志：占32位，用于标识同步信源。该标识符是随机的，参加同一视频会议的两个同步信源不能有相同	的SSRC。是唯一标识。

特约信源（CSRC）：很少用到。和前面CC字段配合使用，如果CC=3,则CSRC是个列表，SSRC1,SSRC2,SSRC3组成。

### 重点

时间戳：重点是  单位值 是多少，比如单位是1ms, timestamp=1000, 换成秒就是1秒。

如果单位是1/90000，换算成秒 就是 1/90000*1000=0.0111111111秒。

序列号：sequence,  RTP over UDP, 丢包，乱序的解决。

乱序，重排解决。比如发包顺序  1  2  3 4 5， 接收  2  1  3  5  4，这时需要设计缓存队列，支持排序，比如队列有	3个包才开始读取，1  2  3。

丢包，FEC解。



### H264 rtp封装

怎么通过RTP传输H264包。

​		H264  NALU |  RTP | UDP

UDP每个包有大小限制，MTU 一般是1500，sendto一次不能发送超过1500的数据。

RTP一般一个包的大小是1400左右。demo是1434，这个长度包括RTP Header + RTP Payload。



怎么把H264数据放到Payload里面？

1）RTP包有大小

​		一般NALU的大小（去掉startcode，RTP传输的时候不传输startcode(00 00 00 01)）。

​		sps   25字节左右

​		pps   5字节左右

​		I帧   1080p的  200K字节      **一个RTP包发送不完一个I帧**。这时候必然有拆帧。

​		sei帧    600字节左右



​	2）也就是同一个NALU划分成多个RTP包进行发送，比如I帧拆分成多个RTP包

​	3）一个RTP包也可以发送多个NALU，比如： sps和pps、sei帧一起发送

​			为了简化程序，sps、pps、sei帧还是发送独立的包

​	

​	封包过程：

​			1）原理讲解

​			2）代码讲解

​		

​		NALU Header + NALU Data

​	打包： rtp size = 1400字节左右

			1) nalu size < = rtp size(RTP payload的size) 就是 1:1发送
			1) nalu size > rtp size,   1: N      一个nalu拆分多个RTP包     FU-A



发送RTP包的时候，  RTP header【12字节】

​		1、Single NAL Unit     RTP header【12字节】 0x67 0x64...。也就是后面直接跟NALU 。

​		2、fu-a这种拆包模式，RTP header【12字节】0x7c 0x85...，第一个字节，FU indication用来区分是否属于FU-A的模式。

​			FU header做了三件事

​			（1）s:一个nalu的起始标志

​			（2）e: 一个nalu的结束标志

​			（3）type:本来nalu的类型，取1-23的那个值



#### 1.1打包方式之Single NAL Unit

就是一个RTP包打包一个单独的NALU方式，就是在RTP固定头后面直接填充NALU单元数据即可，即：

​		RTP Header  +  NALU Header  + NALU  Data  (不包括startcode)。只适合sps、pps,或者一些图像很小的时候B帧用。大多数时候用的是FU-A这种模式。

#### 1.2 打包方式之FU-A



### RTP H264封包解包代码实现

流程：

![image-20220429235611019.png](\\192.168.1.114\samba\github\AVideoLearning\音视频学习总结\截图\image-20220429235611019.png)

1、新建工程

2、rtp header开始



### RTP AAC封包解包代码实现

1、怎么在原有结构添加支持mpeg4-generic (AAC)

​		rtp-payload-interal.h

​		rtp-payload.c

2、具体封包和解包的实现

​		rtp-mpeg4-generic-pack.c

​		rtp-mpeg4-generic-unpack.c

​		注意，RTP的payload type值要和sdp的pt值对应上

​				rtp-mpeg4-generic-pack_input   封包

​				rtp-decode_mpeg4-generic         解包

3、怎么读取aac文件，怎么一帧一帧读取？

​		aac_util.c

​		aac_get_one_frame     假定文件是完整无损的   adts header+data adts header+data

4、怎么实时发送数据

​	1）计算出每一帧数据播放的时长，实际上是固定的frame_duration（注意精度，长时播放误差会增大）。发送了多少帧，可以叠加总共发送的数据可播放的时长。

​	2）设置起始发送时间start_time，然后获取当前的时间cur_time

​		cur_time-start_time,  计算出经过时间。

​		比如sum_time=60 000ms，那发送的时间应该也是cur_time-start_time  = 60 000ms

 	3)  时间控制

5、具体encode和decode过程是怎么样的

​	RTP AAC  mepg4-gereric, AU_HEADER_LENGTH 固定2字节，AU_HEADER 数目不固定，一个AU_HEADER 对应一个AU

​	RTP header  + AU_HEADER_LENGTH + AU_HEADER + AU

​	(1) AU_HEADER 的个数和AU是对应上的，1:1对应，AU_HEADER 描述AU的size。

​			AU-size, 描述AU的size

​			AU-Index/AU-Index-delta, 描述AU的序号

### 对AAC进行RTP封包

in.aac->读取一帧数据->RTP 打包->RTP 解包->封装成一帧数据->写入文件out.aac

过程：

​	1、需要将AAC的前7个（或9个）字节的ADTS去掉，即是跳过adts header

​	2、添加RTP Header

​	3、添加2字节的AU_HEADER_LENGTH

​	4、添加2字节的AU_HEADER

​	5、从第17字节开始就是payload（去掉ADTS的aac数据）数据了



