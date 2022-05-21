# HITSZ Spring Comp-Network lab - maillab

## Intro
哈尔滨工业大学（深圳） 2022 年春季学期计算机网络实验——邮件客户端设计与实现
实现了一个

## How to use?
在 `send.c` 和 `recv.c` 里填入邮箱和授权码；
之后就可进行编译：
```
make
```

按照如下格式命令进行邮件的发送：
```
./send RECIPIENT [-s SUBJECT] [-m MESSAGE] [-a ATTACHMENT]
```
- RECIPIENT: 收件人地址

- SUBJECT: 邮件主题

- MESSAGE: 邮件正文或含有邮件正文的文件路径

- ATTACHMENT: 邮件附件


运行如下命令与 POP3 服务器进行交互
```
./recv
```